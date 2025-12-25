/**
 * Vector Addition with SIMD Example
 * 
 * Demonstrates SIMD vectorization for simple operations.
 */

#include <iostream>
#include <vector>
#include <chrono>

#ifdef __AVX2__
#include <immintrin.h>
#endif

void vector_add_scalar(const float* A, const float* B, float* C, int N) {
    for (int i = 0; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}

#ifdef __AVX2__
void vector_add_simd(const float* A, const float* B, float* C, int N) {
    int i = 0;
    
    // Process 8 floats at a time using AVX2
    for (; i + 8 <= N; i += 8) {
        __m256 va = _mm256_loadu_ps(&A[i]);
        __m256 vb = _mm256_loadu_ps(&B[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&C[i], vc);
    }
    
    // Handle remaining elements
    for (; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}
#endif

int main(int argc, char** argv) {
    int N = 10000000;
    if (argc > 1) N = std::stoi(argv[1]);
    
    std::cout << "Vector addition: " << N << " elements\n";
    
    std::vector<float> A(N, 1.0f);
    std::vector<float> B(N, 2.0f);
    std::vector<float> C(N, 0.0f);
    
    // Scalar version
    auto start = std::chrono::high_resolution_clock::now();
    vector_add_scalar(A.data(), B.data(), C.data(), N);
    auto end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Scalar time: " << scalar_time.count() / 1000.0 << " ms\n";
    
#ifdef __AVX2__
    // SIMD version
    start = std::chrono::high_resolution_clock::now();
    vector_add_simd(A.data(), B.data(), C.data(), N);
    end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "SIMD time: " << simd_time.count() / 1000.0 << " ms\n";
    std::cout << "Speedup: " << (double)scalar_time.count() / simd_time.count() << "x\n";
#else
    std::cout << "AVX2 not available. Compile with -mavx2 flag.\n";
#endif
    
    return 0;
}

