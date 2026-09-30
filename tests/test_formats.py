from pathlib import Path

import pytest
from PIL import Image

from zbtools.formats import Unconvertible, image, lzss, midi, script, tables
from zbtools.formats.base import chunk
from zbtools.formats.cursor import CursorFormat
from zbtools.formats.image import ImageFile, ImageFormat, ImageInfo, Picture, pack_row, unpack_row
from zbtools.formats.midi import MidiFormat
from zbtools.formats.palette import PaletteFormat, ShapeListFormat
from zbtools.formats.script import Frame, Script, ScriptFormat
from zbtools.formats.sound import Sound, SoundFormat, parse_mohawk, parse_wav, to_mohawk, to_wav

LOOPING = Sound(
    rate=11025,
    channels=1,
    bits=8,
    samples=bytes(range(7)),
    loops=0xFFFF,
    loop_start=2,
    loop_end=5,
    cues=True,
)


def test_sound_layout() -> None:
    data = to_mohawk(LOOPING)
    assert data[:12] == b"MHWK" + (len(data) - 8).to_bytes(4, "big") + b"WAVE"
    assert data[12:22] == b"Cue#" + bytes.fromhex("00000002 0000")
    body = data[30:]
    assert body[:20] == bytes.fromhex("2b11 00000007 08 01 0000 ffff 00000002 00000005")
    # Seven samples, padded to an even length.
    assert body[20:] == bytes(range(7)) + b"\0"


@pytest.mark.parametrize("sound", [LOOPING, Sound(22050, 2, 8, bytes(8))])
def test_sound_round_trip(sound: Sound, tmp_path: Path) -> None:
    assert parse_mohawk(to_mohawk(sound)) == sound
    assert parse_wav(to_wav(sound)) == sound
    SoundFormat().save(to_mohawk(sound), tmp_path / "1", [])
    assert SoundFormat().load(tmp_path / "1") == to_mohawk(sound)


def test_wav_ignores_other_chunks() -> None:
    wav = to_wav(LOOPING)
    extra = chunk(b"LIST", b"INFOISFT\x05\0\0\0edit\0", big_endian=False)
    edited = b"RIFF" + (len(wav) - 8 + len(extra)).to_bytes(4, "little") + wav[8:] + extra
    assert parse_wav(edited) == LOOPING


def test_wav_must_be_8_bit() -> None:
    wav = bytearray(to_wav(LOOPING))
    wav[34] = 16  # fmt's bits per sample
    with pytest.raises(ValueError, match="8-bit"):
        parse_wav(bytes(wav))


def test_sound_with_cue_points_is_unconvertible() -> None:
    data = bytearray(to_mohawk(LOOPING))
    data[21] = 1  # one cue point
    with pytest.raises(Unconvertible):
        parse_mohawk(bytes(data))


# Images


def test_pack_row() -> None:
    # Colour 0 always runs; other colours run from four; the row ends with a run.
    row = bytes([0, 5, 5, 5, 6, 6, 6, 6, 7, 8])
    assert pack_row(row) == bytes([0x80, 0, 0x02, 5, 5, 5, 0x83, 6, 0x00, 7, 0x80, 8])
    # Runs split at 128, and what's left is judged afresh.
    assert pack_row(bytes([9]) * 130) == bytes([0xFF, 9, 0x81, 9])
    assert pack_row(bytes([9]) * 130 + b"\x01") == bytes([0xFF, 9, 0x01, 9, 9, 0x80, 1])
    for sample in (row, bytes([9]) * 300, bytes(range(1, 200)), b"\x00"):
        assert unpack_row(pack_row(sample), len(sample)) == sample


def test_lzss_round_trip() -> None:
    for data in (b"", b"a", bytes(1000), bytes(range(256)) * 20, b"abcabcabd" * 300):
        assert lzss.decompress(lzss.compress(data, 10), len(data), 10) == data


def test_lzss_layout() -> None:
    # Three literals, then a match of 6 at ring position 1024 - 66 (where
    # writing starts): flags 0b0111, low bit first.
    assert lzss.compress(b"abcabcabc", 10) == bytes([0x07]) + b"abc" + bytes([0x0F, 0xBE])


