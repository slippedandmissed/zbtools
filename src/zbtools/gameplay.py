"""Instrumented gameplay tests: the headless port plays scripted flows and its screenshots are
compared with baselines checked into the repository.

`tests/gameplay/cases.toml` holds the cases: each a string of debug commands (the handbook's
"Debug tools") that takes the game to a scene and plays something (a puzzle won or lost in some
way, a dialog, a journey), with `screenshot NAME` wherever a picture should be compared and
`assert` wherever the game's own state should be checked. `uv run gameplay` runs every case on the
headless build (`uv run port build headless_wasm`), several at a time, each on a fresh C: drive,
and compares each screenshot with `tests/gameplay/baselines/<case>/<NAME>.png`. It fails if a
picture differs by more than the tolerance, if a baseline is missing or no case takes it, or if
a case crashes, fails an `assert` or does not finish. `uv run gameplay --rebaseline` writes the
pictures instead (only those that changed, so git sees only real differences) and removes
baselines no case takes any more.

Every run writes a report, build/gameplay/report.html: one self-contained file (the pictures are
inside it) with a summary, every case, and for each failing picture the baseline, the new
picture and the differences highlighted, plus the game's own output for a case that crashed. The
same summary as Markdown goes to `--summary FILE` (the pull request pipeline passes the job's
summary file, which GitHub shows on the run's page). The failing pictures are also written to
build/gameplay/<case>/ as `NAME.actual.png` and `NAME.diff.png` (baseline | new | differences).

The game runs in real time on the host, but with `seed` (every case starts with one) and the
scripts' waits its pictures are repeatable: a case whose pictures differ is played again before
it counts as failed (`--retries`), and the report says which cases needed it.
"""

import base64
import io
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import tomllib
from collections.abc import Callable
from concurrent.futures import FIRST_COMPLETED, Future, ThreadPoolExecutor, wait
from dataclasses import dataclass, field
from pathlib import Path
from typing import Annotated, NamedTuple

import jinja2
import typer
from PIL import Image, ImageChops
from pydantic import BaseModel, ConfigDict, field_validator

from zbtools import assets, paths, port, shots

CASES = paths.REPO_ROOT / "tests" / "gameplay" / "cases.toml"
BASELINES = paths.REPO_ROOT / "tests" / "gameplay" / "baselines"
# A pixel differs when some channel differs by more than this (of 255); a picture fails when more
# than a case's tolerance of its pixels differ.
CHANNEL_DELTA = 24
TOLERANCE = 0.005
# The most the report embeds of failing pictures (bytes of data URIs): a change that breaks every
# picture must not make a report too big to open (the files are in build/gameplay/ either way).
REPORT_BUDGET = 24_000_000
_NAME = re.compile(r"[a-z0-9][a-z0-9-]*")
_RUN_VARIABLES = ("GITHUB_SERVER_URL", "GITHUB_REPOSITORY", "GITHUB_RUN_ID")

app = typer.Typer(add_completion=False, help=__doc__)


class Case(BaseModel):
    """One scripted flow."""

    model_config = ConfigDict(extra="forbid")

    description: str
    commands: str  # debug commands; `screenshot NAME` takes a picture to compare
    seed: int = 1
    seconds: float = 90  # the longest the game may run: the case fails if it isn't done by then
    tolerance: float = TOLERANCE  # the fraction of pixels allowed to differ

    @field_validator("commands")
    @classmethod
    def _no_quit(cls, commands: str) -> str:
        if "quit" in [part.split()[0] for part in commands.split(";") if part.split()]:
            raise ValueError("the commands end by themselves: leave out `quit`")
        return commands

    @property
    def script(self) -> str:
        """The commands as the game runs them: scripted drags need the game's "sticky mouse"
        off (a release would only pick up), and its random numbers are seeded."""
        return f"set clickToDragOption 0; set dragClicks 0; seed {self.seed}; {self.commands}; quit"

    @property
    def asserts(self) -> bool:
        """Whether the commands check the game's state."""
        return any(part.split()[:1] == ["assert"] for part in self.commands.split(";"))

    @property
    def pictures(self) -> list[str]:
        """The names of the pictures the commands take, in order."""
        return screenshot_names(self.commands)


