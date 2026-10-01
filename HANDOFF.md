# HANDOFF: Build a portable, no-admin Windows package for SedSAT3

## Who this is for

Claude Code, running on Arash's Windows development machine, where SedSAT3 is
already built and where Advanced Installer is already producing a working
installer.

## Why

SedSAT3 is going through USGS Fundamental Science Practices review (IPDS-190994).
The supervisory reviewer could not install or test the application, because the
current installer requires administrative rights and USGS staff generally do not
have them. Two more reviewers — Mike Fienen (code) and Lilly Gorman-Sanisaca
(domain/manual) — are about to receive the package and will hit the same wall.

The fix is a portable ZIP: unzip anywhere in the user's own profile, double-click
the executable, it runs. No installer, no elevation, no writes outside the user's
own space.

This is a **repackaging** job, not a build job. The binary already exists. Do not
change the source, the build configuration, or the Advanced Installer project.

## Definition of done

A single file, `SedSAT3-<version>-windows-x64-portable.zip`, that:

1. Extracts to any folder the user can write to (`%LOCALAPPDATA%`, Desktop, a USB stick).
2. Launches by double-clicking the executable, with **no UAC prompt**.
3. Runs correctly under a **restricted (non-elevated) token**, verified — not assumed.
4. Contains **no absolute paths** baked into config files pointing at Arash's machine.
5. Includes a short README for reviewers and a SHA-256 checksum.
6. Is confirmed to have zero missing DLLs when checked against a dependency walk.

## Before you start: discover, don't assume

Report these back before proceeding. Do not guess.

- Path to the built Release executable (`SedSAT3.exe` or similar).
- Qt version and kit used for the build (e.g. Qt 6.x MSVC2019/2022 64-bit).
  The `windeployqt.exe` you use **must** come from that exact kit. Using
  windeployqt from a different Qt version or a MinGW kit will produce a broken
  package that fails in ways that are hard to diagnose.
- MSVC toolset version (VS 2019 / 2022, v142 / v143).
- Which third-party libraries are linked dynamically vs statically. Known
  dependencies are Qt, GSL, Armadillo, and QXlsx — but confirm from the project
  files rather than taking that list as complete.
- Whether the app writes settings via `QSettings`, and to where.

## Shortcut worth taking first

The Advanced Installer project already defines the complete set of files needed at
runtime. That file list is the answer to "what goes in the ZIP," already worked out
and already tested.

Open the `.aip` project (it is XML) and extract the payload file list. Alternatively,
install the current package on this machine and enumerate the installed directory.
Either gives you a known-good manifest to check your staged folder against.

**Exception:** anything the installer registers system-wide — VC++ redistributable
via merge module, registry entries under HKLM, file associations, services — must be
handled differently for the portable build. See step 4.

## Steps

### 1. Stage

Create a clean staging folder outside the build tree, e.g. `C:\portable\SedSAT3`.
Copy the Release executable into it.

### 2. Deploy Qt

From a **Qt developer command prompt matching the build kit**:

```
windeployqt.exe --release --compiler-runtime C:\portable\SedSAT3\SedSAT3.exe
```

This pulls in the Qt DLLs, the `platforms\qwindows.dll` plugin, image format
plugins, styles, and — via `--compiler-runtime` — the MSVC runtime DLLs.

Consider `--no-translations` only if the app is English-only; check first.

### 3. Add non-Qt dependencies

`windeployqt` only knows about Qt. Copy in everything else the executable links
against. Expect at least:

- GSL: `gsl.dll`, `gslcblas.dll` (names vary by build)
- Armadillo: often header-only, **but** it frequently links a BLAS/LAPACK
  implementation. Look for `blas_win64_MT.dll` / `lapack_win64_MT.dll`, or an
  OpenBLAS DLL. This is a common miss.
- QXlsx: DLL if built shared, nothing if static.

Verify against the actual import table (step 6), not against this list.

### 4. Handle the MSVC runtime — this is the critical piece

The Visual C++ runtime is the usual reason a package "needs admin." The standard
installer approach runs `vc_redist.exe`, which requires elevation.

For portable deployment, the CRT DLLs are copied next to the executable instead
(app-local deployment). Microsoft supports this. `--compiler-runtime` in step 2
should have handled it, but **verify these are actually present**:

