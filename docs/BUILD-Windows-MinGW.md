# Building SedSAT3 on Windows with MinGW (no Visual Studio)

This guide builds SedSAT3 on Windows using only free, openly licensed tools. It exists
so that reviewers and users who cannot obtain a Visual Studio license can still build,
install, and test the software.

Every tool used here is free for any purpose, including commercial and government use:

| Tool | Role | License |
|---|---|---|
| MSYS2 + MinGW-w64 (UCRT64) | compiler toolchain and dependency manager | BSD / GPL |
| Qt 6 (via MSYS2) | GUI framework | LGPL-3.0 |
| Armadillo, GSL | numerical libraries | Apache-2.0 / GPL-3.0 |
| CMake + Ninja | build system | BSD / Apache-2.0 |
| Inno Setup (optional) | installer generation | free, source-available |

No Visual Studio, no MSVC license, and no Advanced Installer license is required.

---

## 1. Install MSYS2

Download and run the installer from <https://www.msys2.org> (default location
`C:\msys64`). Then open **MSYS2 UCRT64** from the Start menu — not the "MSYS" or
"MINGW32" shortcut. The prompt must read `UCRT64`.

Update the package database, closing and reopening the terminal if it asks you to:

```bash
pacman -Syu
```

## 2. Install the toolchain and dependencies

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-qt6-charts \
  mingw-w64-ucrt-x86_64-qt6-tools \
  mingw-w64-ucrt-x86_64-armadillo \
  mingw-w64-ucrt-x86_64-gsl \
  git
```

This covers every dependency in `CMakeLists.txt`: Qt6 (Core, Gui, Widgets,
PrintSupport, Charts), Armadillo, GSL, and OpenMP (via the toolchain's `libgomp`).
QXlsx and qcustomplot are vendored in the repository and need no separate install.

Verify the compiler and Qt are the UCRT64 ones:

```bash
which g++ cmake qmake6
```

All three paths must begin with `/ucrt64/`. If they point at `/usr/bin` or a Windows
path, you are in the wrong MSYS2 shell.

## 3. Clone the repository with submodules

SedSAT3 has two submodules (`Utilities` and `thirdparty/QXlsx`). The `--recursive` flag
is required; a plain clone will fail to configure.

```bash
git clone --recursive https://github.com/ArashMassoudieh/CMBSource.git
cd CMBSource
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

## 4. Configure and build

> **Build inside the repository, in a directory named `build`.** This is not a style
> preference. The application locates its resources at runtime with the hardcoded
> relative path `applicationDirPath()/../../resources`, so the executable must land in
> `<repo>/build/bin` for `<repo>/resources` to resolve. Building elsewhere compiles
> fine but produces an application with no icons, no tool definitions, and no forms.

```bash
mkdir -p build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

The executable is written to `build/bin/SedSat3.exe`.

## 5. Run from the build tree

```bash
./bin/SedSat3.exe
```

Launched from the MSYS2 UCRT64 shell, the Qt DLLs are already on `PATH`. Double-clicking
the `.exe` in Explorer will fail with a missing-DLL error until you complete step 6.

## 6. Produce a portable, no-administrator package

This step yields a folder that runs on a machine with no MSYS2, no Qt, and no
administrator rights — which is what a reviewer needs in order to test without
installing anything.

The directory layout is dictated by the same `../../resources` lookup described above,
so the executable must sit two levels below the `resources` folder:

```
SedSAT3-portable/
├── resources/            <- copied from the repository
└── app/
    └── bin/
        └── SedSat3.exe   <- plus all Qt DLLs
```

Build it:

```bash
cd <repo>
mkdir -p SedSAT3-portable/app/bin
cp -r resources SedSAT3-portable/
cp build/bin/SedSat3.exe SedSAT3-portable/app/bin/
windeployqt6 --release --compiler-runtime SedSAT3-portable/app/bin/SedSat3.exe
```

`windeployqt6` copies the Qt runtime, platform plugins, and the C++ runtime into
`app/bin`. Then copy the remaining MinGW runtime libraries, which `windeployqt` does not
always detect:

```bash
cp /ucrt64/bin/{libgcc_s_seh-1,libstdc++-6,libwinpthread-1,libgomp-1}.dll \
   SedSAT3-portable/app/bin/
cp /ucrt64/bin/{libopenblas,libgsl-*,libgslcblas-*}.dll SedSAT3-portable/app/bin/ 2>/dev/null
```

Test the result on a machine that has never had Qt or MSYS2 installed. If it reports a
missing DLL, find it with `ldd build/bin/SedSat3.exe | grep ucrt64` and copy that one too.

Zip the `SedSAT3-portable` folder and it is ready to distribute.

## 7. Optional: build an installer without Advanced Installer

If a conventional Windows installer is wanted alongside the portable ZIP,
[Inno Setup](https://jrsoftware.org/isinfo.php) is free and produces a per-user
installer that needs no administrator rights. A minimal script:

```
[Setup]
AppName=SedSAT3
AppVersion=1.1.6
DefaultDirName={autopf}\SedSAT3
PrivilegesRequired=lowest
OutputBaseFilename=SedSAT3-1.1.6-setup

[Files]
Source: "SedSAT3-portable\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{autoprograms}\SedSAT3"; Filename: "{app}\app\bin\SedSat3.exe"
```

`PrivilegesRequired=lowest` is what removes the administrator requirement.

---

## Required change to CMakeLists.txt

The install and packaging rules in `CMakeLists.txt` are Linux-specific — they hardcode
`/usr/local/sedsat3`, `/usr/share/applications`, and `/usr/share/icons`. They do not
prevent the build in step 4 from succeeding, but `cmake --install` and `cpack` will
misbehave on Windows. Guard them:

```cmake
if(UNIX AND NOT APPLE)
    # ... existing install() rules, CPACK_* settings, and include(CPack) ...
endif()
```

## Known issues worth fixing

These are not blockers for the build, but they affect portability and are worth
addressing before release:

1. **Hardcoded resource path.** `applicationDirPath()/../../resources` appears in
   `mainwindow.cpp`, `resultswindow.cpp`, and `genericform.cpp`. It forces the awkward
   `app/bin` nesting in step 6 and silently degrades the UI when it fails. Embedding the
   resources in a Qt resource file (`.qrc`), or resolving them through a single helper
   with a fallback, would remove the constraint entirely.

2. **`resources/icons` and `resources/Icons` both exist.** These are separate
   directories on Linux but collide on Windows' case-insensitive filesystem, and git
   will warn on checkout. The code only references `Icons/`; `icons/` holds a single
   file, `sedsat3.png`, used by the Linux desktop entry. Merging them would remove the
   collision.

---

**Verification status:** these instructions were written against the repository's
`CMakeLists.txt`, `.gitmodules`, and runtime resource lookups, but have not been
executed on a Windows machine. The MSYS2 package names and the `windeployqt6` step in
particular should be confirmed on a first run.
