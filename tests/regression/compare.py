#!/usr/bin/env python3
"""Compare a SedSAT3 CLI report against the stored baseline.

Usage: compare.py BASELINE.json CURRENT.json [--rtol 1e-9] [--atol 1e-12]

Timing fields are ignored. Numbers are compared with a relative tolerance
(default 1e-9) and an absolute floor (default 1e-12), so platform-level
floating-point noise does not count as a change. Exit status is 0 when the
reports agree, 1 otherwise; every difference is printed with its JSON path.
"""
import argparse
import json
import math
import sys

IGNORED_KEYS = {"seconds", "started", "finished", "script", "project",
                "working_folder", "output"}


def compare(a, b, path, rtol, atol, diffs):
    if isinstance(a, dict) and isinstance(b, dict):
        for key in sorted(set(a) | set(b)):
            if key in IGNORED_KEYS:
                continue
            if key not in a:
                diffs.append(f"{path}/{key}: only in current")
            elif key not in b:
                diffs.append(f"{path}/{key}: only in baseline")
            else:
                compare(a[key], b[key], f"{path}/{key}", rtol, atol, diffs)
    elif isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b):
            diffs.append(f"{path}: length {len(a)} -> {len(b)}")
        for i, (x, y) in enumerate(zip(a, b)):
            compare(x, y, f"{path}[{i}]", rtol, atol, diffs)
    elif isinstance(a, (int, float)) and isinstance(b, (int, float)) \
            and not isinstance(a, bool) and not isinstance(b, bool):
        if math.isnan(a) and math.isnan(b):
            return
        if not math.isclose(a, b, rel_tol=rtol, abs_tol=atol):
            diffs.append(f"{path}: {a!r} -> {b!r}")
    elif a != b:
        diffs.append(f"{path}: {str(a)[:120]!r} -> {str(b)[:120]!r}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("baseline")
    parser.add_argument("current")
    parser.add_argument("--rtol", type=float, default=1e-9)
    parser.add_argument("--atol", type=float, default=1e-12)
    args = parser.parse_args()

    with open(args.baseline, encoding="utf-8") as f:
        baseline = json.load(f)
    with open(args.current, encoding="utf-8") as f:
        current = json.load(f)

    diffs = []
    compare(baseline, current, "", args.rtol, args.atol, diffs)
    for d in diffs:
        print(d)
    print(f"{len(diffs)} difference(s)")
    return 1 if diffs else 0


if __name__ == "__main__":
    sys.exit(main())
