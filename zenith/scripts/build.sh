#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[zenith]${NC} $*"; }
warn()  { echo -e "${YELLOW}[zenith]${NC} $*"; }
error() { echo -e "${RED}[zenith] ERROR:${NC} $*"; exit 1; }

info "Fetching Wayland protocols..."
bash scripts/fetch-protocols.sh

info "Configuring with CMake..."
mkdir -p build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${PREFIX:-/usr/local}"

info "Building..."
make -j"$(nproc)"

info "Build successful."
info "Binary: build/zenith"
info "Run: sudo make install   (to install system-wide)"
info "Or:  ./build/zenith      (to run directly, needs DRM access)"
