#include "pie/llvm/CacheTilingPass.h"
#include "pie/llvm/SIMDVectorizationPass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace pie {
namespace llvm_passes {

/**
 * @brief Initialize PIE optimization passes
 */
void RegisterPIEPasses(PassBuilder& PB) {
    // Register cache tiling pass
    PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager& FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "pie-cache-tiling") {
                FPM.addPass(CacheTilingPass(32, 64));  // L1: 32KB, Line: 64B
                return true;
            }
            return false;
        });
    
    // Register SIMD vectorization pass
    PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager& FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "pie-simd-vectorize") {
                FPM.addPass(SIMDVectorizationPass(256));  // AVX2
                return true;
            }
            return false;
        });
    
    // Register combined PIE optimization pipeline
    PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager& FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "pie-optimize") {
                FPM.addPass(CacheTilingPass(32, 64));
                FPM.addPass(SIMDVectorizationPass(256));
                return true;
            }
            return false;
        });
}

} // namespace llvm_passes
} // namespace pie

