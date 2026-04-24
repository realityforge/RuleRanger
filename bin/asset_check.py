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

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

from unreal_locator import find_unreal_home, get_unreal_commands

DEFAULT_REPORT_PATH = Path("Saved/AutomationReports/RuleRangerAssetCheck/report.json")
DEFAULT_LOG_PATH = Path("Saved/Logs/RuleRangerAssetCheck.log")
DEFAULT_OUTPUT_LOG_PATH = Path("Saved/Logs/RuleRangerAssetCheckOutput.log")
PACKAGE_LIST_LIMIT = 5


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Rule Ranger Asset Checker")
    parser.add_argument("--verbose", action="store_true", help="Increase output verbosity")
    parser.add_argument("--staged-only", action="store_true", help="Only analyze staged assets")
    parser.add_argument("--report", type=str, help="Path to the JSON report")
    parser.add_argument("--asset-path", type=str, help="Additional asset paths to analyze")
    parser.add_argument("files", type=str, nargs="*", help="The files to analyze")
    return parser.parse_args()


def rel_posix_path(path: Path, root: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return str(path).replace("\\", "/")


def find_project_file(repo_root: Path) -> Path:
    uproject_matches = list(repo_root.glob("*.uproject"))
    if 1 != len(uproject_matches):
        raise RuntimeError(f"Error: expected exactly 1 .uproject file, found {len(uproject_matches)}")
    return uproject_matches[0].resolve()


def resolve_staged_asset_packages(repo_root: Path, files: list[str]) -> tuple[list[str], list[str]]:
    index_files = subprocess.check_output(
        [
            "git",
            "-c",
            f"safe.directory={repo_root.as_posix()}",
            "diff-index",
            "--cached",
            "--name-only",
            "HEAD",
            *files,
        ],
        cwd=repo_root,
        text=True,
    ).splitlines()

    asset_packages: list[str] = []
    for filename in index_files:
        file_path = repo_root / filename
        if not file_path.exists():
            continue
        if filename.startswith("Content/") and filename.lower().endswith((".uasset", ".umap")):
            rel_path = filename.split("Content/", 1)[-1]
            rel_path_noext = Path(rel_path).with_suffix("")
            game_path = "/Game/" + rel_path_noext.as_posix()
            asset_packages.append(game_path)

    return index_files, asset_packages


def resolve_report_path(repo_root: Path, report_arg: str | None) -> Path:
    if report_arg:
        return Path(report_arg).resolve()
    return (repo_root / DEFAULT_REPORT_PATH).resolve()


def resolve_log_paths(repo_root: Path) -> tuple[Path, Path]:
    return (repo_root / DEFAULT_LOG_PATH).resolve(), (repo_root / DEFAULT_OUTPUT_LOG_PATH).resolve()


def format_scope_header(args: argparse.Namespace) -> str:
    if args.staged_only:
        return "Rule Ranger Asset Checking: (staged assets only)"
    if args.asset_path:
        return "Rule Ranger Asset Checking: (selected asset paths)"
    return "Rule Ranger Asset Checking"


def format_packages_line(asset_packages: list[str]) -> str:
    if len(asset_packages) > PACKAGE_LIST_LIMIT:
        return f"{len(asset_packages)} staged assets"
    return ", ".join(f"'{package}'" for package in asset_packages)


def clean_output_paths(report_path: Path, log_path: Path, output_log_path: Path) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    output_log_path.parent.mkdir(parents=True, exist_ok=True)

    if report_path.exists():
        report_path.unlink()
    if log_path.exists():
        log_path.unlink()
    if output_log_path.exists():
        output_log_path.unlink()


def load_report(report_path: Path) -> dict:
    try:
        with report_path.open("r", encoding="utf-8-sig") as report_file:
            return json.load(report_file)
    except FileNotFoundError as error:
        raise RuntimeError(f"Report not found: {report_path}") from error
    except json.JSONDecodeError as error:
        raise RuntimeError(f"Report failed decoding JSON: {error}") from error


def print_summary(
    repo_root: Path,
    args: argparse.Namespace,
    asset_packages: list[str],
    asset_paths: list[str],
    report_path: Path,
    log_path: Path,
    output_log_path: Path,
    command: list[str],
    report_data: dict | None,
) -> None:
    print(format_scope_header(args), flush=True)
    if asset_packages:
        print(f"  Packages:   {format_packages_line(asset_packages)}", flush=True)
    if asset_paths:
        print(f"  Paths:      {', '.join(asset_paths)}", flush=True)
    print(f"  Report:     {rel_posix_path(report_path, repo_root)}", flush=True)
    print(f"  Log:        {rel_posix_path(log_path, repo_root)}", flush=True)
    print(f"  Output Log: {rel_posix_path(output_log_path, repo_root)}", flush=True)
    print(f"  Command:    {' '.join(command)}", flush=True)

    if report_data is None:
        return

    summary = report_data.get("Summary") or {}
    print(
        (
            "\n  Results:     "
            f"ProjectRulesExecuted={summary.get('ProjectRulesExecuted', 0)} "
            f"AssetsScanned={summary.get('AssetsScanned', 0)} "
            f"Errors={summary.get('Errors', 0)} "
            f"Warnings={summary.get('Warnings', 0)} "
            f"Fatals={summary.get('Fatals', 0)}"
        ),
        flush=True,
    )


def iter_detail_lines(results: list[dict], label_key: str, severity_key: str) -> list[str]:
    detail_lines: list[str] = []
    for result in results:
        label = result.get(label_key, "<missing>")
        for message in result.get(severity_key) or []:
            detail_lines.append(f"    - {label}: {message}")
    return detail_lines


def print_detail_section(title: str, lines: list[str]) -> None:
    if not lines:
        return
    print(f"\n  {title}:", flush=True)
    for line in lines:
        print(line, flush=True)


def print_report_details(report_data: dict) -> None:
    project_results = report_data.get("ProjectRuleResults") or []
    asset_results = report_data.get("AssetRuleResults") or []

    print_detail_section(
        "Project Rule Warnings",
        iter_detail_lines(project_results, "RuleName", "Warnings"),
    )
    print_detail_section(
        "Project Rule Errors",
        iter_detail_lines(project_results, "RuleName", "Errors"),
    )
    print_detail_section(
        "Asset Warnings",
        iter_detail_lines(asset_results, "AssetPath", "Warnings"),
    )
    print_detail_section(
        "Asset Errors",
        iter_detail_lines(asset_results, "AssetPath", "Errors"),
    )


def build_command(
    editor_cmd: Path,
    uproject: Path,
    log_path: Path,
    report_path: Path,
    argfile_path: Path,
) -> list[str]:
    return [
        str(editor_cmd),
        str(uproject),
        "-Unattended",
        "-NullRHI",
        "-nop4",
        "-NoSplash",
        "-NoSound",
        "-stdout",
        "-FullStdOutLogOutput",
        f"-abslog={log_path}",
        f"-ReportExportPath={report_path.parent}",
        f"-CmdLineFile={argfile_path}",
    ]


def main() -> int:
    args = parse_args()
    repo_root = Path.cwd().resolve()

    if args.staged_only and args.asset_path:
        print("--staged-only is not compatible with --asset-path", file=sys.stderr)
        return 1

    try:
        uproject = find_project_file(repo_root)
        ue_home = find_unreal_home(uproject)
        editor_cmd = get_unreal_commands(ue_home, ["UnrealEditor-Cmd"])["UnrealEditor-Cmd"]
    except subprocess.CalledProcessError as error:
        print(str(error), file=sys.stderr)
        return error.returncode
    except Exception as error:
        print(f"An error occurred: {error}", file=sys.stderr)
        return 1

    asset_packages: list[str] = []
    asset_paths: list[str] = []

    try:
        if args.staged_only:
            _, asset_packages = resolve_staged_asset_packages(repo_root, args.files)
            if 0 == len(asset_packages):
                print("No staged Unreal assets. Skipping RuleRanger validation.")
                return 0
        if args.asset_path:
            asset_paths.append(args.asset_path)
    except subprocess.CalledProcessError as error:
        print(str(error), file=sys.stderr)
        return error.returncode

    report_path = resolve_report_path(repo_root, args.report)
    log_path, output_log_path = resolve_log_paths(repo_root)
    clean_output_paths(report_path, log_path, output_log_path)

    command_args = ["-run=RuleRanger", f"-report={report_path}"]
    if asset_packages:
        command_args.append(f"-packages={','.join(asset_packages)}")
    if asset_paths:
        command_args.append(f"-paths={','.join(asset_paths)}")
    command_args.append("-quiet")

    with tempfile.NamedTemporaryFile("w", delete=False, suffix=".txt", encoding="utf-8") as argfile:
        for command_arg in command_args:
            argfile.write(f"{command_arg}\n")
        argfile_path = Path(argfile.name).resolve()

    command = build_command(editor_cmd, uproject, log_path, report_path, argfile_path)

    try:
        with output_log_path.open("w", encoding="utf-8") as output_log_file:
            process = subprocess.run(
                command,
                cwd=repo_root,
                check=False,
                stdout=output_log_file,
                stderr=subprocess.STDOUT,
                text=True,
            )
    finally:
        argfile_path.unlink(missing_ok=True)

    report_data = None
    report_error = None
    try:
        report_data = load_report(report_path)
    except RuntimeError as error:
        report_error = str(error)

    print_summary(
        repo_root,
        args,
        asset_packages,
        asset_paths,
        report_path,
        log_path,
        output_log_path,
        command,
        report_data,
    )

    if report_data is not None:
        print_report_details(report_data)

    if report_error is not None:
        print(f"\n  Report Error: {report_error}", flush=True)
        return 1

    return process.returncode


if __name__ == "__main__":
    raise SystemExit(main())
