"""The instrumented gameplay tests' own logic (zbtools.gameplay) and their checked-in files.

Playing the cases needs the headless build, so `uv run gameplay` is run separately (and in the pull
request pipeline); what is checked here needs nothing: the comparison, the report, and that the
cases and the baselines in tests/gameplay/ agree.
"""

from pathlib import Path

import pytest
from PIL import Image, ImageDraw

from zbtools import gameplay

SCREEN = (640, 480)


def picture(colour: tuple[int, int, int] = (40, 90, 60)) -> Image.Image:
    return Image.new("RGB", SCREEN, colour)


def case(commands: str = "scene 1; screenshot map") -> gameplay.Case:
    return gameplay.Case(description="a case", commands=commands)


# The cases and baselines in the repository


def test_the_cases_are_well_formed() -> None:
    assert gameplay.problems_with(gameplay.load_cases()) == []


def test_every_picture_a_case_takes_has_a_baseline_and_no_baseline_is_left_over() -> None:
    cases = gameplay.load_cases()
    on_disk = gameplay.baseline_files()
    wanted = gameplay.expected_baselines(cases)
    assert sorted(wanted - on_disk) == [], "run `uv run gameplay --rebaseline`"
    assert sorted(on_disk - wanted) == [], "run `uv run gameplay --rebaseline` (it removes them)"


def test_baselines_are_whole_screens() -> None:
    for case_name, name in sorted(gameplay.baseline_files()):
        with Image.open(gameplay.BASELINES / case_name / f"{name}.png") as image:
            assert image.size == SCREEN, f"{case_name}/{name}"


# Cases


def test_a_case_runs_with_sticky_mouse_off_and_a_seed_and_ends_itself() -> None:
    script = gameplay.Case(description="x", commands="scene 4", seed=7).script
    assert script == "set clickToDragOption 0; set dragClicks 0; seed 7; scene 4; quit"


def test_a_case_may_not_end_the_run_itself() -> None:
    with pytest.raises(ValueError, match="quit"):
        gameplay.Case(description="x", commands="scene 1; quit")


def test_the_pictures_a_case_takes_are_its_screenshot_commands() -> None:
    commands = "scene 1; screenshot first; wait 100; screenshot second; get scene"
    assert case(commands).pictures == ["first", "second"]
    assert gameplay.screenshot_names("click 1 2; screenshot") == []  # (no name: not a command)


def test_problems_with_the_cases_are_reported() -> None:
    cases = {
        "Bad Name": case(),
        "silent": case("scene 1"),
        "asserting": case("scene 1; assert scene 1"),
        "twice": case("screenshot a; screenshot a"),
        "bad-picture": case("screenshot Not_ok"),
    }
    found = "\n".join(gameplay.problems_with(cases))
    assert "Bad Name" in found
    assert "silent: checks nothing" in found
    assert "asserting" not in found  # (a case may check the state alone)
    assert "twice: takes a more than once" in found
    assert "Not_ok" in found


def test_unknown_fields_are_rejected() -> None:
    with pytest.raises(ValueError, match="extra"):
        gameplay.Case.model_validate({"description": "x", "commands": "screenshot a", "colour": 1})


# Comparing pictures


def test_identical_pictures_do_not_differ() -> None:
    found = gameplay.difference(picture(), picture())
    assert found.fraction == 0
    assert not found.size_differs


def test_a_small_change_is_a_small_fraction() -> None:
    changed = picture()
    ImageDraw.Draw(changed).rectangle((0, 0, 63, 47), fill=(250, 250, 250))  # 1% of the screen
    assert gameplay.difference(picture(), changed).fraction == pytest.approx(0.01, abs=0.0005)


def test_a_change_below_the_channel_threshold_is_ignored() -> None:
    nearly = picture((40 + gameplay.CHANNEL_DELTA, 90, 60))
    assert gameplay.difference(picture(), nearly).fraction == 0
    assert (
        gameplay.difference(picture(), picture((41 + gameplay.CHANNEL_DELTA, 90, 60))).fraction == 1
    )


def test_pictures_of_different_sizes_differ_entirely() -> None:
    found = gameplay.difference(picture(), Image.new("RGB", (320, 240)))
    assert found.size_differs
    assert found.fraction == 1


def test_the_marked_picture_is_red_where_it_differs() -> None:
    changed = picture()
    ImageDraw.Draw(changed).rectangle((10, 10, 19, 19), fill=(250, 250, 250))
    marked = gameplay.marked_picture(picture(), changed)
    assert marked.getpixel((15, 15)) == (255, 0, 0)
    assert marked.getpixel((300, 300)) != (255, 0, 0)


def test_a_comparison_fails_a_picture_over_the_tolerance(tmp_path: Path) -> None:
    gameplay.save_baseline(picture(), tmp_path / "a-case" / "map.png")
    changed = picture()
    ImageDraw.Draw(changed).rectangle((0, 0, 127, 95), fill=(250, 250, 250))  # 4%
    pictures = gameplay.compare("a-case", {"map": changed}, tmp_path, 0.05)
    assert not pictures[0].failed
    pictures = gameplay.compare("a-case", {"map": changed}, tmp_path, 0.01)
    assert pictures[0].failed
    assert "4.00%" in pictures[0].note


