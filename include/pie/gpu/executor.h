#ifndef PIE_GPU_EXECUTOR_H
#define PIE_GPU_EXECUTOR_H

#include "pie/core/types.h"
#include <string>
#include <vector>
#include <memory>

#ifdef HAVE_OPENCL
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#endif

namespace pie {
namespace gpu {

/**
 * @brief GPU execution engine using OpenCL
 */
class GPUExecutor {
public:
    GPUExecutor();
    ~GPUExecutor();
    
    /**
     * @brief Check if GPU is available
     */
    static bool IsAvailable();
    
    /**
     * @brief Initialize OpenCL context and device
     */
    bool Initialize();
    
    /**
     * @brief Execute kernel on GPU
     * @param kernel_source OpenCL kernel source code
     * @param kernel_name Name of kernel function
     * @param global_size Global work size
     * @param local_size Local work size (0 for auto)
     * @return Execution time in milliseconds
     */
    double ExecuteKernel(const std::string& kernel_source,
                        const std::string& kernel_name,
                        size_t global_size,
                        size_t local_size = 0);
    
    /**
     * @brief Allocate buffer on GPU
     */
    void* AllocateBuffer(size_t size, bool read_only = false);
    
    /**
     * @brief Copy data to GPU
     */
    bool CopyToGPU(void* gpu_buffer, const void* host_data, size_t size);
    
    /**
     * @brief Copy data from GPU
     */
    bool CopyFromGPU(void* host_data, const void* gpu_buffer, size_t size);
    
    /**
     * @brief Free GPU buffer
     */
    void FreeBuffer(void* gpu_buffer);
    
    /**
     * @brief Get device info
     */
    std::string GetDeviceInfo() const;
    
    /**
     * @brief Get device name
     */
    std::string GetDeviceName() const;
    
    /**
     * @brief Get compute units
     */
    int GetComputeUnits() const;
    
private:
#ifdef HAVE_OPENCL
    cl_platform_id platform_;
    cl_device_id device_;
    cl_context context_;
    cl_command_queue queue_;
    
    bool initialized_;
#else
    bool initialized_{false};
#endif
};

} // namespace gpu
} // namespace pie

#endif // PIE_GPU_EXECUTOR_H

