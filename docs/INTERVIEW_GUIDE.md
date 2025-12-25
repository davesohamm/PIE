# PIE Interview Guide

## How to Present This Project

### Elevator Pitch (30 seconds)

> "I built a Performance Intelligence Engine that combines hardware profiling, architectural analysis, and compiler optimization. It profiles code using hardware counters, classifies bottlenecks as memory-bound, compute-bound, or control-flow-bound, then automatically applies LLVM optimization passes like cache tiling and SIMD vectorization. The system validates improvements in a closed-loop, achieving 2-5x speedups on real workloads."

### Deep Technical Explanation (5 minutes)

**Problem Statement**:
- Profilers give raw metrics, not insights
- Compilers use generic heuristics without hardware feedback
- No unified system connecting profiling → analysis → optimization

**Solution Architecture**:
1. **Profiler**: Uses Linux perf to collect PMU counters (IPC, cache misses, branch mispredictions)
2. **Analyzer**: Classifies bottlenecks using rule-based scoring
3. **LLVM Passes**: Custom optimization passes (cache tiling, vectorization)
4. **Optimizer**: Orchestrates closed-loop: profile → optimize → validate
5. **Comparator**: Benchmarks CPU vs GPU using OpenCL

**Technical Highlights**:
- Custom LLVM passes with loop analysis and IR transformation
- Hardware counter integration (perf_event_open)
- Dependency analysis for safe vectorization
- Cache-aware tile size computation
- Web dashboard with real-time monitoring

**Results**:
- Matrix multiplication: 2.5x speedup via cache tiling
- Vector operations: 4-8x speedup via SIMD
- Proven validation: before/after comparisons

## Expected Interview Questions

### Q1: "How does your profiler work?"

**Answer**:
"I use Linux perf to collect hardware performance counters via PMU. I wrap the target workload in a `perf stat` command that specifies events like cycles, instructions, cache-references, cache-misses, branches, and branch-misses. The output is parsed to extract metrics.

For in-process profiling, I use wall-clock timing with `chrono`. In production, I'd use libperf or PAPI for lower overhead.

The key insight is that hardware counters tell you **why** code is slow, not just that it's slow. For example, high L2 cache miss rate (>20%) + low IPC (<1.0) indicates memory-bound behavior."

**Follow-up**: "What about overhead?"
"Perf has ~1-2% overhead with event multiplexing. For production, I'd use sampling-based profiling instead of counting, which has <1% overhead."

### Q2: "How does cache tiling work?"

**Answer**:
"Cache tiling transforms nested loops to improve temporal and spatial locality by processing data in blocks that fit in cache.

**Algorithm**:
1. Detect nested loops using LLVM's LoopInfo
2. Analyze working set: N×N matrix uses 3 arrays (A, B, C), so 3×N²×sizeof(float) bytes
3. Compute tile size to fit in L1: tile = sqrt(L1_size / (3 × sizeof(float)))
4. Transform loop nest: add outer ii/jj loops that step by tile size

**IR Transformation**:
```
for i in [0..N):
  for j in [0..N):
    C[i][j] += A[i][k] * B[k][j]

→

for ii in [0..N) step TILE:
  for jj in [0..N) step TILE:
    for i in [ii..ii+TILE):
      for j in [jj..jj+TILE):
        C[i][j] += A[i][k] * B[k][j]
```

**Why it works**: Instead of streaming entire rows/columns (evicting cache), we process small tiles that stay resident in L1/L2."

**Follow-up**: "How do you choose tile size?"
"Based on cache capacity: L1 is 32KB on typical CPUs. For float matrices, tile = sqrt(32KB / 12B) ≈ 52. I round to cache line multiples (64B), so tile=48 or 64."

### Q3: "How do you ensure vectorization is safe?"

**Answer**:
"SIMD vectorization requires three safety checks:

1. **No loop-carried dependencies**: Use LLVM's DependenceAnalysis to check if iteration N depends on iteration N-1. Example of unsafe:
   ```cpp
   for (i = 1; i < N; ++i)
       A[i] = A[i-1] + 1;  // Can't vectorize
   ```

2. **Contiguous memory access**: Check if pointer arithmetic is affine with stride 1:
   ```cpp
   A[i]   ← vectorizable
   A[2*i] ← not vectorizable (stride 2)
   ```

3. **No aliasing**: Ensure pointers don't overlap. Use LLVM's AliasAnalysis or require `__restrict__` qualifier.

If all checks pass, I transform scalar operations to vector operations and generate tail handling for remainder elements."

**Follow-up**: "What about tail handling?"
"If N isn't divisible by vector width (8 for AVX2), I generate a scalar epilogue loop:
```cpp
for (i = 0; i < N/8*8; i += 8)  // Vectorized
    v_C[i] = v_A[i] + v_B[i];
for (; i < N; ++i)              // Scalar tail
    C[i] = A[i] + B[i];
```"

