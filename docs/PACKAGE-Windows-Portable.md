# Producing the portable, no-administrator Windows package (MSVC)

This is the procedure for turning an existing MSVC Release build of SedSAT3 into
`SedSAT3-<version>-windows-x64-portable.zip`: a package that extracts anywhere the user
can write, runs by double-clicking the executable, raises no UAC prompt, and installs
nothing.

It exists because the Advanced Installer package requires administrative rights — it
runs `vc_redist.x64.exe` as a prerequisite — and many institutional users (USGS staff,
for example) do not have them. The portable package is an addition, not a replacement:
the installer stays as-is for users who *can* elevate.

This is a **repackaging** procedure, not a build procedure. For building from source
without Visual Studio, see [BUILD-Windows-MinGW.md](BUILD-Windows-MinGW.md), whose
section 6 covers the equivalent packaging step for the MinGW toolchain.

> **Read section 9 before you start.** Several of the pitfalls there silently produce a
> package that works on the build machine and fails everywhere else. They are the reason
> this document exists.

---

## 1. Discover the toolchain — do not assume

Every path below is machine-specific and changes with Qt and Visual Studio updates.
Establish the real values first; a mismatch here produces a broken package that fails in
ways that are hard to diagnose.

```powershell
# Qt kit actually used by the build (read it from the project, not from memory)
Select-String -Path SedSat3.vcxproj -Pattern 'QtInstall|PlatformToolset' | Select-Object -First 4

# Qt kits present, and the matching windeployqt
Get-ChildItem C:\Qt -Directory | Select-Object Name
Get-ChildItem "C:\Qt\*\msvc*\bin\windeployqt.exe" | Select-Object FullName

# Visual Studio and its redistributable payloads
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath
Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022\*\VC\Redist\MSVC\*\x64" -Directory |
  Select-Object FullName

# dumpbin, for the dependency verification in section 8
Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022\*\VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe" |
  Select-Object FullName
```

**`windeployqt.exe` must come from the exact kit the project was built against.** Using
one from a different Qt version, or from a MinGW kit, produces a package that fails
unpredictably at runtime.

Values as of version 1.1.6, for reference:

| Item | Value |
|---|---|
| Qt kit | `C:\Qt\6.7.2\msvc2019_64` |
| Toolset | MSVC v143 (Visual Studio 2022 17.14.x) |
| CRT redist | `...\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT` |
| OpenMP redist | `...\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.OpenMP` |
| Windows SDK | 10.0.22621.0 |

## 2. Confirm the version and the build configuration

The version lives in three places and they must agree. There is **no version resource in
the executable** — `FileVersion` reads `0.0.0.0` — so the binary cannot tell you.

```powershell
Select-String -Path mainwindow.cpp -Pattern '#define version'
Select-String -Path CMakeLists.txt -Pattern 'CPACK_PACKAGE_VERSION'
Select-String -Path CMB_Source.aip -Pattern 'Property="ProductVersion"'
```

Build **Release | x64** — from Visual Studio, or headlessly:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
    SedSat3.vcxproj -p:Configuration=Release -p:Platform=x64 -m
```

Then confirm from the import table that it really is a Release build. You must see
`MSVCP140.dll` and **not** `MSVCP140D.dll`:

```powershell
& $dumpbin /dependents x64\Release\SedSat3.exe
```

Debug CRT DLLs are not redistributable. If you see the `D`-suffixed names, stop and
rebuild in Release.

While you are here, note the full list of imports. **This list — not the Advanced
Installer payload — is the authority on what the package needs.** See section 9.

## 3. Lay out the staging folder

```
C:\portable\SedSAT3\
├── README-PORTABLE.txt
├── bin\bin\                  <- the executable and every DLL
│   ├── platforms\ styles\ imageformats\ iconengines\ tls\ ...
├── resources\
│   ├── tools.json
│   ├── forms_structures.json
│   └── Icons\
└── sample-data\
```

**The two `bin` levels are mandatory.** The application resolves its configuration and
icons as `applicationDirPath() + "/../../resources/..."` — hardcoded, with no fallback
(see `mainwindow.cpp`, `setupToolsView()` and `setupToolBar()`; also `genericform.cpp`
and `resultswindow.cpp`). With the executable at `bin\bin\SedSat3.exe`, that resolves to
the sibling `resources\`. Flatten the layout and the application launches but finds no
`tools.json`, no form definitions and no icons.

This is why the installer buries the executable two levels deep as well. If you ever
want a flat layout, that is a source change — add a fallback to `applicationDirPath()`
— not a packaging change.

## 4. Deploy Qt

From the kit identified in section 1:

```powershell
& "C:\Qt\6.7.2\msvc2019_64\bin\windeployqt.exe" --release --no-translations `
    C:\portable\SedSAT3\bin\bin\SedSat3.exe
```

