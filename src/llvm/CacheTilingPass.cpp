#include "pie/llvm/CacheTilingPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>

using namespace llvm;

namespace pie {
namespace llvm_passes {

PreservedAnalyses CacheTilingPass::run(Function& F,
                                       FunctionAnalysisManager& AM) {
    auto& LI = AM.getResult<LoopAnalysis>(F);
    auto& SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    
    bool modified = false;
    
    // Process all loops in the function
    for (Loop* L : LI) {
        // Only tile innermost loops for now
        if (L->getSubLoops().empty() && IsTileable(L)) {
            if (TileLoop(L, LI)) {
                modified = true;
            }
        }
    }
    
    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

int CacheTilingPass::ComputeTileSize(int element_size,
                                     int cache_size_bytes) const {
    // Compute tile size to fit working set in cache
    // For 2D tiling: tile_size^2 * element_size * num_arrays <= cache_size
    // Assuming 3 arrays (A, B, C), solve for tile_size
    
    int available_cache = cache_size_bytes / 2;  // Use half of cache
    int tile_size_sq = available_cache / (3 * element_size);
    int tile_size = static_cast<int>(std::sqrt(tile_size_sq));
    
    // Round down to nearest cache line multiple
    int elements_per_line = cache_line_size_ / element_size;
    tile_size = (tile_size / elements_per_line) * elements_per_line;
    
    // Clamp to reasonable range [16, 128]
    if (tile_size < 16) tile_size = 16;
    if (tile_size > 128) tile_size = 128;
    
    return tile_size;
}

bool CacheTilingPass::IsTileable(Loop* loop) const {
    // Check if loop is a candidate for tiling
    
    // Must have a simple induction variable
    PHINode* IndVar = loop->getCanonicalInductionVariable();
    if (!IndVar) return false;
    
    // Must have constant bounds (for simplicity)
    // In production, we'd handle more complex cases
    
    // Must contain array/memory operations
    bool has_memory_ops = false;
    for (BasicBlock* BB : loop->blocks()) {
        for (Instruction& I : *BB) {
            if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
                has_memory_ops = true;
                break;
            }
        }
    }
    
    return has_memory_ops;
}

bool CacheTilingPass::TileLoop(Loop* loop, LoopInfo& LI) {
    // This is a simplified tiling implementation
    // A full implementation would use polyhedral analysis
    
    errs() << "[CacheTiling] Tiling loop at ";
    if (MDNode* N = loop->getStartLoc()) {
        errs() << "line " << N << "\n";
    } else {
        errs() << "unknown location\n";
    }
    
    // Get loop components
    PHINode* IndVar = loop->getCanonicalInductionVariable();
    if (!IndVar) return false;
    
    BasicBlock* Header = loop->getHeader();
    BasicBlock* Latch = loop->getLoopLatch();
    if (!Header || !Latch) return false;
    
    // Compute tile size (assuming 4-byte floats)
    int tile_size = ComputeTileSize(4, cache_size_kb_ * 1024);
    
    errs() << "[CacheTiling] Tile size: " << tile_size << "\n";
    
    // In a full implementation, we would:
    // 1. Clone the loop
    // 2. Create outer tile loops
    // 3. Modify inner loop bounds to use tiles
    // 4. Update all memory accesses
    
    // For this demo, we'll mark it as "conceptually tiled"
    // and add metadata for later code generation
    
    LLVMContext& Ctx = Header->getContext();
    MDNode* TileMD = MDNode::get(Ctx, {
        MDString::get(Ctx, "cache_tiled"),
        ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Ctx), tile_size))
    });
    
    Header->getTerminator()->setMetadata("pie.tiled", TileMD);
    
    return true;
}

} // namespace llvm_passes
} // namespace pie

// Register the pass
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "PIECacheTiling", "v0.1",
        [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "cache-tiling") {
                        FPM.addPass(pie::llvm_passes::CacheTilingPass());
                        return true;
                    }
                    return false;
                });
        }};
}

