# PIE Examples

This directory contains example programs demonstrating performance optimization techniques.

## Examples

### 1. Simple Matrix Multiplication (`simple_matmul.cpp`)

Naive matrix multiplication with poor cache locality.

```bash
g++ -O2 simple_matmul.cpp -o simple_matmul
./simple_matmul 512
```

### 2. Optimized Matrix Multiplication (`optimized_matmul.cpp`)

Cache-tiled matrix multiplication for better locality.

```bash
g++ -O2 optimized_matmul.cpp -o optimized_matmul
./optimized_matmul 512 64
```

**Compare the two:**
```bash
time ./simple_matmul 1024
time ./optimized_matmul 1024 64
```

You should see 2-3x speedup with tiling!

### 3. Vector SIMD (`vector_simd.cpp`)

Demonstrates SIMD vectorization with AVX2.

```bash
g++ -O2 -mavx2 vector_simd.cpp -o vector_simd
./vector_simd 10000000
```

## Using with PIE

Profile any example with PIE:

```bash
# Generate LLVM IR
clang++ -S -emit-llvm -O1 simple_matmul.cpp -o simple_matmul.ll

# Apply PIE optimizations
opt -load ../build/lib/pie/PIEPasses.so \
    -passes="pie-optimize" \
    simple_matmul.ll -o optimized.ll

# Compile optimized version
clang++ -O3 optimized.ll -o optimized_binary

# Compare
perf stat ./simple_matmul 512
perf stat ./optimized_binary 512
```

## Profiling

Use PIE's profiler:

```bash
../build/pie profile matmul --size 1024
../build/pie analyze matmul --size 1024
../build/pie optimize matmul --size 1024
```

## Expected Results

| Example | Bottleneck | Optimization | Expected Speedup |
|---------|-----------|--------------|------------------|
| simple_matmul | Memory-bound | Cache tiling | 2-3x |
| vector_simd | Compute-bound | SIMD (AVX2) | 4-8x |

## Tips

1. **Always profile first** - Don't optimize blindly
2. **Understand bottlenecks** - Use PIE's analysis
3. **Validate improvements** - Measure before/after
4. **Consider hardware** - Different CPUs, different results