`--no-translations` is safe and saves several megabytes: `main.cpp` installs no
`QTranslator`, and `CMBSource_en_US.ts` is empty, so Qt's `.qm` files are never loaded.
Re-check this if localisation is ever added.

**Do not use `--compiler-runtime`.** See section 9, pitfall 1 — it does not do what the
name suggests on this machine class. Section 5 handles the runtime instead.

If `windeployqt` warns that it cannot find `dxcompiler.dll` and `dxil.dll`, ignore it.
Those are the Direct3D 12 shader compiler; SedSAT3 is a QWidgets/QtCharts application
that never loads them, and omitting them saves about 15 MB. Confirmed by inspecting the
loaded module list of a running instance.

## 5. Add the MSVC and OpenMP runtimes, app-local

```powershell
$crt = "...\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT"
$omp = "...\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.OpenMP"
$bin = "C:\portable\SedSAT3\bin\bin"

'msvcp140.dll','msvcp140_1.dll','msvcp140_2.dll','vcruntime140.dll','vcruntime140_1.dll' |
    ForEach-Object { Copy-Item "$crt\$_" $bin -Force }
Copy-Item "$omp\vcomp140.dll" $bin -Force

Remove-Item "$bin\vc_redist.x64.exe" -Force -ErrorAction SilentlyContinue
```

| DLL | Why | Source |
|---|---|---|
| `msvcp140.dll` | direct import of the exe | `Microsoft.VC143.CRT` |
| `vcruntime140.dll`, `vcruntime140_1.dll` | direct imports of the exe | `Microsoft.VC143.CRT` |
| `msvcp140_1.dll` | required by `Qt6Core.dll` | `Microsoft.VC143.CRT` |
| `msvcp140_2.dll` | required by `Qt6Gui.dll` | `Microsoft.VC143.CRT` |
| **`vcomp140.dll`** | **OpenMP — direct import of the exe** | **`Microsoft.VC143.OpenMP`** |

`concrt140.dll` is not needed — nothing in the dependency tree imports it. Add it only
if a future build's import table asks for it.

`vc_redist.x64.exe` must never ship. It is an installer that requires elevation, which
is the entire problem this package exists to solve.

## 6. Add the numerical libraries

Microsoft's runtime and Qt are handled. Everything else the executable links against
must be copied in by hand — `windeployqt` knows nothing about them.

```powershell
'gsl.dll','gslcblas.dll','blas_win64_MT.dll','lapack_win64_MT.dll' |
    ForEach-Object { Copy-Item "x64\Release\$_" $bin -Force }
```

- **GSL** — `gsl.dll`, which in turn needs `gslcblas.dll`.
- **BLAS / LAPACK** — `blas_win64_MT.dll`, `lapack_win64_MT.dll`. Armadillo itself is
  header-only here, but it is compiled with `ARMA_USE_LAPACK` and `ARMA_USE_BLAS`, so
  these are hard requirements. This is the classic omission.
