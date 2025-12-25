#include "pie/core/profiler.h"
#include "pie/core/analyzer.h"
#include "pie/core/optimizer.h"
#include "pie/workloads/workload.h"
#include "pie/gpu/executor.h"
#include <iostream>
#include <cstring>
#include <map>

using namespace pie;

void PrintUsage(const char* program) {
    std::cout << "PIE - Performance Intelligence Engine\n\n";
    std::cout << "Usage: " << program << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  profile <workload> [--size N]    Profile a workload\n";
    std::cout << "  analyze <workload> [--size N]    Analyze bottlenecks\n";
    std::cout << "  optimize <workload> [--size N]   Full optimization pipeline\n";
    std::cout << "  compare <workload> [--size N]    Compare CPU vs GPU\n";
    std::cout << "  list                             List available workloads\n";
    std::cout << "  info                             Show system information\n";
    std::cout << "\n";
    std::cout << "Workloads:\n";
    for (const auto& wl : workloads::WorkloadFactory::ListWorkloads()) {
        std::cout << "  " << wl << "\n";
    }
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program << " profile matmul --size 1024\n";
    std::cout << "  " << program << " optimize vector_add --size 10000000\n";
    std::cout << "  " << program << " compare matmul --size 512\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "list") {
        std::cout << "Available Workloads:\n";
        for (const auto& wl : workloads::WorkloadFactory::ListWorkloads()) {
            std::cout << "  " << wl << "\n";
        }
        return 0;
    }
    
    if (command == "info") {
        std::cout << "PIE System Information\n";
        std::cout << "======================\n\n";
        
        std::cout << "Profiler: ";
        std::cout << (Profiler::IsAvailable() ? "Available (perf)" : "Not available") << "\n";
        
        std::cout << "GPU: ";
        std::cout << (gpu::GPUExecutor::IsAvailable() ? "Available (OpenCL)" : "Not available") << "\n";
        
        if (gpu::GPUExecutor::IsAvailable()) {
            gpu::GPUExecutor executor;
            if (executor.Initialize()) {
                std::cout << "\n" << executor.GetDeviceInfo() << "\n";
            }
        }
        
        return 0;
    }
    
    if (argc < 3) {
        std::cerr << "Error: Missing workload name\n";
        PrintUsage(argv[0]);
        return 1;
    }
    
    std::string workload_name = argv[2];
    int size = -1;
    
    // Parse options
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            size = std::stoi(argv[i + 1]);
            i++;
        }
    }
    
    // Create workload
    auto workload = workloads::WorkloadFactory::Create(workload_name);
    
    std::map<std::string, int> params;
    if (size > 0) {
        params["size"] = size;
    }
    
    workload->Initialize(params);
    
    if (command == "profile") {
        std::cout << "=== Profiling " << workload->Name() << " ===\n";
        std::cout << workload->Description() << "\n\n";
        
        if (!Profiler::IsAvailable()) {
            std::cerr << "Error: perf not available\n";
            return 1;
        }
        
        Profiler profiler;
        
        std::cout << "Running workload...\n";
        auto metrics = profiler.Profile([&]() {
            workload->Execute();
        });
        
        std::cout << "\nPerformance Metrics:\n";
        std::cout << "====================\n";
        std::cout << "Wall Time:        " << metrics.wall_time_ms << " ms\n";
        std::cout << "Cycles:           " << metrics.cycles << "\n";
        std::cout << "Instructions:     " << metrics.instructions << "\n";
        std::cout << "IPC:              " << metrics.ipc() << "\n";
        std::cout << "L1 Miss Rate:     " << metrics.l1_miss_rate() << "%\n";
        std::cout << "L2 Miss Rate:     " << metrics.l2_miss_rate() << "%\n";
        std::cout << "Branch Miss Rate: " << metrics.branch_miss_rate() << "%\n";
        
    } else if (command == "analyze") {
        std::cout << "=== Analyzing " << workload->Name() << " ===\n";
        std::cout << workload->Description() << "\n\n";
        
        Profiler profiler;
        Analyzer analyzer;
        
        std::cout << "Profiling...\n";
        auto metrics = profiler.Profile([&]() {
            workload->Execute();
        });
        
        std::cout << "Analyzing bottlenecks...\n\n";
        auto analysis = analyzer.Analyze(metrics);
        
        std::cout << analyzer.GenerateReport(metrics, analysis);
        
    } else if (command == "optimize") {
        std::cout << "=== Optimizing " << workload->Name() << " ===\n";
        std::cout << workload->Description() << "\n\n";
        
        Optimizer optimizer;
        optimizer.SetVerbose(true);
        
        WorkloadConfig wl_config;
        wl_config.name = workload_name;
        wl_config.params = params;
        
        OptimizationConfig opt_config;
        opt_config.enable_cache_tiling = true;
        opt_config.enable_simd_vectorization = true;
        
        auto result = optimizer.OptimizeWorkload(workload_name, wl_config, opt_config);
        
        if (result.success) {
            std::cout << "\n✓ Optimization successful!\n";
            std::cout << "Speedup: " << result.speedup << "x\n";
        } else {
            std::cout << "\n✗ Optimization did not improve performance\n";
        }
        
    } else if (command == "compare") {
        std::cout << "=== CPU vs GPU Comparison for " << workload->Name() << " ===\n";
        std::cout << workload->Description() << "\n\n";
        
        // CPU execution
        std::cout << "[1/2] Running on CPU...\n";
        Profiler profiler;
        auto cpu_metrics = profiler.Profile([&]() {
            workload->Execute();
        });
        
        std::cout << "CPU Time: " << cpu_metrics.wall_time_ms << " ms\n\n";
        
        // GPU execution
        std::cout << "[2/2] Running on GPU...\n";
        
        if (!gpu::GPUExecutor::IsAvailable()) {
            std::cerr << "GPU not available\n";
            return 1;
        }
        
        gpu::GPUExecutor gpu_executor;
        if (!gpu_executor.Initialize()) {
            std::cerr << "Failed to initialize GPU\n";
            return 1;
        }
        
        std::cout << "Device: " << gpu_executor.GetDeviceName() << "\n";
        
        std::string kernel_source = workload->GetGPUKernel();
        if (kernel_source.empty()) {
            std::cout << "GPU kernel not available for this workload\n";
        } else {
            double gpu_time = gpu_executor.ExecuteKernel(
                kernel_source, "kernel_main", 10000, 256);
            
            std::cout << "GPU Time: " << gpu_time << " ms\n\n";
            
            double speedup = cpu_metrics.wall_time_ms / gpu_time;
            std::cout << "=== Results ===\n";
            std::cout << "Speedup: " << speedup << "x\n";
            std::cout << "Winner: " << (speedup > 1.0 ? "GPU" : "CPU") << "\n";
        }
        
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        PrintUsage(argv[0]);
        return 1;
    }
    
    workload->Cleanup();
    
    return 0;
}

