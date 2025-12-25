#include "pie/core/optimizer.h"
#include "pie/workloads/workload.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

namespace pie {

Optimizer::Optimizer() 
    : profiler_(std::make_unique<Profiler>()),
      analyzer_(std::make_unique<Analyzer>()) {
}

Optimizer::~Optimizer() = default;

OptimizationResult Optimizer::Optimize(const std::string& source_file,
                                       const OptimizationConfig& config) {
    OptimizationResult result;
    
    if (verbose_) {
        std::cout << "=== PIE Optimization Pipeline ===\n";
        std::cout << "Source: " << source_file << "\n\n";
    }
    
    // Step 1: Compile baseline
    std::string baseline_bin = "/tmp/pie_baseline";
    if (!Compile(source_file, baseline_bin, {"-O1"})) {
        std::cerr << "Failed to compile baseline\n";
        return result;
    }
    
    // Step 2: Profile baseline
    if (verbose_) {
        std::cout << "[1/5] Profiling baseline...\n";
    }
    result.baseline_metrics = profiler_->ProfileBinary(baseline_bin);
    
    // Step 3: Analyze bottlenecks
    if (verbose_) {
        std::cout << "[2/5] Analyzing bottlenecks...\n";
    }
    auto analysis = analyzer_->Analyze(result.baseline_metrics);
    
    if (verbose_) {
        std::cout << "Bottleneck: " << to_string(analysis.bottleneck) << "\n";
        std::cout << "Recommendations:\n";
        for (const auto& rec : analysis.recommendations) {
            std::cout << "  - " << rec << "\n";
        }
        std::cout << "\n";
    }
    
    // Step 4: Apply LLVM passes
    if (verbose_) {
        std::cout << "[3/5] Applying compiler optimizations...\n";
    }
    
    auto passes = SelectPassesFromAnalysis(analysis, config);
    std::string optimized_ir = "/tmp/pie_optimized.ll";
    std::string optimized_bin = "/tmp/pie_optimized";
    
    if (!GenerateLLVMIR(source_file, optimized_ir)) {
        std::cerr << "Failed to generate LLVM IR\n";
        return result;
    }
    
    if (!ApplyLLVMPasses(optimized_ir, optimized_ir, passes)) {
        std::cerr << "Failed to apply LLVM passes\n";
        return result;
    }
    
    // Step 5: Compile optimized version
    if (!Compile(optimized_ir, optimized_bin, {"-O3", "-march=" + config.target_arch})) {
        std::cerr << "Failed to compile optimized version\n";
        return result;
    }
    
    // Step 6: Profile optimized version
    if (verbose_) {
        std::cout << "[4/5] Profiling optimized version...\n";
    }
    result.optimized_metrics = profiler_->ProfileBinary(optimized_bin);
    
    // Step 7: Validate and report
    if (verbose_) {
        std::cout << "[5/5] Validating improvements...\n\n";
    }
    
    result.success = ValidateOptimization(result.baseline_metrics,
                                         result.optimized_metrics);
    result.speedup = analyzer_->CalculateSpeedup(result.baseline_metrics,
                                                 result.optimized_metrics);
    result.applied_optimizations = passes;
    result.analysis = analyzer_->CompareMetrics(result.baseline_metrics,
                                               result.optimized_metrics);
    
    if (verbose_) {
        std::cout << result.analysis << "\n";
    }
    
    return result;
}

OptimizationResult Optimizer::OptimizeWorkload(
    const std::string& workload_name,
    const WorkloadConfig& workload_config,
    const OptimizationConfig& opt_config) {
    
    // Create workload
    auto workload = workloads::WorkloadFactory::Create(workload_name);
    workload->Initialize(workload_config.params);
    
    // Generate source file for workload
    std::string source_file = "/tmp/pie_workload_" + workload_name + ".cpp";
    // In a full implementation, we'd generate proper source code
    // For now, we'll use pre-written workload executables
    
    // Use the optimize method
    return Optimize(source_file, opt_config);
}

bool Optimizer::ApplyLLVMPasses(const std::string& source_file,
                               const std::string& output_file,
                               const std::vector<std::string>& passes) {
    std::string cmd = "opt ";
    
    // Add each pass
    for (const auto& pass : passes) {
        cmd += "-" + pass + " ";
    }
    
    cmd += source_file + " -o " + output_file;
    
    if (verbose_) {
        std::cout << "Running: " << cmd << "\n";
    }
    
    int ret = system(cmd.c_str());
    return ret == 0;
}

bool Optimizer::Compile(const std::string& source_file,
                       const std::string& output_file,
                       const std::vector<std::string>& flags) {
    std::string cmd;
    
    // Determine if source is IR or C++
    bool is_ir = source_file.find(".ll") != std::string::npos;
    
    if (is_ir) {
        cmd = "clang " + source_file;
    } else {
        cmd = "clang++ -std=c++17 " + source_file;
    }
    
    // Add flags
    for (const auto& flag : flags) {
        cmd += " " + flag;
    }
    
    cmd += " -o " + output_file;
    
    if (verbose_) {
        std::cout << "Compiling: " << cmd << "\n";
    }
    
    int ret = system(cmd.c_str());
    return ret == 0;
}

bool Optimizer::GenerateLLVMIR(const std::string& source_file,
                              const std::string& output_ir) {
    std::string cmd = "clang++ -std=c++17 -S -emit-llvm " + 
                     source_file + " -o " + output_ir;
    
    if (verbose_) {
        std::cout << "Generating IR: " << cmd << "\n";
    }
    
    int ret = system(cmd.c_str());
    return ret == 0;
}

std::vector<std::string> Optimizer::SelectPassesFromAnalysis(
    const AnalysisResult& analysis,
    const OptimizationConfig& config) {
    
    std::vector<std::string> passes;
    
    // Always apply basic optimizations
    passes.push_back("mem2reg");
    passes.push_back("instcombine");
    
    // Select passes based on bottleneck
    switch (analysis.bottleneck) {
        case BottleneckType::MEMORY_BOUND:
            if (config.enable_cache_tiling) {
                passes.push_back("loop-rotate");
                passes.push_back("loop-simplify");
                passes.push_back("licm");  // Loop-invariant code motion
            }
            break;
            
        case BottleneckType::COMPUTE_BOUND:
            if (config.enable_simd_vectorization) {
                passes.push_back("loop-vectorize");
                passes.push_back("slp-vectorizer");
            }
            if (config.enable_loop_unrolling) {
                passes.push_back("loop-unroll");
            }
            break;
            
        case BottleneckType::CONTROL_FLOW_BOUND:
            passes.push_back("simplifycfg");
            passes.push_back("jump-threading");
            break;
            
        default:
            // Apply general optimizations
            passes.push_back("loop-vectorize");
            passes.push_back("licm");
            break;
    }
    
    return passes;
}

bool Optimizer::ValidateOptimization(const PerformanceMetrics& baseline,
                                    const PerformanceMetrics& optimized) {
    // Check if optimization actually helped
    double speedup = baseline.wall_time_ms / optimized.wall_time_ms;
    
    // Consider it successful if speedup > 1.05 (5% improvement)
    return speedup > 1.05;
}

} // namespace pie

