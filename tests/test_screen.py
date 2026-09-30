from PIL import Image

from zbtools.screen import is_run_dialog

TEAL, GREY, NAVY, WHITE = (0, 128, 128), (192, 192, 192), (0, 0, 128), (255, 255, 255)


def _desktop() -> Image.Image:
    img = Image.new("RGB", (640, 480), TEAL)
    img.paste(GREY, (0, 452, 640, 480))  # the taskbar
    return img


def test_run_dialog() -> None:
    img = _desktop()
    img.paste(GREY, (5, 287, 350, 447))
    img.paste(NAVY, (8, 290, 347, 305))
    img.paste(WHITE, (10, 292, 40, 302))  # the title's text
    assert is_run_dialog(img)


def test_not_a_run_dialog() -> None:
    img = _desktop()
    assert not is_run_dialog(img)
    img.paste(WHITE, (25, 140, 435, 390))  # a window (the Recycle Bin)
    assert not is_run_dialog(img)
