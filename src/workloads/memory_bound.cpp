#include "pie/workloads/workload.h"
#include <cstdlib>
#include <cstring>

namespace pie {
namespace workloads {

void MemoryStreaming::Initialize(const std::map<std::string, int>& params) {
    auto it = params.find("size");
    if (it != params.end()) {
        size_ = it->second;
    } else {
        size_ = 100000000; // 100M elements (800MB)
    }
    
    data_ = new double[size_];
    
    // Initialize
    for (int i = 0; i < size_; ++i) {
        data_[i] = static_cast<double>(i);
    }
}

void MemoryStreaming::Execute() {
    // STREAM Triad: a = b + scalar * c
    // This is a memory bandwidth benchmark
    const double scalar = 3.0;
    
    // Copy
    for (int i = 0; i < size_; ++i) {
        data_[i] = data_[i] * 2.0;
    }
    
    // Scale
    for (int i = 0; i < size_; ++i) {
        data_[i] = scalar * data_[i];
    }
    
    // Add (using same array for simplicity)
    for (int i = 1; i < size_; ++i) {
        data_[i] = data_[i] + data_[i-1];
    }
}

void MemoryStreaming::Cleanup() {
    delete[] data_;
    data_ = nullptr;
}

void* MemoryStreaming::GetResultData() {
    return data_;
}

size_t MemoryStreaming::GetResultSize() const {
    return size_ * sizeof(double);
}

} // namespace workloads
} // namespace pie

