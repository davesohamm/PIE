#include "pie/workloads/workload.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

namespace pie {
namespace workloads {

void VectorAddition::Initialize(const std::map<std::string, int>& params) {
    auto it = params.find("size");
    if (it != params.end()) {
        size_ = it->second;
    } else {
        size_ = 10000000; // 10M elements default
    }
    
    // Allocate vectors
    A_ = new float[size_];
    B_ = new float[size_];
    C_ = new float[size_];
    
    // Initialize with random values
    for (int i = 0; i < size_; ++i) {
        A_[i] = static_cast<float>(rand()) / RAND_MAX;
        B_[i] = static_cast<float>(rand()) / RAND_MAX;
        C_[i] = 0.0f;
    }
}

void VectorAddition::Execute() {
    // Scalar vector addition (should benefit from vectorization)
    for (int i = 0; i < size_; ++i) {
        C_[i] = A_[i] + B_[i];
    }
}

void VectorAddition::Cleanup() {
    delete[] A_;
    delete[] B_;
    delete[] C_;
    A_ = B_ = C_ = nullptr;
}

std::string VectorAddition::GetGPUKernel() const {
    return R"(
__kernel void vector_add(__global const float* A,
                         __global const float* B,
                         __global float* C,
                         const int N) {
    int i = get_global_id(0);
    if (i < N) {
        C[i] = A[i] + B[i];
    }
}
)";
}

bool VectorAddition::Verify() const {
    if (!A_ || !B_ || !C_) return false;
    
    const float epsilon = 1e-5f;
    
    for (int i = 0; i < size_; ++i) {
        float expected = A_[i] + B_[i];
        if (std::fabs(C_[i] - expected) > epsilon) {
            std::cerr << "Verification failed at index " << i << std::endl;
            return false;
        }
    }
    
    return true;
}

void* VectorAddition::GetResultData() {
    return C_;
}

size_t VectorAddition::GetResultSize() const {
    return size_ * sizeof(float);
}

} // namespace workloads
} // namespace pie

