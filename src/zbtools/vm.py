"""Windows 98 virtual machine for running the original game under QEMU.

  uv run vm install     unattended Windows 98 SE install (once; 30-60 minutes)
  uv run vm run         boot the VM with the game disc in the CD drive
  uv run vm reset       discard all changes made since the install
  uv run vm screenshot  save a PNG of the running VM's screen

`install` writes a pristine base disk that is never modified afterwards. The VM
runs from a copy-on-write overlay on top of it, so `reset` (or
`uv run clean vm-state`) returns to a fresh install in seconds.
"""

import argparse
import socket
import struct
import subprocess
import sys
import time
from collections.abc import Callable
from pathlib import Path
from typing import Literal

import pycdlib
from pydantic import BaseModel, ConfigDict

from zbtools import env, host, paths
from zbtools.fat12 import Fat12Image

FLOPPY_SIZE = 1474560

# Emulated PC: hardware Windows 98 has inbox drivers for, and no network card
# (so setup never waits on network configuration).
MACHINE_ARGS = [
    "-machine",
    "pc",
    "-cpu",
    "pentium2",
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

# Setup answer file. The final RunOnce entry powers the VM off after the first
# logon, which is how `vm install` knows setup has finished.
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
Org=""
Display=0

[Network]
Display=0

[Install]
AddReg=ZbAddReg
DelReg=ZbDelReg

[ZbAddReg]
HKLM,%KEY_RUNONCE%,ZbPowerOff,,"rundll32.exe shell32.dll,SHExitWindowsEx 5"

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
        floppy = Fat12Image(f.read(FLOPPY_SIZE))

    for name in UNUSED_FLOPPY_FILES:
        if name in floppy.listdir():
            floppy.remove(name)
    floppy.write("CONFIG.SYS", _dos_text(CONFIG_SYS))
    floppy.write("AUTOEXEC.BAT", _dos_text(AUTOEXEC_BAT))
    floppy.write("MSBATCH.INF", _dos_text(MSBATCH_INF.format(product_key=product_key)))
    out.write_bytes(floppy.img)


def build_disk(out: Path, size_gib: int) -> None:
    """Create a qcow2 disk holding one active FAT32 (LBA) partition, unformatted."""
    sectors = size_gib * 1024**3 // 512
    start = 63
    mbr = bytearray(512)
    # status, CHS start (head 1, sector 1, cyl 0), type 0x0C, CHS end (max), LBA start, length
    struct.pack_into(
        "<B3sB3sII",
        mbr,
        446,
        0x80,
        bytes([1, 1, 0]),
        0x0C,
        bytes([0xFE, 0xFF, 0xFF]),
        start,
        sectors - start,
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
        "-audiodev",
        host.qemu_audiodev("snd"),
        "-device",
        "sb16,audiodev=snd",
        *host.qemu_display_args(headless),
        "-monitor",
        f"unix:{paths.VM_MONITOR},server,nowait",
        "-qmp",
        f"unix:{paths.VM_QMP},server,nowait",
        "-drive",
        f"file={disk},format=qcow2,if=ide,index=0",
    ]
    if cdrom:
        cmd += ["-drive", f"file={cdrom},format=raw,if=ide,index=2,media=cdrom,readonly=on"]
    if floppy:
        cmd += ["-drive", f"file={floppy},format=raw,if=floppy,index=0"]
    cmd += ["-boot", f"order=c,once={boot_once}" if boot_once else "order=c"]
    return cmd


# QMP (QEMU Machine Protocol) messages. Only the fields we use are modelled;
# QEMU sends many more, which are ignored.
class _QmpMessage(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    event: str | None = None


class _ShutdownData(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    guest: bool
    reason: str  # e.g. "guest-shutdown", "host-signal", "host-ui"


class _ShutdownEvent(BaseModel):
    model_config = ConfigDict(extra="ignore", frozen=True)
    event: Literal["SHUTDOWN"]
    data: _ShutdownData


def run_qemu(cmd: list[str], on_reset: Callable[[], None] | None = None) -> str | None:
    """Run QEMU until it exits. Returns the reason from QEMU's SHUTDOWN event
    ("guest-shutdown" when the guest powered itself off), or None if unknown."""
    for sock in (paths.VM_MONITOR, paths.VM_QMP):
        sock.unlink(missing_ok=True)
    proc = subprocess.Popen(cmd)
    reason = None
    try:
        qmp = _qmp_connect(proc)
        if qmp:
            with qmp, qmp.makefile("rb") as events:
                qmp.sendall(b'{"execute": "qmp_capabilities"}\n')
                for line in events:
                    event = _QmpMessage.model_validate_json(line).event
                    if event == "SHUTDOWN":
                        reason = _ShutdownEvent.model_validate_json(line).data.reason
                    elif event == "RESET" and on_reset:
                        on_reset()
        proc.wait()
    except KeyboardInterrupt:
        proc.terminate()
        proc.wait()
        raise
    finally:
        for sock in (paths.VM_MONITOR, paths.VM_QMP):
            sock.unlink(missing_ok=True)
    return reason


def _qmp_connect(proc: subprocess.Popen[bytes], timeout: float = 10) -> socket.socket | None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline and proc.poll() is None:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        try:
            s.connect(str(paths.VM_QMP))
        except OSError:
            s.close()
            time.sleep(0.1)
        else:
            return s
    return None


def vm_running() -> bool:
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        try:
            s.connect(str(paths.VM_MONITOR))
        except OSError:
            return False
        return True


def monitor(command: str) -> str:
    """Send one command to the running VM's QEMU monitor and return its output."""
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        try:
            s.connect(str(paths.VM_MONITOR))
        except OSError:
            sys.exit("error: the VM is not running")
        s.settimeout(5)
        s.sendall(command.encode() + b"\n")
        out = b""
        try:
            while chunk := s.recv(4096):
                out += chunk
                if out.rstrip().endswith(b"(qemu)") and out.count(b"(qemu)") >= 2:
                    break
        except TimeoutError:
            pass
    return out.decode(errors="replace")


def ensure_overlay() -> None:
    """Create the copy-on-write overlay on top of the base disk if missing."""
    if not paths.WIN98_BASE.exists():
        sys.exit("error: Windows 98 isn't installed yet; run `uv run vm install` first")
    if not paths.WIN98_OVERLAY.exists():
        # A relative backing path keeps the pair valid if the repo moves.
        subprocess.run(
            [
                host.qemu_img(),
                "create",
                "-q",
                "-f",
                "qcow2",
                "-b",
                paths.WIN98_BASE.name,
                "-F",
                "qcow2",
                str(paths.WIN98_OVERLAY),
            ],
            check=True,
        )
        print(f"Created a fresh overlay on the base install: {paths.WIN98_OVERLAY}")


class _Args(argparse.Namespace):
    func: Callable[["_Args"], None]
    windows_iso: Path
    disk_size: int
    force: bool
    headless: bool
    cdrom: Path
    output: Path


def cmd_install(args: _Args) -> None:
    if vm_running():
        sys.exit("error: the VM is already running")
    if not args.windows_iso.is_file():
        sys.exit(f"error: Windows ISO not found: {args.windows_iso} (see README: Setup)")
    product_key = env.get("WINDOWS_PRODUCT_KEY")
    if paths.WIN98_BASE.exists() and not args.force:
        sys.exit(
            f"error: Windows 98 is already installed ({paths.WIN98_BASE}); use --force to reinstall"
        )

    paths.VM_DIR.mkdir(parents=True, exist_ok=True)
    disk = paths.WIN98_BASE_PARTIAL
    disk.unlink(missing_ok=True)
    build_setup_floppy(args.windows_iso, product_key, paths.WIN98_SETUP_FLOPPY)
    build_disk(disk, args.disk_size)

    print(
        "Installing Windows 98 SE unattended. This takes 30-60 minutes; the VM "
        "powers off by itself when setup has finished. Don't close its window."
    )
    cmd = qemu_command(
        disk,
        cdrom=args.windows_iso,
        floppy=paths.WIN98_SETUP_FLOPPY,
        boot_once="a",
        headless=args.headless,
    )
    started = time.monotonic()

    def on_reset() -> None:
        print(f"  [{(time.monotonic() - started) / 60:4.1f} min] VM rebooted")

    reason = run_qemu(cmd, on_reset=on_reset)
    paths.WIN98_SETUP_FLOPPY.unlink(missing_ok=True)
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


def cmd_run(args: _Args) -> None:
    if vm_running():
        sys.exit("error: the VM is already running")
    ensure_overlay()
    run_qemu(qemu_command(paths.WIN98_OVERLAY, cdrom=args.cdrom, headless=args.headless))


def cmd_reset(_args: _Args) -> None:
    if vm_running():
        sys.exit("error: the VM is running; shut it down first")
    if paths.WIN98_OVERLAY.exists():
        paths.WIN98_OVERLAY.unlink()
        print("Discarded all changes; the next `vm run` starts from a fresh install.")
    else:
        print("Nothing to reset.")


def cmd_screenshot(args: _Args) -> None:
    out = args.output.resolve()
    monitor(f"screendump {out} -f png")
    print(out)


def main() -> None:
    doc = __doc__ or ""
    parser = argparse.ArgumentParser(
        description=doc.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="\n".join(doc.splitlines()[2:]),
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("install", help="unattended Windows 98 SE install")
    p.add_argument(
        "--windows-iso",
        type=Path,
        default=paths.WINDOWS_ISO,
        help="Windows 98 SE install CD image (default: %(default)s)",
    )
    p.add_argument(
        "--disk-size",
        type=int,
        default=2,
        metavar="GIB",
        help="virtual hard disk size in GiB (default: %(default)s)",
    )
    p.add_argument(
        "--force", action="store_true", help="reinstall even if Windows is already installed"
    )
    p.add_argument("--headless", action="store_true", help="don't open a VM window")
    p.set_defaults(func=cmd_install)

    p = sub.add_parser("run", help="boot the VM")
    p.add_argument(
        "--cdrom",
        type=Path,
        default=paths.GAME_ISO,
        help="disc image in the CD drive (default: %(default)s)",
    )
    p.add_argument("--headless", action="store_true", help="don't open a VM window")
    p.set_defaults(func=cmd_run)

    p = sub.add_parser("reset", help="discard all changes made since the install")
    p.set_defaults(func=cmd_reset)

    p = sub.add_parser("screenshot", help="save a PNG of the running VM's screen")
    p.add_argument("output", type=Path, nargs="?", default=paths.VM_DIR / "screen.png")
    p.set_defaults(func=cmd_screenshot)

    args = parser.parse_args(namespace=_Args())
    args.func(args)


if __name__ == "__main__":
    main()
