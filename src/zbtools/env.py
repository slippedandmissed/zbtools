"""Read settings from the environment, falling back to the repo's .env file."""

import os
import sys

from zbtools import paths


def _read_dotenv() -> dict[str, str]:
    values: dict[str, str] = {}
    if paths.ENV_FILE.is_file():
        for raw in paths.ENV_FILE.read_text().splitlines():
            line = raw.strip()
            if line and not line.startswith("#") and "=" in line:
                key, value = line.split("=", 1)
                values[key.strip()] = value.strip().strip("\"'")
    return values


def get(key: str) -> str:
    """Return the setting, or exit explaining where to set it."""
    value = os.environ.get(key) or _read_dotenv().get(key)
    if not value:
        sys.exit(f"error: {key} is not set. Add it to {paths.ENV_FILE} (see README: Setup).")
    return value
