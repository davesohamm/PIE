#include "pie/core/analyzer.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace pie {

Analyzer::Analyzer() = default;
Analyzer::~Analyzer() = default;

AnalysisResult Analyzer::Analyze(const PerformanceMetrics& metrics) {
    AnalysisResult result;
    
    // Classify bottleneck
    result.bottleneck = ClassifyBottleneck(metrics);
    
    // Calculate scores
    result.scores["memory_bound"] = MemoryBoundScore(metrics);
    result.scores["compute_bound"] = ComputeBoundScore(metrics);
    result.scores["control_flow_bound"] = ControlFlowBoundScore(metrics);
    
    // Generate recommendations
    result.recommendations = GenerateRecommendations(metrics, result.bottleneck);
    
    // Detailed analysis
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Performance Analysis:\n";
    oss << "====================\n\n";
    oss << "Basic Metrics:\n";
    oss << "  IPC: " << metrics.ipc() << "\n";
    oss << "  Cycles: " << metrics.cycles << "\n";
    oss << "  Instructions: " << metrics.instructions << "\n";
    oss << "  Wall Time: " << metrics.wall_time_ms << " ms\n\n";
    
    oss << "Memory Hierarchy:\n";
    oss << "  L1 Miss Rate: " << metrics.l1_miss_rate() << "%\n";
    oss << "  L2 Miss Rate: " << metrics.l2_miss_rate() << "%\n";
    oss << "  LLC Miss Rate: " << metrics.llc_miss_rate() << "%\n\n";
    
    oss << "Control Flow:\n";
    oss << "  Branch Miss Rate: " << metrics.branch_miss_rate() << "%\n";
    oss << "  Branches: " << metrics.branches << "\n";
    oss << "  Branch Misses: " << metrics.branch_misses << "\n\n";
    
    oss << "Bottleneck Classification:\n";
    oss << "  Primary: " << to_string(result.bottleneck) << "\n";
    oss << "  Memory-Bound Score: " << result.scores["memory_bound"] << "\n";
    oss << "  Compute-Bound Score: " << result.scores["compute_bound"] << "\n";
    oss << "  Control-Flow-Bound Score: " << result.scores["control_flow_bound"] << "\n";
    
    result.detailed_analysis = oss.str();
    
    return result;
}

BottleneckType Analyzer::ClassifyBottleneck(const PerformanceMetrics& metrics) {
    double mem_score = MemoryBoundScore(metrics);
    double compute_score = ComputeBoundScore(metrics);
    double control_score = ControlFlowBoundScore(metrics);
    
    // Find dominant bottleneck
    double max_score = std::max({mem_score, compute_score, control_score});
    
    if (max_score < 0.3) {
        return BottleneckType::UNKNOWN;
    }
    
    // Check if multiple bottlenecks
    int high_scores = 0;
    if (mem_score > 0.5) high_scores++;
    if (compute_score > 0.5) high_scores++;
    if (control_score > 0.5) high_scores++;
    
    if (high_scores > 1) {
        return BottleneckType::MIXED;
    }
    
    if (max_score == mem_score) {
        return BottleneckType::MEMORY_BOUND;
    } else if (max_score == compute_score) {
        return BottleneckType::COMPUTE_BOUND;
    } else {
        return BottleneckType::CONTROL_FLOW_BOUND;
    }
}

double Analyzer::MemoryBoundScore(const PerformanceMetrics& metrics) {
    // Higher cache miss rates indicate memory-bound behavior
    double l2_miss = metrics.l2_miss_rate();
    double llc_miss = metrics.llc_miss_rate();
    double ipc = metrics.ipc();
    
    double score = 0.0;
    
    // L2 miss rate contribution (0-1)
    if (l2_miss > 50.0) score += 0.5;
    else if (l2_miss > 30.0) score += 0.4;
    else if (l2_miss > memory_bound_threshold_) score += 0.3;
    else if (l2_miss > 10.0) score += 0.2;
    
    // LLC miss rate contribution (0-0.3)
    if (llc_miss > 20.0) score += 0.3;
    else if (llc_miss > 10.0) score += 0.2;
    else if (llc_miss > 5.0) score += 0.1;
    
    // Low IPC suggests stalls (0-0.2)
    if (ipc < 0.5) score += 0.2;
    else if (ipc < 1.0) score += 0.1;
    
    return std::min(score, 1.0);
}

double Analyzer::ComputeBoundScore(const PerformanceMetrics& metrics) {
    // High IPC with many instructions suggests compute-bound
    double ipc = metrics.ipc();
    double l2_miss = metrics.l2_miss_rate();
    
    double score = 0.0;
    
    // High IPC contribution (0-0.6)
    if (ipc > compute_bound_ipc_threshold_) {
        score += 0.6;
    } else if (ipc > 1.0) {
        score += 0.4;
    } else if (ipc > 0.7) {
        score += 0.2;
    }
    
    // Low cache miss rate suggests not memory-bound (0-0.4)
    if (l2_miss < 5.0) score += 0.4;
    else if (l2_miss < 10.0) score += 0.2;
    
    return std::min(score, 1.0);
}