def screenshot_names(commands: str) -> list[str]:
    """The NAMEs of the `screenshot NAME` commands in a script."""
    names: list[str] = []
    for part in commands.split(";"):
        words = part.split()
        if len(words) == 2 and words[0] == "screenshot":
            names.append(words[1])
    return names


def load_cases(path: Path = CASES) -> dict[str, Case]:
    """The cases in `path`, by name."""
    with path.open("rb") as file:
        data = tomllib.load(file)
    return {name: Case.model_validate(table) for name, table in data.items()}


def problems_with(cases: dict[str, Case]) -> list[str]:
    """What is wrong with the cases themselves (names, pictures taken twice, nothing checked)."""
    found: list[str] = []
    for name, case in cases.items():
        if not _NAME.fullmatch(name):
            found.append(f"{name}: a case's name is lower-case words with hyphens")
        if not case.pictures and not case.asserts:
            found.append(f"{name}: checks nothing (add `assert NAME VALUE` or `screenshot NAME`)")
        for picture in case.pictures:
            if not _NAME.fullmatch(picture):
                found.append(f"{name}: picture name {picture!r} is lower-case words with hyphens")
        for picture in {p for p in case.pictures if case.pictures.count(p) > 1}:
            found.append(f"{name}: takes {picture} more than once")
    return found


# --- Comparing pictures


class Difference(NamedTuple):
    """How a picture differs from its baseline."""

    fraction: float  # of the pixels, those that differ by more than CHANNEL_DELTA
    size_differs: bool


def changed_mask(expected: Image.Image, actual: Image.Image) -> Image.Image:
    """A mask (255 where the pictures differ by more than CHANNEL_DELTA in some channel)."""
    delta = ImageChops.difference(expected.convert("RGB"), actual.convert("RGB"))
    red, green, blue = (c.point(lambda v: 255 if v > CHANNEL_DELTA else 0) for c in delta.split())
    return ImageChops.lighter(ImageChops.lighter(red, green), blue)


def difference(expected: Image.Image, actual: Image.Image) -> Difference:
    """Compares two pictures."""
    if expected.size != actual.size:
        return Difference(1.0, True)
    changed = changed_mask(expected, actual)
    return Difference(sum(changed.tobytes()) / 255 / (changed.width * changed.height), False)


def marked_picture(expected: Image.Image, actual: Image.Image) -> Image.Image:
    """The new picture dimmed, with the pixels that differ from the baseline in red."""
    actual = actual.convert("RGB").resize(expected.size)
    mask = changed_mask(expected, actual)
    dimmed = Image.blend(actual, Image.new("RGB", actual.size, "black"), 0.6)
    return Image.composite(Image.new("RGB", actual.size, (255, 0, 0)), dimmed, mask)


def side_by_side(expected: Image.Image, actual: Image.Image) -> Image.Image:
    """The baseline, the new picture and the differences, in a row."""
    expected, actual = expected.convert("RGB"), actual.convert("RGB").resize(expected.size)
    width, height = expected.size
    sheet = Image.new("RGB", (width * 3 + 8, height), "white")
    for i, picture in enumerate((expected, actual, marked_picture(expected, actual))):
        sheet.paste(picture, (i * (width + 4), 0))
    return sheet


def palette_png(picture: Image.Image) -> Image.Image:
    """The picture with a 256-colour palette: the screen has at most that many, so nothing is
    lost, and the file is much smaller."""
    return picture.convert("RGB").convert("P", palette=Image.Palette.ADAPTIVE, colors=256)


def save_baseline(picture: Image.Image, target: Path) -> None:
    """Saves a picture as a baseline."""
    target.parent.mkdir(parents=True, exist_ok=True)
    palette_png(picture).save(target, optimize=True)


def baseline_files(root: Path = BASELINES) -> set[tuple[str, str]]:
    """The (case, picture) of every baseline on disk."""
    if not root.is_dir():
        return set()
    return {(p.parent.name, p.stem) for p in root.glob("*/*.png")}


