#include <string>

namespace pie {
namespace gpu {
namespace kernels {

// OpenCL kernel library

const char* MATMUL_KERNEL = R"(
__kernel void matmul(__global const float* A,
                     __global const float* B,
                     __global float* C,
                     const int N) {
    int row = get_global_id(0);
    int col = get_global_id(1);
    
    if (row < N && col < N) {
        float sum = 0.0f;
        for (int k = 0; k < N; k++) {
            sum += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}
)";

const char* VECTOR_ADD_KERNEL = R"(
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

const char* REDUCTION_KERNEL = R"(
__kernel void reduction(__global const float* input,
                        __global float* output,
                        __local float* scratch,
                        const int N) {
    int gid = get_global_id(0);
    int lid = get_local_id(0);
    int wg_size = get_local_size(0);
    
    // Load data into local memory
    scratch[lid] = (gid < N) ? input[gid] : 0.0f;
    barrier(CLK_LOCAL_MEM_FENCE);
    
    // Reduction in local memory
    for (int offset = wg_size / 2; offset > 0; offset >>= 1) {
        if (lid < offset) {
            scratch[lid] += scratch[lid + offset];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    // Write result
    if (lid == 0) {
        output[get_group_id(0)] = scratch[0];
    }
}
)";

} // namespace kernels
} // namespace gpu
} // namespace pie

