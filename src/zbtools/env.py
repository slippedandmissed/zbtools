"""Read settings from the environment, falling back to the repo's .env file."""

import os
import sys

from dotenv import dotenv_values

from zbtools import paths


def get(key: str) -> str:
    """Return the setting, or exit explaining where to set it."""
    value = os.environ.get(key) or dotenv_values(paths.ENV_FILE).get(key)
    if not value:
        sys.exit(f"error: {key} is not set. Add it to {paths.ENV_FILE} (see README: Setup).")
    return value
