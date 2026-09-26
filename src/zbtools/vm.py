"""Windows 98 virtual machine for running the original game under QEMU.

`install` writes a pristine base disk that is never modified afterwards. The VM
runs from a copy-on-write overlay on top of it, so `reset` (or
`uv run clean vm-state`) returns to a fresh install in seconds.
"""

import asyncio
import contextlib
import os
import subprocess
import sys
import tempfile
import time
from collections.abc import Awaitable, Callable, Iterable
from pathlib import Path
from typing import Annotated, Literal

import pycdlib
import typer
from PIL import Image
from pydantic import BaseModel, ConfigDict
from qemu.qmp import ConnectError, Message, QMPClient, QMPError, Runstate

from zbtools import env, host, paths, screen

FLOPPY_SIZE = 1474560

# Emulated PC: hardware Windows 98 has inbox drivers for, and no network card.
# The CPU has no local APIC (Windows 98 doesn't use one): Windows 9x restarts
# through a BIOS warm-boot path that leaves QEMU's APIC blocking the legacy
# timer interrupt, which hung setup on the splash screen at its first reboot.
MACHINE_ARGS = [
    "-machine",
    "pc",
    "-cpu",
    "pentium2,-apic",
    "-m",
    "256",
    "-vga",
    "cirrus",
    "-rtc",
    "base=localtime",
    "-nic",
    "none",
]

# Replaces the CD boot floppy's menu with a non-interactive install: write MBR
# boot code, format the (pre-partitioned) disk, then run setup with MSBATCH.INF.
CONFIG_SYS = """\
DEVICE=A:\\HIMEM.SYS /TESTMEM:OFF
DEVICE=A:\\OAKCDROM.SYS /D:OEMCD001
FILES=60
BUFFERS=20
DOS=HIGH
STACKS=9,256
LASTDRIVE=Z
"""

AUTOEXEC_BAT = """\
@ECHO OFF
PATH=A:\\
MSCDEX.EXE /D:OEMCD001 /L:D
FDISK /MBR
IF NOT EXIST A:\\FORMAT.COM EXTRACT /Y /L A:\\ A:\\EBD.CAB FORMAT.COM
FORMAT C: /U /V:WIN98 /AUTOTEST
D:\\WIN98\\SMARTDRV.EXE
D:
CD \\WIN98
SETUP.EXE A:\\MSBATCH.INF /IS /IQ /IE /IM /NF
"""

# Setup answer file. The per-user RunOnce entry powers the VM off after the
# first logon (the machine-wide RunOnce key would run before the logon prompt),
# which is how `vm install` knows setup has finished. Some Windows CDs
# (OEM and upgrade) still stop on the pre-filled wizard pages until Next is
# clicked; that's by design and can't be turned off from here.
MSBATCH_INF = """\
[BatchSetup]
Version=3.0 (32-bit)

[Version]
Signature="$CHICAGO$"

[Setup]
Express=1
InstallDir="C:\\WINDOWS"
InstallType=1
ProductKey="{product_key}"
EBD=0
ShowEula=0
ChangeDir=0
OptionalComponents=1
Network=0
System=0
CCP=0
CleanBoot=0
Display=0
DevicePath=0
NoDirWarn=1
TimeZone="Pacific"
Uninstall=0
VRC=0
NoPrompt2Boot=1

[System]
Locale=L0409
SelectedKeyboard=KEYBOARD_00000409

[NameAndOrg]
Name="Zoombinis"
Org="Zoombinis Decompilation"
Display=0

[Network]
ComputerName="ZOOMBINIS"
Workgroup="WORKGROUP"
Description="Zoombinis VM"
Display=0

[Install]
AddReg=ZbAddReg
DelReg=ZbDelReg

[ZbAddReg]
HKCU,%KEY_RUNONCE%,ZbPowerOff,,"rundll32.exe shell32.dll,SHExitWindowsEx 5"

[ZbDelReg]
HKLM,%KEY_RUN%,Welcome

[Strings]
KEY_RUN="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
KEY_RUNONCE="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
"""

