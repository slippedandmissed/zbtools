from zbtools.match import Marker, Sibling, Target, _local_calls, find_targets


def _call(source: int, destination: int) -> bytes:
    return b"\xe8" + (destination - source - 5).to_bytes(4, "little", signed=True)


def test_local_call_to_the_right_sibling() -> None:
    # Compiled at offset 0x20 of CODE, calling a sibling at offset 0x00; the
    # original (at 0x401000) calls that sibling's address, 0x402000.
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x402000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(),
        siblings=siblings,
    )
    assert found == {2: "@sibling$qv"}


def test_local_call_to_another_function() -> None:
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x403000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(),
        siblings=siblings,
    )
    assert found == {}


def test_calls_with_fixups_are_left_to_the_linker() -> None:
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x402000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(range(2, 6)),
        siblings=siblings,
    )
    assert found == {}


def test_markers() -> None:
    source = """
/* @zoombi32 0x00401000 */
void exact(long) {}
/* @zoombi32-nonmatching 0x00401010 */
void close(long) {}
/* @zoombi32-functional 0x00401020 */
long portable(long *value) { return 0; }
"""
    assert find_targets(source) == [
        Target("exact", 0x401000, Marker.EXACT),
        Target("close", 0x401010, Marker.NONMATCHING),
        Target("portable", 0x401020, Marker.FUNCTIONAL),
    ]