double Analyzer::ControlFlowBoundScore(const PerformanceMetrics& metrics) {
    // High branch miss rate indicates control-flow issues
    double branch_miss = metrics.branch_miss_rate();
    double ipc = metrics.ipc();
    
    double score = 0.0;
    
    // Branch miss rate contribution (0-0.7)
    if (branch_miss > 15.0) score += 0.7;
    else if (branch_miss > 10.0) score += 0.5;
    else if (branch_miss > control_flow_threshold_) score += 0.3;
    else if (branch_miss > 2.0) score += 0.1;
    
    // Low IPC suggests pipeline stalls (0-0.3)
    if (ipc < 0.7) score += 0.3;
    else if (ipc < 1.2) score += 0.1;
    
    return std::min(score, 1.0);
}

std::vector<std::string> Analyzer::GenerateRecommendations(
    const PerformanceMetrics& metrics,
    BottleneckType bottleneck) {
    
    std::vector<std::string> recommendations;
    
    switch (bottleneck) {
        case BottleneckType::MEMORY_BOUND:
            recommendations.push_back("Apply cache-aware loop tiling to improve locality");
            if (metrics.l2_miss_rate() > 30.0) {
                recommendations.push_back("Consider blocking algorithms to fit in L2 cache");
            }
            if (metrics.llc_miss_rate() > 10.0) {
                recommendations.push_back("Reduce working set size or use streaming optimizations");
            }
            recommendations.push_back("Use software prefetching for predictable access patterns");
            break;
            
        case BottleneckType::COMPUTE_BOUND:
            recommendations.push_back("Apply SIMD vectorization for data-parallel operations");
            recommendations.push_back("Use loop unrolling to reduce loop overhead");
            recommendations.push_back("Consider GPU offloading for highly parallel workloads");
            if (metrics.ipc() < 2.0) {
                recommendations.push_back("Reduce instruction dependencies to improve ILP");
            }
            break;
            
        case BottleneckType::CONTROL_FLOW_BOUND:
            recommendations.push_back("Reduce branch mispredictions with predication or branchless code");
            recommendations.push_back("Reorganize conditionals to make branches more predictable");
            recommendations.push_back("Consider lookup tables instead of complex branching");
            if (metrics.branch_miss_rate() > 10.0) {
                recommendations.push_back("Profile branch patterns and optimize hot branches");
            }
            break;
            
        case BottleneckType::MIXED:
            recommendations.push_back("Multiple bottlenecks detected - prioritize by impact");
            recommendations.push_back("Start with cache optimization to reduce memory stalls");
            recommendations.push_back("Then apply vectorization for compute improvements");
            break;
            
        default:
            recommendations.push_back("Insufficient data for specific recommendations");
            recommendations.push_back("Run with more iterations for stable metrics");
            break;
    }
    
    return recommendations;
}

std::string Analyzer::CompareMetrics(const PerformanceMetrics& baseline,
                                    const PerformanceMetrics& optimized) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "Performance Comparison:\n";
    oss << "======================\n\n";
    
    double speedup = CalculateSpeedup(baseline, optimized);
    oss << "Overall Speedup: " << speedup << "x\n\n";
    
    oss << "Metric                Baseline    Optimized   Improvement\n";
    oss << "------------------------------------------------------------\n";
    
    auto print_metric = [&](const std::string& name, double base, double opt, bool higher_better = false) {
        double improvement = higher_better ? (opt / base) : (base / opt);
        oss << std::left << std::setw(20) << name 
            << std::right << std::setw(11) << base 
            << std::setw(12) << opt 
            << std::setw(12) << improvement << "x\n";
    };
    
    print_metric("IPC", baseline.ipc(), optimized.ipc(), true);
    print_metric("Time (ms)", baseline.wall_time_ms, optimized.wall_time_ms);
    print_metric("Cycles", baseline.cycles, optimized.cycles);
    print_metric("L1 Miss Rate (%)", baseline.l1_miss_rate(), optimized.l1_miss_rate());
    print_metric("L2 Miss Rate (%)", baseline.l2_miss_rate(), optimized.l2_miss_rate());
    print_metric("Branch Miss (%)", baseline.branch_miss_rate(), optimized.branch_miss_rate());
    
    return oss.str();
}

double Analyzer::CalculateSpeedup(const PerformanceMetrics& baseline,
                                  const PerformanceMetrics& optimized) {
    if (optimized.wall_time_ms == 0) return 0.0;
    return baseline.wall_time_ms / optimized.wall_time_ms;
}

std::string Analyzer::GenerateReport(const PerformanceMetrics& metrics,
                                    const AnalysisResult& analysis) {
    std::ostringstream oss;
    
    oss << analysis.detailed_analysis;
    oss << "\nRecommendations:\n";
    oss << "================\n";
    for (size_t i = 0; i < analysis.recommendations.size(); ++i) {
        oss << (i + 1) << ". " << analysis.recommendations[i] << "\n";
    }
    
    return oss.str();
}

void Analyzer::SetThresholds(double memory_bound_cache_miss,
                            double compute_bound_ipc,
                            double control_flow_branch_miss) {
    memory_bound_threshold_ = memory_bound_cache_miss;
    compute_bound_ipc_threshold_ = compute_bound_ipc;
    control_flow_threshold_ = control_flow_branch_miss;
}

} // namespace pie

