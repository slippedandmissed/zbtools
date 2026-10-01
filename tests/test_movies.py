import random
import struct
from pathlib import Path

import pytest

from zbtools import movies
from zbtools.formats import Unconvertible, mov, qkbk
from zbtools.formats import scene as movies_scene

GREY = [f"#{i:02x}{i:02x}{i:02x}" for i in range(256)]


def test_runs_of_8_or_more_are_run_tokens_and_shorter_ones_literals() -> None:
    assert qkbk.encode_row(bytes([5] * 8)) == bytes([0x87, 5])
    # seven 5s are literal (and the odd length is padded), 0s always run
    assert qkbk.encode_row(bytes([5] * 7)) == bytes([6, 5, 5, 5, 5, 5, 5, 5])
    assert qkbk.encode_row(bytes([0, 3, 3, 0])) == bytes([0x80, 0, 1, 3, 3, 0x80, 0, 0])


def test_a_long_run_is_cut_at_128_and_its_short_remainder_joins_the_next_literal() -> None:
    row = bytes([7] * 130 + [9])
    assert qkbk.encode_row(row) == bytes([0xFF, 7, 2, 7, 7, 9])


def test_rows_are_padded_to_an_even_length() -> None:
    assert len(qkbk.encode_row(bytes([1, 2, 3]))) % 2 == 0
    assert qkbk.encode_row(bytes([1, 2, 3])) == bytes([2, 1, 2, 3])
    assert qkbk.encode_row(bytes([1, 2])) == bytes([1, 1, 2, 0])


def test_random_rows_round_trip() -> None:
    rng = random.Random(1)
    for _ in range(300):
        width = rng.randrange(1, 400)
        row = bytes(rng.choice((0, 0, 1, 2, 3)) if rng.random() < 0.5 else 9 for _ in range(width))
        assert qkbk.decode_row(qkbk.encode_row(row), width) == row


def test_row_data_the_encoder_wouldnt_write_is_refused() -> None:
    with pytest.raises(Unconvertible):  # a run of 3 that should be literal
        qkbk.decode_row(bytes([0x82, 5]), 3)
    with pytest.raises(Unconvertible):  # ends before the row does
        qkbk.decode_row(bytes([0x87, 5]), 9)
    with pytest.raises(Unconvertible):  # a run past the row's end
        qkbk.decode_row(bytes([0x87, 5]), 4)


def _palette(palette_id: int, shade: int = 0) -> qkbk.Palette:
    colours = [f"#{(i + shade) % 256:02x}0000" for i in range(256)]
    return qkbk.Palette(id=palette_id, head=(1, 4, 0), colours=colours)


def _bitmap(cast: int, width: int, height: int, seed: int) -> qkbk.Bitmap:
    rng = random.Random(seed)
    pixels = bytes(rng.choice((0, 0, 4, 4, 4, 5, 200)) for _ in range(width * height))
    return qkbk.Bitmap(id=cast, head=(3, 50, 512), width=width, height=height, pixels=pixels)


def _scene() -> qkbk.Movie:
    casts: dict[int, qkbk.Cast] = {
        10: _palette(10),
        11: _palette(11, 1),
        20: _bitmap(20, 13, 5, 1),
        21: _bitmap(21, 4, 4, 2),
    }
    frames = [
        qkbk.Frame(
            f4=4,
            f6=5,
            fill=2,
            flags=0,
            palette=10,
            sprites=[qkbk.Sprite(1, 20, -2, 0)],
            casts=[10, 11, 20, 21],
            xfrm=qkbk.Rect(0, 0, 6, 13),
            tail=b"Scal\0\0\0\x0c\1\2\3\4",
        ),
        qkbk.Frame(
            f4=2,
            f6=1,
            fill=2,
            flags=qkbk.FLAG_KEEP_BACKGROUND,
            palette=11,
            sprites=[qkbk.Sprite(1, 20, -2, 0), qkbk.Sprite(4, 21, 6, 1)],
            back=[qkbk.Rect(0, 0, 6, 13), qkbk.Rect(1, 1, 2, 2)],
            front=[qkbk.Rect(3, 3, 4, 4)],
            xfrm=qkbk.Rect(0, 0, 0, 0),
        ),
        qkbk.Frame(f4=3, f6=1, fill=2, flags=qkbk.FLAG_LAST, palette=11),
    ]
    return qkbk.Movie(slots=6, f23=10, frames=frames, casts=casts)


