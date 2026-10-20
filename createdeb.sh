#!/usr/bin/env bash
set -e  # stop on first error

# ============================================================================
# SedSAT3 Debian package script
# ============================================================================
# Builds SedSAT3 with CMake, creates the .deb package with CPack and writes a
# SHA-256 checksum file. Publishing is done separately, as a release on the
# USGS GitLab repository.
# ============================================================================

PROJECT_NAME="sedsat3"
BUILD_DIR="build"
VERSION="1.1.8"
SKIP_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --help)
            echo "Usage: ./createdeb.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --version X.Y.Z  Version used in the checksum file name (default: $VERSION)"
            echo "  --skip-build     Skip the build and only write checksums for an existing package"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run './createdeb.sh --help' for usage"
            exit 1
            ;;
    esac
done

if [ "$SKIP_BUILD" = false ]; then
    echo "=== [1/4] Cleaning old build directory ==="
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    echo "=== [2/4] Running CMake configuration ==="
    cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local/sedsat3

    echo "=== [3/4] Building project ==="
    cmake --build . --parallel

    echo "=== [4/4] Creating .deb package ==="
    cpack -G DEB

    cd ..
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: build directory not found. Run without --skip-build first."
    exit 1
fi

cd "$BUILD_DIR"

DEB_FILE=$(ls ${PROJECT_NAME}*.deb 2>/dev/null | grep -v "latest" | head -n 1)
if [ -z "$DEB_FILE" ]; then
    echo "Error: no .deb file found in $BUILD_DIR. Check the build output above."
    exit 1
fi

CHECKSUM_FILE="${PROJECT_NAME}_${VERSION}_checksums.txt"
sha256sum "$DEB_FILE" > "$CHECKSUM_FILE"

echo ""
echo "Build artifacts ready in $BUILD_DIR/:"
echo "  $DEB_FILE ($(du -h "$DEB_FILE" | cut -f1))"
echo "  $CHECKSUM_FILE"
echo ""
echo "Attach both files to a release on the USGS GitLab repository."