def expected_baselines(cases: dict[str, Case]) -> set[tuple[str, str]]:
    """The (case, picture) of every baseline the cases need."""
    return {(name, picture) for name, case in cases.items() for picture in case.pictures}


# --- Playing the cases


@dataclass
class Picture:
    """One picture's comparison."""

    name: str
    actual: Image.Image
    expected: Image.Image | None  # None: there is no baseline
    fraction: float  # of the pixels that differ (1 when there is no baseline or the size differs)
    note: str = ""  # why it fails, if it does

    @property
    def failed(self) -> bool:
        return bool(self.note)


@dataclass
class Outcome:
    """How one case went."""

    name: str
    case: Case
    pictures: list[Picture] = field(default_factory=list)
    problem: str = ""  # the game crashed, an assert failed or it didn't finish
    output: str = ""  # the end of the game's output
    attempts: int = 1
    seconds: float = 0.0

    @property
    def failed(self) -> bool:
        return bool(self.problem) or any(p.failed for p in self.pictures)

    @property
    def worst(self) -> float:
        return max((p.fraction for p in self.pictures), default=0.0)


class Settings(NamedTuple):
    """How to run the cases."""

    setup: shots.Setup
    baselines: Path
    tolerance: float | None  # for every case (default: each case's own)
    retries: int


class Play(NamedTuple):
    """What playing a case gave."""

    pictures: dict[str, Image.Image]
    problem: str
    output: str


_NOISE = ("miniwin: GetProcAddress", "mCreateFile", "miniwin: CreateFile")


def _text(captured: str | bytes | None) -> str:
    """What a timed-out process had printed so far (the exception holds bytes or text)."""
    if captured is None:
        return ""
    return captured if isinstance(captured, str) else captured.decode(errors="replace")


def play(name: str, case: Case, setup: shots.Setup) -> Play:
    """Runs one case on a fresh C: drive; its pictures, or what went wrong."""
    with tempfile.TemporaryDirectory(prefix=f"gameplay-{name}-") as work:
        c = port.lay_out_drives(c=Path(work) / "c")
        recipe = shots.Recipe(commands=case.script, seconds=case.seconds)
        limit = case.seconds + 60
        try:
            run = subprocess.run(
                shots.game_command(setup, c, recipe, Path(work) / "screen.bmp"),
                capture_output=True,
                text=True,
                timeout=limit,
                check=False,
            )
            printed = run.stdout + run.stderr
        except subprocess.TimeoutExpired as hung:
            # (The game quits itself after `seconds`; one still there a minute later is stuck where
            # it never returns to its frame, such as a modal box nobody can answer.)
            lines = (_text(hung.stdout) + _text(hung.stderr)).splitlines()
            output = "\n".join(line for line in lines if not line.startswith(_NOISE))[-6000:]
            return Play({}, f"the game hung (still running {limit:g} s after it started)", output)
        lines = [line for line in printed.splitlines() if not line.startswith(_NOISE)]
        output = "\n".join(lines[-60:])
        errors = [
            line
            for line in lines
            if "RuntimeError" in line or "[zbdebug] error" in line or "assertion failed" in line
        ]
        if errors:
            return Play({}, "; ".join(errors[:2]), output)
        if not any("[zbdebug] quitting" in line for line in lines):
            problem = f"not finished after {case.seconds:g} s (a wait that never ends?)"
            return Play({}, problem, output)
        pictures: dict[str, Image.Image] = {}
        for picture in case.pictures:
            taken = Path(work) / f"{picture}.bmp"
            if not taken.exists():
                return Play({}, f"no picture {picture}", output)
            with Image.open(taken) as image:
                pictures[picture] = image.convert("RGB")
        return Play(pictures, "", output)


