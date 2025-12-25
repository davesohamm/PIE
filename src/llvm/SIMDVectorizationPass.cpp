#include "pie/llvm/SIMDVectorizationPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace pie {
namespace llvm_passes {

PreservedAnalyses SIMDVectorizationPass::run(Function& F,
                                             FunctionAnalysisManager& AM) {
    auto& LI = AM.getResult<LoopAnalysis>(F);
    auto& DI = AM.getResult<DependenceAnalysis>(F);
    
    bool modified = false;
    
    // Process all loops
    for (Loop* L : LI) {
        if (IsVectorizable(L, DI)) {
            if (VectorizeLoop(L, LI)) {
                modified = true;
            }
        }
    }
    
    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool SIMDVectorizationPass::IsVectorizable(Loop* loop,
                                           DependenceInfo& DI) const {
    // Check if loop can be safely vectorized
    
    // Must have canonical induction variable
    PHINode* IndVar = loop->getCanonicalInductionVariable();
    if (!IndVar) return false;
    
    // Check for memory dependencies
    if (HasMemoryDependencies(loop, DI)) {
        return false;
    }
    
    // Check for contiguous memory access
    if (!HasContiguousAccess(loop)) {
        return false;
    }
    
    // Must not have function calls (for simplicity)
    for (BasicBlock* BB : loop->blocks()) {
        for (Instruction& I : *BB) {
            if (isa<CallInst>(I)) {
                return false;
            }
        }
    }
    
    return true;
}

bool SIMDVectorizationPass::HasMemoryDependencies(Loop* loop,
                                                  DependenceInfo& DI) const {
    // Check for loop-carried dependencies
    
    SmallVector<Instruction*, 16> MemOps;
    
    // Collect all memory operations
    for (BasicBlock* BB : loop->blocks()) {
        for (Instruction& I : *BB) {
            if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
                MemOps.push_back(&I);
            }
        }
    }
    
    // Check pairwise dependencies
    for (size_t i = 0; i < MemOps.size(); ++i) {
        for (size_t j = i + 1; j < MemOps.size(); ++j) {
            if (auto Dep = DI.depends(MemOps[i], MemOps[j], true)) {
                // Found a dependency
                if (!Dep->isConfused() && Dep->isFlow()) {
                    return true;  // Has loop-carried flow dependency
                }
            }
        }
    }
    
    return false;
}

bool SIMDVectorizationPass::HasContiguousAccess(Loop* loop) const {
    // Check if memory accesses are contiguous (stride-1)
    // Simplified check: look for pattern like A[i]
    
    for (BasicBlock* BB : loop->blocks()) {
        for (Instruction& I : *BB) {
            if (auto* Load = dyn_cast<LoadInst>(&I)) {
                // Check if address is of form base + i
                Value* Ptr = Load->getPointerOperand();
                if (auto* GEP = dyn_cast<GetElementPtrInst>(Ptr)) {
                    // Simplified check
                    if (GEP->getNumIndices() != 1) {
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

int SIMDVectorizationPass::GetVectorWidth(Type* element_type) const {
    // Compute number of elements that fit in vector register
    int element_bits = element_type->getScalarSizeInBits();
    return vector_width_bits_ / element_bits;
}

bool SIMDVectorizationPass::VectorizeLoop(Loop* loop, LoopInfo& LI) {
    errs() << "[SIMDVectorization] Vectorizing loop\n";
    
    // Get loop components
    PHINode* IndVar = loop->getCanonicalInductionVariable();
    if (!IndVar) return false;
    
    BasicBlock* Header = loop->getHeader();
    if (!Header) return false;
    
    // Determine vector width based on first load/store
    int vector_width = 8;  // Default to AVX2 (8 x float)
    
    for (BasicBlock* BB : loop->blocks()) {
        for (Instruction& I : *BB) {
            if (auto* Load = dyn_cast<LoadInst>(&I)) {
                vector_width = GetVectorWidth(Load->getType());
                break;
            }
        }
        if (vector_width != 8) break;
    }
    
    errs() << "[SIMDVectorization] Vector width: " << vector_width << "\n";
    
    // In a full implementation, we would:
    // 1. Widen the iteration space by vector_width
    // 2. Replace scalar loads/stores with vector operations
    // 3. Replace scalar arithmetic with vector arithmetic
    // 4. Generate tail loop for remainder iterations
    
    // For this demo, we'll add metadata indicating vectorization
    LLVMContext& Ctx = Header->getContext();
    MDNode* VecMD = MDNode::get(Ctx, {
        MDString::get(Ctx, "simd_vectorized"),
        ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Ctx), vector_width))
    });
    
    Header->getTerminator()->setMetadata("pie.vectorized", VecMD);
    
    // Also add LLVM's vectorization hint
    MDNode* EnableVec = MDNode::get(Ctx, {
        MDString::get(Ctx, "llvm.loop.vectorize.enable"),
        ConstantAsMetadata::get(ConstantInt::get(Type::getInt1Ty(Ctx), true))
    });
    
    MDNode* VecWidth = MDNode::get(Ctx, {
        MDString::get(Ctx, "llvm.loop.vectorize.width"),
        ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Ctx), vector_width))
    });
    
    MDNode* LoopMD = MDNode::get(Ctx, {EnableVec, VecWidth});
    loop->setLoopID(LoopMD);
    
    return true;
}

} // namespace llvm_passes
} // namespace pie

// Register the pass
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "PIESIMDVectorization", "v0.1",
        [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "simd-vectorize") {
                        FPM.addPass(pie::llvm_passes::SIMDVectorizationPass());
                        return true;
                    }
                    return false;
                });
        }};
}

