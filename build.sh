#!/bin/bash
# PIE Build Script

set -e

echo "🚀 Building PIE - Performance Intelligence Engine"
echo "=================================================="

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check dependencies
echo -e "\n${YELLOW}[1/5] Checking dependencies...${NC}"

check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} $1"
        return 0
    else
        echo -e "  ${RED}✗${NC} $1 (missing)"
        return 1
    fi
}

MISSING=0

check_command cmake || MISSING=1
check_command clang++ || MISSING=1
check_command python3 || MISSING=1
check_command perf || echo -e "  ${YELLOW}⚠${NC} perf (optional, for profiling)"

# Check LLVM
if llvm-config --version &> /dev/null; then
    LLVM_VERSION=$(llvm-config --version)
    echo -e "  ${GREEN}✓${NC} LLVM ($LLVM_VERSION)"
else
    echo -e "  ${RED}✗${NC} LLVM (required)"
    MISSING=1
fi

# Check OpenCL
if [ -f "/usr/include/CL/cl.h" ] || [ -f "/usr/local/include/CL/cl.h" ]; then
    echo -e "  ${GREEN}✓${NC} OpenCL (GPU support enabled)"
else
    echo -e "  ${YELLOW}⚠${NC} OpenCL (optional, for GPU comparison)"
fi

if [ $MISSING -eq 1 ]; then
    echo -e "\n${RED}Error: Missing required dependencies${NC}"
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt install cmake clang llvm-14-dev python3"
    exit 1
fi

# Create build directory
echo -e "\n${YELLOW}[2/5] Creating build directory...${NC}"
mkdir -p build
cd build

# Configure
echo -e "\n${YELLOW}[3/5] Configuring CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "\n${YELLOW}[4/5] Building PIE...${NC}"
make -j$(nproc)

# Install Python dependencies
echo -e "\n${YELLOW}[5/5] Installing Python dependencies...${NC}"
cd ..
pip3 install --user -r requirements.txt 2>/dev/null || echo "  (skipped)"

# Success
echo -e "\n${GREEN}✓ Build successful!${NC}"
echo ""
echo "Next steps:"
echo "  1. Test the build:"
echo "     ${GREEN}cd build && ./pie info${NC}"
echo ""
echo "  2. Run an example:"
echo "     ${GREEN}./pie profile matmul --size 1024${NC}"
echo ""
echo "  3. Start the dashboard:"
echo "     ${GREEN}cd dashboard && python3 server.py${NC}"
echo ""
echo "Documentation: docs/"
echo "Examples: examples/"
echo ""
echo "Happy optimizing! 🚀"