# Removed from the boot floppy. JO.SYS is run by IO.SYS before anything else
# and shows a "boot from hard disk or CD-ROM" menu that defaults to the hard
# disk. The rest are drivers for hardware QEMU doesn't emulate, removed to make
# room for FORMAT.COM, which AUTOEXEC.BAT unpacks from EBD.CAB at boot.
UNUSED_FLOPPY_FILES = [
    "JO.SYS",
    "ASPI2DOS.SYS",
    "ASPI4DOS.SYS",
    "ASPI8DOS.SYS",
    "ASPI8U2.SYS",
    "ASPICD.SYS",
    "BTCDROM.SYS",
    "BTDOSM.SYS",
    "FLASHPT.SYS",
]


def _dos_text(text: str) -> bytes:
    return text.replace("\n", "\r\n").encode("ascii")


def _mtools(tool: str, image: Path, *args: str) -> str:
    """Run an mtools program (mdir, mdel, mcopy, ...) on a floppy image."""
    result = subprocess.run(
        [host.mtools(tool), "-i", str(image), *args],
        check=True,
        capture_output=True,
        text=True,
        env={**os.environ, "MTOOLS_SKIP_CHECK": "1"},
    )
    return result.stdout


def build_setup_floppy(windows_iso: Path, product_key: str, out: Path) -> None:
    """Customize the Windows 98 CD's El Torito boot floppy for unattended setup."""
    iso = pycdlib.PyCdlib()
    iso.open(str(windows_iso))
    try:
        catalog = iso.eltorito_boot_catalog
        if catalog is None or catalog.initial_entry.boot_media_type != 2:
            sys.exit(
                f"error: {windows_iso} has no 1.44 MB boot floppy image; "
                "is it a Windows 98 SE install CD?"
            )
        lba = catalog.initial_entry.load_rba
    finally:
        iso.close()
    with windows_iso.open("rb") as f:
        f.seek(lba * 2048)
        out.write_bytes(f.read(FLOPPY_SIZE))

    present = {
        line.rsplit("/", 1)[-1].upper() for line in _mtools("mdir", out, "-b", "::/").split()
    }
    unused = [f"::/{name}" for name in UNUSED_FLOPPY_FILES if name in present]
    if unused:
        _mtools("mdel", out, *unused)
    files = {
        "CONFIG.SYS": CONFIG_SYS,
        "AUTOEXEC.BAT": AUTOEXEC_BAT,
        "MSBATCH.INF": MSBATCH_INF.format(product_key=product_key),
    }
    with tempfile.TemporaryDirectory() as tmp:
        for name, text in files.items():
            src = Path(tmp) / name
            src.write_bytes(_dos_text(text))
            _mtools("mcopy", out, "-o", str(src), f"::/{name}")


def build_disk(out: Path, size_gib: int) -> None:
    """Create a qcow2 disk holding one active FAT32 (LBA) partition, unformatted."""
    sectors = size_gib * 1024**3 // 512
    start = 63
    mbr = bytearray(512)
    # status, CHS start (head 1, sector 1, cyl 0), type 0x0C, CHS end (max), LBA start, length
    mbr[446:462] = (
        bytes([0x80, 1, 1, 0, 0x0C, 0xFE, 0xFF, 0xFF])
        + start.to_bytes(4, "little")
        + (sectors - start).to_bytes(4, "little")
    )
    mbr[510:512] = b"\x55\xaa"
    raw = out.with_suffix(".raw")
    with raw.open("wb") as f:
        f.write(mbr)
        f.truncate(sectors * 512)
    try:
        subprocess.run(
            [host.qemu_img(), "convert", "-q", "-f", "raw", "-O", "qcow2", str(raw), str(out)],
            check=True,
        )
    finally:
        raw.unlink()


def create_overlay(path: Path, backing: Path) -> None:
    """Create a copy-on-write qcow2 disk on top of backing (in the same directory;
    the relative backing path keeps the pair valid if the repo moves)."""
    subprocess.run(
        [
            host.qemu_img(), "create", "-q", "-f", "qcow2",
            "-b", backing.name, "-F", "qcow2", str(path),
        ],
        check=True,
    )  # fmt: skip


