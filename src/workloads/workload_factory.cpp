#include "pie/workloads/workload.h"
#include <stdexcept>

namespace pie {
namespace workloads {

std::unique_ptr<Workload> WorkloadFactory::Create(const std::string& name) {
    if (name == "matmul" || name == "matrix_multiply") {
        return std::make_unique<MatrixMultiplication>();
    } else if (name == "vector_add" || name == "vadd") {
        return std::make_unique<VectorAddition>();
    } else if (name == "memory_stream" || name == "stream") {
        return std::make_unique<MemoryStreaming>();
    } else if (name == "branch_heavy" || name == "branch") {
        return std::make_unique<BranchHeavy>();
    } else if (name == "reduction" || name == "reduce") {
        return std::make_unique<Reduction>();
    } else {
        throw std::runtime_error("Unknown workload: " + name);
    }
}

std::vector<std::string> WorkloadFactory::ListWorkloads() {
    return {
        "matmul - Matrix Multiplication (Cache Locality Test)",
        "vector_add - Vector Addition (SIMD Throughput Test)",
        "memory_stream - Memory Streaming (Bandwidth Test)",
        "branch_heavy - Branch-Heavy Loop (Control Flow Test)",
        "reduction - Parallel Reduction (Dependency Chain Test)"
    };
}

} // namespace workloads
} // namespace pie

