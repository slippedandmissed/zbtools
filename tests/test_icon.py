from pathlib import Path

import pytest
from PIL import Image

from zbtools.formats import icon
from zbtools.formats.base import Unconvertible

VGA = (
    (0, 0, 0), (128, 0, 0), (0, 128, 0), (128, 128, 0), (0, 0, 128), (128, 0, 128),
    (0, 128, 128), (192, 192, 192), (128, 128, 128), (255, 0, 0), (0, 255, 0),
    (255, 255, 0), (0, 0, 255), (255, 0, 255), (0, 255, 255), (255, 255, 255),
)  # fmt: skip


def _icon(size: int) -> icon.Icon:
    pixels = tuple((x + y) % 16 for y in range(size) for x in range(size))
    mask = tuple(x < 3 for y in range(size) for x in range(size))  # a transparent strip
    return icon.Icon(
        size, size, VGA, tuple(0 if m else p for p, m in zip(pixels, mask, strict=True)), mask
    )


@pytest.mark.parametrize("size", [16, 32])
def test_resource_data_round_trips(size: int) -> None:
    original = _icon(size)
    data = icon.to_bytes(original)
    assert len(data) == 40 + 64 + size * size // 2 + size * 4
    assert icon.parse(data) == original


def test_png_round_trips(tmp_path: Path) -> None:
    original = _icon(32)
    icon.save_png(original, tmp_path / "1.png")
    assert icon.load_png(tmp_path / "1.png") == original


def test_group_and_ico_file() -> None:
    images = [_icon(32), _icon(16)]
    group = icon.group([(1, images[0]), (2, images[1])])
    assert group[:6] == bytes([0, 0, 1, 0, 2, 0])
    assert group[6:14] == bytes([32, 32, 16, 0, 1, 0, 4, 0])  # width, height, colours, planes, bits
    ico = icon.ico_file(images)
    assert ico[:6] == group[:6]
    first = int.from_bytes(ico[18:22], "little")
    assert ico[first : first + 744] == icon.to_bytes(images[0])


def test_an_edited_rgba_png_takes_its_colours_in_order(tmp_path: Path) -> None:
    image = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    image.putpixel((5, 5), (10, 20, 30, 255))
    image.putpixel((6, 5), (40, 50, 60, 255))
    image.save(tmp_path / "edited.png")
    loaded = icon.load_png(tmp_path / "edited.png")
    assert loaded.palette[:2] == ((10, 20, 30), (40, 50, 60))
    assert loaded.pixels[5 * 16 + 6] == 1
    assert loaded.mask[0] and not loaded.mask[5 * 16 + 5]


def test_more_than_16_colours_is_an_error(tmp_path: Path) -> None:
    image = Image.new("RGB", (16, 16))
    for x in range(16):
        image.putpixel((x, 0), (x, 0, 0))
    image.putpixel((0, 1), (99, 99, 99))
    image.save(tmp_path / "too many.png")
    with pytest.raises(ValueError, match="17 colours"):
        icon.load_png(tmp_path / "too many.png")


def test_a_masked_pixel_with_a_colour_is_unconvertible(tmp_path: Path) -> None:
    inverting = icon.Icon(16, 16, VGA, (5,) + (0,) * 255, (True,) * 256)
    with pytest.raises(Unconvertible):
        icon.save_png(inverting, tmp_path / "x.png")