def compare(
    name: str, played: dict[str, Image.Image], baselines: Path, limit: float
) -> list[Picture]:
    """Each picture of a case against its baseline."""
    found: list[Picture] = []
    for picture, actual in played.items():
        path = baselines / name / f"{picture}.png"
        if not path.exists():
            note = f"no baseline (run `uv run gameplay --rebaseline {name}`)"
            found.append(Picture(picture, actual, None, 1.0, note))
            continue
        with Image.open(path) as image:
            expected = image.convert("RGB")
        diff = difference(expected, actual)
        note = ""
        if diff.size_differs:
            wanted = f"{expected.width}x{expected.height}"
            note = f"the picture is {actual.width}x{actual.height}, not {wanted}"
        elif diff.fraction > limit:
            note = f"{diff.fraction:.2%} of the pixels differ (more than {limit:.2%})"
        found.append(Picture(picture, actual, expected, diff.fraction, note))
    return found


def run_case(
    name: str,
    case: Case,
    settings: Settings,
    on_retry: Callable[[str], None] = lambda _: None,
) -> Outcome:
    """Plays a case, again if its pictures differ, and compares them with the baselines."""
    started = time.monotonic()
    limit = case.tolerance if settings.tolerance is None else settings.tolerance
    outcome = Outcome(name, case)
    for attempt in range(settings.retries + 1):
        played = play(name, case, settings.setup)
        outcome.attempts = attempt + 1
        outcome.problem, outcome.output = played.problem, played.output
        outcome.pictures = compare(name, played.pictures, settings.baselines, limit)
        if outcome.problem or not outcome.failed:
            break
        if attempt < settings.retries:
            on_retry(name)
    outcome.seconds = time.monotonic() - started
    return outcome


def write_failures(outcome: Outcome, out: Path) -> None:
    """Writes a case's failing pictures to `out`/<case>/: the new one, and baseline | new | diff."""
    for picture in outcome.pictures:
        if not picture.failed:
            continue
        target = out / outcome.name
        target.mkdir(parents=True, exist_ok=True)
        picture.actual.save(target / f"{picture.name}.actual.png")
        if picture.expected is not None:
            side_by_side(picture.expected, picture.actual).save(target / f"{picture.name}.diff.png")


def rebaseline(outcome: Outcome, baselines: Path) -> tuple[int, int]:
    """Writes the pictures that changed in any way: (written, unchanged)."""
    written = unchanged = 0
    for picture in outcome.pictures:
        if (
            picture.expected is not None
            and not changed_mask(picture.expected, picture.actual).getbbox()
        ):
            unchanged += 1
            continue
        save_baseline(picture.actual, baselines / outcome.name / f"{picture.name}.png")
        written += 1
    return written, unchanged


# --- The report


def data_uri(picture: Image.Image) -> str:
    """The picture as a PNG data URI, to embed in the report."""
    buffer = io.BytesIO()
    palette_png(picture).save(buffer, format="PNG", optimize=True)
    return "data:image/png;base64," + base64.b64encode(buffer.getvalue()).decode("ascii")


@dataclass
class Shown:
    """A failing picture as the report shows it: the pictures, embedded."""

    baseline: str = ""
    actual: str = ""
    marked: str = ""  # the differences highlighted
    omitted: bool = False  # not embedded, the report being full


def shown_pictures(outcomes: list[Outcome], budget: int) -> dict[tuple[str, str], Shown]:
    """Embeds the failing pictures, until the report would pass `budget`."""
    shown: dict[tuple[str, str], Shown] = {}
    used = 0
    for outcome in outcomes:
        for picture in outcome.pictures:
            if not picture.failed:
                continue
            if picture.expected is None:
                encoded = [data_uri(picture.actual)]
            else:
                marked = marked_picture(picture.expected, picture.actual)
                encoded = [data_uri(x) for x in (picture.expected, picture.actual, marked)]
            size = sum(len(e) for e in encoded)
            item = Shown(omitted=used + size > budget)
            if not item.omitted:
                used += size
                if len(encoded) == 1:
                    item.actual = encoded[0]
                else:
                    item.baseline, item.actual, item.marked = encoded
            shown[(outcome.name, picture.name)] = item
    return shown


