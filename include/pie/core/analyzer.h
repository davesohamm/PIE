#ifndef PIE_CORE_ANALYZER_H
#define PIE_CORE_ANALYZER_H

#include "pie/core/types.h"
#include <memory>

namespace pie {

/**
 * @brief Performance analyzer and bottleneck detector
 * 
 * Analyzes hardware performance metrics to identify:
 * - Memory-bound workloads (high cache miss rates)
 * - Compute-bound workloads (instruction throughput limited)
 * - Control-flow-bound workloads (branch mispredictions)
 * 
 * Provides actionable optimization recommendations
 */
class Analyzer {
public:
    Analyzer();
    ~Analyzer();
    
    /**
     * @brief Analyze performance metrics and classify bottleneck
     * @param metrics Performance metrics from profiler
     * @return Analysis result with recommendations
     */
    AnalysisResult Analyze(const PerformanceMetrics& metrics);
    
    /**
     * @brief Compare two performance profiles
     * @param baseline Baseline metrics
     * @param optimized Optimized metrics
     * @return Detailed comparison and improvement analysis
     */
    std::string CompareMetrics(const PerformanceMetrics& baseline,
                               const PerformanceMetrics& optimized);
    
    /**
     * @brief Calculate speedup between two runs
     */
    double CalculateSpeedup(const PerformanceMetrics& baseline,
                           const PerformanceMetrics& optimized);
    
    /**
     * @brief Generate human-readable performance report
     */
    std::string GenerateReport(const PerformanceMetrics& metrics,
                              const AnalysisResult& analysis);
    
    /**
     * @brief Set thresholds for bottleneck detection
     */
    void SetThresholds(double memory_bound_cache_miss,
                      double compute_bound_ipc,
                      double control_flow_branch_miss);
    
private:
    // Thresholds for classification
    double memory_bound_threshold_{20.0};      // L2 cache miss rate %
    double compute_bound_ipc_threshold_{1.5};  // IPC threshold
    double control_flow_threshold_{5.0};       // Branch miss rate %
    
    // Classification logic
    BottleneckType ClassifyBottleneck(const PerformanceMetrics& metrics);
    std::vector<std::string> GenerateRecommendations(
        const PerformanceMetrics& metrics,
        BottleneckType bottleneck);
    
    // Scoring functions
    double MemoryBoundScore(const PerformanceMetrics& metrics);
    double ComputeBoundScore(const PerformanceMetrics& metrics);
    double ControlFlowBoundScore(const PerformanceMetrics& metrics);
};

} // namespace pie

#endif // PIE_CORE_ANALYZER_H

