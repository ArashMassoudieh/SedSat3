#!/usr/bin/env bash
# Run the regression scripts with a SedSAT3 CLI build and compare the reports
# against tests/baseline/.
#
#   tests/regression/run.sh path/to/sedsat3-cli [output-dir]
#
# With UPDATE_BASELINE=1 the reports are written to tests/baseline/ instead.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
cli="$(realpath "$1")"
if [[ "${UPDATE_BASELINE:-0}" == 1 ]]; then
    out="$root/tests/baseline"
else
    out="$(realpath -m "${2:-$root/build-regression}")"
fi
mkdir -p "$out"
export SEDSAT3_RESOURCES="${SEDSAT3_RESOURCES:-$root/resources}"
status=0
for script in "$here"/*.json; do
    name="$(basename "$script" .json)"
    python3 - "$script" "$here" "$out" "$name" <<'PY'
import json, os, sys
script, script_dir, out, name = sys.argv[1:]
d = json.load(open(script))
d["project"] = os.path.normpath(os.path.join(script_dir, d["project"]))
d["output"] = os.path.join(out, name + "_report.json")
d["working_folder"] = os.path.join(out, name + "_files")
json.dump(d, open(os.path.join(out, name + "_script.json"), "w"), indent=2)
PY
    "$cli" "$out/${name}_script.json" --quiet > "$out/${name}.log" 2>&1 || true
    rm -f "$out/${name}_script.json"
    # Drop machine-specific paths and times, which compare.py ignores anyway
    python3 - "$out/${name}_report.json" <<'PY'
import json, sys
path = sys.argv[1]
report = json.load(open(path))
for key in ("project", "script", "working_folder", "output", "started", "finished"):
    report.pop(key, None)
for step in report.get("steps", []):
    step.pop("seconds", None)
json.dump(report, open(path, "w"), indent=1, sort_keys=True)
PY
    if [[ "${UPDATE_BASELINE:-0}" != 1 ]]; then
        echo "== $name"
        python3 "$here/compare.py" "$root/tests/baseline/${name}_report.json" \
            "$out/${name}_report.json" || status=1
    fi
done
exit $status
