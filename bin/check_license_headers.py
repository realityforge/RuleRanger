#!/usr/bin/env python3
"""
Verify or fix the canonical Apache 2.0 license header on RuleRanger C++ files.

Usage:
  python3 bin/check_license_headers.py
  python3 bin/check_license_headers.py --fix
  python3 bin/check_license_headers.py --root <plugin-root>

Exit codes:
  0 = all files already compliant, or were fixed successfully
  1 = one or more files are non-compliant in check mode
  2 = invalid invocation or unexpected error
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


CANONICAL_HEADER = """/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"""

CPP_EXTENSIONS = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".inl",
}


def normalize_newlines(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def is_cpp_source(path: Path) -> bool:
    return path.is_file() and path.suffix.lower() in CPP_EXTENSIONS


def iter_cpp_sources(source_root: Path) -> list[Path]:
    return sorted(path for path in source_root.rglob("*") if is_cpp_source(path))


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="surrogateescape")


def split_bom(text: str) -> tuple[str, str]:
    if text.startswith("\ufeff"):
        return "\ufeff", text[1:]
    return "", text


def find_existing_license_end(text: str) -> int | None:
    if not text.startswith("/*"):
        return None

    comment_end = text.find("*/")
    if comment_end == -1:
        return None

    comment_block = text[: comment_end + 2]
    if "Licensed under the Apache License" not in comment_block:
        return None

    return comment_end + 2


def make_fixed_content(text: str) -> str:
    bom, body = split_bom(normalize_newlines(text))

    if body.startswith(CANONICAL_HEADER):
        return bom + body

    existing_license_end = find_existing_license_end(body)
    if existing_license_end is not None:
        remainder = body[existing_license_end:]
        remainder = remainder.lstrip("\n")
    else:
        remainder = body.lstrip("\n")

    return bom + CANONICAL_HEADER + remainder


def has_canonical_header(text: str) -> bool:
    _, body = split_bom(normalize_newlines(text))
    return body.startswith(CANONICAL_HEADER)


def main() -> int:
    parser = argparse.ArgumentParser(description="Check or fix RuleRanger C++ license headers.")
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
        help="Plugin root containing Source/ and bin/.",
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Rewrite non-compliant files with the canonical header.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print compliant files as well as violations.",
    )
    args = parser.parse_args()

    plugin_root = args.root.resolve()
    source_root = plugin_root / "Source"
    if not source_root.exists():
        print(f"ERROR: Source/ directory not found under {plugin_root}", file=sys.stderr)
        return 2

    files = iter_cpp_sources(source_root)
    if not files:
        print(f"No C++ source files found under {source_root}")
        return 0

    non_compliant: list[Path] = []
    fixed: list[Path] = []

    for path in files:
        original = read_text(path)
        if has_canonical_header(original):
            if args.verbose:
                print(f"OK   {path.relative_to(plugin_root)}")
            continue

        non_compliant.append(path)
        if args.fix:
            updated = make_fixed_content(original)
            path.write_text(updated, encoding="utf-8", newline="\n")
            fixed.append(path)
            print(f"FIX  {path.relative_to(plugin_root)}")
        else:
            print(f"MISS {path.relative_to(plugin_root)}")

    if args.fix:
        print(f"Processed {len(files)} file(s): fixed {len(fixed)}, already compliant {len(files) - len(fixed)}.")
        return 0

    if non_compliant:
        print(
            f"Found {len(non_compliant)} file(s) without the canonical license header.",
            file=sys.stderr,
        )
        return 1

    print(f"All {len(files)} C++ source file(s) have the canonical license header.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