def test_a_scene_round_trips_through_frames() -> None:
    scene = _scene()
    samples = qkbk.build_movie(scene)
    assert qkbk.parse_movie(samples) == scene
    assert all(s[:4] == b"\0\x09\0" + bytes([i]) for i, s in enumerate(samples, 1))


def test_a_frames_sprites_are_as_big_as_their_bitmaps() -> None:
    sample = qkbk.build_movie(_scene())[1]
    entry = sample[qkbk.HEADER_SIZE + 4 * qkbk.SPRITE_SIZE :][: qkbk.SPRITE_SIZE]
    # cast 21, at y 1 and x 6, 4 high and 4 wide
    assert entry == b"".join(v.to_bytes(2, "big") for v in (21, 1, 6, 4, 4))


def _rects(sample: bytes) -> tuple[qkbk.Rect, qkbk.Rect]:
    """A sample's bounds and what it changes."""

    def rect(at: int) -> qkbk.Rect:
        words = [
            int.from_bytes(sample[at + 2 * i : at + 2 * i + 2], "big", signed=True)
            for i in range(4)
        ]
        return qkbk.Rect(*words)

    return rect(0x08), rect(0x10)


def test_a_frames_bounds_and_changes_are_worked_out_from_its_sprites() -> None:
    first, second, third = qkbk.build_movie(_scene())
    big = qkbk.Rect(0, -2, 5, 11)  # the 13 by 5 bitmap at x -2
    small = qkbk.Rect(1, 6, 5, 10)  # the 4 by 4 one at x 6, y 1
    assert _rects(first) == (big, big)  # everything is new
    assert _rects(second) == (qkbk.Rect(0, -2, 5, 11), small)  # only slot 4 changed
    assert _rects(third) == (qkbk.Rect(0, 0, 0, 0), big)  # both sprites are gone


def test_a_sprite_of_a_cast_item_not_defined_yet_is_refused() -> None:
    scene = _scene()
    late = scene.frames[0].model_copy(update={"casts": [10, 11, 20]})
    with pytest.raises(ValueError, match="isn't a bitmap"):
        qkbk.build_movie(scene.model_copy(update={"frames": [late, *scene.frames[1:]]}))


def test_frames_with_flags_nobody_understands_are_refused() -> None:
    samples = qkbk.build_movie(_scene())
    bad = bytearray(samples[2])
    bad[0x1B] |= 2  # the flag for a block of callbacks
    with pytest.raises(Unconvertible):
        qkbk.parse_movie([*samples[:2], bytes(bad)])


def test_frames_are_drawn_in_slot_order_with_colour_0_transparent() -> None:
    scene = _scene()
    frame = scene.frames[1]
    image = qkbk.render_frame(scene, 2, (13, 6))
    background = scene.casts[20]
    small = scene.casts[21]
    assert isinstance(background, qkbk.Bitmap)
    assert isinstance(small, qkbk.Bitmap)
    # the bitmap at x -2 covers columns 0 to 10, and only where it isn't colour 0
    assert image.getpixel((0, 0)) == (background.pixels[2] or frame.fill)
    assert image.getpixel((12, 0)) == frame.fill  # outside both sprites
    # slot 4 draws over slot 1
    assert image.getpixel((6, 1)) == (small.pixels[0] or image.getpixel((6, 1)))
    assert image.getpixel((6, 1)) in {small.pixels[0], background.pixels[8]}


def _info(frames: list[bytes], sound: bytes) -> mov.MovInfo:
    return mov.MovInfo(
        movie=mov.Times(10, 11),
        volume=255,
        timescale=600,
        video_track=mov.Times(12, 13),
        video_media=mov.Times(14, 15),
        width=13,
        height=6,
        frame_duration=60,
        sync=[1, 3],
        audio_track=mov.Times(16, 17),
        audio_media=mov.Times(18, 19),
        rate=11025,
        audio_edits=[(120, -1), (60 * len(frames) - 120, 0)],
        head=b"\x01\x02\x03",
        chunks=["a4", "v2", "xf8", "a4", "v1"][:5],
    )


