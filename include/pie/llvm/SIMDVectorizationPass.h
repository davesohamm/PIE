#ifndef PIE_LLVM_SIMD_VECTORIZATION_PASS_H
#define PIE_LLVM_SIMD_VECTORIZATION_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace pie {
namespace llvm_passes {

/**
 * @brief SIMD auto-vectorization pass
 * 
 * Automatically vectorizes data-parallel loops by:
 * 1. Identifying vectorizable loops (no dependencies)
 * 2. Analyzing memory access patterns (contiguous)
 * 3. Selecting appropriate vector width (AVX2/AVX-512)
 * 4. Transforming scalar operations to vector operations
 * 5. Generating tail handling for non-divisible sizes
 * 
 * Example transformation:
 *   for (i = 0; i < N; i++)
 *     C[i] = A[i] + B[i];
 * 
 * Becomes (conceptually):
 *   for (i = 0; i < N/8; i++)
 *     C_vec[i] = A_vec[i] + B_vec[i];  // 8-wide SIMD
 *   for (i = N/8*8; i < N; i++)
 *     C[i] = A[i] + B[i];  // scalar tail
 */
class SIMDVectorizationPass : public llvm::PassInfoMixin<SIMDVectorizationPass> {
public:
    explicit SIMDVectorizationPass(int vector_width_bits = 256)
        : vector_width_bits_(vector_width_bits) {}
    
    llvm::PreservedAnalyses run(llvm::Function& F,
                               llvm::FunctionAnalysisManager& AM);
    
    static bool isRequired() { return true; }
    
private:
    int vector_width_bits_;  // 128 (SSE), 256 (AVX2), 512 (AVX-512)
    
    // Safety analysis
    bool IsVectorizable(llvm::Loop* loop, llvm::DependenceInfo& DI) const;
    bool HasMemoryDependencies(llvm::Loop* loop, llvm::DependenceInfo& DI) const;
    bool HasContiguousAccess(llvm::Loop* loop) const;
    
    // Vectorization logic
    int GetVectorWidth(llvm::Type* element_type) const;
    bool VectorizeLoop(llvm::Loop* loop, llvm::LoopInfo& LI);
};

} // namespace llvm_passes
} // namespace pie

#endif // PIE_LLVM_SIMD_VECTORIZATION_PASS_H