def render_html(outcomes: list[Outcome], mode: str, budget: int = REPORT_BUDGET) -> str:
    """The self-contained report: a summary, every case, and the failing pictures."""
    environment = jinja2.Environment(
        loader=jinja2.PackageLoader("zbtools", "templates"),
        autoescape=True,
        undefined=jinja2.StrictUndefined,
    )
    failed = [o for o in outcomes if o.failed]
    passed = [o for o in outcomes if not o.failed]
    return environment.get_template("gameplay.html.j2").render(
        mode=mode,
        failed=failed,
        passed=passed,
        flaky=[o for o in passed if o.attempts > 1],
        total=len(outcomes),
        pictures=sum(len(o.pictures) for o in outcomes),
        shown=shown_pictures(failed, budget),
        seconds=sum(o.seconds for o in outcomes),
        generated=time.strftime("%Y-%m-%d %H:%M"),
    )


def run_url() -> str:
    """The page of the workflow run this is in, if it is in one (GitHub sets these variables)."""
    server, repository, run = (os.environ.get(k) for k in _RUN_VARIABLES)
    return f"{server}/{repository}/actions/runs/{run}" if server and repository and run else ""


def render_summary(outcomes: list[Outcome], mode: str, stale: list[str]) -> str:
    """The report as Markdown, for the job summary GitHub shows on a workflow run's page."""
    failed = [o for o in outcomes if o.failed]
    flaky = [o for o in outcomes if not o.failed and o.attempts > 1]
    if mode == "rebaseline":
        lines = [f"## Gameplay tests: baselines written for {len(outcomes)} cases", ""]
    else:
        passed = len(outcomes) - len(failed)
        lines = [f"## Gameplay tests: {passed} of {len(outcomes)} cases passed"]
        lines.append("")
    if failed or stale:
        lines += ["| Case | Result |", "| --- | --- |"]
        for outcome in failed:
            what = outcome.problem or "; ".join(
                f"`{p.name}`: {p.note}" for p in outcome.pictures if p.failed
            )
            lines.append(f"| `{outcome.name}` | {what.replace('|', '/')} |")
        lines += [
            f"| `{item}` | a baseline no case takes (`--rebaseline` removes it) |" for item in stale
        ]
        where = (
            f"[the `gameplay-report-N.html` artifacts]({run_url()}#artifacts)"
            if run_url()
            else "`report.html`"
        )
        lines += [
            "",
            "The baseline, the new picture and the differences of each failing picture are in"
            f" {where} (one file, with the pictures inside it: download it from the run's artifacts"
            " and open it in a browser).",
            "If the change is intended, run `uv run gameplay --rebaseline` and commit"
            " `tests/gameplay/baselines/`.",
        ]
    if flaky:
        lines += ["", "Passed only on a second try: " + ", ".join(f"`{o.name}`" for o in flaky)]
    return "\n".join(lines) + "\n"


# --- The command


