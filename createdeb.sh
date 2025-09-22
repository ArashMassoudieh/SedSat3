#!/usr/bin/env bash
set -e  # stop on first error

# ----------------------------
# Config
# ----------------------------
PROJECT_NAME="sedsat3"
BUILD_DIR="build"

# ----------------------------
# Step 1: Clean build folder
# ----------------------------
echo "[1/4] Cleaning old build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ----------------------------
# Step 2: Configure CMake
# ----------------------------
echo "[2/4] Running CMake configuration..."
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/sedsat3

# ----------------------------
# Step 3: Build
# ----------------------------
echo "[3/4] Building project..."
cmake --build . --parallel

# ----------------------------
# Step 4: Create .deb package
# ----------------------------
echo "[4/4] Creating Debian package..."
cpack -G DEB

echo
echo "✅ Done! Look in $BUILD_DIR/ for your .deb package."