def test_image_round_trip(tmp_path: Path) -> None:
    frames = [Picture(3, 2, bytes([0, 1, 1, 2, 2, 0])), Picture(5, 1, bytes([4, 4, 4, 4, 4]))]
    bank = ImageFile(
        compressed=True,
        bank=True,
        images=[ImageInfo(packed=True, tail="010203"), ImageInfo(packed=False)],
    )
    single = ImageFile(compressed=False, bank=False, images=[ImageInfo(packed=True)])
    for info, pictures in ((bank, frames), (single, frames[:1])):
        data = image.build(info, pictures)
        assert image.parse(data) == (info, pictures)
        stem = tmp_path / "7"
        ImageFormat().save(data, stem, [])
        assert ImageFormat().load(stem) == data


# Scripts and tables


def test_script_round_trip(tmp_path: Path) -> None:
    zoombini_script = Script(
        facing=2,
        frames=[
            Frame(cels=[(1, 0, 0), (0, 0, 0), (2, -5, 10)]),
            Frame(cels=[], event=3, sound=1100),
        ],
    )
    data = script.script_bytes(zoombini_script, zoombini=True)
    assert data == bytes.fromhex(
        "0002 0002 0001 0000 0000 0000 0000 0000 0002 fffb 000a ff00 fe03 044c"
    )
    assert script.parse(data, zoombini=True) == zoombini_script
    ScriptFormat(zoombini=True).save(data, tmp_path / "1", [])
    assert ScriptFormat(zoombini=True).load(tmp_path / "1") == data


@pytest.mark.parametrize(
    ("kind", "data"),
    [
        ("words", bytes.fromhex("0001 fffe 8000")),
        ("nodes", bytes.fromhex("0002 0010 0020 0030 0040")),
        ("paths", bytes.fromhex("0001") + bytes([1, 2, 0, 3]) + bytes(20)),
        ("messages", bytes.fromhex("b07900 c005 e00040")),
    ],
)
def test_table_round_trip(kind: str, data: bytes, tmp_path: Path) -> None:
    formats: dict[
        str,
        tables.WordTableFormat | tables.NodesFormat | tables.PathsFormat | tables.MessagesFormat,
    ] = {
        "words": tables.WordTableFormat(),
        "nodes": tables.NodesFormat(),
        "paths": tables.PathsFormat(),
        "messages": tables.MessagesFormat(),
    }
    formats[kind].save(data, tmp_path / "1", [])
    assert formats[kind].load(tmp_path / "1") == data


# Cursors, palettes, MIDI


def test_cursor_round_trip(tmp_path: Path) -> None:
    # Image rows 0xf0f0, mask rows 0xff00: black, white, then an inverted
    # pixel and a transparent one per byte; hot spot (v 3, h 5).
    data = bytes.fromhex("f0f0") * 16 + bytes.fromhex("ff00") * 16 + bytes.fromhex("00030005")
    CursorFormat().save(data, tmp_path / "1", [])
    assert CursorFormat().load(tmp_path / "1") == data
    with Image.open(tmp_path / "1.cur") as cursor:
        assert cursor.size == (16, 16)


def test_palette_round_trip(tmp_path: Path) -> None:
    palette = bytes.fromhex("000a 0002 fefefe01 20304001")
    PaletteFormat().save(palette, tmp_path / "1", [])
    assert PaletteFormat().load(tmp_path / "1") == palette
    shapes = bytes.fromhex("03e8 0003") + palette
    ShapeListFormat().save(shapes, tmp_path / "2", [])
    assert ShapeListFormat().load(tmp_path / "2") == shapes


def test_midi_round_trip(tmp_path: Path) -> None:
    # Program changes on channels 0 and 2 (program 5) and 1 (program 3).
    track = bytes.fromhex("00c005 00c103 00c205 00ff2f00")
    smf = chunk(b"MThd", bytes.fromhex("0000 0001 01e0"), True, padded=False)
    smf += chunk(b"MTrk", track, True, padded=False)
    data = midi.to_mohawk(smf)
    assert bytes.fromhex("0002 0003 0002 0005 0005") in data  # Prg#
    MidiFormat().save(data, tmp_path / "1", [])
    assert (tmp_path / "1.mid").read_bytes() == smf
    assert MidiFormat().load(tmp_path / "1") == data
