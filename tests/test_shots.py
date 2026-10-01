"""The handbook's screenshot recipes (docs/screenshots.toml) agree with its placeholders."""

from pathlib import Path

import pytest

from zbtools import book, shots

SCREEN = (640, 480)


def test_every_recipe_is_for_a_placeholder() -> None:
    placeholders = {item.id for item in book.placeholders()}
    recipes = shots.load_recipes()
    named = set(recipes) | {extra for recipe in recipes.values() for extra in recipe.also}
    unknown = sorted(named - placeholders)
    assert not unknown, f"recipes for ids that no chapter has: {unknown}"


def test_every_extra_picture_is_taken_by_its_recipe() -> None:
    for id_, recipe in shots.load_recipes().items():
        commands = [c.split() for c in recipe.commands.split(";")]
        taken = {c[1] for c in commands if len(c) == 2 and c[0] == "screenshot"}
        assert set(recipe.also) <= taken, f"{id_}: `screenshot` for {set(recipe.also) - taken}"
        assert taken <= {id_, *recipe.also}, f"{id_}: pictures taken for no id: {taken}"


def test_recipes_end_the_run_and_stay_on_screen() -> None:
    for id_, recipe in shots.load_recipes().items():
        commands = [c.strip() for c in recipe.commands.split(";")]
        assert commands[-1] == "quit", f"{id_}: the commands must end with quit"
        if recipe.crop:
            left, top, right, bottom = recipe.crop
            assert 0 <= left < right <= SCREEN[0], id_
            assert 0 <= top < bottom <= SCREEN[1], id_


def test_a_recipe_runs_the_game_with_its_commands(tmp_path: Path) -> None:
    recipe = shots.Recipe(commands="scene 1; quit", seconds=12.5)
    setup = shots.Setup(Path("node"), Path("zoombinis.js"), Path("d"), Path("images"))
    command = shots.game_command(setup, Path("c"), recipe, tmp_path / "s.bmp")
    assert command[:2] == ["node", "zoombinis.js"]
    assert command[command.index("--cmd") + 1] == "scene 1; quit"
    assert command[command.index("--run-for") + 1] == "12500"


def test_unknown_recipe_fields_are_rejected() -> None:
    with pytest.raises(ValueError, match="extra"):
        shots.Recipe.model_validate({"commands": "quit", "secs": 3})
