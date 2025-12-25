#ifndef PIE_CORE_PROFILER_H
#define PIE_CORE_PROFILER_H

#include "pie/core/types.h"
#include <functional>
#include <memory>
#include <string>

namespace pie {

/**
 * @brief Hardware performance profiler using Linux perf
 * 
 * Collects detailed hardware performance metrics including:
 * - CPU cycles and instructions
 * - Cache hierarchy (L1/L2/LLC) hits and misses
 * - Branch prediction statistics
 * - Memory operations
 */
class Profiler {
public:
    Profiler();
    ~Profiler();
    
    /**
     * @brief Check if hardware performance counters are available
     */
    static bool IsAvailable();
    
    /**
     * @brief Profile a workload function
     * @param workload Function to profile
     * @return Performance metrics
     */
    PerformanceMetrics Profile(std::function<void()> workload);
    
    /**
     * @brief Profile an external binary
     * @param binary_path Path to executable
     * @param args Command line arguments
     * @return Performance metrics
     */
    PerformanceMetrics ProfileBinary(const std::string& binary_path,
                                     const std::vector<std::string>& args = {});
    
    /**
     * @brief Set profiling mode (detailed vs fast)
     */
    void SetDetailedMode(bool detailed) { detailed_mode_ = detailed; }
    
    /**
     * @brief Export metrics to JSON file
     */
    bool ExportMetrics(const PerformanceMetrics& metrics,
                       const std::string& output_path) const;
    
private:
    bool detailed_mode_{true};
    
    // Parse perf stat output
    PerformanceMetrics ParsePerfOutput(const std::string& output) const;
    
    // Execute command and capture output
    std::string ExecuteCommand(const std::string& command) const;
};

} // namespace pie

#endif // PIE_CORE_PROFILER_H

