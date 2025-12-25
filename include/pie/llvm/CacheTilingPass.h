#ifndef PIE_LLVM_CACHE_TILING_PASS_H
#define PIE_LLVM_CACHE_TILING_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace pie {
namespace llvm_passes {

/**
 * @brief Cache-aware loop tiling optimization pass
 * 
 * Transforms nested loops to improve cache locality by:
 * 1. Detecting nested loops with array accesses
 * 2. Analyzing working set size
 * 3. Computing optimal tile size based on cache capacity
 * 4. Transforming loops into tiled (blocked) form
 * 
 * Example transformation:
 *   for (i = 0; i < N; i++)
 *     for (j = 0; j < N; j++)
 *       C[i][j] = A[i][j] * B[i][j]
 * 
 * Becomes:
 *   for (ii = 0; ii < N; ii += TILE)
 *     for (jj = 0; jj < N; jj += TILE)
 *       for (i = ii; i < min(ii+TILE, N); i++)
 *         for (j = jj; j < min(jj+TILE, N); j++)
 *           C[i][j] = A[i][j] * B[i][j]
 */
class CacheTilingPass : public llvm::PassInfoMixin<CacheTilingPass> {
public:
    explicit CacheTilingPass(int cache_size_kb = 32, int cache_line_size = 64)
        : cache_size_kb_(cache_size_kb), cache_line_size_(cache_line_size) {}
    
    llvm::PreservedAnalyses run(llvm::Function& F,
                               llvm::FunctionAnalysisManager& AM);
    
    static bool isRequired() { return true; }
    
private:
    int cache_size_kb_;
    int cache_line_size_;
    
    // Compute optimal tile size for given data type
    int ComputeTileSize(int element_size, int cache_size_bytes) const;
    
    // Check if loop is tileable
    bool IsTileable(llvm::Loop* loop) const;
    
    // Apply tiling transformation
    bool TileLoop(llvm::Loop* loop, llvm::LoopInfo& LI);
};

} // namespace llvm_passes
} // namespace pie

#endif // PIE_LLVM_CACHE_TILING_PASS_H

