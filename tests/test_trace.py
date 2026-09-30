from pathlib import Path

from zbtools.trace import functions, read_map

MAP = """\
 Start         Length     Name                   Class
 0001:00000000 000082817H _TEXT                  CODE

  Address         Publics by Name

 0001:00000074  Idle freeanim(anim**)

  Address         Publics by Value

 0001:00000000  Idle __acrtused
 0001:00000074  Idle freeanim(anim**)
 0001:000000CD  Idle stepanim(anim*)
 0002:00000000       _someData
"""

LOG = """\
Trace 0: 0x311b5ef00 [00000000/0000000000410074/00000ab0/ff020200]
Trace 0: 0x311b5ef40 [00000000/0000000000410080/00000ab0/ff020200]
Trace 0: 0x311b5ef80 [00000000/00000000004100d0/00000ab0/ff020200]
Stopped execution of TB chain before 0x311b5ef00 [0000000000410074]
Trace 0: 0x311b5efc0 [00000000/0000000000410010/00000ab0/ff020200]
"""


def test_functions_as_they_change(tmp_path: Path) -> None:
    (tmp_path / "zoombi32.map").write_text(MAP)
    code_map = read_map(tmp_path / "zoombi32.map", 0x410000)
    assert code_map.names == ["__acrtused", "freeanim(anim**)", "stepanim(anim*)"]
    assert functions(LOG, code_map) == [
        (0x410074, "freeanim(anim**)"),
        (0x4100D0, "stepanim(anim*)"),
        (0x410010, "__acrtused"),
    ]
