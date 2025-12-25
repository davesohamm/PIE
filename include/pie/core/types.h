#ifndef PIE_CORE_TYPES_H
#define PIE_CORE_TYPES_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>

namespace pie {

// Performance metrics collected from hardware counters
struct PerformanceMetrics {
    uint64_t cycles{0};
    uint64_t instructions{0};
    uint64_t l1_dcache_loads{0};
    uint64_t l1_dcache_load_misses{0};
    uint64_t l2_cache_references{0};
    uint64_t l2_cache_misses{0};
    uint64_t llc_references{0};
    uint64_t llc_misses{0};
    uint64_t branches{0};
    uint64_t branch_misses{0};
    uint64_t mem_loads{0};
    uint64_t mem_stores{0};
    double wall_time_ms{0.0};
    double cpu_time_ms{0.0};
    
    // Derived metrics
    double ipc() const {
        return cycles > 0 ? static_cast<double>(instructions) / cycles : 0.0;
    }
    
    double l1_miss_rate() const {
        return l1_dcache_loads > 0 ? 
            static_cast<double>(l1_dcache_load_misses) / l1_dcache_loads * 100.0 : 0.0;
    }
    
    double l2_miss_rate() const {
        return l2_cache_references > 0 ?
            static_cast<double>(l2_cache_misses) / l2_cache_references * 100.0 : 0.0;
    }
    
    double llc_miss_rate() const {
        return llc_references > 0 ?
            static_cast<double>(llc_misses) / llc_references * 100.0 : 0.0;
    }
    
    double branch_miss_rate() const {
        return branches > 0 ?
            static_cast<double>(branch_misses) / branches * 100.0 : 0.0;
    }
    
    double gflops() const {
        return wall_time_ms > 0 ? 
            (instructions / wall_time_ms) / 1e6 : 0.0;
    }
};

// Bottleneck classification
enum class BottleneckType {
    UNKNOWN,
    MEMORY_BOUND,
    COMPUTE_BOUND,
    CONTROL_FLOW_BOUND,
    MIXED
};

inline std::string to_string(BottleneckType type) {
    switch (type) {
        case BottleneckType::MEMORY_BOUND: return "Memory-Bound";
        case BottleneckType::COMPUTE_BOUND: return "Compute-Bound";
        case BottleneckType::CONTROL_FLOW_BOUND: return "Control-Flow-Bound";
        case BottleneckType::MIXED: return "Mixed";
        default: return "Unknown";
    }
}

// Analysis result
struct AnalysisResult {
    BottleneckType bottleneck{BottleneckType::UNKNOWN};
    std::vector<std::string> recommendations;
    std::map<std::string, double> scores;
    std::string detailed_analysis;
};

// Optimization configuration
struct OptimizationConfig {
    bool enable_cache_tiling{true};
    bool enable_simd_vectorization{true};
    bool enable_loop_unrolling{false};
    bool enable_prefetching{false};
    int cache_line_size{64};
    int l1_cache_size{32 * 1024};       // 32 KB
    int l2_cache_size{256 * 1024};      // 256 KB
    int llc_cache_size{8 * 1024 * 1024}; // 8 MB
    int vector_width{256};               // AVX2 default
    std::string target_arch{"native"};
};

// Workload configuration
struct WorkloadConfig {
    std::string name;
    std::string type;
    std::map<std::string, int> params;
    int iterations{1};
    bool warmup{true};
};

// Comparison result (CPU vs GPU)
struct ComparisonResult {
    PerformanceMetrics cpu_metrics;
    PerformanceMetrics gpu_metrics;
    double speedup{0.0};
    std::string winner;
    std::string analysis;
};

// Optimization result
struct OptimizationResult {
    bool success{false};
    PerformanceMetrics baseline_metrics;
    PerformanceMetrics optimized_metrics;
    double speedup{0.0};
    std::vector<std::string> applied_optimizations;
    std::string analysis;
    std::map<std::string, double> improvement_breakdown;
};

} // namespace pie

#endif // PIE_CORE_TYPES_H