- `msvcp140.dll`
- `vcruntime140.dll`
- `vcruntime140_1.dll` (x64, VS2019+)
- `concrt140.dll` (only if the app uses the Concurrency Runtime)

If any are missing, copy them from:

```
C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\<ver>\x64\Microsoft.VC143.CRT\
```

Adjust the path for the actual VS edition and toolset. If the app was built
against the debug CRT, stop and report it — debug CRT DLLs are not
redistributable and the package must be built from a Release configuration.

### 5. Make settings portable (check, then decide)

If the app uses `QSettings` with default behaviour, it writes to `HKEY_CURRENT_USER`.
That does **not** require admin, so it is not a blocker. But it leaves traces on the
reviewer's machine and it means two extracted copies share state.

If it is a small change, prefer writing an INI next to the executable:

```cpp
QSettings::setDefaultFormat(QSettings::IniFormat);
QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                   QCoreApplication::applicationDirPath());
```

Do not make this change unilaterally — it touches source, which is out of scope
for this task. Report whether it is needed and let Arash decide.

Separately, scan any shipped config, `.ini`, or resource files for absolute paths
referencing `C:\Users\<arash>\...`. Those break on any other machine.

### 6. Verify dependencies

Walk the import tree and confirm nothing resolves outside the package folder or
`C:\Windows\System32`. Any DLL resolving from Qt's install directory, the build
tree, or a Program Files path means it is missing from the ZIP and the package
only works on this machine.

Options, in order of preference:

- `dumpbin /dependents` on each binary (VS command prompt), walked recursively
- Dependencies.exe (the maintained successor to Dependency Walker) — portable, no install
- `Get-Command`/PowerShell scripting over the folder

### 7. Test under a restricted token — do not skip this

Testing on an admin account proves nothing. The whole point is behaviour without
elevation.

```
runas /trustlevel:0x20000 "C:\portable\SedSAT3\SedSAT3.exe"
```

This launches with a restricted token. Then test each of these:

- Launch from `%LOCALAPPDATA%\SedSAT3\`
- Launch from a path containing spaces (`C:\Users\...\My Test Folder\`)
- Launch from a different drive letter if one is available
- Load a sample dataset and run at least one full analysis end to end — including
  an MCMC run, since that is the longest path and most likely to touch a missing
  numerical library
- Export a result to Excel, which exercises QXlsx
- Confirm no UAC prompt appears at any point

If anything fails, the missing piece is almost always a DLL from step 3 or 4.

### 8. Include sample data

Lilly's domain review requires confirming the output is technically correct. She
needs data to run. Include a small example dataset with expected results, in a
`sample-data\` subfolder. Ask Arash which dataset to use.

### 9. Write the reviewer README

Inside the ZIP, a short `README-PORTABLE.txt`:

- No installation or administrator rights required
- Unzip and run `SedSAT3.exe`
- **Mark of the Web:** Windows tags files extracted from a downloaded ZIP, which
  can cause SmartScreen warnings or block execution. If that happens, right-click
  the ZIP → Properties → Unblock **before** extracting. Or in PowerShell:
  `Get-ChildItem -Recurse <folder> | Unblock-File`
- The executable is unsigned. If the organisation's application-control policy
  blocks unsigned binaries from user-writable folders, this is a policy matter
  and needs local IT, not a different package
- Which version and which toolchain produced the binary
- Where to find the manual and the sample data

### 10. Package

```
SedSAT3-<version>-windows-x64-portable.zip
```

Use the real version number — confirm it from the build, not from memory. Generate
a SHA-256 checksum alongside it:

```powershell
Get-FileHash SedSAT3-<version>-windows-x64-portable.zip -Algorithm SHA256
```

The checksum matters for Mike's security review.

## Report back

- Full file list in the package, with total size
- Any DLL that was missing from the Advanced Installer payload and had to be added
- Result of the restricted-token test, per scenario in step 7
- Whether `QSettings` writes to the registry, and your recommendation
- Any absolute paths found in shipped files
- The SHA-256 checksum
- Anything that required a source or build-config change (should be nothing —
  flag it rather than doing it)

## Out of scope

- Modifying source, build configuration, or the Advanced Installer project
- Code signing (worth doing eventually, not needed for this)
- The MinGW build path — that is a separate document, for reproducibility, not
  for producing this package
- Any change to the existing installer, which stays as-is for users who do have
  administrative rights
