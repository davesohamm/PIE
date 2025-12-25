/**
 * Optimized Matrix Multiplication Example
 * 
 * This version uses cache tiling to improve locality.
 * Compare with simple_matmul.cpp to see the improvement.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

void matrix_multiply_tiled(const std::vector<float>& A,
                           const std::vector<float>& B,
                           std::vector<float>& C,
                           int N,
                           int TILE_SIZE = 64) {
    // Tiled implementation for better cache locality
    for (int ii = 0; ii < N; ii += TILE_SIZE) {
        for (int jj = 0; jj < N; jj += TILE_SIZE) {
            for (int kk = 0; kk < N; kk += TILE_SIZE) {
                // Process tile
                int i_max = std::min(ii + TILE_SIZE, N);
                int j_max = std::min(jj + TILE_SIZE, N);
                int k_max = std::min(kk + TILE_SIZE, N);
                
                for (int i = ii; i < i_max; ++i) {
                    for (int j = jj; j < j_max; ++j) {
                        float sum = C[i * N + j];
                        for (int k = kk; k < k_max; ++k) {
                            sum += A[i * N + k] * B[k * N + j];
                        }
                        C[i * N + j] = sum;
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    int N = 512;
    int TILE = 64;
    
    if (argc > 1) N = std::stoi(argv[1]);
    if (argc > 2) TILE = std::stoi(argv[2]);
    
    std::cout << "Optimized matrix multiplication: " << N << "x" << N << "\n";
    std::cout << "Tile size: " << TILE << "\n";
    
    // Allocate matrices
    std::vector<float> A(N * N, 1.0f);
    std::vector<float> B(N * N, 2.0f);
    std::vector<float> C(N * N, 0.0f);
    
    // Measure time
    auto start = std::chrono::high_resolution_clock::now();
    
    matrix_multiply_tiled(A, B, C, N, TILE);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Result sample: " << C[0] << "\n";
    
    return 0;
}

