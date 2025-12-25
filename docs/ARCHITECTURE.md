# PIE Architecture Documentation

## Overview

PIE (Performance Intelligence Engine) is designed as a modular, extensible performance optimization framework.

## Core Components

### 1. Profiler (`src/core/profiler.cpp`)

**Responsibility**: Collect hardware performance metrics

**Key Features**:
- Linux `perf` integration
- Hardware counter collection (PMU)
- Metrics: IPC, cache misses, branch mispredictions

**Interface**:
```cpp
PerformanceMetrics Profile(std::function<void()> workload);
PerformanceMetrics ProfileBinary(const std::string& binary_path);
```

### 2. Analyzer (`src/core/analyzer.cpp`)

**Responsibility**: Classify bottlenecks and generate recommendations

**Classification Logic**:
- **Memory-bound**: High cache miss rate (>20%), low IPC
- **Compute-bound**: High IPC (>1.5), low cache misses
- **Control-flow-bound**: High branch miss rate (>5%)

**Interface**:
```cpp
AnalysisResult Analyze(const PerformanceMetrics& metrics);
std::string CompareMetrics(baseline, optimized);
```

### 3. Optimizer (`src/core/optimizer.cpp`)

**Responsibility**: Orchestrate the optimization pipeline

**Pipeline**:
1. Compile baseline (`-O1`)
2. Profile baseline
3. Analyze bottlenecks
4. Select LLVM passes
5. Apply optimizations
6. Recompile (`-O3`)
7. Re-profile
8. Validate improvements

**Interface**:
```cpp
OptimizationResult Optimize(const std::string& source_file);
```

### 4. LLVM Passes (`src/llvm/`)

#### Cache Tiling Pass

**Algorithm**:
1. Detect nested loops
2. Analyze working set size
3. Compute optimal tile size:
   ```
   tile_size = sqrt(L1_cache / (3 * sizeof(float)))
   ```
4. Transform to tiled form

**IR Transformation**:
```llvm
; Before
for.body:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.body ]
  ...
  
; After
for.outer:
  %ii = phi i64 [ 0, %entry ], [ %ii.next, %for.outer ]
  for.inner:
    %i = phi i64 [ %ii, %for.outer ], [ %i.next, %for.inner ]
    ...
```

#### SIMD Vectorization Pass

**Safety Analysis**:
1. Check for loop-carried dependencies (using DependenceInfo)
2. Verify contiguous memory access
3. Ensure no aliasing

**Vectorization**:
- Vector width selection based on architecture
- Tail handling for non-divisible sizes
- Metadata for LLVM's vectorizer

### 5. GPU Executor (`src/gpu/executor.cpp`)

**Responsibility**: Execute workloads on GPU via OpenCL

**Features**:
- Device initialization
- Kernel compilation
- Memory management
- Timing measurement

**Interface**:
```cpp
double ExecuteKernel(const std::string& kernel_source,
                    const std::string& kernel_name,
                    size_t global_size);
```

## Data Flow

```
┌─────────────┐
│ Source Code │
└──────┬──────┘
       │
       ▼
┌─────────────┐     ┌──────────────┐
│ Compile (-O1)│────▶│ Baseline Bin │
└─────────────┘     └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   Profile    │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐     ┌─────────────┐
                    │   Analyze    │────▶│ Bottleneck  │
                    └──────────────┘     │   Memory    │
                                         └──────┬──────┘
                                                │
                    ┌───────────────────────────┘
                    │
                    ▼
            ┌───────────────┐
            │ Select Passes │
            │ - Cache Tiling│
            └───────┬───────┘
                    │
                    ▼
            ┌───────────────┐
            │  Apply LLVM   │
            │    Passes     │
            └───────┬───────┘
                    │
                    ▼
            ┌───────────────┐     ┌──────────────┐
            │ Compile (-O3) │────▶│ Optimized Bin│
            └───────────────┘     └──────┬───────┘
                                         │
                                         ▼
                                  ┌──────────────┐
                                  │  Re-Profile  │
                                  └──────┬───────┘
                                         │
                                         ▼
                                  ┌──────────────┐
                                  │   Validate   │
                                  │  Speedup: 2x │
                                  └──────────────┘
```

## Extension Points

### Adding New Workloads

1. Inherit from `Workload` base class
2. Implement required methods:
   - `Initialize()`
   - `Execute()`
   - `Verify()`
   - `GetGPUKernel()` (optional)

3. Register in `WorkloadFactory`

### Adding New LLVM Passes

1. Inherit from `llvm::PassInfoMixin<YourPass>`
2. Implement `run()` method
3. Register in `PassManager.cpp`

### Adding New Metrics

1. Extend `PerformanceMetrics` struct in `types.h`
2. Update `Profiler::ParsePerfOutput()`
3. Update `Analyzer::Analyze()` logic

## Performance Considerations

### Profiler Overhead

- Hardware counter collection: ~1-2% overhead
- Subprocess execution: Adds I/O overhead
- For production, use in-process profiling (libperf/PAPI)

### LLVM Pass Efficiency

- Tiling pass: O(n) where n = number of loops
- Vectorization pass: O(n*m) where m = instructions per loop
- Typical overhead: <100ms for most programs

### GPU Overhead

- Kernel compilation: 10-100ms
- Memory transfer: Dominates for small workloads
- Best for data-parallel workloads >1M elements

## Design Patterns

### Strategy Pattern

Used in workload selection and pass selection.

### Factory Pattern

`WorkloadFactory` creates workload instances.

### Template Method

`Workload` base class defines execution template.

### Observer Pattern

Dashboard observes optimization progress (future enhancement).

## Thread Safety

- **Profiler**: Not thread-safe (uses global perf state)
- **Analyzer**: Thread-safe (stateless)
- **Optimizer**: Not thread-safe (uses temporary files)
- **GPU Executor**: Thread-safe per instance

## Error Handling

- Profiling failures: Return empty metrics, log error
- Compilation failures: Return false, preserve original
- GPU unavailable: Graceful degradation to CPU-only

## Configuration

Configuration stored in `OptimizationConfig`:
- Cache sizes
- Vector width
- Optimization flags
- Target architecture

## Logging

Uses simple console logging. Future: structured logging with spdlog.

## Testing Strategy

1. **Unit tests**: Individual components
2. **Integration tests**: Full pipeline
3. **Performance tests**: Validate speedups
4. **Regression tests**: Ensure no performance loss

## Future Enhancements

1. **Adaptive Tuning**: ML-based pass selection
2. **Cloud Integration**: Distributed profiling
3. **Real-time Monitoring**: Live performance dashboard
4. **AutoFDO**: Profile-guided optimization
5. **NUMA Awareness**: Multi-socket optimization

