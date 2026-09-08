#!/usr/bin/env bash
set -e  # stop on first error

# ============================================================================
# SedSat3 Build and Release Script
# ============================================================================
# This script:
# 1. Cleans and builds the project
# 2. Creates .deb packages
# 3. Generates checksums
# 4. Creates git tag
# 5. Creates GitHub release
# 6. Uploads all files
# ============================================================================

# ----------------------------
# Config
# ----------------------------
PROJECT_NAME="sedsat3"
BUILD_DIR="build"
VERSION="1.1.6"
GITHUB_REPO="ArashMassoudieh/SedSat3"

# ----------------------------
# Parse command line arguments
# ----------------------------
PUBLISH_TO_GITHUB=false
SKIP_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --publish)
            PUBLISH_TO_GITHUB=true
            shift
            ;;
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
            echo "  --publish        Create GitHub release and upload files"
            echo "  --version X.Y.Z  Set version number (default: $VERSION)"
            echo "  --skip-build     Skip build, just upload existing files"
            echo "  --help           Show this help message"
            echo ""
            echo "Examples:"
            echo "  ./createdeb.sh                    # Just build locally"
            echo "  ./createdeb.sh --publish          # Build and release to GitHub"
            echo "  ./createdeb.sh --skip-build --publish  # Upload existing build to GitHub"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run './createdeb.sh --help' for usage"
            exit 1
            ;;
    esac
done

TAG="v${VERSION}"

# ----------------------------
# Pre-flight checks
# ----------------------------
if [ "$PUBLISH_TO_GITHUB" = true ]; then
    echo "=== Pre-flight Checks ==="
    
    # Check if gh CLI is installed
    if ! command -v gh &> /dev/null; then
        echo "❌ Error: GitHub CLI (gh) is not installed!"
        echo "Install it with: sudo apt install gh"
        echo "Or visit: https://cli.github.com/"
        exit 1
    fi
    
    # Check if authenticated
    if ! gh auth status &> /dev/null; then
        echo "❌ Error: Not authenticated with GitHub CLI"
        echo "Run: gh auth login"
        exit 1
    fi
    
    echo "✅ GitHub CLI authenticated"
fi

# ----------------------------
# Build Process
# ----------------------------
if [ "$SKIP_BUILD" = false ]; then
    echo ""
    echo "=== [1/7] Cleaning old build directory ==="
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    echo ""
    echo "=== [2/7] Running CMake configuration ==="
    cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local/sedsat3

    echo ""
    echo "=== [3/7] Building project ==="
    cmake --build . --parallel

    echo ""
    echo "=== [4/7] Creating .deb package ==="
    cpack -G DEB

    cd ..
else
    echo ""
    echo "=== Skipping build (using existing files) ==="
fi

# ----------------------------
# Verify build files exist
# ----------------------------
echo ""
echo "=== [5/7] Verifying build artifacts ==="

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: build directory not found!"
    echo "Run without --skip-build first"
    exit 1
fi

cd "$BUILD_DIR"

# Find the generated .deb file
DEB_FILE=$(ls ${PROJECT_NAME}*.deb 2>/dev/null | grep -v "latest" | head -n 1)
if [ -z "$DEB_FILE" ]; then
    echo "❌ Error: No .deb file found in $BUILD_DIR!"
    echo "Build may have failed. Check the output above."
    exit 1
fi

echo "✅ Found: $DEB_FILE"

# Create generic filename
GENERIC_NAME="${PROJECT_NAME}-latest-Linux.deb"
if [ ! -f "$GENERIC_NAME" ]; then
    cp "$DEB_FILE" "$GENERIC_NAME"
    echo "✅ Created: $GENERIC_NAME"
fi

# Create checksums
CHECKSUM_FILE="${PROJECT_NAME}_${VERSION}_checksums.txt"
sha256sum "$DEB_FILE" "$GENERIC_NAME" > "$CHECKSUM_FILE"
echo "✅ Created: $CHECKSUM_FILE"

# Show file info
echo ""
echo "Build artifacts ready:"
echo "  📦 $DEB_FILE ($(du -h "$DEB_FILE" | cut -f1))"
echo "  📦 $GENERIC_NAME ($(du -h "$GENERIC_NAME" | cut -f1))"
echo "  🔐 $CHECKSUM_FILE"

cd ..