def test_a_missing_baseline_fails_and_says_how_to_make_it(tmp_path: Path) -> None:
    (found,) = gameplay.compare("a-case", {"map": picture()}, tmp_path, 0.01)
    assert found.failed
    assert found.expected is None
    assert "--rebaseline a-case" in found.note


def test_rebaselining_writes_only_what_changed(tmp_path: Path) -> None:
    outcome = gameplay.Outcome("a-case", case())
    gameplay.save_baseline(picture(), tmp_path / "a-case" / "same.png")
    outcome.pictures = [
        gameplay.compare("a-case", {"same": picture()}, tmp_path, 0.0)[0],
        gameplay.compare("a-case", {"new": picture((200, 10, 10))}, tmp_path, 0.0)[0],
    ]
    before = (tmp_path / "a-case" / "same.png").read_bytes()
    assert gameplay.rebaseline(outcome, tmp_path) == (1, 1)
    assert (tmp_path / "a-case" / "same.png").read_bytes() == before
    assert (tmp_path / "a-case" / "new.png").exists()


# The reports


def failing_outcome(tmp_path: Path) -> gameplay.Outcome:
    gameplay.save_baseline(picture(), tmp_path / "broken" / "map.png")
    changed = picture()
    ImageDraw.Draw(changed).rectangle((0, 0, 200, 200), fill=(250, 250, 250))
    outcome = gameplay.Outcome("broken", case("scene 1; screenshot map"), attempts=2)
    outcome.pictures = gameplay.compare("broken", {"map": changed}, tmp_path, 0.005)
    return outcome


def test_the_html_report_embeds_the_baseline_the_new_picture_and_the_differences(
    tmp_path: Path,
) -> None:
    ok = gameplay.Outcome("fine", case())
    html = gameplay.render_html([failing_outcome(tmp_path), ok], "compare")
    assert html.count("data:image/png;base64,") == 3
    assert "broken" in html
    assert "fine" in html
    assert "<script" not in html  # one file, nothing to fetch


def test_the_html_report_shows_what_a_crashed_case_printed() -> None:
    crashed = gameplay.Outcome("crashed", case(), problem="a trap", output="trap in openNet")
    html = gameplay.render_html([crashed], "compare")
    assert "a trap" in html
    assert "trap in openNet" in html


def test_the_html_report_leaves_pictures_out_when_it_would_be_too_big(tmp_path: Path) -> None:
    html = gameplay.render_html([failing_outcome(tmp_path)], "compare", budget=10)
    assert "data:image/png" not in html
    assert "Not embedded" in html


def test_the_html_report_escapes_what_cases_say() -> None:
    crashed = gameplay.Outcome("x", case(), problem="<b>bold</b>", output="")
    assert "<b>bold</b>" not in gameplay.render_html([crashed], "compare")


def test_the_markdown_summary_lists_failures_and_says_where_the_pictures_are(
    tmp_path: Path,
) -> None:
    ok = gameplay.Outcome("fine", case(), attempts=2)
    text = gameplay.render_summary([failing_outcome(tmp_path), ok], "compare", ["gone/old.png"])
    assert "0 of 2 cases passed" not in text  # `fine` passed
    assert "1 of 2 cases passed" in text
    assert "`broken`" in text
    assert "`gone/old.png`" in text
    assert "report.html" in text
    assert "Passed only on a second try: `fine`" in text


def test_failing_pictures_are_written_beside_the_report(tmp_path: Path) -> None:
    baselines = tmp_path / "baselines"
    outcome = failing_outcome(baselines)
    gameplay.write_failures(outcome, tmp_path / "out")
    assert (tmp_path / "out" / "broken" / "map.actual.png").exists()
    with Image.open(tmp_path / "out" / "broken" / "map.diff.png") as image:
        assert image.size == (SCREEN[0] * 3 + 8, SCREEN[1])


def test_the_summary_links_to_the_workflow_run_when_there_is_one(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setenv("GITHUB_SERVER_URL", "https://github.com")
    monkeypatch.setenv("GITHUB_REPOSITORY", "owner/repo")
    monkeypatch.setenv("GITHUB_RUN_ID", "42")
    text = gameplay.render_summary([failing_outcome(tmp_path)], "compare", [])
    assert "(https://github.com/owner/repo/actions/runs/42#artifacts)" in text
    assert "gameplay-report.html" in text


def test_progress_says_what_starts_finishes_and_is_still_running(
    capsys: pytest.CaptureFixture[str],
) -> None:
    progress = gameplay.Progress(2)
    progress.started("slow")
    progress.started("quick")
    progress.heartbeat()
    quick = gameplay.Outcome("quick", case())
    progress.finish(quick, None)
    out = capsys.readouterr().out
    assert "start   slow" in out
    assert "0/2 done, running: slow (0s), quick (0s)" in out
    assert "[1/2  00:00] ok      quick" in out
    progress.heartbeat()
    assert "1/2 done, running: slow" in capsys.readouterr().out
