#ifndef PIE_WORKLOADS_WORKLOAD_H
#define PIE_WORKLOADS_WORKLOAD_H

#include <string>
#include <map>
#include <memory>
#include <functional>

namespace pie {
namespace workloads {

/**
 * @brief Base class for all benchmark workloads
 */
class Workload {
public:
    virtual ~Workload() = default;
    
    /**
     * @brief Get workload name
     */
    virtual std::string Name() const = 0;
    
    /**
     * @brief Get workload description
     */
    virtual std::string Description() const = 0;
    
    /**
     * @brief Initialize workload with parameters
     */
    virtual void Initialize(const std::map<std::string, int>& params) = 0;
    
    /**
     * @brief Execute the workload
     */
    virtual void Execute() = 0;
    
    /**
     * @brief Cleanup resources
     */
    virtual void Cleanup() = 0;
    
    /**
     * @brief Get OpenCL kernel source (if applicable)
     */
    virtual std::string GetGPUKernel() const { return ""; }
    
    /**
     * @brief Verify correctness of results
     */
    virtual bool Verify() const { return true; }
    
    /**
     * @brief Get result data pointer (for verification)
     */
    virtual void* GetResultData() = 0;
    
    /**
     * @brief Get result data size
     */
    virtual size_t GetResultSize() const = 0;
};

/**
 * @brief Matrix multiplication workload
 * Tests cache locality and memory hierarchy
 */
class MatrixMultiplication : public Workload {
public:
    std::string Name() const override { return "matmul"; }
    std::string Description() const override { 
        return "Matrix Multiplication (Cache Locality Test)";
    }
    
    void Initialize(const std::map<std::string, int>& params) override;
    void Execute() override;
    void Cleanup() override;
    std::string GetGPUKernel() const override;
    bool Verify() const override;
    void* GetResultData() override;
    size_t GetResultSize() const override;
    
private:
    int size_{0};
    float* A_{nullptr};
    float* B_{nullptr};
    float* C_{nullptr};
    float* C_ref_{nullptr};
};

/**
 * @brief Vector addition workload
 * Tests SIMD throughput
 */
class VectorAddition : public Workload {
public:
    std::string Name() const override { return "vector_add"; }
    std::string Description() const override {
        return "Vector Addition (SIMD Throughput Test)";
    }
    
    void Initialize(const std::map<std::string, int>& params) override;
    void Execute() override;
    void Cleanup() override;
    std::string GetGPUKernel() const override;
    bool Verify() const override;
    void* GetResultData() override;
    size_t GetResultSize() const override;
    
private:
    int size_{0};
    float* A_{nullptr};
    float* B_{nullptr};
    float* C_{nullptr};
};

/**
 * @brief Memory streaming workload
 * Tests memory bandwidth saturation
 */
class MemoryStreaming : public Workload {
public:
    std::string Name() const override { return "memory_stream"; }
    std::string Description() const override {
        return "Memory Streaming (Bandwidth Test)";
    }
    
    void Initialize(const std::map<std::string, int>& params) override;
    void Execute() override;
    void Cleanup() override;
    void* GetResultData() override;
    size_t GetResultSize() const override;
    
private:
    int size_{0};
    double* data_{nullptr};
};

/**
 * @brief Branch-heavy workload
 * Tests branch prediction efficiency
 */
class BranchHeavy : public Workload {
public:
    std::string Name() const override { return "branch_heavy"; }
    std::string Description() const override {
        return "Branch-Heavy Loop (Control Flow Test)";
    }
    
    void Initialize(const std::map<std::string, int>& params) override;
    void Execute() override;
    void Cleanup() override;
    void* GetResultData() override;
    size_t GetResultSize() const override;
    
private:
    int size_{0};
    int* data_{nullptr};
    long long result_{0};
};

/**
 * @brief Reduction workload
 * Tests dependency chains
 */
class Reduction : public Workload {
public:
    std::string Name() const override { return "reduction"; }
    std::string Description() const override {
        return "Parallel Reduction (Dependency Chain Test)";
    }
    
    void Initialize(const std::map<std::string, int>& params) override;
    void Execute() override;
    void Cleanup() override;
    std::string GetGPUKernel() const override;
    void* GetResultData() override;
    size_t GetResultSize() const override;
    
private:
    int size_{0};
    float* data_{nullptr};
    float result_{0.0f};
};

/**
 * @brief Workload factory
 */
class WorkloadFactory {
public:
    static std::unique_ptr<Workload> Create(const std::string& name);
    static std::vector<std::string> ListWorkloads();
};

} // namespace workloads
} // namespace pie

#endif // PIE_WORKLOADS_WORKLOAD_H

