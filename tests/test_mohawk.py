from pathlib import Path

import pytest

from zbtools import assets, mohawk
from zbtools.mohawk import Resource

RESOURCES = [
    Resource(b"tBMP", 100, b"image"),
    Resource(b"\0SND", 7, b"sound"),
    Resource(b"tBMP", 50, b"another image"),
    Resource(b"tMID", 1, b"music", purgeable=True),
]


def test_round_trip() -> None:
    assert mohawk.read(mohawk.write(RESOURCES)) == RESOURCES


def test_layout() -> None:
    data = mohawk.write(RESOURCES)
    assert data[:4] == b"MHWK" and data[8:12] == b"RSRC"
    assert int.from_bytes(data[4:8], "big") == len(data) - 8
    # The data follows the header's unused bytes, in the given order.
    assert data[0x24:0x29] == b"image" and data[0x29:0x2E] == b"sound"
    directory = int.from_bytes(data[20:24], "big")
    # The type table is sorted by type ...
    types = [data[directory + 4 + t * 8 : directory + 8 + t * 8] for t in range(3)]
    assert types == [b"\0SND", b"tBMP", b"tMID"]
    # ... and the types' tables are in the order the types first occur.
    tables = [
        int.from_bytes(data[directory + 8 + t * 8 : directory + 10 + t * 8], "big")
        for t in range(3)
    ]
    assert tables[1] < tables[0] < tables[2]
    # tBMP's table lists its ids in order, with their file-table indexes.
    table = directory + tables[1]
    assert data[table : table + 10] == bytes.fromhex("0002 0032 0003 0064 0001")


def test_rejects_other_layouts() -> None:
    data = bytearray(mohawk.write(RESOURCES))
    data[0x1D] = 0
    with pytest.raises(mohawk.FormatError):
        mohawk.read(bytes(data))
    with pytest.raises(mohawk.FormatError):
        mohawk.read(b"RIFF" + bytes(40))


def test_duplicate_ids() -> None:
    with pytest.raises(ValueError, match="duplicate"):
        mohawk.write([Resource(b"tBMP", 1, b""), Resource(b"tBMP", 1, b"")])


def test_type_names() -> None:
    assert assets.type_name(b"\0SND") == "SND"
    assert assets.type_tag("SND") == b"\0SND"
    assert assets.type_name(b"tBMP") == "tBMP"
    with pytest.raises(ValueError, match="can't name"):
        assets.type_name(b"a b\0")


def test_manifest_round_trip(tmp_path: Path) -> None:
    manifest = assets.Manifest(
        path="DATA/TEST.MHK",
        resources=[
            assets.Entry(type="SND", id=7),
            assets.Entry(type="tMID", id=1, purgeable=True),
        ],
    )
    assets.write_manifest(manifest, tmp_path / "archive.toml")
    assert assets.read_manifest(tmp_path / "archive.toml") == manifest
