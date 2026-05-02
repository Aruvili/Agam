#include "agam/codegen/optimizer.h"

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar.h"

namespace agam {

Optimizer::Level Optimizer::parseLevel(const std::string &s) {
    if (s == "0")
        return Level::O0;
    if (s == "1")
        return Level::O1;
    if (s == "2")
        return Level::O2;
    if (s == "3")
        return Level::O3;
    return Level::O0;
}

void Optimizer::optimize(llvm::Module &module, Level level) {
    if (level == Level::O0)
        return; // No optimization

    llvm::PassBuilder passBuilder;

    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    passBuilder.registerModuleAnalyses(MAM);
    passBuilder.registerCGSCCAnalyses(CGAM);
    passBuilder.registerFunctionAnalyses(FAM);
    passBuilder.registerLoopAnalyses(LAM);
    passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::OptimizationLevel optLevel;
    switch (level) {
    case Level::O1:
        optLevel = llvm::OptimizationLevel::O1;
        break;
    case Level::O2:
        optLevel = llvm::OptimizationLevel::O2;
        break;
    case Level::O3:
        optLevel = llvm::OptimizationLevel::O3;
        break;
    default:
        return;
    }

    auto MPM = passBuilder.buildPerModuleDefaultPipeline(optLevel);
    MPM.run(module, MAM);
}

} // namespace agam
