#include "pie/workloads/workload.h"
#include <cstdlib>

namespace pie {
namespace workloads {

void Reduction::Initialize(const std::map<std::string, int>& params) {
    auto it = params.find("size");
    if (it != params.end()) {
        size_ = it->second;
    } else {
        size_ = 100000000; // 100M elements
    }
    
    data_ = new float[size_];
    
    // Initialize
    for (int i = 0; i < size_; ++i) {
        data_[i] = static_cast<float>(i % 100) / 100.0f;
    }
    
    result_ = 0.0f;
}

void Reduction::Execute() {
    // Simple reduction with dependency chain
    result_ = 0.0f;
    
    for (int i = 0; i < size_; ++i) {
        result_ += data_[i];
    }
}

void Reduction::Cleanup() {
    delete[] data_;
    data_ = nullptr;
}

std::string Reduction::GetGPUKernel() const {
    return R"(
__kernel void reduction(__global const float* input,
                        __global float* partial_sums,
                        __local float* local_sums,
                        const int N) {
    int global_id = get_global_id(0);
    int local_id = get_local_id(0);
    int group_size = get_local_size(0);
    
    // Load into local memory
    local_sums[local_id] = (global_id < N) ? input[global_id] : 0.0f;
    barrier(CLK_LOCAL_MEM_FENCE);
    
    // Parallel reduction in local memory
    for (int stride = group_size / 2; stride > 0; stride >>= 1) {
        if (local_id < stride) {
            local_sums[local_id] += local_sums[local_id + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    // Write result
    if (local_id == 0) {
        partial_sums[get_group_id(0)] = local_sums[0];
    }
}
)";
}

void* Reduction::GetResultData() {
    return &result_;
}

size_t Reduction::GetResultSize() const {
    return sizeof(result_);
}

} // namespace workloads
} // namespace pie

