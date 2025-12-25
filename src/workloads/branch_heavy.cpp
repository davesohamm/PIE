#include "pie/workloads/workload.h"
#include <cstdlib>
#include <algorithm>

namespace pie {
namespace workloads {

void BranchHeavy::Initialize(const std::map<std::string, int>& params) {
    auto it = params.find("size");
    if (it != params.end()) {
        size_ = it->second;
    } else {
        size_ = 10000000; // 10M elements
    }
    
    data_ = new int[size_];
    
    // Initialize with random values (makes branches unpredictable)
    for (int i = 0; i < size_; ++i) {
        data_[i] = rand() % 100;
    }
    
    result_ = 0;
}

void BranchHeavy::Execute() {
    // Multiple unpredictable branches
    result_ = 0;
    
    for (int i = 0; i < size_; ++i) {
        int val = data_[i];
        
        // Unpredictable branches
        if (val < 25) {
            result_ += val * 2;
        } else if (val < 50) {
            result_ += val + 10;
        } else if (val < 75) {
            result_ -= val;
        } else {
            result_ += val / 2;
        }
        
        // More branches
        if (result_ % 2 == 0) {
            result_++;
        }
        
        if (val % 3 == 0) {
            result_ *= 2;
        }
    }
}

void BranchHeavy::Cleanup() {
    delete[] data_;
    data_ = nullptr;
}

void* BranchHeavy::GetResultData() {
    return &result_;
}

size_t BranchHeavy::GetResultSize() const {
    return sizeof(result_);
}

} // namespace workloads
} // namespace pie

