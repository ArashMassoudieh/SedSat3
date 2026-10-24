# SedSAT3 – Sediment Source Assessment Tool, version 3
# U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.
#
# Stages the files the Advanced Installer project (CMB_Source.aip) installs
# under bin\bin, from the Visual Studio build of the GUI.
#
#   1. Build SedSat3.vcxproj, Release x64 (x64\Release\SedSat3.exe).
#   2. From the repository root, in Windows PowerShell:
#        powershell -ExecutionPolicy Bypass -File installer\stage.ps1
#   3. Build the installer: open CMB_Source.aip in Advanced Installer and
#      build, or AdvancedInstaller.com /build CMB_Source.aip
#
# The script builds the command line program with the same toolchain as the
# GUI (Qt 6.7.2 MSVC, the armadillo and vcpkg folders next to the
# repository, see SedSat3CLI.pro), so both programs share one set of DLLs.
# installer\payload is build output, not in git; the command line program is
# built in %TEMP%\sedsat3-installer-build-cli.
#
# The Visual C++ and OpenMP runtimes are not staged: the installer installs
# them with its VC_redist.x64.exe prerequisite.

param(
    [string]$QtDir   = 'C:\Qt\6.7.2\msvc2019_64',
    [string]$VsDir   = 'C:\Program Files\Microsoft Visual Studio\2022\Community',
    [string]$Jom     = 'C:\Qt\Tools\QtCreator\bin\jom\jom.exe'
)

$ErrorActionPreference = 'Stop'
$root    = Split-Path -Parent $PSScriptRoot
$payload = Join-Path $PSScriptRoot 'payload\bin\bin'
# Built outside the repository: from a folder inside it, qmake writes
# dependency paths to the Qt headers that climb too many levels for jom
$cliDir  = Join-Path $env:TEMP 'sedsat3-installer-build-cli'
$gui     = Join-Path $root 'x64\Release\SedSat3.exe'

if (-not (Test-Path $gui)) { throw "Build SedSat3.vcxproj (Release x64) first: $gui not found." }
foreach ($dll in 'blas_win64_MT.dll','lapack_win64_MT.dll','gsl.dll','gslcblas.dll') {
    if (-not (Test-Path (Join-Path $root "dlls\$dll"))) { throw "dlls\$dll not found." }
}

# 1. The command line program, Release, with the GUI's toolchain
New-Item -ItemType Directory -Force $cliDir | Out-Null
$vcvars = Join-Path $VsDir 'VC\Auxiliary\Build\vcvars64.bat'
$jomCmd = if (Test-Path $Jom) { "`"$Jom`"" } else { 'nmake' }
cmd /c "`"$vcvars`" >nul && cd /d `"$cliDir`" && `"$QtDir\bin\qmake.exe`" `"$root\SedSat3CLI.pro`" CONFIG+=release && $jomCmd release"
if ($LASTEXITCODE -ne 0) { throw 'The command line build failed.' }
$cli = Join-Path $cliDir 'release\sedsat3-cli.exe'

# 2. A clean payload: both programs, Qt for both, the numerical libraries
if (Test-Path (Join-Path $PSScriptRoot 'payload')) { Remove-Item (Join-Path $PSScriptRoot 'payload') -Recurse -Force }
New-Item -ItemType Directory -Force $payload | Out-Null
Copy-Item $gui, $cli $payload
foreach ($exe in 'SedSat3.exe','sedsat3-cli.exe') {
    & "$QtDir\bin\windeployqt.exe" --release --no-translations --no-compiler-runtime (Join-Path $payload $exe) | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed for $exe." }
}
Remove-Item (Join-Path $payload 'vc_redist*.exe') -Force -ErrorAction SilentlyContinue
foreach ($dll in 'blas_win64_MT.dll','lapack_win64_MT.dll','gsl.dll','gslcblas.dll') {
    Copy-Item (Join-Path $root "dlls\$dll") $payload
}

$files = Get-ChildItem $payload -Recurse -File
Write-Host ("Staged {0} files ({1:N1} MB) in {2}" -f $files.Count, (($files | Measure-Object Length -Sum).Sum / 1MB), $payload)