### Q4: "How do you classify bottlenecks?"

**Answer**:
"I use a scoring system based on hardware metrics:

**Memory-Bound Score**:
- L2 miss rate >30%: +0.4
- LLC miss rate >10%: +0.2
- Low IPC (<1.0): +0.2
- Max score: 1.0

**Compute-Bound Score**:
- High IPC (>1.5): +0.6
- Low cache misses (<5%): +0.4

**Control-Flow-Bound Score**:
- Branch miss rate >10%: +0.5
- Low IPC (<1.2): +0.3

The highest score determines primary bottleneck. Scores >0.5 in multiple categories → Mixed.

This is based on Intel VTune methodology and academic research (e.g., CPI stacks)."

### Q5: "What's the closed-loop part?"

**Answer**:
"Most tools either profile OR optimize. PIE does both and validates:

1. **Profile baseline**: Collect metrics
2. **Analyze**: Identify bottleneck
3. **Optimize**: Apply LLVM passes
4. **Recompile**: Generate optimized binary
5. **Re-profile**: Collect optimized metrics
6. **Validate**: Compare speedup

If speedup <5%, reject optimization. This prevents compiler passes from making things worse.

Example: Tiling helps matrix multiply but hurts vector addition (adds loop overhead). The closed loop catches this."

### Q6: "What did you learn from this project?"

**Answer**:
"Three key insights:

1. **Hardware matters more than algorithms**: A 2x speedup from cache tiling often beats algorithmic tricks. Modern CPUs are memory-bound, not compute-bound.

2. **Profiling is essential**: You can't optimize what you don't measure. Hardware counters reveal non-obvious bottlenecks.

3. **Compiler IR is powerful**: LLVM gives you architecture-independent optimization. Same pass works on x86, ARM, RISC-V.

**Technical growth**:
- Deep understanding of CPU microarchitecture (caches, pipelines, branch prediction)
- LLVM internals (IR, passes, analysis frameworks)
- Systems programming (perf integration, process management)
- Performance engineering mindset: measure, analyze, optimize, validate"

### Q7: "How would you extend this for production?"

**Answer**:
**Short-term**:
1. **Sampling profiler**: Replace perf stat with perf record for lower overhead
2. **Profile-guided optimization**: Use runtime profiles to guide compilation
3. **Auto-tuning**: Search tile sizes via reinforcement learning
4. **GPU power modeling**: Add energy efficiency metrics

**Long-term**:
1. **Cloud integration**: Distributed profiling across data centers
2. **MLIR backend**: Support for domain-specific optimizations
3. **NUMA awareness**: Optimize for multi-socket systems
4. **Real-time analysis**: Online performance regression detection

**Production challenges**:
- Multi-threaded profiling (need thread-aware counters)
- Security (perf requires privileges)
- Reproducibility (hardware variability)
- CI/CD integration (automated performance testing)"

## Whiteboard Exercises

### Exercise 1: Explain Cache Tiling

**Draw on whiteboard**:
```
Without Tiling:
[====A====][====B====][====C====]  Cache
   miss       miss       miss

With Tiling (64x64):
[A_tile][B_tile][C_tile]  Cache ← fits!
   hit     hit      hit
```

### Exercise 2: SIMD Diagram

```
Scalar:
for i:
  C[i] = A[i] + B[i]
  
Time: ████████████████  (16 cycles)

SIMD (AVX2, 8-wide):
for i += 8:
  C[i:i+8] = A[i:i+8] + B[i:i+8]
  
Time: ████  (2 cycles)
```

## Demo Script

If you can show the project live:

```bash
# 1. Show system info
./pie info

# 2. Profile workload
./pie analyze matmul --size 512

# 3. Highlight bottleneck classification
# Point to "Memory-Bound" output

# 4. Run optimization
./pie optimize matmul --size 512

# 5. Highlight speedup
# Point to "2.3x" improvement

# 6. Show dashboard
cd dashboard && python3 server.py
# Open browser, run interactive profiling
```

## Body Language & Delivery

1. **Enthusiasm**: Show passion for performance engineering
2. **Depth**: Ready to dive into technical details
3. **Clarity**: Explain complex concepts simply
4. **Confidence**: You built something impressive
5. **Humility**: Acknowledge limitations and future work

## Red Flags to Avoid

❌ "It's just a simple profiler"  
✅ "It's a complete performance engineering framework"

❌ "The LLVM passes don't fully work"  
✅ "The passes demonstrate the concept; production would need polyhedral analysis"

❌ "I didn't have time to..."  
✅ "Given more time, I'd add [specific enhancement]"

## Closing Statement

> "PIE demonstrates end-to-end systems thinking: from hardware counters to compiler IR to measurable performance gains. It's the kind of tool that performance teams actually use—and I can explain every line of code, every design decision, and every optimization technique."

**This project shows you understand how computers actually work.**