def test_a_movie_file_round_trips() -> None:
    frames = qkbk.build_movie(_scene())
    sound = bytes(range(8))
    info = _info(frames, sound)
    data = mov.build(info, frames, sound)
    assert mov.parse(data) == (info, frames, sound)
    assert data[4:8] == b"moov"
    assert b"mdat" in data


def test_chunks_that_dont_hold_the_frames_and_sound_are_refused() -> None:
    frames = qkbk.build_movie(_scene())
    with pytest.raises(ValueError, match="exactly"):
        mov.build(_info(frames, bytes(3)), frames, bytes(3))


def test_a_file_that_isnt_laid_out_like_the_games_is_refused() -> None:
    with pytest.raises(Unconvertible):
        mov.parse(b"\0\0\0\x08free\0\0\0\x08mdat")


def test_a_movie_round_trips_through_assets(tmp_path: Path) -> None:
    frames = qkbk.build_movie(_scene())
    sound = bytes(range(0, 256, 32))
    disc = tmp_path / "disc"
    (disc / "DATA").mkdir(parents=True)
    data = mov.build(_info(frames, sound), frames, sound)
    (disc / "DATA" / "T.MOV").write_bytes(data)
    out = tmp_path / "assets" / movies.DIRECTORY / "T"
    movies.extract_movie(disc / "DATA" / "T.MOV", disc, out)
    assert (out / "casts" / "20.png").is_file()
    manifest, packed = movies.pack_movie(out)
    assert manifest.path == "DATA/T.MOV"
    assert packed == data
    assert movies.movie_directories(tmp_path / "assets") == [out]
    assert movies.verify_all(disc, tmp_path / "assets") == [("T", [])]


def test_editing_a_movies_files_changes_the_packed_movie(tmp_path: Path) -> None:
    frames = qkbk.build_movie(_scene())
    sound = bytes(range(0, 256, 32))
    disc = tmp_path / "disc"
    (disc / "DATA").mkdir(parents=True)
    (disc / "DATA" / "T.MOV").write_bytes(mov.build(_info(frames, sound), frames, sound))
    out = tmp_path / "assets" / movies.DIRECTORY / "T"
    movies.extract_movie(disc / "DATA" / "T.MOV", disc, out)
    frames_file = out / movies.FRAMES
    frames_file.write_text(
        frames_file.read_text().replace("sprites = [", "sprites = [\n    [5, 21, 0, 0],", 1)
    )
    assert movies.verify_all(disc, tmp_path / "assets") == [("T", ["DATA/T.MOV: differs"])]
    [written] = movies.render_frames(out, [1], tmp_path / "frames")
    assert written.name == "T-0001.png"


def test_a_scene_file_holds_the_frames_casts_and_sound() -> None:
    scene = _scene()
    data = movies_scene.build(
        scene, size=(13, 6), milliseconds=100, rate=11025, sound=bytes([1, 2, 3])
    )
    assert data[:4] == b"ZBSC"
    version, width, height, ms, frames, rate, samples, casts = struct.unpack_from(
        "<IHHIIIII", data, 4
    )
    assert (version, width, height, ms, frames, rate, samples, casts) == (
        1, 13, 6, 100, 3, 11025, 3, 4,
    )  # fmt: skip
    assert data.endswith(bytes([1, 2, 3]))
    at = 32
    fill, count, palette = struct.unpack_from("<BBH", data, at)
    assert (fill, count, palette) == (2, 1, 10)
    assert struct.unpack_from("<Hhh", data, at + 4) == (20, -2, 0)


def test_the_edit_list_turns_into_silence_and_sound_on_one_timeline() -> None:
    signed = bytes([0, 1, 2, 3, 4, 5, 6, 7])  # unsigned: 128, 129, ...
    # 600 units a second and 10 samples a second: each unit is 1/60 of a sample
    track = movies_scene.expand_edits(signed, [(120, -1), (180, 2), (60, -1)], 600, 10)
    assert track == bytes([128, 128, 130, 131, 132, 128])
    # (the sound runs out after one sample: the rest is silence)
    assert movies_scene.expand_edits(signed, [(120, 7)], 600, 10) == bytes([135, 128])
