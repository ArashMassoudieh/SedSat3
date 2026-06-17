#!/usr/bin/env bash
# createdmg.sh — Build a distributable macOS .dmg for SedSat3
#
# Usage: ./createdmg.sh
# Run from the project root (the folder containing this script).
#
# Prerequisites:
#   - The Release build must already exist at BUILD_DIR/SedSat3.app
#     (build it in Qt Creator first, or via qmake + make Release)
#   - Homebrew packages installed: armadillo, gsl, libomp, gcc (for libgfortran/libgcc_s)
#
# What this script does:
#   1. Validate prerequisites
#   2. Bundle Qt + Homebrew dependencies via macdeployqt
#   3. Patch transitive dependencies macdeployqt misses (libgcc_s etc.)
#   4. Verify no unbundled dependencies remain
#   5. Ad-hoc code-sign the bundle
#   6. Package as a versioned .dmg

set -e   # exit on first error
set -u   # error on unset variables

# ----------------------------
# Configuration
# ----------------------------
PROJECT_NAME="SedSat3"
VERSION="1.1.6"
BUILD_DIR="bin/Release"
QT_BIN="/Users/arash/Qt/6.10.1/macos/bin"
APP_BUNDLE="${BUILD_DIR}/${PROJECT_NAME}.app"
FRAMEWORKS_DIR="${APP_BUNDLE}/Contents/Frameworks"
DMG_NAME="${PROJECT_NAME}-${VERSION}.dmg"
DMG_PATH="${BUILD_DIR}/${DMG_NAME}"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'   # No Color

info()    { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# ----------------------------
# Step 1: Validate prerequisites
# ----------------------------
info "[1/6] Validating prerequisites..."

if [ ! -d "$APP_BUNDLE" ]; then
    error "App bundle not found at $APP_BUNDLE"
    error "Build the Release configuration in Qt Creator first."
    exit 1
fi

if [ ! -x "${QT_BIN}/macdeployqt" ]; then
    error "macdeployqt not found at ${QT_BIN}/macdeployqt"
    error "Update QT_BIN in this script to match your Qt installation."
    exit 1
fi

# Verify resources landed correctly during the qmake build
if [ ! -d "${APP_BUNDLE}/Contents/Resources" ] || \
   [ ! -f "${APP_BUNDLE}/Contents/Resources/tools.json" ]; then
    error "Resources missing from ${APP_BUNDLE}/Contents/Resources/"
    error "Check that COPY_RESOURCES in SedSat3.pro is configured correctly."
    exit 1
fi

# Warn if a stale resources/ folder exists at the bundle root
if [ -d "${APP_BUNDLE}/resources" ]; then
    warn "Stale resources/ folder at bundle root — removing it"
    rm -rf "${APP_BUNDLE}/resources"
fi

info "Prerequisites OK."

# ----------------------------
# Step 2: Run macdeployqt
# ----------------------------
info "[2/6] Bundling Qt frameworks and Homebrew dylibs..."

# Remove any prior signature first; macdeployqt-modified bundles always need re-signing
codesign --remove-signature "$APP_BUNDLE" 2>/dev/null || true

"${QT_BIN}/macdeployqt" "$APP_BUNDLE"

# ----------------------------
# Step 3: Patch transitive dependencies macdeployqt misses
# ----------------------------
info "[3/6] Patching transitive dependencies..."

# libgcc_s.1.1.dylib — required by libgfortran (transitively by Armadillo via OpenBLAS)
# macdeployqt typically misses this one.
patch_libgcc_s() {
    local target_lib="libgcc_s.1.1.dylib"
    local dest="${FRAMEWORKS_DIR}/${target_lib}"

    if [ -f "$dest" ]; then
        info "  $target_lib already bundled — skipping copy"
        return
    fi

    local src
    src=$(find /opt/homebrew -name "$target_lib" 2>/dev/null | head -1)

    if [ -z "$src" ]; then
        warn "  $target_lib not found on system — skipping (app may fail to launch)"
        return
    fi

    info "  Bundling $target_lib from $src"
    cp "$src" "$dest"

    # Make the library's own install name point to its bundle location
    install_name_tool -id "@executable_path/../Frameworks/${target_lib}" "$dest"

    # Fix any reference in libgfortran (and other bundled libs) to use the bundled copy
    for lib in "${FRAMEWORKS_DIR}"/*.dylib; do
        if otool -L "$lib" 2>/dev/null | grep -q "@rpath/${target_lib}"; then
            info "    Rewriting ${target_lib} reference in $(basename "$lib")"
            install_name_tool -change "@rpath/${target_lib}" \
                "@executable_path/../Frameworks/${target_lib}" \
                "$lib"
        fi
    done
}

patch_libgcc_s

# ----------------------------
# Step 4: Verify no unbundled dependencies remain
# ----------------------------
info "[4/6] Verifying bundled dependencies..."

UNBUNDLED=0
check_lib() {
    local lib="$1"
    local bad
    # Anything pointing to @rpath, /opt/homebrew, or /usr/local is unbundled
    bad=$(otool -L "$lib" 2>/dev/null | tail -n +2 | \
        grep -E "@rpath|/opt/homebrew|/usr/local" || true)
    if [ -n "$bad" ]; then
        warn "Unbundled dependencies in $(basename "$lib"):"
        echo "$bad" | sed 's/^/    /'
        UNBUNDLED=$((UNBUNDLED + 1))
    fi
}

# Check the main executable
check_lib "${APP_BUNDLE}/Contents/MacOS/${PROJECT_NAME}"

# Check every bundled dylib
for lib in "${FRAMEWORKS_DIR}"/*.dylib; do
    [ -f "$lib" ] && check_lib "$lib"
done

if [ "$UNBUNDLED" -gt 0 ]; then
    warn "$UNBUNDLED libraries have unbundled dependencies."
    warn "The DMG may fail to launch on machines without Homebrew."
    warn "Continuing anyway — fix manually if testing fails."
else
    info "All dependencies properly bundled."
fi

# ----------------------------
# Step 5: Code sign (ad-hoc)
# ----------------------------
info "[5/6] Ad-hoc signing bundle..."

codesign --force --deep --sign - "$APP_BUNDLE"

if ! codesign --verify --verbose "$APP_BUNDLE" 2>&1 | grep -q "valid on disk"; then
    # Some macOS versions print "valid on disk", others print nothing on success.
    # Re-check explicitly.
    if ! codesign --verify "$APP_BUNDLE"; then
        error "Code signing verification failed."
        exit 1
    fi
fi

info "Signature verified."

# ----------------------------
# Step 6: Build the DMG
# ----------------------------
info "[6/6] Building DMG: ${DMG_NAME}..."

rm -f "$DMG_PATH"

hdiutil create \
    -volname "${PROJECT_NAME}" \
    -srcfolder "$APP_BUNDLE" \
    -ov \
    -format UDZO \
    "$DMG_PATH" >/dev/null

info ""
info "✅ Done! DMG created at:"
info "   $(cd "$BUILD_DIR" && pwd)/${DMG_NAME}"
info ""
info "Next steps:"
info "  1. Test by mounting the DMG and dragging to /Applications"
info "  2. Run: xattr -cr /Applications/${PROJECT_NAME}.app  (clear quarantine)"
info "  3. Launch: open /Applications/${PROJECT_NAME}.app"
info ""
info "Recipients should right-click → Open the first time (ad-hoc signature)."