type BootDevice = Literal["a", "c", "d"]


def qemu_command(
    disk: Path,
    cdrom: Path | None = None,
    floppy: Path | None = None,
    boot_once: BootDevice | None = None,
    headless: bool = False,
) -> list[str]:
    cmd = [
        host.qemu_system(),
        *MACHINE_ARGS,
        "-audiodev", host.qemu_audiodev("snd"),
        "-device", "sb16,audiodev=snd",
        *host.qemu_display_args(headless),
        "-qmp", f"unix:{paths.VM_QMP},server=on,wait=off",
        "-qmp", f"unix:{paths.VM_QMP_CONTROL},server=on,wait=off",
        "-drive", f"file={disk},format=qcow2,if=ide,index=0",
    ]  # fmt: skip
    if cdrom:
        cmd += ["-drive", f"file={cdrom},format=raw,if=ide,index=2,media=cdrom,readonly=on"]
    if floppy:
        cmd += ["-drive", f"file={floppy},format=raw,if=floppy,index=0"]
    cmd += ["-boot", f"order=c,once={boot_once}" if boot_once else "order=c"]
    return cmd


# QMP events. Only the fields we use are modelled; the rest are ignored.
class _QmpEvent(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    event: str


class _ShutdownData(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    guest: bool
    reason: str  # e.g. "guest-shutdown", "host-signal", "host-ui"


class _ShutdownEvent(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    event: Literal["SHUTDOWN"]
    data: _ShutdownData


# Something to do with the running VM, e.g. watch its screen and type into it.
type GuestTask = Callable[[QMPClient], Awaitable[None]]


async def _disconnect(qmp: QMPClient) -> None:
    """Disconnect, ignoring how the connection ended: disconnect() re-raises the
    error that closed it, which is EOFError whenever QEMU has exited."""
    with contextlib.suppress(QMPError, EOFError, OSError):
        await qmp.disconnect()


async def _connect(qmp: QMPClient, address: Path, timeout: float = 10) -> None:
    """Connect to a QMP socket, waiting for QEMU to create it."""
    deadline = time.monotonic() + timeout
    while True:
        try:
            await qmp.connect(str(address))
        except ConnectError:
            if time.monotonic() > deadline:
                raise
            await asyncio.sleep(0.1)
        else:
            return


async def _supervise(
    cmd: list[str], on_reset: Callable[[], None] | None, tasks: Iterable[GuestTask]
) -> str | None:
    for sock in (paths.VM_QMP, paths.VM_QMP_CONTROL):
        sock.unlink(missing_ok=True)
    proc = await asyncio.create_subprocess_exec(*cmd)
    qmp = QMPClient("zbtools")
    reason: str | None = None

    def handle(event: Message) -> None:
        nonlocal reason
        name = _QmpEvent.model_validate(dict(event)).event
        if name == "SHUTDOWN":
            reason = _ShutdownEvent.model_validate(dict(event)).data.reason
        elif name == "RESET" and on_reset:
            on_reset()

    async def watch_events() -> None:
        async for event in qmp.events:
            handle(event)

    try:
        await _connect(qmp, paths.VM_QMP)
        workers = [asyncio.ensure_future(watch_events())]
        workers += [asyncio.ensure_future(task(qmp)) for task in tasks]
        await proc.wait()
        # QEMU sends SHUTDOWN just before exiting; let the reader catch up.
        with contextlib.suppress(TimeoutError):
            async with asyncio.timeout(5):
                while qmp.runstate == Runstate.RUNNING:
                    await qmp.runstate_changed()
        for worker in workers:
            worker.cancel()
        await asyncio.gather(*workers, return_exceptions=True)
        while not qmp.events.empty():
            handle(await qmp.events.get())
    finally:
        await _disconnect(qmp)
        if proc.returncode is None:
            proc.terminate()
            await proc.wait()
        for sock in (paths.VM_QMP, paths.VM_QMP_CONTROL):
            sock.unlink(missing_ok=True)
    return reason


def run_qemu(
    cmd: list[str],
    on_reset: Callable[[], None] | None = None,
    tasks: Iterable[GuestTask] = (),
) -> str | None:
    """Run QEMU until it exits, running tasks against the VM meanwhile. Returns
    the reason from QEMU's SHUTDOWN event ("guest-shutdown" when the guest
    powered itself off), or None if unknown."""
    return asyncio.run(_supervise(cmd, on_reset, tasks))


async def _control[T](action: Callable[[QMPClient], Awaitable[T]]) -> T:
    """Run action against the running VM through its second QMP socket."""
    qmp = QMPClient("zbtools-cli")
    await qmp.connect(str(paths.VM_QMP_CONTROL))
    try:
        return await action(qmp)
    finally:
        await _disconnect(qmp)


def vm_running() -> bool:
    async def nothing(_: QMPClient) -> None:
        return None

    try:
        asyncio.run(_control(nothing))
    except (ConnectError, OSError):
        return False
    return True


async def screenshot(qmp: QMPClient, path: Path) -> Image.Image:
    await qmp.execute("screendump", {"filename": str(path), "format": "png"})
    with Image.open(path) as img:
        return img.convert("RGB")


# QEMU key names for characters that aren't themselves key names.
_KEYS = {
    " ": "spc", "\\": "backslash", ".": "dot", ":": "shift-semicolon", "/": "slash",
    ",": "comma", "-": "minus", "_": "shift-minus", "=": "equal",
}  # fmt: skip


async def press(qmp: QMPClient, *combos: str) -> None:
    """Press key combinations such as "ret", "a" or "ctrl-esc"."""
    for combo in combos:
        keys = [{"type": "qcode", "data": key} for key in combo.split("-")]
        await qmp.execute("send-key", {"keys": keys})
        await asyncio.sleep(0.15)


async def type_text(qmp: QMPClient, text: str) -> None:
    await press(qmp, *(_KEYS.get(c) or (f"shift-{c.lower()}" if c.isupper() else c) for c in text))


async def when_screen(
    qmp: QMPClient,
    test: Callable[[Image.Image], bool],
    action: GuestTask,
    interval: float = 5.0,
) -> None:
    """Poll the screen until test matches, then run action once."""
    while True:
        await asyncio.sleep(interval)
        try:
            img = await screenshot(qmp, paths.VM_SCREEN_CHECK)
        except (QMPError, OSError):
            continue  # not ready, or between screens
        if test(img):
            await action(qmp)
            return


async def answer_logon_prompt(qmp: QMPClient) -> None:
    """Press Enter at the Windows logon prompt the first time it appears. With a
    blank password, Windows never shows the prompt again."""

    async def enter(qmp: QMPClient) -> None:
        await press(qmp, "ret")
        print("  Answered the Windows logon prompt (blank password)")

    await when_screen(qmp, screen.is_logon_prompt, enter)


def ensure_overlay() -> None:
    """Create the copy-on-write overlay on top of the base disk if missing."""
    if not paths.WIN98_BASE.exists():
        sys.exit("error: Windows 98 isn't installed yet; run `uv run vm install` first")
    if not paths.WIN98_OVERLAY.exists():
        create_overlay(paths.WIN98_OVERLAY, paths.WIN98_BASE)
        print(f"Created a fresh overlay on the base install: {paths.WIN98_OVERLAY}")


app = typer.Typer(help=__doc__, add_completion=False, no_args_is_help=True)


@app.command()
def install(
    windows_iso: Annotated[
        Path, typer.Option(help="Windows 98 SE install CD image")
    ] = paths.WINDOWS_ISO,
    disk_size: Annotated[
        int, typer.Option(metavar="GIB", help="Virtual hard disk size in GiB")
    ] = 2,
    force: Annotated[
        bool, typer.Option("--force", help="Reinstall even if Windows is already installed")
    ] = False,
    headless: Annotated[bool, typer.Option("--headless", help="Don't open a VM window")] = False,
) -> None:
    """Install Windows 98 SE from the answer file (once; 30-60 minutes)."""
    if vm_running():
        sys.exit("error: the VM is already running")
    if not windows_iso.is_file():
        sys.exit(f"error: Windows ISO not found: {windows_iso} (see README: Setup)")
    product_key = env.get("WINDOWS_PRODUCT_KEY")
    if paths.WIN98_BASE.exists() and not force:
        sys.exit(
            f"error: Windows 98 is already installed ({paths.WIN98_BASE}); use --force to reinstall"
        )

    paths.VM_DIR.mkdir(parents=True, exist_ok=True)
    disk = paths.WIN98_BASE_PARTIAL
    disk.unlink(missing_ok=True)
    build_setup_floppy(windows_iso, product_key, paths.WIN98_SETUP_FLOPPY)
    build_disk(disk, disk_size)

    print(
        "Installing Windows 98 SE from the answer file. This takes 30-60 minutes; the VM\n"
        "powers off by itself when setup has finished. Don't close its window.\n"
        "Depending on your Windows CD, setup may stop on a few wizard pages with the\n"
        "answers already filled in: click Next on each one to continue."
    )
    cmd = qemu_command(
        disk, cdrom=windows_iso, floppy=paths.WIN98_SETUP_FLOPPY, boot_once="a", headless=headless
    )
    started = time.monotonic()

    def on_reset() -> None:
        print(f"  [{(time.monotonic() - started) / 60:4.1f} min] VM rebooted")

    try:
        reason = run_qemu(cmd, on_reset=on_reset, tasks=[answer_logon_prompt])
    finally:
        paths.WIN98_SETUP_FLOPPY.unlink(missing_ok=True)
        paths.VM_SCREEN_CHECK.unlink(missing_ok=True)
    if reason != "guest-shutdown":
        disk.unlink(missing_ok=True)
        sys.exit(
            f"error: setup did not finish (QEMU stopped: {reason or 'unknown'}); "
            "the partial VM disk was deleted"
        )

    # The old overlay (if any) was layered on the previous base, so it's invalid.
    paths.WIN98_OVERLAY.unlink(missing_ok=True)
    if paths.WIN98_BASE.exists():
        paths.WIN98_BASE.chmod(0o644)
    disk.replace(paths.WIN98_BASE)
    paths.WIN98_BASE.chmod(0o444)  # guard the base against accidental writes
    minutes = (time.monotonic() - started) / 60
    print(f"Windows 98 installed in {minutes:.0f} min: {paths.WIN98_BASE}")


@app.command()
def run(
    cdrom: Annotated[Path, typer.Option(help="Disc image in the CD drive")] = paths.GAME_ISO,
    headless: Annotated[bool, typer.Option("--headless", help="Don't open a VM window")] = False,
) -> None:
    """Boot the VM with the game disc in the CD drive."""
    if vm_running():
        sys.exit("error: the VM is already running")
    ensure_overlay()
    run_qemu(qemu_command(paths.WIN98_OVERLAY, cdrom=cdrom, headless=headless))


@app.command()
def reset() -> None:
    """Discard all changes made since the install."""
    if vm_running():
        sys.exit("error: the VM is running; shut it down first")
    if paths.WIN98_OVERLAY.exists():
        paths.WIN98_OVERLAY.unlink()
        print("Discarded all changes; the next `vm run` starts from a fresh install.")
    else:
        print("Nothing to reset.")


@app.command(name="screenshot")
def screenshot_command(
    output: Annotated[Path, typer.Argument(help="PNG file to write")] = paths.VM_DIR / "screen.png",
) -> None:
    """Save a PNG of the running VM's screen."""
    out = output.resolve()

    async def dump(qmp: QMPClient) -> None:
        await qmp.execute("screendump", {"filename": str(out), "format": "png"})

    try:
        asyncio.run(_control(dump))
    except (ConnectError, OSError):
        sys.exit("error: the VM is not running")
    print(out)
