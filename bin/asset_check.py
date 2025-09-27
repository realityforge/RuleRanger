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

import subprocess
import argparse
import json
import os
import glob
import sys
import tempfile
from unreal_locator import find_unreal_home, get_unreal_commands
from pathlib import Path

parser = argparse.ArgumentParser(description="Rule Ranger Asset Checker")

parser.add_argument("--verbose", action="store_true", help="Increase output verbosity")
parser.add_argument(
    "--staged-only", action="store_true", help="Only analyze staged assets"
)
parser.add_argument("--report", type=str, help="Path to the JSON report")
parser.add_argument("--asset-path", type=str, help="Additional asset paths to analyze")
parser.add_argument("files", type=str, nargs="*", help="The files to analyze")

args = parser.parse_args()

if args.verbose:
    print(f"Performing Rule Ranger Asset Checking")

if args.staged_only and 0 != len(args.asset_path):
    print(f"--staged-only is not compatible with --asset-path", file=sys.stderr)
    sys.exit(1)

exitcode = 0

try:
    # Find the .uproject file (must be exactly one)
    uproject_matches = glob.glob("*.uproject")
    if 1 != len(uproject_matches):
        print(
            f"Error: expected exactly 1 .uproject file, found {len(uproject_matches)}",
            file=sys.stderr,
        )
        sys.exit(1)
    uproject = os.path.abspath(uproject_matches[0])
    if args.verbose:
        print(f" -> Project: {uproject}")

    ue_home = find_unreal_home(Path(uproject))
    if args.verbose:
        print(f" -> Unreal Engine Home: {ue_home}")

    editor_cmd = get_unreal_commands(ue_home, ["UnrealEditor-Cmd"])["UnrealEditor-Cmd"]
    if args.verbose:
        print(f" -> Unreal Engine Editor Cmd: {editor_cmd}")

    asset_paths = []

    if args.staged_only:
        print(f" -> Restricting analysis to staged assets only")

        index_files = subprocess.check_output(
            ["git", "diff-index", "--cached", "--name-only", "HEAD", *args.files],
            universal_newlines=True,
        ).splitlines()
        for file in index_files:
            if not os.path.exists(file):
                # File does not exist. Probably means that the index list
                # includes deleted files. We just skip it
                pass
            elif file.startswith("Content/") and (
                file.lower().endswith(".uasset") or file.lower().endswith(".umap")
            ):
                # Strip "Content/" prefix if present
                rel_path = file.split("Content/", 1)[-1]
                # Remove extension
                rel_path_noext = os.path.splitext(rel_path)[0]
                # Normalize slashes
                game_path = "/Game/" + rel_path_noext.replace("\\", "/").replace(
                    "//", "/"
                )
                asset_paths.append(game_path)

        if 0 != len(index_files) and 0 == len(asset_paths):
            print("No staged Unreal assets. Skipping RuleRanger validation.")
            sys.exit(0)

    if args.asset_path:
        asset_paths.append(args.asset_path)

    if 0 == len(asset_paths):
        asset_paths.append("/Game")

    if args.verbose:
        print(f" -> Assets {asset_paths}")

    asset_paths_str = ",".join(asset_paths)
    report_file = os.path.abspath("RuleRangerReport.json")

    command_args = [
        f"-run=RuleRanger",
        f"-paths={asset_paths_str}",
        f"-report={report_file}",
    ]
    if not args.verbose:
        command_args.append("-quiet")
    if args.report:
        report_path = os.path.abspath(args.report)
        command_args.append(f"-report={report_path}")
        print(f" -> Report: {report_path}")

    with tempfile.NamedTemporaryFile("w", delete=False, suffix=".txt") as argfile:
        for a in command_args:
            argfile.write(f"{a}\n")

    argfile_path = os.path.abspath(argfile.name)
    try:
        subprocess.run(
            [editor_cmd, uproject, f"-CmdLineFile={argfile_path}"], check=True
        )
    finally:
        # Clean up response file
        os.remove(argfile_path)

    sys.exit(0)
except subprocess.CalledProcessError as e:
    print(f"Error executing process: {e}", file=sys.stderr)
    sys.exit(e.returncode)
except Exception as e:
    print(f"An error occurred: {e}", file=sys.stderr)
    exit(-1)
