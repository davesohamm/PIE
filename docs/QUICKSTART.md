# PIE Quick Start Guide

## Installation

### Prerequisites

```bash
# Ubuntu/Debian
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

# Install Python dependencies
pip3 install -r requirements.txt
```

### Build PIE

```bash
git clone https://github.com/yourusername/PIE.git
cd PIE
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## First Steps

### 1. Check System Compatibility

```bash
./pie info
```

Expected output:
```
PIE System Information
======================

Profiler: Available (perf)
GPU: Available (OpenCL)

Device: NVIDIA GeForce RTX 3080
Vendor: NVIDIA Corporation
Compute Units: 68
Global Memory: 10240 MB
```

### 2. List Available Workloads

```bash
./pie list
```

### 3. Profile a Workload

```bash
./pie profile matmul --size 1024
```

This will:
- Execute matrix multiplication (1024×1024)
- Collect hardware performance counters
- Display metrics (IPC, cache misses, etc.)

### 4. Analyze Bottlenecks

```bash
./pie analyze matmul --size 1024
```

This will:
- Profile the workload
- Classify bottleneck type
- Provide specific recommendations

Example output:
```
=== Analyzing matmul ===

Performance Analysis:
====================

Basic Metrics:
  IPC: 0.68
  Cycles: 15438921
  Instructions: 10498432
  Wall Time: 2143.23 ms

Memory Hierarchy:
  L1 Miss Rate: 31.24%
  L2 Miss Rate: 18.73%
  LLC Miss Rate: 7.42%

Bottleneck Classification:
  Primary: Memory-Bound
  Memory-Bound Score: 0.73
  Compute-Bound Score: 0.21
  Control-Flow-Bound Score: 0.08

Recommendations:
================
1. Apply cache-aware loop tiling to improve locality
2. Consider blocking algorithms to fit in L2 cache
3. Use software prefetching for predictable access patterns
```

### 5. Run Full Optimization

```bash
./pie optimize matmul --size 1024
```

This will:
1. Profile baseline
2. Analyze bottlenecks
3. Apply LLVM optimization passes
4. Recompile optimized version
5. Re-profile and compare

Example output:
```
=== PIE Optimization Pipeline ===
Source: /tmp/pie_workload_matmul.cpp

[1/5] Profiling baseline...
[2/5] Analyzing bottlenecks...
Bottleneck: Memory-Bound
Recommendations:
  - Apply cache-aware loop tiling to improve locality

[3/5] Applying compiler optimizations...
Running: opt -mem2reg -instcombine -loop-vectorize ...

[4/5] Profiling optimized version...
[5/5] Validating improvements...

Performance Comparison:
======================
Overall Speedup: 2.46x

Metric                Baseline    Optimized   Improvement
------------------------------------------------------------
IPC                       0.68        1.52         2.24x
Time (ms)              2143.23      871.45         2.46x
Cycles               15438921     6284732         2.46x
L1 Miss Rate (%)        31.24         8.72         3.58x
L2 Miss Rate (%)        18.73         4.13         4.54x

✓ Optimization successful!
Speedup: 2.46x
```

### 6. Compare CPU vs GPU

```bash
./pie compare matmul --size 512
```

This will:
- Execute workload on CPU
- Execute same workload on GPU (OpenCL)
- Compare performance

## Using the Dashboard

### Start the Dashboard Server

```bash
cd dashboard
python3 server.py
```

### Access Dashboard

Open your browser to: `http://localhost:5000`

The dashboard provides:
- Real-time system monitoring
- Interactive workload selection
- Visual performance comparisons
- Result history

## Example Workflow

### Optimize Your Own Code

```bash
# 1. Write your code
cat > mycode.cpp << 'EOF'
#include <vector>

void compute(std::vector<float>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = data[i] * 2.0f + 1.0f;
    }
}

int main() {
    std::vector<float> data(10000000, 1.0f);
    for (int iter = 0; iter < 100; ++iter) {
        compute(data);
    }
    return 0;
}
EOF

# 2. Compile baseline
clang++ -O1 mycode.cpp -o mycode_baseline

# 3. Profile baseline
perf stat -e cycles,instructions,cache-misses,branches,branch-misses \
    ./mycode_baseline

# 4. Generate LLVM IR
clang++ -S -emit-llvm -O1 mycode.cpp -o mycode.ll

# 5. Apply PIE optimizations
opt -load ./build/lib/pie/PIEPasses.so \
    -passes="pie-optimize" \
    mycode.ll -o mycode_optimized.ll

# 6. Compile optimized
clang++ -O3 mycode_optimized.ll -o mycode_optimized

# 7. Profile optimized
perf stat -e cycles,instructions,cache-misses,branches,branch-misses \
    ./mycode_optimized

# 8. Compare
echo "Baseline:"
time ./mycode_baseline
echo "Optimized:"
time ./mycode_optimized
```

## Common Issues

### Issue: "perf not found"

**Solution**: Install linux-tools
```bash
sudo apt install linux-tools-common linux-tools-generic
```

### Issue: "Permission denied" when running perf

**Solution**: Allow unprivileged perf access
```bash
sudo sysctl kernel.perf_event_paranoid=0
```

### Issue: "OpenCL not found"

**Solution**: Install OpenCL ICD and headers
```bash
sudo apt install ocl-icd-opencl-dev opencl-headers
```

For NVIDIA GPUs, install CUDA toolkit.
For AMD GPUs, install ROCm.
For Intel GPUs, install compute-runtime.

### Issue: "LLVM version mismatch"

**Solution**: Ensure LLVM 14+ is installed
```bash
sudo apt install llvm-14-dev clang-14
export LLVM_DIR=/usr/lib/llvm-14/cmake
cmake .. -DLLVM_DIR=$LLVM_DIR
```

## Next Steps

- Read [ARCHITECTURE.md](ARCHITECTURE.md) for design details
- Check [examples/](../examples/) for sample programs
- Explore LLVM passes in [src/llvm/](../src/llvm/)
- Contribute optimizations!

## Tips

1. **Start small**: Profile with small sizes first
2. **Measure twice**: Run multiple iterations for stable results
3. **Understand first**: Analyze bottlenecks before optimizing
4. **Validate always**: Verify that optimizations actually help
5. **Learn hardware**: Understanding CPU architecture is key

## Performance Expectations

| Workload | Typical Speedup | Best Case | Worst Case |
|----------|----------------|-----------|------------|
| Matrix Multiplication | 2-3x | 5x | 1.1x |
| Vector Operations | 4-8x | 16x | 1.5x |
| Memory Streaming | 1.2-1.5x | 2x | 1x |
| Branch Heavy | 1.1-1.3x | 2x | 0.9x |

Note: Results depend heavily on:
- CPU architecture
- Cache sizes
- Memory bandwidth
- Problem size
- Compiler version

## Support

- GitHub Issues: Report bugs and feature requests
- Documentation: Check docs/ directory
- Examples: See examples/ directory