# ----------------------------
# Publish to GitHub
# ----------------------------
if [ "$PUBLISH_TO_GITHUB" = true ]; then
    echo ""
    echo "=== [6/7] Creating GitHub Release ==="
    
    # Check if tag already exists
    if git rev-parse "$TAG" >/dev/null 2>&1; then
        echo "⚠️  Tag $TAG already exists locally"
        read -p "Delete and recreate? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Deleting local tag..."
            git tag -d "$TAG"
            echo "Deleting remote tag..."
            git push origin --delete "$TAG" 2>/dev/null || echo "Remote tag doesn't exist or already deleted"
            
            # Check if release exists and delete it
            if gh release view "$TAG" --repo "$GITHUB_REPO" &>/dev/null; then
                echo "Deleting existing release..."
                gh release delete "$TAG" --repo "$GITHUB_REPO" --yes
            fi
        else
            echo "❌ Aborted. Use a different version number or delete the tag manually."
            exit 1
        fi
    fi
    
    # Create and push tag
    echo "Creating tag $TAG..."
    git tag -a "$TAG" -m "Release version $VERSION"
    
    echo "Pushing tag to GitHub..."
    git push origin "$TAG"
    
    echo "✅ Tag created and pushed"
    
    # Create release with files
    echo ""
    echo "=== [7/7] Uploading to GitHub ==="
    
    cd "$BUILD_DIR"
    
    echo "Creating release..."
    gh release create "$TAG" \
        --repo "$GITHUB_REPO" \
        --title "SedSat3 v${VERSION}" \
        --notes "## SedSat3 v${VERSION}

### Quick Install

\`\`\`bash
wget https://github.com/${GITHUB_REPO}/releases/latest/download/${GENERIC_NAME}
sudo dpkg -i ${GENERIC_NAME}
sudo apt-get install -f
\`\`\`

### One-liner Install

\`\`\`bash
wget https://github.com/${GITHUB_REPO}/releases/latest/download/${GENERIC_NAME} && sudo dpkg -i ${GENERIC_NAME} && sudo apt-get install -f
\`\`\`

### Requirements

- Qt 6.2+
- Armadillo
- GSL
- OpenMP

### Package Information

- **Versioned Package**: \`${DEB_FILE}\` - Specific version for archival
- **Latest Package**: \`${GENERIC_NAME}\` - Always points to latest version
- **Checksums**: \`${CHECKSUM_FILE}\` - SHA256 verification

### Verification

\`\`\`bash
# Download checksum file
wget https://github.com/${GITHUB_REPO}/releases/download/${TAG}/${CHECKSUM_FILE}

# Verify downloaded package
sha256sum -c ${CHECKSUM_FILE}
\`\`\`

---

**Full Changelog**: https://github.com/${GITHUB_REPO}/compare/v1.1.5...${TAG}" \
        "$DEB_FILE" \
        "$GENERIC_NAME" \
        "$CHECKSUM_FILE"
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Release published successfully!"
        echo ""
        echo "🎉 All done! Your release is live at:"
        echo "   https://github.com/${GITHUB_REPO}/releases/tag/$TAG"
        echo ""
        echo "📥 Users can install with:"
        echo "   wget https://github.com/${GITHUB_REPO}/releases/latest/download/${GENERIC_NAME}"
        echo "   sudo dpkg -i ${GENERIC_NAME}"
        echo "   sudo apt-get install -f"
    else
        echo ""
        echo "❌ Error: Failed to create release or upload files"
        echo "Check the error messages above"
        exit 1
    fi
    
    cd ..
else
    echo ""
    echo "=== [6/7] Skipping GitHub publish ==="
    echo ""
    echo "✅ Build complete! Files are in $BUILD_DIR/"
    echo ""
    echo "To publish to GitHub, run:"
    echo "   ./createdeb.sh --publish"
    echo ""
    echo "To just upload existing files without rebuilding:"
    echo "   ./createdeb.sh --skip-build --publish"
fi

echo ""
echo "================================================"
echo "Summary:"
echo "================================================"
echo "Version:    $VERSION"
echo "Tag:        $TAG"
echo "Files:"
echo "  - $BUILD_DIR/$DEB_FILE"
echo "  - $BUILD_DIR/$GENERIC_NAME"
echo "  - $BUILD_DIR/$CHECKSUM_FILE"
if [ "$PUBLISH_TO_GITHUB" = true ]; then
    echo "Status:     Published to GitHub ✅"
    echo "URL:        https://github.com/${GITHUB_REPO}/releases/tag/$TAG"
else
    echo "Status:     Built locally (not published)"
fi
echo "================================================"