- **QXlsx** — nothing to copy. It is **statically linked** (`QXlsxQt6.lib` is ~5.6 MB,
  a static library, and the exe has no `QXlsx.dll` import). Do not ship the
  `QXlsx.dll` sitting in `dlls\`.
- **OpenCL** — `OpenCL.dll` appears in `x64\Release\` as a side effect of the project's
  post-build `copy` of the vcpkg bin folder. It is not imported. Do not ship it.

Always confirm against the current import table rather than this list.

## 7. Resources, sample data and the README

```powershell
Copy-Item resources\tools.json,resources\forms_structures.json C:\portable\SedSAT3\resources\
Copy-Item resources\Icons\* C:\portable\SedSAT3\resources\Icons\
```

`resources\sedsat3.desktop` is for Linux packaging and is not needed.

### Sample data hygiene

Step 8 of the original handover asked for a small example dataset with expected results.
Whatever dataset you pick, **audit the spreadsheet before shipping it.** Every one of
these was present in the 1.1.6 sample data and each would have wasted a reviewer's time:

| Defect | Effect on import |
|---|---|
| Trailing spaces in element names on one sheet only | Element matching between target and sources fails; import reports invalid data and shows an empty dialog |
| `Mean` / `AVERAGE` summary rows under the data | Imported as an extra sample in that source group |
| Scratch columns of derived values (`LOG(...)`, `LN(...)`) | Imported as extra elements |
| Rows of empty but styled cells below the data | Imported as a blank, nameless sample |
| Mojibake in element names (`µ` stored as `U+FFFD`) | Displays as `Calcium_<?>g/g` in the app and in every export |

Appendix D audits all of these. Run it on any spreadsheet before shipping it; the
summary must read `clean` on every line.

Converting formulas to their cached values is *not* sufficient — a `Mean` row kept as
static numbers still imports as a phantom sample. Delete the rows and columns outright.

A caution on editing `.xlsx` files programmatically: if you extract, modify and re-zip,
you must rebuild the archive with forward-slash entry names (pitfall 4) and keep
`[Content_Types].xml` as the first entry. Validate the result with the OPC check at the
end of Appendix D — a package that opens fine in Excel can still be unreadable to QXlsx.

Finally, write `README-PORTABLE.txt` at the package root. Keep it to unzipping and
running; the user manual covers the software itself. It must cover: no admin required,
the `bin\bin\` path, Mark-of-the-Web unblocking, the unsigned-binary policy note, the
SHA-256, where files get written, and the expected result for the sample dataset.

## 8. Verify before packaging

Two checks. Neither is optional, and neither can be replaced by "it works on my
machine".

### 8a. Dependency walk

Walk the import tree of every binary in the package, resolving names **only** against
the package folder and `C:\Windows\System32`. Anything resolving from the Qt install,
the build tree, or a Program Files path means it is missing from the package and the
package only works on this machine.

Use the script in Appendix B. Interpreting the result:

- Ignore anything matching `^(api|ext)-ms-` — those are Windows API-set contracts
  resolved by the loader, not files on disk.
- A handful of names (`HvsiFileTrust.dll`, `AzureAttestManager.dll`, `wpaxholder.dll`,
  `PdmUtilities.dll`) legitimately do not resolve. They are feature-gated Windows
  components, absent on Home editions and delay-loaded by the OS.
- **What matters is the origin.** Every unresolved name must be referenced *from a
  `System32` DLL*. If one is referenced from a SedSAT3 binary, it is genuinely missing.

### 8b. Restricted-token test

Testing from your own account proves nothing — you are an administrator. Test without
elevation, from a **non-elevated** shell:

```powershell
runas /trustlevel:0x20000 "C:\portable\SedSAT3\bin\bin\SedSat3.exe"
```

Run every scenario and record each separately:

| # | Scenario | What it catches |
|---|---|---|
| 1 | Launch from `%LOCALAPPDATA%\SedSAT3\` | baseline, no elevation |
| 2 | Launch from a path containing spaces | quoting bugs |
| 3 | Launch from a different drive letter (`subst` is fine) | path assumptions |
| 4 | Load the sample dataset and run a full analysis **including MCMC** | missing numerical DLL — the longest code path |
| 5 | *(see note)* | — |
| 6 | Confirm no UAC prompt at any point | the elevation requirement |

For each launch, confirm a **visible main window appears** — not merely that the process
is alive. A missing resource file can leave the process running with no window. Then
enumerate the running process's loaded modules and confirm none resolve from outside the
package or `System32`.

> **Scenario 5 — "export a result to Excel" — is not applicable.** The original handover
> assumed an Excel export path that does not exist. QXlsx is used for *reading* only
> (`Document::load`, `cellAt`, `sheetNames`); results export as tab-delimited `.txt` and
> charts as `.png`. The QXlsx code path is exercised by the **import** in scenario 4.

Note that UI Automation cannot drive a restricted-token process — `Invoke` fails with
"Could not open the process token" — and Qt's UIA bridge does not reliably fire toolbar
or menu actions in any case. Scenario 4 is a manual step.

## 9. Pitfalls

These are the failures actually encountered producing 1.1.6. Most of them produce a
package that works on the build machine.

**1. `windeployqt --compiler-runtime` ships the wrong thing.** It is supposed to deploy
the CRT DLLs app-local. On this machine it instead copies a 25 MB
`vc_redist.x64.exe` — an *installer requiring elevation* — and no CRT DLLs at all,
because it cannot locate the VS redist directory (`VCINSTALLDIR` is unset, and the
Qt 6.7.2 *msvc2019* kit looks for a `Microsoft.VC142.CRT` folder that does not exist
when only VC143 is installed). Copy the runtime by hand, as in section 5, and verify the
DLLs are present rather than trusting the flag.

**2. `vcomp140.dll` is in no manifest.** OpenMP is a direct import of the executable,
but it is absent from the Advanced Installer payload and `--compiler-runtime` would
never supply it even when working, because it lives in `Microsoft.VC143.OpenMP` rather
than `Microsoft.VC143.CRT`. Without it the package fails at load time on any machine
without the redistributable installed.

**3. The `.aip` payload is a useful starting point but is not authoritative.** It is
missing `vcomp140.dll` and the `msvcp140_1/_2` dependencies, and it ships three files
that should not be there: `QXlsx.dll` (statically linked, so unnecessary) and
`libEGL.dll` / `libGLESv2.dll`, which are **Qt 5.15.2 binaries from 2020**. Qt 6 dropped
ANGLE and the 6.7.2 kit contains neither. Cross-check the payload against the import
table every time.

**4. Zip entry names must use forward slashes.** `ZipFile.CreateFromDirectory` on
.NET Framework writes backslash-separated entry names on Windows (`xl\workbook.xml`).
Excel tolerates this; stricter readers — including QXlsx — resolve parts by exact name,
find nothing, and fail. This corrupted a sample spreadsheet and produced a
"Failed to load excel file" error with no other symptom. Build archives entry-by-entry
with explicit `/` separators, as in Appendix A, and validate afterwards.

**5. The `bin\bin` nesting is load-bearing.** See section 3.

**6. An unreadable file in `Documents` could hang startup.** `readRecentFilesList()`
read `%DOCUMENTS%\SedSatrecentFiles.txt` with `while (!in.atEnd()) in.readLine()`. When
that path is a OneDrive online-only placeholder that cannot be hydrated, `QFile::open()`
succeeds but every read fails, so `atEnd()` never becomes true. The loop spun forever
inside the `MainWindow` constructor, before `show()` — the application appeared to do
nothing at all when launched, and the stuck processes could not be killed. Fixed in
1.1.6 by reading the file in one `readAll()` call with an explicit error check. The
general lesson: any unbounded read loop over a user-profile path is a startup hazard,
because `Documents` is frequently redirected to OneDrive in institutional environments.

**7. Mark-of-the-Web looks like a broken package.** Windows tags files extracted from a
downloaded ZIP; the application may fail to start with no message. Always cover this in
the README — it is the single most likely support question.

## 10. Package and publish

Build the archive with a single `SedSAT3/` root folder and forward-slash entry names
(Appendix A), then:

```powershell
$zip = "C:\portable\SedSAT3-<version>-windows-x64-portable.zip"
(Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
```

Write the checksum to `<zipname>.sha256` alongside the archive. It matters for security
review.

Last step, and the one worth not skipping: **extract the finished ZIP to a clean folder
and run it from there under a restricted token.** Verifying the staging folder is not
the same as verifying the artifact you are about to send.

### Release checklist

- [ ] Version agrees in `mainwindow.cpp`, `CMakeLists.txt` and `CMB_Source.aip`
- [ ] Built Release | x64; import table shows `MSVCP140.dll`, not `MSVCP140D.dll`
- [ ] `windeployqt` run from the matching kit
- [ ] `vc_redist.x64.exe` absent from the package
- [ ] `vcomp140.dll` present
- [ ] `msvcp140_1.dll` and `msvcp140_2.dll` present
- [ ] GSL, BLAS and LAPACK present
- [ ] No Qt5-era DLLs, no `.pdb`, `.lib`, `.exp`, no run artifacts (`MCMC_Output.txt`)
- [ ] `bin\bin\` nesting intact; `resources\` a sibling of `bin\`
- [ ] Sample spreadsheet: zero formulas, consistent element names, no summary rows
- [ ] Dependency walk clean — no unresolved name originating in a package binary
- [ ] Restricted-token scenarios 1, 2, 3, 4, 6 pass; window visibly appears each time
- [ ] No absolute paths referencing a developer profile in any shipped file
- [ ] Zip entry names use `/`; single `SedSAT3/` root
- [ ] SHA-256 generated
- [ ] Extracted ZIP launches under a restricted token

---

## Appendix A — build and package

Adjust the paths at the top, then run from the repository root.

```powershell
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.IO.Compression

$version = '1.1.6'
$stage = "C:\portable\SedSAT3"; $bin = "$stage\bin\bin"
$rel   = "x64\Release"
$crt   = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT"
$omp   = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.OpenMP"
$qtbin = "C:\Qt\6.7.2\msvc2019_64\bin"

if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $bin,"$stage\resources\Icons","$stage\sample-data" | Out-Null

Copy-Item "$rel\SedSat3.exe" $bin -Force

# Qt. Note: NOT --compiler-runtime (see pitfall 1)
$ErrorActionPreference = 'Continue'
& "$qtbin\windeployqt.exe" --release --no-translations "$bin\SedSat3.exe" 2>&1 |
    Select-String 'To be deployed|Warning:'
$ErrorActionPreference = 'Stop'
Remove-Item "$bin\vc_redist.x64.exe" -Force -ErrorAction SilentlyContinue

# MSVC + OpenMP runtime, app-local
'msvcp140.dll','msvcp140_1.dll','msvcp140_2.dll','vcruntime140.dll','vcruntime140_1.dll' |
    ForEach-Object { Copy-Item "$crt\$_" $bin -Force }
Copy-Item "$omp\vcomp140.dll" $bin -Force

# numerical libraries
'gsl.dll','gslcblas.dll','blas_win64_MT.dll','lapack_win64_MT.dll' |
    ForEach-Object { Copy-Item "$rel\$_" $bin -Force }

# resources, sample data, readme
Copy-Item 'resources\tools.json','resources\forms_structures.json' "$stage\resources\" -Force
Copy-Item 'resources\Icons\*' "$stage\resources\Icons\" -Force
# Copy-Item <your sample data> "$stage\sample-data\" -Force
# Copy-Item <your README-PORTABLE.txt> "$stage\" -Force

# strip anything that must not ship
Get-ChildItem $stage -Recurse -File |
    Where-Object { $_.Name -match 'MCMC_Output|GA_Output|\.pdb$|\.lib$|\.exp$|\.ilk$|vc_redist|\.bak$' } |
    ForEach-Object { Write-Host "removing $($_.Name)"; Remove-Item $_.FullName -Force }

# archive: explicit forward slashes, single root (see pitfall 4)
$out = "C:\portable\SedSAT3-$version-windows-x64-portable.zip"
if (Test-Path $out) { Remove-Item $out -Force }
$fs  = [System.IO.File]::Open($out,'CreateNew')
$zip = New-Object System.IO.Compression.ZipArchive($fs,[System.IO.Compression.ZipArchiveMode]::Create)
foreach ($f in Get-ChildItem $stage -Recurse -File | Sort-Object FullName) {
    $name = "SedSAT3/" + $f.FullName.Substring($stage.Length+1).Replace([char]92,[char]47)
    $e = $zip.CreateEntry($name,[System.IO.Compression.CompressionLevel]::Optimal)
    $s = $e.Open(); $b = [System.IO.File]::ReadAllBytes($f.FullName)
    $s.Write($b,0,$b.Length); $s.Close()
}
$zip.Dispose(); $fs.Close()

# validate and checksum
$z = [System.IO.Compression.ZipFile]::OpenRead($out)
"entries        : $($z.Entries.Count)"
"backslashes    : $(@($z.Entries | Where-Object { $_.FullName.Contains([char]92) }).Count)"
"root folders   : $(@($z.Entries | ForEach-Object { $_.FullName.Split('/')[0] } | Sort-Object -Unique) -join ', ')"
$z.Dispose()
$h = (Get-FileHash $out -Algorithm SHA256).Hash.ToLower()
"SHA-256        : $h"
"$h  $(Split-Path $out -Leaf)" | Set-Content "$out.sha256" -Encoding ASCII
```

## Appendix B — dependency verification

```powershell
$dumpbin = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe"
$appdir  = "C:\portable\SedSAT3\bin\bin"
$sys     = Join-Path $env:WINDIR 'System32'
$seen = @{}; $problems = @()

function Get-Deps([string]$file) {
    $cap = $false; $deps = @()
    foreach ($l in (& $dumpbin /dependents $file 2>$null)) {
        if ($l -match 'Image has the following dependencies') { $cap = $true; continue }
        if ($cap) {
            if ($l -match '^\s+(\S+\.dll)\s*$') { $deps += $matches[1] }
            elseif ($l -match '^\s*Summary') { break }
        }
    }
    ,$deps
}
function Walk([string]$file) {
    $key = (Split-Path $file -Leaf).ToLower()
    if ($seen.ContainsKey($key)) { return }
    $seen[$key] = $true
    foreach ($d in (Get-Deps $file)) {
        if ($d -match '^(api|ext)-ms-') { continue }
        $inApp = Join-Path $appdir $d; $inSys = Join-Path $sys $d
        if     (Test-Path $inApp) { Walk $inApp }
        elseif (Test-Path $inSys) { Walk $inSys }
        else {
            $origin = if ($file -like '*portable*') { 'PACKAGE' } else { 'system' }
            $script:problems += [pscustomobject]@{ Origin=$origin; Binary=(Split-Path $file -Leaf); Missing=$d }
        }
    }
}

$bins = Get-ChildItem "C:\portable\SedSAT3" -Recurse -Include *.exe,*.dll
foreach ($b in $bins) { Walk $b.FullName }
"binaries: $($bins.Count)   modules walked: $($seen.Count)"
if ($problems.Count -eq 0) { "clean" }
else { $problems | Sort-Object Missing -Unique | Format-Table -AutoSize }
```

Any row with `Origin = PACKAGE` is a real missing dependency. Rows with
`Origin = system` are the feature-gated Windows components described in section 8a.

## Appendix C — restricted-token launch check

Confirms a visible window appears, no UAC prompt is raised, and every loaded module
comes from the package or `System32`.

```powershell
param([string]$Exe)
Add-Type @"
using System;using System.Text;using System.Runtime.InteropServices;using System.Collections.Generic;
public class WV {
  [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc cb, IntPtr p);
  [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowTextW(IntPtr h, StringBuilder s, int n);
  delegate bool EnumProc(IntPtr h, IntPtr p);
  public static List<string> Vis(uint t){ var r=new List<string>();
    EnumWindows((h,p)=>{ uint pid; GetWindowThreadProcessId(h,out pid);
      if(pid==t && IsWindowVisible(h)){ var a=new StringBuilder(512); GetWindowTextW(h,a,512); r.Add(a.ToString()); }
      return true; },IntPtr.Zero); return r; }
}
"@
$before = @(Get-CimInstance Win32_Process -Filter "Name='SedSat3.exe'").Count
$consentBefore = @(Get-Process consent -ErrorAction SilentlyContinue).Count
Push-Location $env:USERPROFILE
cmd /c "runas /trustlevel:0x20000 `"$Exe`"" | Out-Null
Pop-Location

$procId = $null; $deadline = (Get-Date).AddSeconds(25)
while ((Get-Date) -lt $deadline) {
    $c = @(Get-CimInstance Win32_Process -Filter "Name='SedSat3.exe'")
    if ($c.Count -gt $before) { $procId = ($c | Sort-Object CreationDate -Descending)[0].ProcessId; break }
    Start-Sleep -Milliseconds 400
}
if (-not $procId) { "FAIL: process never started"; return }

$wins = @(); for ($i=0; $i -lt 15; $i++) { Start-Sleep -Seconds 2; $wins = [WV]::Vis([uint32]$procId); if ($wins.Count) { break } }
if (-not $wins.Count) { "FAIL: no visible window after 30s"; Stop-Process -Id $procId -Force; return }

"window  : '$($wins[0])'"
"UAC     : $(if ((@(Get-Process consent -ErrorAction SilentlyContinue).Count) -gt $consentBefore) {'PROMPT APPEARED'} else {'none'})"
$p = Get-Process -Id $procId
$root = Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent
$leaks = @($p.Modules | Where-Object {
    $_.FileName -notlike "$root*" -and
    $_.FileName -notlike "$env:WINDIR\System32*" -and
    $_.FileName -notlike "$env:WINDIR\WinSxS*"
})
"modules : $($p.Modules.Count) total, $($leaks.Count) from outside the package"
$leaks | ForEach-Object { "   LEAK $($_.FileName)" }
Stop-Process -Id $procId -Force
```

## Appendix D — sample spreadsheet audit

Checks the five defects in section 7, plus OPC package validity. Every line of the
summary must read `clean`.

```powershell
param([string]$Path)
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.Xml.Linq
$ns  = [System.Xml.Linq.XNamespace]::Get('http://schemas.openxmlformats.org/spreadsheetml/2006/main')
$rns = [System.Xml.Linq.XNamespace]::Get('http://schemas.openxmlformats.org/package/2006/relationships')
$ctns= [System.Xml.Linq.XNamespace]::Get('http://schemas.openxmlformats.org/package/2006/content-types')
$rid = [System.Xml.Linq.XName]::Get('id','http://schemas.openxmlformats.org/officeDocument/2006/relationships')

$z = [System.IO.Compression.ZipFile]::OpenRead($Path)
$names = @{}; foreach ($e in $z.Entries) { $names[$e.FullName] = $e.Length }
function Part([string]$n) {
    $e = $z.Entries | Where-Object { $_.FullName -ceq $n }      # exact, case-sensitive
    if (-not $e) { return $null }
    $ms = New-Object System.IO.MemoryStream; $s = $e.Open(); $s.CopyTo($ms); $s.Close()
    $ms.Position = 0; [System.Xml.Linq.XDocument]::Load($ms)
}

# --- OPC validity: forward slashes, and every declared part resolvable ---
$bad = @($names.Keys | Where-Object { $_.Contains([char]92) })
"entry names         : $(if ($bad.Count) { "FAIL - $($bad.Count) use backslashes" } else { 'clean' })"
$ct = Part '[Content_Types].xml'
$missing = @()
foreach ($o in $ct.Root.Elements($ctns+'Override')) {
    $pn = $o.Attribute('PartName').Value.TrimStart('/')
    if (-not $names.ContainsKey($pn)) { $missing += $pn }
}
"declared parts      : $(if ($missing.Count) { "FAIL - missing: $($missing -join ', ')" } else { 'clean' })"
"calcChain.xml       : $(if ($names.ContainsKey('xl/calcChain.xml')) { 'FAIL - present, remove it' } else { 'clean' })"

# --- sheets ---
$wb = Part 'xl/workbook.xml'
$rl = Part 'xl/_rels/workbook.xml.rels'
$map = @{}; foreach ($r in $rl.Root.Elements($rns+'Relationship')) { $map[$r.Attribute('Id').Value] = $r.Attribute('Target').Value }
$ss = @(); $sd = Part 'xl/sharedStrings.xml'
if ($sd) { foreach ($si in $sd.Root.Elements($ns+'si')) { $ss += ($si.Descendants($ns+'t') | ForEach-Object { $_.Value }) -join '' } }

$headers = @{}; $totalF = 0; $rowIssues = @(); $colIssues = @()
foreach ($s in $wb.Root.Element($ns+'sheets').Elements($ns+'sheet')) {
    $nm  = $s.Attribute('name').Value
    $doc = Part ('xl/' + ($map[$s.Attribute($rid).Value] -replace '^/',''))
    $totalF += @($doc.Descendants($ns+'f')).Count
    $rows = @($doc.Root.Element($ns+'sheetData').Elements($ns+'row'))

    $h = [ordered]@{}
    $r1 = $rows | Where-Object { $_.Attribute('r').Value -eq '1' }
    if ($r1) { foreach ($c in $r1.Elements($ns+'c')) {
        $v = $c.Element($ns+'v'); if (-not $v) { continue }
        $t = $c.Attribute('t')
        $h[($c.Attribute('r').Value -replace '\d','')] = if ($t -and $t.Value -eq 's') { $ss[[int]$v.Value] } else { $v.Value }
    } }
    $headers[$nm] = $h

    $dataRows = @($rows | Where-Object { [int]$_.Attribute('r').Value -gt 1 })
    $named = @($dataRows | Where-Object {
        $a = $_.Elements($ns+'c') | Where-Object { $_.Attribute('r').Value -match '^A\d+$' }
        $a -and $a.Element($ns+'v')
    })
    if ($dataRows.Count -ne $named.Count) {
        $rowIssues += "$nm has $($dataRows.Count) data rows but $($named.Count) named samples"
    }

    # data in a column with no header = a scratch column of intermediate values
    $dataCols = @($dataRows | ForEach-Object { $_.Elements($ns+'c') } |
        Where-Object { $_.Element($ns+'v') } |
        ForEach-Object { $_.Attribute('r').Value -replace '\d','' } | Sort-Object -Unique)
    $orphan = @($dataCols | Where-Object { -not $h.Contains($_) })
    if ($orphan.Count) { $colIssues += "$nm has data in unheaded column(s) $($orphan -join ', ')" }
}
"formulas            : $(if ($totalF) { "FAIL - $totalF formula cells" } else { 'clean' })"
"blank/unnamed rows  : $(if ($rowIssues.Count) { "FAIL - $($rowIssues -join '; ')" } else { 'clean' })"
"scratch columns     : $(if ($colIssues.Count) { "FAIL - $($colIssues -join '; ')" } else { 'clean' })"

# --- element names identical across every sheet, and free of stray whitespace ---
$sheetNames = @($headers.Keys)
$ref = $headers[$sheetNames[0]]
$mismatch = @(); $ws = @()
foreach ($col in $ref.Keys) {
    $vals = @($sheetNames | ForEach-Object { $headers[$_][$col] })
    if (@($vals | Select-Object -Unique).Count -ne 1) { $mismatch += "$col ($(($vals | ForEach-Object { "'$_'" }) -join ' vs '))" }
    if ($ref[$col] -ne $ref[$col].Trim()) { $ws += "$col '$($ref[$col])'" }
}
"header consistency  : $(if ($mismatch.Count) { "FAIL - $($mismatch -join '; ')" } else { 'clean' })"
"header whitespace   : $(if ($ws.Count) { "FAIL - $($ws -join '; ')" } else { 'clean' })"

$fffd = @($ref.Values | Where-Object { $_ -and $_.Contains([char]0xFFFD) })
"name encoding       : $(if ($fffd.Count) { "FAIL - U+FFFD in: $($fffd -join ', ')" } else { 'clean' })"

"`nsheets: " + (($sheetNames | ForEach-Object {
    $n = @($headers[$_].Keys).Count; "$_ ($n cols)" }) -join ', ')
$z.Dispose()
```

---

## What the 1.1.6 package looked like

A reference point for sanity-checking future runs.

| | |
|---|---|
| Archive | 34,443,838 bytes (32.8 MB) |
| Extracted | 80.9 MB, 60 files |
| Executable | 2,207,232 bytes |
| Qt modules | Charts, Core, Gui, Network, OpenGL, OpenGLWidgets, Pdf, Svg, Widgets |
| Qt plugins | platforms, styles, imageformats, iconengines, tls, networkinformation, generic |
| SHA-256 | `5b73b34e4136053dd530dfcb528a67a3315d12db4b0c50837101f23a54febd47` |

`Qt6Network`, `Qt6Pdf` and `Qt6Svg` are pulled in as plugin dependencies rather than by
the application directly, which is expected.
