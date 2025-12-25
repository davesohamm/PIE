#ifndef PIE_CORE_OPTIMIZER_H
#define PIE_CORE_OPTIMIZER_H

#include "pie/core/types.h"
#include "pie/core/profiler.h"
#include "pie/core/analyzer.h"
#include <memory>
#include <string>

namespace pie {

/**
 * @brief Closed-loop optimization orchestrator
 * 
 * Coordinates the full optimization pipeline:
 * 1. Profile baseline workload
 * 2. Analyze bottlenecks
 * 3. Apply LLVM optimization passes
 * 4. Recompile and re-profile
 * 5. Validate improvements
 */
class Optimizer {
public:
    Optimizer();
    ~Optimizer();
    
    /**
     * @brief Run complete optimization pipeline
     * @param source_file Path to source code
     * @param config Optimization configuration
     * @return Optimization result with before/after comparison
     */
    OptimizationResult Optimize(const std::string& source_file,
                               const OptimizationConfig& config);
    
    /**
     * @brief Run optimization on workload function
     */
    OptimizationResult OptimizeWorkload(
        const std::string& workload_name,
        const WorkloadConfig& config,
        const OptimizationConfig& opt_config);
    
    /**
     * @brief Apply specific LLVM passes to source
     */
    bool ApplyLLVMPasses(const std::string& source_file,
                        const std::string& output_file,
                        const std::vector<std::string>& passes);
    
    /**
     * @brief Compile source to executable
     */
    bool Compile(const std::string& source_file,
                const std::string& output_file,
                const std::vector<std::string>& flags = {});
    
    /**
     * @brief Generate LLVM IR from source
     */
    bool GenerateLLVMIR(const std::string& source_file,
                       const std::string& output_ir);
    
    /**
     * @brief Set verbosity for optimization process
     */
    void SetVerbose(bool verbose) { verbose_ = verbose; }
    
private:
    std::unique_ptr<Profiler> profiler_;
    std::unique_ptr<Analyzer> analyzer_;
    bool verbose_{false};
    
    // Helper methods
    std::vector<std::string> SelectPassesFromAnalysis(
        const AnalysisResult& analysis,
        const OptimizationConfig& config);
    
    std::string GenerateOptimizedSource(
        const std::string& original_source,
        const std::vector<std::string>& passes);
    
    bool ValidateOptimization(const PerformanceMetrics& baseline,
                             const PerformanceMetrics& optimized);
};

} // namespace pie

#endif // PIE_CORE_OPTIMIZER_H

