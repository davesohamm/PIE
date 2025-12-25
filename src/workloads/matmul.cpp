#include "pie/workloads/workload.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

namespace pie {
namespace workloads {

void MatrixMultiplication::Initialize(const std::map<std::string, int>& params) {
    auto it = params.find("size");
    if (it != params.end()) {
        size_ = it->second;
    } else {
        size_ = 512; // default
    }
    
    // Allocate matrices
    size_t total_size = size_ * size_;
    A_ = new float[total_size];
    B_ = new float[total_size];
    C_ = new float[total_size];
    C_ref_ = new float[total_size];
    
    // Initialize with random values
    for (size_t i = 0; i < total_size; ++i) {
        A_[i] = static_cast<float>(rand()) / RAND_MAX;
        B_[i] = static_cast<float>(rand()) / RAND_MAX;
        C_[i] = 0.0f;
        C_ref_[i] = 0.0f;
    }
    
    // Compute reference for verification
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < size_; ++k) {
                sum += A_[i * size_ + k] * B_[k * size_ + j];
            }
            C_ref_[i * size_ + j] = sum;
        }
    }
}

void MatrixMultiplication::Execute() {
    // Naive matrix multiplication (intentionally cache-unfriendly)
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < size_; ++k) {
                sum += A_[i * size_ + k] * B_[k * size_ + j];
            }
            C_[i * size_ + j] = sum;
        }
    }
}

void MatrixMultiplication::Cleanup() {
    delete[] A_;
    delete[] B_;
    delete[] C_;
    delete[] C_ref_;
    A_ = B_ = C_ = C_ref_ = nullptr;
}

std::string MatrixMultiplication::GetGPUKernel() const {
    return R"(
__kernel void matmul(__global const float* A,
                     __global const float* B,
                     __global float* C,
                     const int N) {
    int i = get_global_id(0);
    int j = get_global_id(1);
    
    if (i < N && j < N) {
        float sum = 0.0f;
        for (int k = 0; k < N; k++) {
            sum += A[i * N + k] * B[k * N + j];
        }
        C[i * N + j] = sum;
    }
}
)";
}

bool MatrixMultiplication::Verify() const {
    if (!C_ || !C_ref_) return false;
    
    const float epsilon = 1e-3f;
    size_t total = size_ * size_;
    
    for (size_t i = 0; i < total; ++i) {
        if (std::fabs(C_[i] - C_ref_[i]) > epsilon) {
            std::cerr << "Verification failed at index " << i 
                      << ": got " << C_[i] << ", expected " << C_ref_[i] << std::endl;
            return false;
        }
    }
    
    return true;
}

void* MatrixMultiplication::GetResultData() {
    return C_;
}

size_t MatrixMultiplication::GetResultSize() const {
    return size_ * size_ * sizeof(float);
}

} // namespace workloads
} // namespace pie