def prepare(output: Path) -> shots.Setup:
    """Checks the headless build is there and packs the game's data; what the cases run on."""
    target = port.TARGETS["headless_wasm"]
    program = port.build_dir(target) / target.program
    nodes = sorted((paths.EMSDK_DIR / "node").glob("*/bin/node"))
    if not program.exists() or not nodes:
        print("build the headless port first: uv run port build headless_wasm")
        raise typer.Exit(2)
    for line in assets.pack_all(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    for line in port.pack_scenes(paths.ASSETS_DIR, paths.PACKED_ASSETS_DIR):
        print(line)
    shutil.rmtree(output, ignore_errors=True)
    output.mkdir(parents=True)
    return shots.Setup(nodes[-1], program, paths.PACKED_ASSETS_DIR, output)


class Progress:
    """What the run prints as it goes, so that a long run (CI's) can be watched: each case as it
    starts and as it finishes (with the count and the time so far), and a line every
    `HEARTBEAT` seconds naming the cases still running when nothing else was said."""

    def __init__(self, total: int) -> None:
        self.total = total
        self.finished = 0
        self.began = time.monotonic()
        self.running: dict[str, float] = {}
        self.lock = threading.Lock()

    def clock(self) -> str:
        seconds = int(time.monotonic() - self.began)
        return f"{seconds // 60:02d}:{seconds % 60:02d}"

    def started(self, name: str) -> None:
        with self.lock:
            self.running[name] = time.monotonic()
            print(f"        start   {name}  [{self.clock()}]")

    def retrying(self, name: str) -> None:
        with self.lock:
            print(
                f"        retry   {name}: its pictures differed, playing it again  [{self.clock()}]"
            )

    def finish(self, outcome: Outcome, rebaselined: tuple[int, int] | None) -> None:
        with self.lock:
            self.running.pop(outcome.name, None)
            self.finished += 1
            count = f"[{self.finished}/{self.total}  {self.clock()}]"
            say(outcome, rebaselined, count)

    def heartbeat(self) -> None:
        with self.lock:
            now = time.monotonic()
            names = ", ".join(f"{n} ({int(now - t)}s)" for n, t in self.running.items())
            done = f"{self.finished}/{self.total} done"
            print(f"        ...     {done}, running: {names}  [{self.clock()}]")


# Seconds of silence after which the run says which cases it is waiting for.
HEARTBEAT = 60


def say(outcome: Outcome, rebaselined: tuple[int, int] | None, count: str = "") -> None:
    """Prints how a case went (`count`: where the run is, as the first thing on the line)."""
    seconds = f" ({outcome.seconds:.0f} s)"
    prefix = f"{count} " if count else ""
    if outcome.problem:
        print(f"{prefix}FAILED  {outcome.name}: {outcome.problem}{seconds}")
    elif rebaselined is not None:
        done = f"{rebaselined[0]} written, {rebaselined[1]} unchanged"
        print(f"{prefix}ok      {outcome.name}: {done}{seconds}")
    elif outcome.failed:
        print(f"{prefix}FAILED  {outcome.name}{seconds}")
        for picture in outcome.pictures:
            if picture.failed:
                print(f"          {picture.name}: {picture.note}")
    else:
        tries = f" (needed {outcome.attempts} tries)" if outcome.attempts > 1 else ""
        print(f"{prefix}ok      {outcome.name}{tries}{seconds}")


def pick_shard(names: list[str], shard: str | None) -> list[str]:
    """The cases of shard `N/M` (1 to M): every Mth from the Nth, so that the long cases, which
    come in runs (a puzzle's four levels), are spread over the shards. All of them without one."""
    if shard is None:
        return names
    index, _, count = shard.partition("/")
    if not (index.isdigit() and count.isdigit() and 1 <= int(index) <= int(count)):
        raise typer.BadParameter(f"--shard is N/M with N from 1 to M, not {shard!r}")
    return names[int(index) - 1 :: int(count)]


def remove_stale(stale: list[tuple[str, str]]) -> None:
    """Deletes baselines no case takes, and the directories that leaves empty."""
    for case_name, picture_name in stale:
        (BASELINES / case_name / f"{picture_name}.png").unlink()
        print(f"removed {case_name}/{picture_name}.png (no case takes it)")
    for directory in BASELINES.glob("*"):
        if directory.is_dir() and not any(directory.iterdir()):
            directory.rmdir()


def play_all(  # noqa: PLR0917 (what a run is made of)
    chosen: list[str],
    cases: dict[str, Case],
    settings: Settings,
    jobs: int,
    output: Path,
    writing: bool,
) -> tuple[list[Outcome], int]:
    """Plays the cases `jobs` at a time, saying how each goes as it finishes, and writes the
    baselines if `writing`: the outcomes in the cases' order, and how many pictures were written."""
    outcomes: list[Outcome] = []
    written = 0
    progress = Progress(len(chosen))

    def play_case(name: str) -> Outcome:
        progress.started(name)
        return run_case(name, cases[name], settings, progress.retrying)

    with ThreadPoolExecutor(jobs) as pool:
        waiting: set[Future[Outcome]] = {pool.submit(play_case, n) for n in chosen}
        while waiting:
            done, waiting = wait(waiting, timeout=HEARTBEAT, return_when=FIRST_COMPLETED)
            if not done:
                progress.heartbeat()
            for future in done:
                outcome = future.result()
                outcomes.append(outcome)
                rebaselined = (
                    None if outcome.problem or not writing else rebaseline(outcome, BASELINES)
                )
                written += rebaselined[0] if rebaselined else 0
                progress.finish(outcome, rebaselined)
                write_failures(outcome, output)
    outcomes.sort(key=lambda o: chosen.index(o.name))  # (the report keeps the cases' order)
    return outcomes, written


@app.command()
def main(  # noqa: PLR0917 (a CLI's options)
    names: Annotated[list[str] | None, typer.Argument(help="Which cases (default: all)")] = None,
    rebaseline_pictures: Annotated[
        bool,
        typer.Option(
            "--rebaseline", help="Write the pictures as the new baselines instead of comparing"
        ),
    ] = False,
    jobs: Annotated[int, typer.Option(help="How many cases to play at once")] = 4,
    retries: Annotated[
        int, typer.Option(help="How many times to play a case again if its pictures differ")
    ] = 1,
    tolerance: Annotated[
        float | None,
        typer.Option(help="The fraction of pixels allowed to differ (default: each case's)"),
    ] = None,
    output: Annotated[
        Path, typer.Option(help="Where the report and the failing pictures are written")
    ] = paths.GAMEPLAY_DIR,
    shard: Annotated[
        str | None,
        typer.Option(
            help="Play only shard N of M (N/M: every Mth case from the Nth), to split a run"
        ),
    ] = None,
    summary: Annotated[
        Path | None,
        typer.Option(
            help="Also append the summary as Markdown to this file (GitHub's job summary)"
        ),
    ] = None,
) -> None:
    """Play every case and compare its pictures with the baselines (or write them)."""
    if isinstance(sys.stdout, io.TextIOWrapper):
        sys.stdout.reconfigure(line_buffering=True)  # (a pipe, as in CI, would hold it back)
    cases = load_cases()
    wrong = problems_with(cases)
    if wrong:
        print("\n".join(wrong))
        raise typer.Exit(2)
    chosen = names or list(cases)
    unknown = [n for n in chosen if n not in cases]
    if unknown:
        raise typer.BadParameter(f"no case: {', '.join(unknown)}")
    chosen = pick_shard(chosen, shard)
    # (when writing baselines any difference at all is a change to write, and nothing is retried)
    settings = Settings(
        prepare(output),
        BASELINES,
        0.0 if rebaseline_pictures else tolerance,
        0 if rebaseline_pictures else retries,
    )
    outcomes, written = play_all(chosen, cases, settings, jobs, output, rebaseline_pictures)
    stale = (
        sorted(baseline_files() - expected_baselines(cases))
        if names is None and shard is None
        else []
    )
    stale_names = [f"{c}/{p}.png" for c, p in stale]
    mode = "rebaseline" if rebaseline_pictures else "compare"
    if rebaseline_pictures:
        remove_stale(stale)
        stale_names = []
    for item in stale_names:
        print(f"STALE   {item}: no case takes it (--rebaseline removes it)")
    # (when writing baselines a picture that changed is no failure, only a crashed case is)
    failed = sum(bool(o.problem) if rebaseline_pictures else o.failed for o in outcomes)
    failed += len(stale_names)
    (output / "report.html").write_text(render_html(outcomes, mode))
    if summary:
        with summary.open("a") as file:
            file.write(render_summary(outcomes, mode, stale_names))
    if rebaseline_pictures:
        print(f"{written} baselines written")
    else:
        flaky = sum(1 for o in outcomes if not o.failed and o.attempts > 1)
        extra = f", {flaky} needed a second try" if flaky else ""
        passed = len(outcomes) - sum(o.failed for o in outcomes)
        print(f"{passed} of {len(outcomes)} cases passed{extra}")
    print(f"the report is {(output / 'report.html').relative_to(paths.REPO_ROOT)}")
    raise typer.Exit(1 if failed else 0)
