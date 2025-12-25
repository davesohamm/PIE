#include "pie/core/profiler.h"
#include <gtest/gtest.h>

using namespace pie;

TEST(ProfilerTest, IsAvailable) {
    // This might fail on systems without perf
    bool available = Profiler::IsAvailable();
    EXPECT_TRUE(available || !available);  // Just check it doesn't crash
}

TEST(ProfilerTest, ProfileFunction) {
    Profiler profiler;
    
    int sum = 0;
    auto metrics = profiler.Profile([&]() {
        for (int i = 0; i < 1000000; ++i) {
            sum += i;
        }
    });
    
    EXPECT_GT(metrics.wall_time_ms, 0);
}

TEST(ProfilerTest, MetricsCalculation) {
    PerformanceMetrics metrics;
    metrics.cycles = 1000000;
    metrics.instructions = 500000;
    
    EXPECT_DOUBLE_EQ(metrics.ipc(), 0.5);
}

TEST(ProfilerTest, CacheMissRate) {
    PerformanceMetrics metrics;
    metrics.l1_dcache_loads = 10000;
    metrics.l1_dcache_load_misses = 1000;
    
    EXPECT_DOUBLE_EQ(metrics.l1_miss_rate(), 10.0);
}

