/**
 * Simple Matrix Multiplication Example
 * 
 * This is an intentionally naive implementation to demonstrate
 * cache-unfriendly behavior that PIE can optimize.
 */

#include <iostream>
#include <vector>
#include <chrono>

void matrix_multiply(const std::vector<float>& A,
                    const std::vector<float>& B,
                    std::vector<float>& C,
                    int N) {
    // Naive O(N^3) implementation
    // Poor cache locality due to column-major access of B
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

int main(int argc, char** argv) {
    int N = 512;
    if (argc > 1) {
        N = std::stoi(argv[1]);
    }
    
    std::cout << "Matrix multiplication: " << N << "x" << N << "\n";
    
    // Allocate matrices
    std::vector<float> A(N * N, 1.0f);
    std::vector<float> B(N * N, 2.0f);
    std::vector<float> C(N * N, 0.0f);
    
    // Measure time
    auto start = std::chrono::high_resolution_clock::now();
    
    matrix_multiply(A, B, C, N);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Result sample: " << C[0] << "\n";
    
    return 0;
}

