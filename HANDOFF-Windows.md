# HANDOFF: finish the code-review work on the Windows machine

Written 2026-10-19 on the Linux machine. For Arash and Claude Code on the
Windows development machine (Visual Studio 2022, Qt).

## Where things stand

- All review work is on the branch **`review-fixes-oct2026`**, the source of
  GitLab merge request **!4**
  (<https://code.usgs.gov/cphipps/SedimentSourceAssessmentTool3/-/merge_requests/4>).
  GitHub `main` is at the same commit. Cara has not merged yet.
- Everything that can be done on Linux is done and verified there: all review
  defects and the follow-up findings fixed, 49 unit tests passing, the
  regression suite unchanged, and clang-tidy down to 13 known findings.
  `docs/review/REVIEW_RESPONSE.md` records each item.
- The response letter to Steve is
  `docs/review/response-letter/SedSAT3_code_review_response.tex` (also `.pdf`
  and `.docx`). Its marked items are:
  - **red**: decisions for Cara, Allen or the release coordinator (license,
    ownership wording, author list, manual URL, DOI, AI tag). Not yours to fix
    in code.
  - **blue**: exactly one, in Section 7, and it is the subject of this
    handoff: run the pixi build on Windows and record the results.

Nothing Windows-specific has been run yet. MSVC, `/W4`, cppcheck, the pixi
environment and the Visual Studio project have **not** been tested since the
changes. Expect some fixing.

## 1. Get the code

```powershell
cd <your SedSat3 clone>
git fetch origin
git checkout review-fixes-oct2026      # or: git pull origin main
git pull
git submodule sync
git submodule update --init --recursive
```

- **`Utilities` is no longer a submodule** (its files are now in the
  repository). If git complains about a leftover `Utilities` submodule, run
  `git submodule deinit -f Utilities`, then check that `Utilities\*.cpp` exist.
- **QXlsx now points at `https://github.com/QtExcel/QXlsx.git`**, with the same
  pinned commit as before.

## 2. Build, test and analyse with pixi

Install pixi if it isn't there (<https://pixi.sh>). Then, from a
**Developer PowerShell for VS 2022** at the repository root:

```powershell
pixi run build       # CMake + Ninja -> build\bin\SedSat3.exe, sedsat3-cli.exe, sedsat3-unittests.exe
pixi run test        # ctest: unit tests and the regression suite
pixi run lint        # clang-tidy, repository .clang-tidy (some checks are errors)
pixi run cppcheck    # writes cppcheck.txt
pixi run warnings    # MSVC /W4 build in build-w4\, log in msvc-w4.log
```

`pixi.toml` is based on Steve's working file
(`docs/review/BUILDING_ON_WINDOWS_WITH_PIXI.md`), corrected for the current
repository. It has never been run. Likely problems and what to do about them:

| Symptom | Likely cause and fix |
|---|---|
| `pixi install` fails resolving `python` / `qt6-main` | Loosen the version pins in `pixi.toml`. |
| CMake cannot find `Qt6Test` | `qt6-main` should include it; if not, configure with `-DBUILD_TESTING=OFF` to get going, then find the package that provides it. |
| Link errors `wrapper_dgssv` etc. (SuperLU) | Should be gone: the forced `#define ARMA_USE_SUPERLU` was removed. If they persist, conda's Armadillo config enables SuperLU. Then add `ARMA_DONT_USE_SUPERLU` to `target_compile_definitions` for MSVC in `CMakeLists.txt`, or leave `Utilities/Matrix_arma_sp.cpp` out of `SEDSAT3_CORE_SOURCES` on `WIN32` (nothing in the analyses uses it). |
| C4005 / C4828 warnings | Were addressed (`USE_QCHARTS` / `NOMINMAX` guards, `/utf-8`). Report any that remain. |
| `regression` test fails immediately | It runs `tests/regression/run.sh` with bash. CMake may have found `C:\Windows\System32\bash.exe` (WSL) rather than Git's bash. Point it at Git's: `cmake -S . -B build -DBASH_EXECUTABLE="C:/Program Files/Git/bin/bash.exe"`. The Python interpreter is passed in `PYTHON` automatically. |
| `regression` reports differences | They should be within 1e-9 relative. If values differ slightly more because of MSVC or OpenBLAS round-off, report the size; **do not** regenerate the baseline on Windows. |
| cppcheck aborts looking for `std.cfg` | The `_cppcheck-stage-cfg` task copies the cfg files (Steve's workaround). Check the source path `.pixi/envs/default/share/Cppcheck/cfg` exists. |
| `lint` errors on `cppcoreguidelines-init-variables`, `pro-type-member-init` or `concurrency-mt-unsafe` | These are errors on purpose. Fix the code, the way the other 128 were fixed. |
| `warnings` fails to configure | The `/W4` task restates CMake's default MSVC flags. If CMake rejects the quoting, run the two commands by hand from the `pixi.toml` task. |

Run the GUI once (`pixi run run-app`) and check four things:
- the main window opens;
- an Excel import works (`examples/SampleData.xlsx`);
- the "Include in analysis" column toggles with one click;
- Distribution Fitting no longer crashes.

## 3. Record the results (the blue item)

Count the unique MSVC `/W4` warnings in first-party code:

```powershell
Select-String -Path msvc-w4.log -Pattern ': warning C\d+' |
  Where-Object { $_.Line -notmatch 'thirdparty|\\Qt\\|Windows Kits|MSVC\\' } |
  ForEach-Object { ($_.Line -split ': warning ')[1].Substring(0,5) } |
  Group-Object | Sort-Object Count -Descending | Format-Table Count,Name
```

For **real defects** (C4715, C4700, C4172, C4717, C4834, C4146, C4244/C4267 that
affect an index or count), fix them and rerun `pixi run test`. Leave the
conversion warnings for later, as on Linux (`-Wconversion`, 293 there).

Go through `cppcheck.txt` the same way: fix real findings, and note false
positives with `// cppcheck-suppress <id>` only where they are clearly wrong.

Then update the letter:
1. In `docs/review/response-letter/SedSAT3_code_review_response.tex`, replace
   the `\mine{Run on Windows: ...}` note in Section 7 with the outcome: that
   pixi build/test/lint/cppcheck/warnings run, the MSVC `/W4` count by warning
   number, and the cppcheck count and what was fixed.
2. Add the same to `docs/review/REVIEW_RESPONSE.md` under "Compiler warnings
   and static analysis".
3. Rebuild the Word version (needs only Python):
   `pixi run python docs/review/response-letter/tex2docx.py docs/review/response-letter/SedSAT3_code_review_response.tex docs/review/response-letter/SedSAT3_code_review_response.docx`.
   The PDF needs LaTeX; if it isn't on Windows, leave the PDF for the Linux
   machine.

## 4. The Visual Studio project (optional, only if you keep using it)

`SedSat3.vcxproj` still links a prebuilt QXlsx from `C:\Projects\QXlsx\bin\Qt6`
and includes `C:\Projects\QXlsxfromSassan\...`. Every other build compiles
QXlsx from `thirdparty/QXlsx`. Choose one of:

- **(a) Build from pixi/CMake from now on, and stop maintaining the vcxproj.**
  Recommended: one build path, and the same one CI and Steve use.
- **(b) Fix the vcxproj:** add the `thirdparty\QXlsx\QXlsx\source\*.cpp` files
  (and moc for its headers), add `thirdparty\QXlsx\QXlsx\header` and the Qt
  private header folders to the include path, remove the `C:\Projects\...`
  paths and `QXlsxQt6*.lib`, then build Release x64.

The vcxproj has already been updated for the other changes: the new and
removed files, `excelimport.cpp`, `numericparsing.cpp`, the vendored
QCustomPlot, and `/we4715`. It may still need small fixes; only an MSVC build
will show.

## 5. Portable Windows package for the reviewers

Follow `docs/PACKAGE-Windows-Portable.md`, using the build from step 2 (or the
vcxproj build):
- `pixi run deploy-win` stages the Qt and numerical DLLs. Its DLL names are
  Steve's (`gsl-28.dll`, `openblas.dll`, ...). Check them against
  `.pixi\envs\default\Library\bin`.
- An MSVC build with OpenMP needs **`vcomp140.dll`**, which `deploy-win` does
  not copy. Take it from the VS redistributable folder (procedure, section 5).
- Run `dumpbin /dependents` on the exe, the restricted-token launch check, and
  the zip/SHA-256 steps as described.

The package name is `SedSAT3-1.1.7-windows-x64-portable.zip`, unless Cara sets
a different version.

## 6. Commit, push, then the email

```powershell
git add -A
git commit -m "Windows build: pixi results, MSVC /W4 and cppcheck fixes"
git push origin review-fixes-oct2026:main      # GitHub
git push usgs review-fixes-oct2026             # GitLab, updates merge request !4
```

GitLab needs a personal access token with `write_repository` scope as the
password (the account has 2FA).

After that:
- Regenerate the `.docx` (step 3).
- Replace the attachment on the Gmail draft to Cara (cc Allen), "SedSAT3:
  response letter to Steve's code review — items for you", then send it.
- The red items in the letter are theirs to fill in.

## Do not

- Regenerate `tests/baseline/` on Windows. The baseline is the reviewed
  Linux output; Windows differences are something to report.
- Edit `thirdparty/`.
- Rename serialized keys in the forms or project files; old projects and
  scripts must keep loading.
