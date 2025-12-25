# PIE Installation Guide

## Platform Support

PIE is designed for Linux systems with x86-64 CPUs. It uses Linux-specific features (perf, PMU counters) and LLVM.

**Supported platforms**:
- ✅ Ubuntu 20.04/22.04/24.04
- ✅ Debian 11/12
- ✅ Arch Linux
- ✅ Fedora 37+
- ⚠️ WSL2 (limited perf support)
- ❌ Windows native (not supported)
- ❌ macOS (perf not available)

## Quick Installation

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    llvm-14 \
    llvm-14-dev \
    clang-14 \
    python3 \
    python3-pip \
    linux-tools-common \
    linux-tools-generic \
    opencl-headers \
    ocl-icd-opencl-dev

# Optional: GPU support
# For NVIDIA: Install CUDA toolkit
# For AMD: Install ROCm
# For Intel: Install compute-runtime

# Clone repository
git clone https://github.com/yourusername/PIE.git
cd PIE

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install Python dependencies
cd ..
pip3 install --user -r requirements.txt

# Test installation
cd build
./pie info
```

### Arch Linux

```bash
# Install dependencies
sudo pacman -S cmake clang llvm python python-pip opencl-headers ocl-icd

# Clone and build
git clone https://github.com/yourusername/PIE.git
cd PIE
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test
./pie info
```

### Fedora

```bash
# Install dependencies
sudo dnf install cmake clang llvm-devel python3 python3-pip opencl-headers ocl-icd-devel

# Clone and build
git clone https://github.com/yourusername/PIE.git
cd PIE
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test
./pie info
```

## Detailed Installation

### Step 1: Install System Dependencies

#### Compiler and Build Tools

```bash
# GCC/G++ (required for building)
sudo apt install build-essential

# CMake (version 3.15+)
sudo apt install cmake

# Git (for cloning)
sudo apt install git
```

#### LLVM and Clang

PIE requires LLVM 14 or higher for the optimization passes.

```bash
# Ubuntu/Debian
sudo apt install llvm-14 llvm-14-dev clang-14

# Link to default
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-14 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-14 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-14 100

# Verify
llvm-config --version  # Should show 14.x or higher
```

#### Linux perf

```bash
# Ubuntu/Debian
sudo apt install linux-tools-common linux-tools-generic

# The exact package depends on your kernel version
# Check with: uname -r
# Install: sudo apt install linux-tools-$(uname -r)

# Verify
perf --version

# Allow unprivileged access (optional, for non-root profiling)
sudo sysctl kernel.perf_event_paranoid=0
```

#### OpenCL (Optional, for GPU support)

```bash
# Headers and ICD loader
sudo apt install opencl-headers ocl-icd-opencl-dev

# For NVIDIA GPUs
# Download and install CUDA toolkit from nvidia.com/cuda-downloads

# For AMD GPUs
# Install ROCm: https://rocmdocs.amd.com/

# For Intel GPUs
sudo apt install intel-opencl-icd
```

#### Python and Dependencies

```bash
# Python 3.8+
sudo apt install python3 python3-pip

# Virtual environment (recommended)
python3 -m venv venv
source venv/bin/activate

# Install packages
pip install -r requirements.txt
```

### Step 2: Clone PIE

```bash
git clone https://github.com/yourusername/PIE.git
cd PIE
```

### Step 3: Build PIE

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DLLVM_DIR=/usr/lib/llvm-14/cmake

# Build (use all CPU cores)
make -j$(nproc)

# Optional: Install system-wide
sudo make install
```

### Step 4: Test Installation

```bash
# Check PIE is working
./pie info

# Expected output:
# PIE System Information
# ======================
# Profiler: Available (perf)
# GPU: Available (OpenCL) or Not available
# [GPU info if available]

# Run a quick test
./pie profile matmul --size 128
```

### Step 5: Start Dashboard (Optional)

```bash
# From project root
cd dashboard
python3 server.py

# Open browser to http://localhost:5000
```

## Troubleshooting

### Issue: CMake can't find LLVM

**Error**: `Could not find LLVM (missing: LLVM_DIR)`

**Solution**:
```bash
# Find LLVM install location
llvm-config --cmakedir

# Use that path in cmake
cmake .. -DLLVM_DIR=$(llvm-config --cmakedir)
```

### Issue: perf command not found

**Error**: `perf: command not found`

**Solution**:
```bash
# Install linux-tools for your kernel
sudo apt install linux-tools-$(uname -r)

# If that fails, try generic
sudo apt install linux-tools-generic
```

### Issue: Permission denied when running perf

**Error**: `perf_event_open failed: Permission denied`

**Solution**:
```bash
# Temporary (until reboot)
sudo sysctl kernel.perf_event_paranoid=0

# Permanent
echo 'kernel.perf_event_paranoid=0' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

### Issue: OpenCL not found

**Error**: `GPU: Not available`

**Solution**:
```bash
# Check if OpenCL is installed
ls /usr/lib/x86_64-linux-gnu/libOpenCL.so*

# If missing, install ICD loader
sudo apt install ocl-icd-opencl-dev

# Install vendor-specific driver
# NVIDIA: CUDA toolkit
# AMD: ROCm
# Intel: intel-opencl-icd
```

### Issue: Build fails with "undefined reference to `LLVMContextCreate`"

**Error**: Linker errors with LLVM symbols

**Solution**:
```bash
# Make sure LLVM libraries are in linker path
export LD_LIBRARY_PATH=/usr/lib/llvm-14/lib:$LD_LIBRARY_PATH

# Rebuild
cd build
make clean
make -j$(nproc)
```

### Issue: Python packages fail to install

**Error**: `pip install` fails

**Solution**:
```bash
# Upgrade pip
python3 -m pip install --upgrade pip

# Install with user flag
pip3 install --user -r requirements.txt

# Or use virtual environment
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Verification

After installation, verify everything works:

```bash
cd build

# 1. Check system
./pie info

# 2. List workloads
./pie list

# 3. Run quick profile
./pie profile vector_add --size 1000

# 4. Run analysis
./pie analyze matmul --size 256

# 5. Start dashboard
cd ../dashboard
python3 server.py
```

All commands should complete without errors.

## Uninstallation

```bash
# If installed system-wide
sudo make uninstall

# Remove build directory
rm -rf build

# Remove Python packages
pip uninstall -r requirements.txt
```

## Docker Installation (Alternative)

For a containerized environment:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake build-essential llvm-14-dev clang-14 \
    python3 python3-pip linux-tools-generic \
    opencl-headers ocl-icd-opencl-dev

WORKDIR /pie
COPY . .

RUN mkdir build && cd build && cmake .. && make -j$(nproc)
RUN pip3 install -r requirements.txt

CMD ["/pie/build/pie", "info"]
```

```bash
# Build image
docker build -t pie .

# Run
docker run --privileged pie
```

Note: `--privileged` is needed for perf access.

## Next Steps

- Read [QUICKSTART.md](docs/QUICKSTART.md) for usage guide
- Check [examples/](examples/) for sample programs
- Explore [docs/](docs/) for detailed documentation
- Start optimizing! 🚀

