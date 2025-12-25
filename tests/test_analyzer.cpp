#include "pie/core/analyzer.h"
#include <gtest/gtest.h>

using namespace pie;

TEST(AnalyzerTest, MemoryBoundDetection) {
    Analyzer analyzer;
    
    PerformanceMetrics metrics;
    metrics.cycles = 10000000;
    metrics.instructions = 5000000;
    metrics.l2_cache_references = 100000;
    metrics.l2_cache_misses = 30000;
    metrics.branches = 50000;
    metrics.branch_misses = 1000;
    
    auto result = analyzer.Analyze(metrics);
    
    EXPECT_EQ(result.bottleneck, BottleneckType::MEMORY_BOUND);
    EXPECT_GT(result.recommendations.size(), 0);
}

TEST(AnalyzerTest, ComputeBoundDetection) {
    Analyzer analyzer;
    
    PerformanceMetrics metrics;
    metrics.cycles = 10000000;
    metrics.instructions = 20000000;  // High IPC
    metrics.l2_cache_references = 100000;
    metrics.l2_cache_misses = 1000;   // Low miss rate
    metrics.branches = 50000;
    metrics.branch_misses = 500;
    
    auto result = analyzer.Analyze(metrics);
    
    EXPECT_EQ(result.bottleneck, BottleneckType::COMPUTE_BOUND);
}

TEST(AnalyzerTest, SpeedupCalculation) {
    Analyzer analyzer;
    
    PerformanceMetrics baseline;
    baseline.wall_time_ms = 1000.0;
    
    PerformanceMetrics optimized;
    optimized.wall_time_ms = 500.0;
    
    double speedup = analyzer.CalculateSpeedup(baseline, optimized);
    
    EXPECT_DOUBLE_EQ(speedup, 2.0);
}

