#include "pie/gpu/executor.h"
#include <iostream>
#include <chrono>
#include <cstring>

namespace pie {
namespace gpu {

#ifdef HAVE_OPENCL

GPUExecutor::GPUExecutor() 
    : platform_(nullptr), device_(nullptr), 
      context_(nullptr), queue_(nullptr), initialized_(false) {
}

GPUExecutor::~GPUExecutor() {
    if (queue_) clReleaseCommandQueue(queue_);
    if (context_) clReleaseContext(context_);
}

bool GPUExecutor::IsAvailable() {
    cl_platform_id platform;
    cl_uint num_platforms;
    
    cl_int err = clGetPlatformIDs(1, &platform, &num_platforms);
    return err == CL_SUCCESS && num_platforms > 0;
}

bool GPUExecutor::Initialize() {
    cl_int err;
    
    // Get platform
    err = clGetPlatformIDs(1, &platform_, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get OpenCL platform\n";
        return false;
    }
    
    // Get device
    err = clGetDeviceIDs(platform_, CL_DEVICE_TYPE_GPU, 1, &device_, nullptr);
    if (err != CL_SUCCESS) {
        // Try CPU as fallback
        err = clGetDeviceIDs(platform_, CL_DEVICE_TYPE_CPU, 1, &device_, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to get OpenCL device\n";
            return false;
        }
    }
    
    // Create context
    context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL context\n";
        return false;
    }
    
    // Create command queue
    queue_ = clCreateCommandQueue(context_, device_, 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue\n";
        return false;
    }
    
    initialized_ = true;
    return true;
}

double GPUExecutor::ExecuteKernel(const std::string& kernel_source,
                                  const std::string& kernel_name,
                                  size_t global_size,
                                  size_t local_size) {
    if (!initialized_) {
        std::cerr << "GPU executor not initialized\n";
        return -1.0;
    }
    
    cl_int err;
    
    // Create program
    const char* src = kernel_source.c_str();
    size_t src_len = kernel_source.length();
    
    cl_program program = clCreateProgramWithSource(context_, 1, &src, &src_len, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create program\n";
        return -1.0;
    }
    
    // Build program
    err = clBuildProgram(program, 1, &device_, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        // Get build log
        char log[4096];
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
                             sizeof(log), log, nullptr);
        std::cerr << "Build error:\n" << log << "\n";
        clReleaseProgram(program);
        return -1.0;
    }
    
    // Create kernel
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create kernel\n";
        clReleaseProgram(program);
        return -1.0;
    }
    
    // Execute kernel
    size_t global_work_size = global_size;
    size_t local_work_size = local_size > 0 ? local_size : 64;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    err = clEnqueueNDRangeKernel(queue_, kernel, 1, nullptr,
                                &global_work_size, &local_work_size,
                                0, nullptr, nullptr);
    
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to enqueue kernel\n";
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        return -1.0;
    }
    
    // Wait for completion
    clFinish(queue_);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Cleanup
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    
    return duration.count() / 1000.0;  // Return milliseconds
}

void* GPUExecutor::AllocateBuffer(size_t size, bool read_only) {
    cl_int err;
    cl_mem_flags flags = read_only ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE;
    
    cl_mem buffer = clCreateBuffer(context_, flags, size, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to allocate GPU buffer\n";
        return nullptr;
    }
    
    return buffer;
}

bool GPUExecutor::CopyToGPU(void* gpu_buffer, const void* host_data, size_t size) {
    cl_mem buffer = static_cast<cl_mem>(gpu_buffer);
    cl_int err = clEnqueueWriteBuffer(queue_, buffer, CL_TRUE, 0, size,
                                      host_data, 0, nullptr, nullptr);
    return err == CL_SUCCESS;
}

bool GPUExecutor::CopyFromGPU(void* host_data, const void* gpu_buffer, size_t size) {
    cl_mem buffer = static_cast<cl_mem>(const_cast<void*>(gpu_buffer));
    cl_int err = clEnqueueReadBuffer(queue_, buffer, CL_TRUE, 0, size,
                                     host_data, 0, nullptr, nullptr);
    return err == CL_SUCCESS;
}

void GPUExecutor::FreeBuffer(void* gpu_buffer) {
    if (gpu_buffer) {
        clReleaseMemObject(static_cast<cl_mem>(gpu_buffer));
    }
}

std::string GPUExecutor::GetDeviceInfo() const {
    if (!initialized_) return "Not initialized";
    
    char name[256];
    char vendor[256];
    cl_uint compute_units;
    cl_ulong mem_size;
    
    clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(name), name, nullptr);
    clGetDeviceInfo(device_, CL_DEVICE_VENDOR, sizeof(vendor), vendor, nullptr);
    clGetDeviceInfo(device_, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), 
                   &compute_units, nullptr);
    clGetDeviceInfo(device_, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), 
                   &mem_size, nullptr);
    
    std::string info = "Device: ";
    info += name;
    info += "\nVendor: ";
    info += vendor;
    info += "\nCompute Units: " + std::to_string(compute_units);
    info += "\nGlobal Memory: " + std::to_string(mem_size / (1024*1024)) + " MB";
    
    return info;
}

std::string GPUExecutor::GetDeviceName() const {
    if (!initialized_) return "Unknown";
    
    char name[256];
    clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(name), name, nullptr);
    return std::string(name);
}

int GPUExecutor::GetComputeUnits() const {
    if (!initialized_) return 0;
    
    cl_uint compute_units;
    clGetDeviceInfo(device_, CL_DEVICE_MAX_COMPUTE_UNITS, 
                   sizeof(compute_units), &compute_units, nullptr);
    return compute_units;
}

#else

// Stub implementation when OpenCL is not available

GPUExecutor::GPUExecutor() : initialized_(false) {}
GPUExecutor::~GPUExecutor() {}

bool GPUExecutor::IsAvailable() { return false; }
bool GPUExecutor::Initialize() { return false; }

double GPUExecutor::ExecuteKernel(const std::string&, const std::string&,
                                  size_t, size_t) {
    std::cerr << "OpenCL not available\n";
    return -1.0;
}

void* GPUExecutor::AllocateBuffer(size_t, bool) { return nullptr; }
bool GPUExecutor::CopyToGPU(void*, const void*, size_t) { return false; }
bool GPUExecutor::CopyFromGPU(void*, const void*, size_t) { return false; }
void GPUExecutor::FreeBuffer(void*) {}
std::string GPUExecutor::GetDeviceInfo() const { return "OpenCL not available"; }
std::string GPUExecutor::GetDeviceName() const { return "None"; }
int GPUExecutor::GetComputeUnits() const { return 0; }

#endif

} // namespace gpu
} // namespace pie

