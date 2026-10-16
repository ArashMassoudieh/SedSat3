#!/usr/bin/env python3
"""Count unique compiler warnings in a build log, split by origin.

Usage: count-warnings.py BUILD_LOG [--list]

A warning is counted once per file:line:column:flag, so a header warning
reported from many translation units counts once. Origins: thirdparty/
(vendored, not ours), Qt/system headers, and first-party code.
"""
import re
import sys

pattern = re.compile(r"^(.*?):(\d+):(\d+): warning: (.*?)(?: \[(-W[^\]]+)\])?$")
unique = {}
for line in open(sys.argv[1], encoding="utf-8", errors="replace"):
    m = pattern.match(line.strip())
    if not m:
        continue
    path, ln, col, msg, flag = m.groups()
    path = re.sub(r".*/CMBSource/", "", path)
    unique[(path, ln, col, flag or "")] = msg

def origin(path):
    if path.startswith("thirdparty/"):
        return "third-party"
    if path.startswith("/"):
        return "system/Qt headers"
    return "first-party"

counts = {}
flags = {}
for (path, _, _, flag) in unique:
    o = origin(path)
    counts[o] = counts.get(o, 0) + 1
    if o == "first-party":
        flags[flag] = flags.get(flag, 0) + 1
for o in sorted(counts):
    print(f"{o:20s} {counts[o]}")
print(f"{'total':20s} {len(unique)}")
print("first-party by flag:")
for f, n in sorted(flags.items(), key=lambda x: -x[1]):
    print(f"  {f or '(no flag)':40s} {n}")
if "--list" in sys.argv:
    for (path, ln, col, flag), msg in sorted(unique.items()):
        if origin(path) == "first-party":
            print(f"{path}:{ln}:{col}: {msg} [{flag}]")
