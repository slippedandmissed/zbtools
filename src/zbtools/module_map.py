"""The map of the game's source modules, decomp/modules.toml (see `uv run
modules`, which checks it against the executable)."""

import bisect
import tomllib

import typer
from pydantic import BaseModel, ConfigDict

from zbtools import paths

MODULES = paths.DECOMP_DIR / "modules.toml"


class Module(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    start: int
    name: str
    note: str = ""


class ModuleMap(BaseModel):
    model_config = ConfigDict(frozen=True, extra="forbid")
    module: list[Module]

    def of(self, address: int) -> Module | None:
        """The module an address in the game's code belongs to."""
        i = bisect.bisect_right([m.start for m in self.module], address) - 1
        return self.module[i] if i >= 0 else None


def load() -> ModuleMap:
    modules = ModuleMap.model_validate(tomllib.loads(MODULES.read_text()))
    starts = [m.start for m in modules.module]
    names = [m.name for m in modules.module]
    if starts != sorted(set(starts)) or len(names) != len(set(names)):
        raise typer.BadParameter(f"{MODULES}: starts must increase and names be unique")
    return modules
