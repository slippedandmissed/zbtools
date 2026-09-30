"""The formats resources are stored in in assets/, by resource type (see
base.Format). Types without one, and resources a format can't represent
exactly, are kept raw."""

from zbtools import paths
from zbtools.formats.base import Format, Raw, Unconvertible
from zbtools.formats.cursor import CursorFormat
from zbtools.formats.image import ImageFormat
from zbtools.formats.midi import MidiFormat
from zbtools.formats.palette import PaletteFormat, ShapeListFormat
from zbtools.formats.script import ScriptFormat
from zbtools.formats.sound import SoundFormat
from zbtools.formats.tables import MessagesFormat, NodesFormat, PathsFormat, WordTableFormat

FORMATS: dict[bytes, Format] = {
    b"\0SND": SoundFormat(),
    b"tMID": MidiFormat(),
    b"CURS": CursorFormat(),
    b"tPAL": PaletteFormat(),
    b"SHPL": ShapeListFormat(),
    b"tBMP": ImageFormat(paths.ASSETS_CACHE / "lzss"),
    b"SCRB": ScriptFormat(zoombini=False),
    b"SCRS": ScriptFormat(zoombini=True),
    b"REGS": WordTableFormat(),
    b"NODE": NodesFormat(),
    b"PATH": PathsFormat(),
    b"SYSX": MessagesFormat(),
}

__all__ = ["FORMATS", "Format", "Raw", "Unconvertible"]
