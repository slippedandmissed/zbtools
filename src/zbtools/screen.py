"""Recognisers for the Windows 98 screens the tooling reacts to. All of them
assume Windows' default 640x480 desktop."""

from PIL import Image

type Rgb = tuple[int, int, int]

_NAVY: Rgb = (0, 0, 128)
_WHITE: Rgb = (255, 255, 255)
_GREY: Rgb = (192, 192, 192)
_TEAL: Rgb = (0, 128, 128)  # default desktop background


def _rgb(img: Image.Image, x: int, y: int) -> Rgb:
    px = img.getpixel((x, y))
    if not isinstance(px, tuple) or len(px) < 3:
        raise ValueError("expected an RGB image")
    return px[0], px[1], px[2]


def is_logon_prompt(img: Image.Image) -> bool:
    """Windows 98's "Enter Windows Password" dialog: navy title bar, two white
    text fields, and the yellow key icon (which setup's wizard pages lack)."""
    if img.size != (640, 480):
        return False
    if _rgb(img, 300, 73) != _NAVY:
        return False
    if _rgb(img, 300, 188) != _WHITE or _rgb(img, 300, 223) != _WHITE:
        return False
    yellow = 0
    for y in range(95, 145):
        for x in range(110, 150):
            r, g, b = _rgb(img, x, y)
            if r > 200 and g > 200 and b < 80:
                yellow += 1
    return yellow > 100


def is_idle_desktop(img: Image.Image) -> bool:
    """The Windows desktop with nothing open: teal background across the screen
    (sampled away from the centre, where the mouse pointer starts) and the grey
    taskbar along the bottom."""
    if img.size != (640, 480):
        return False
    background = [(400, 60), (600, 150), (250, 350), (550, 400)]
    return all(_rgb(img, x, y) == _TEAL for x, y in background) and (_rgb(img, 320, 470) == _GREY)
