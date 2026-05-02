#pragma once

#include "agam/ast/ast.h" // TypeKind
#include "agam/mir/mir.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace agam {

/// LLVM IR code generator: walks MIR basic blocks and emits LLVM IR.
class CodeGenerator {
  public:
    CodeGenerator();
    ~CodeGenerator();

    /// Generate LLVM IR from a MIR program.
    bool generate(MirProgram &program);

    /// Legacy: generate from AST (kept for backward compat in existing tests).
    bool generate(class Program &program);

    llvm::Module *getModule() { return module_.get(); }
    std::string getIRString() const;
    void dumpIR() const;
    bool verify() const;

  private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    /// Local ID → LLVM alloca instruction.
    std::unordered_map<MirLocalId, llvm::AllocaInst *> localAllocas_;

    std::unordered_map<std::string, llvm::StructType *> structTypes_;
    std::unordered_map<std::string, const HirStructDef *> structDefs_;

    std::unordered_map<std::string, llvm::StructType *> enumTypes_;
    std::unordered_map<std::string, const HirEnumDef *> enumDefs_;

    /// Zone name → LLVM value (arena pointer).
    std::unordered_map<std::string, llvm::Value *> activeZones_;

    llvm::Type *toLLVMType(const TypeInfo &typeInfo);
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *func, const std::string &name,
                                             llvm::Type *type);

    // ── MIR lowering ────────────────────────────────────────────────────────
    void emitFunctionSignature(MirFunction &func);
    void emitFunctionBody(MirFunction &func);
    void emitBlock(MirBasicBlock &bb, llvm::Function *func,
                   std::unordered_map<MirBlockId, llvm::BasicBlock *> &blockMap);
    void emitStatement(const MirStatement &stmt, llvm::Function *func);
    void emitTerminator(const MirTerminator &term, llvm::Function *func,
                        std::unordered_map<MirBlockId, llvm::BasicBlock *> &blockMap);

    llvm::Value *emitOperand(const MirOperand &op);
    llvm::Value *emitRvalue(const MirRvalue &rv, llvm::Function *func);
};

} // namespace agam
