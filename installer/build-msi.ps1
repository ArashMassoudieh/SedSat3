# SedSAT3 – Sediment Source Assessment Tool, version 3
# U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.
#
# Builds the Windows installer (SedSat3-<version>-x64.msi) with the WiX
# Toolset from a portable package folder, so the installer and the portable
# zip install the same files. Replaces the Advanced Installer project
# (CMB_Source.aip).
#
#   1. Lay out the portable package (docs\PACKAGE-Windows-Portable.md): the
#      folder that holds README-PORTABLE.txt, bin\bin, resources, docs and
#      sample-data.
#   2. From the repository root, in Windows PowerShell:
#        powershell -ExecutionPolicy Bypass -File installer\build-msi.ps1 -Stage C:\portable\1.1.8-1027\SedSAT3
#
# Needs the .NET SDK (8 or later). The script installs the WiX tool and its
# UI extension (from nuget.org) the first time. The version is read from
# pixi.toml unless -Version is given. The installer is written to
# installer\out, which is not in git.

param(
    [Parameter(Mandatory = $true)][string]$Stage,
    [string]$Version,
    [string]$WixVersion = '5.0.2'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$Stage = (Resolve-Path $Stage).Path

foreach ($required in 'bin\bin\SedSat3.exe', 'bin\bin\sedsat3-cli.exe', 'resources\forms_structures.json',
                      'resources\Icons\CMBSource_Icon.ico', 'docs\SedSAT3_User_Manual.pdf', 'docs\sedsat3-cli-manual.pdf') {
    if (-not (Test-Path (Join-Path $Stage $required))) { throw "$required is not in $Stage" }
}

if (-not $Version) {
    $line = Select-String -Path (Join-Path $root 'pixi.toml') -Pattern '^version\s*=\s*"([^"]+)"' | Select-Object -First 1
    if (-not $line) { throw 'No version in pixi.toml; pass -Version' }
    $Version = $line.Matches[0].Groups[1].Value
}

# The WiX tool and the UI extension, once per machine
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) { throw 'The .NET SDK is not installed (winget install Microsoft.DotNet.SDK.8)' }
if (-not (Get-Command wix -ErrorAction SilentlyContinue)) {
    dotnet tool install --global wix --version $WixVersion
    $env:PATH += ";$env:USERPROFILE\.dotnet\tools"
}
if (-not (wix extension list --global | Select-String 'WixToolset.UI.wixext')) {
    wix extension add --global "WixToolset.UI.wixext/$WixVersion"
}

# The license page: LICENSE.md and DISCLAIMER.md as plain RTF
$obj = Join-Path $PSScriptRoot 'out\obj'
New-Item -ItemType Directory -Force $obj | Out-Null
$text = (Get-Content (Join-Path $root 'DISCLAIMER.md') -Raw) + "`n`n" + (Get-Content (Join-Path $root 'LICENSE.md') -Raw)
$escaped = ($text -replace '\\', '\\\\' -replace '\{', '\{' -replace '\}', '\}') -replace "`r?`n", "\par`n"
$escaped = [regex]::Replace($escaped, '[^\x00-\x7F]', { param($m) '\u' + [int][char]$m.Value + '?' })
$license = Join-Path $obj 'License.rtf'
Set-Content $license "{\rtf1\ansi\deff0{\fonttbl{\f0 Segoe UI;}}\fs18`n$escaped}" -Encoding ASCII

$out = Join-Path $PSScriptRoot "out\SedSat3-$Version-x64.msi"
wix build (Join-Path $PSScriptRoot 'wix\SedSat3.wxs') `
    -arch x64 `
    -ext WixToolset.UI.wixext `
    -d "Version=$Version" `
    -d "License=$license" `
    -bindpath "stage=$Stage" `
    -intermediatefolder $obj `
    -o $out
if ($LASTEXITCODE -ne 0) { throw "wix build failed ($LASTEXITCODE)" }

$hash = (Get-FileHash $out -Algorithm SHA256).Hash.ToLower()
"$hash  $(Split-Path $out -Leaf)" | Set-Content "$out.sha256" -Encoding ASCII
"{0}`n  {1:N1} MB, SHA-256 {2}" -f $out, ((Get-Item $out).Length / 1MB), $hash
