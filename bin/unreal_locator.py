#!/usr/bin/env python

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import annotations
import json
import os
import sys
from pathlib import Path
from typing import Iterable

class UnrealError(Exception):
    """Base exception for Unreal helpers."""

class UnrealHomeNotFound(UnrealError):
    pass

class UnrealVersionNotFound(UnrealError):
    pass

class UnrealCommandNotFound(UnrealError):
    pass


def _read_unrealengine_home_file(project_root: Path) -> Path | None:
    """Read .unrealengine-home file if present."""
    home_file = project_root / ".unrealengine-home"
    if home_file.exists():
        home_path = Path(home_file.read_text().strip())
        if not home_path.exists():
            raise UnrealHomeNotFound(f".unrealengine-home points to non-existent path: {home_path}")
        return home_path
    return None


def _engine_version_from_uproject(uproject_path: Path) -> str:
    """Extract EngineAssociation string from .uproject JSON."""
    data = json.loads(uproject_path.read_text(encoding="utf-8"))
    version = data.get("EngineAssociation")
    if not version:
        raise UnrealVersionNotFound(f"No EngineAssociation found in {uproject_path}")
    # normalize
    # remove UE_ prefix
    v = str(version).strip()
    if v.startswith("UE_"):
        v = v[3:]
    # strip patch numbers beyond x.y
    parts = v.split(".")
    if len(parts) >= 2:
        v = f"{parts[0]}.{parts[1]}"
    return f"UE_{v}"


def _default_homes(version_tag: str) -> list[Path]:
    """Return OS-specific default home locations to try."""
    homes: list[Path] = []
    if sys.platform.startswith("win"):
        homes.append(Path(r"C:\Program Files\Epic Games") / version_tag)
    elif sys.platform == "darwin":  # macOS
        homes.append(Path("/Users/Shared/Epic Games") / version_tag)
        homes.append(Path("/Applications/Epic Games") / version_tag)
    else:
        # unsupported
        pass
    return homes


def find_unreal_home(uproject_path: Path) -> Path:
    """
    Determine Unreal Engine home directory for a given project.
    Checks .unrealengine-home in project root,
    then UNREAL_HOME env var,
    then EngineAssociation in the .uproject file.
    """
    project_root = uproject_path.parent

    # 1. .unrealengine-home
    home = _read_unrealengine_home_file(project_root)
    if home:
        return home

    # 2. env var
    env_home = os.environ.get("UNREAL_HOME")
    if env_home:
        env_path = Path(env_home)
        if not env_path.exists():
            raise UnrealHomeNotFound(
                f"UNREAL_HOME environment variable points to non-existent path: {env_home}"
            )
        return env_path

    # 3. derive from version
    version_tag = _engine_version_from_uproject(uproject_path)
    for candidate in _default_homes(version_tag):
        if candidate.exists():
            return candidate

    raise UnrealHomeNotFound(
        f"Could not locate Unreal Engine home for version {version_tag}.\n"
        f"Tried: {[str(c) for c in _default_homes(version_tag)]}"
    )


def get_unreal_commands(home: Path, commands: Iterable[str]) -> dict[str, Path]:
    """
    Given a home directory and command names, return a map of command -> binary path.
    Raises UnrealCommandNotFound if any command missing.
    """
    cmd_map: dict[str, Path] = {}
    is_windows = sys.platform.startswith("win")

    for cmd in commands:
        if cmd == "UnrealEditor":
            if is_windows:
                path = home / "Engine" / "Binaries" / "Win64" / "UnrealEditor.exe"
            else:  # macOS
                path = (home / "Engine" / "Binaries" / "Mac" / "UnrealEditor.app" / "Contents" / "MacOS" / "UnrealEditor")
        elif cmd == "UnrealEditor-Cmd":
            if is_windows:
                path = home / "Engine" / "Binaries" / "Win64" / "UnrealEditor-Cmd.exe"
            else:
                # no real separate binary; mimic CLI version of UnrealEditor
                path = (home / "Engine" / "Binaries" / "Mac" / "UnrealEditor.app" / "Contents" / "MacOS" / "UnrealEditor")
        else:
            raise UnrealCommandNotFound(f"Unknown command name requested: {cmd}")

        if not path.exists():
            raise UnrealCommandNotFound(f"Command {cmd} not found at {path}")
        cmd_map[cmd] = path

    return cmd_map
