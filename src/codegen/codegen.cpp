#include "agam/codegen/codegen.h"
#include "agam/mir/mir_printer.h"
#include <variant>
#include <iostream>
#include "agam/ast/ast.h"
#include "agam/hir/hir_builder.h"
#include "agam/thir/thir_builder.h"
#include "agam/mir/mir_builder.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>


namespace agam {

CodeGenerator::CodeGenerator()
    : context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>("agam_module", *context_)),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {}

CodeGenerator::~CodeGenerator() = default;

// ── Legacy AST entry point (for backward compat with tests) ─────────────────
bool CodeGenerator::generate(Program &program) {
    // Run full pipeline: AST → HIR → THIR → MIR → LLVM IR
    SourceManager sm;
    DiagnosticEngine diag(sm);

    HirBuilder hirBuilder;
    auto hir = hirBuilder.build(program);
    if (hirBuilder.hasErrors()) {
        return false;
    }

    ThirBuilder thirBuilder;
    auto thir = thirBuilder.build(*hir, diag);
    if (!thir || diag.hasErrors()) {
        return false;
    }

    MirBuilder mirBuilder;
    auto mir = mirBuilder.build(*thir);

    return generate(*mir);
}

// ── MIR entry point ─────────────────────────────────────────────────────────
bool CodeGenerator::generate(MirProgram &program) {
    // First, emit all global constants
    for (auto &cn : program.constants) {
        llvm::Type *ty = toLLVMType(cn.typeInfo);
        llvm::Constant *init = nullptr;
        
        std::visit([this, &init, ty](auto &&c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, MirConstInt>) {
                if (ty->isFloatingPointTy()) {
                    init = llvm::ConstantFP::get(ty, (double)c.value);
                } else {
                    init = llvm::ConstantInt::get(ty, (uint64_t)c.value);
                }
            } else if constexpr (std::is_same_v<T, MirConstFloat>) {
                if (ty->isIntegerTy()) {
                    init = llvm::ConstantInt::get(ty, (uint64_t)c.value);
                } else {
                    init = llvm::ConstantFP::get(ty, c.value);
                }
            } else if constexpr (std::is_same_v<T, MirConstBool>) {
                init = llvm::ConstantInt::get(ty, c.value ? 1 : 0);
            } else if constexpr (std::is_same_v<T, MirConstString>) {
                // For strings, we'll need more complex logic
                init = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ty));
            }
        }, cn.value);

        if (init) {
            new llvm::GlobalVariable(*module_, ty, true, llvm::GlobalValue::ExternalLinkage, init, cn.name);
        }
    }

    structDefs_.clear();
    structTypes_.clear();

    // First, declare all structs as opaque types so we can reference them
    for (auto &sd : program.structs) {
        structDefs_[sd.name] = &sd;
        structTypes_[sd.name] = llvm::StructType::create(*context_, sd.name);
    }
    // Now fill in the struct bodies (handles non-recursive for now)
    for (auto &sd : program.structs) {
        std::vector<llvm::Type*> fieldTypes;
        for (auto &f : sd.fields) {
            fieldTypes.push_back(toLLVMType(f.typeInfo));
        }
        structTypes_[sd.name]->setBody(fieldTypes);
    }

    // Calculate enum types
    enumDefs_.clear();
    enumTypes_.clear();
    for (auto &ed : program.enums) {
        enumDefs_[ed.name] = &ed;
        
        uint64_t maxSize = 0;
        for (auto &v : ed.variants) {
            if (v.hasPayload) {
                llvm::Type *payloadTy = toLLVMType(v.payloadType);
                uint64_t size = module_->getDataLayout().getTypeAllocSize(payloadTy);
                if (size > maxSize) maxSize = size;
            }
        }
        
        llvm::Type *tagTy = llvm::Type::getInt32Ty(*context_);
        llvm::Type *payloadTy = llvm::ArrayType::get(llvm::Type::getInt8Ty(*context_), maxSize);
        llvm::StructType *enumTy = llvm::StructType::create(*context_, {tagTy, payloadTy}, ed.name);
        enumTypes_[ed.name] = enumTy;
    }

    // First pass: emit all function signatures
    for (auto &func : program.functions) {
        emitFunctionSignature(func);
    }

    // Second pass: emit all function bodies
    for (auto &func : program.functions) {
        emitFunctionBody(func);
    }
    return verify();
}

std::string CodeGenerator::getIRString() const {
    std::string ir;
    llvm::raw_string_ostream stream(ir);
    module_->print(stream, nullptr);
    return ir;
}

void CodeGenerator::dumpIR() const {
    module_->print(llvm::errs(), nullptr);
}

bool CodeGenerator::verify() const {
    std::string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyModule(*module_, &errStream)) {
        std::cerr << "LLVM Module verification failed:\n" << errStr << std::endl;
        return false;
    }
    return true;
}

llvm::Type *CodeGenerator::toLLVMType(const TypeInfo& typeInfo) {
    // 1. Handle Slices (Fat Pointers)
    // Slices are { T*, i64 }
    if (typeInfo.isSlice && typeInfo.pointerDepth == 0) {
        llvm::Type *ptrTy = llvm::PointerType::getUnqual(*context_);
        llvm::Type *lenTy = llvm::Type::getInt64Ty(*context_);
        return llvm::StructType::get(*context_, {ptrTy, lenTy});
    }

    // 2. Handle Pointers
    if (typeInfo.pointerDepth > 0) {
        return llvm::PointerType::getUnqual(*context_);
    }

    // 3. Handle Arrays
    if (typeInfo.isArray) {
        llvm::Type *elemTy = toLLVMType(TypeInfo::scalar(typeInfo.elementType));
        return llvm::ArrayType::get(elemTy, typeInfo.arraySize);
    }

    // 4. Handle Named Types (Structs, Enums)
    if (typeInfo.isStruct) {
        auto it = structTypes_.find(typeInfo.structName);
        if (it != structTypes_.end()) return it->second;
    }
    if (typeInfo.isEnum) {
        auto it = enumTypes_.find(typeInfo.enumName);
        if (it != enumTypes_.end()) return it->second;
    }

    // 5. Handle Scalar Types
    switch (typeInfo.kind) {
    case TypeKind::Int8:    case TypeKind::UInt8:   return llvm::Type::getInt8Ty(*context_);
    case TypeKind::Int16:   case TypeKind::UInt16:  return llvm::Type::getInt16Ty(*context_);
    case TypeKind::Int32:   case TypeKind::UInt32:  return llvm::Type::getInt32Ty(*context_);
    case TypeKind::Int64:   case TypeKind::UInt64:  return llvm::Type::getInt64Ty(*context_);
    case TypeKind::Int128:  case TypeKind::UInt128: return llvm::Type::getInt128Ty(*context_);
    case TypeKind::Float32:                         return llvm::Type::getFloatTy(*context_);
    case TypeKind::Float64:                         return llvm::Type::getDoubleTy(*context_);
    case TypeKind::Bool:                            return llvm::Type::getInt1Ty(*context_);
    case TypeKind::String:                          return llvm::PointerType::getUnqual(*context_);
    case TypeKind::Void:                            return llvm::Type::getVoidTy(*context_);
    default:
        // Fallback for Int alias or Unknown
        if (typeInfo.kind == TypeKind::Int) return llvm::Type::getInt32Ty(*context_);
        if (typeInfo.kind == TypeKind::Float) return llvm::Type::getDoubleTy(*context_);
        return llvm::Type::getInt32Ty(*context_);
    }
}

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *func,
                                                         const std::string &name,
                                                         llvm::Type *type) {
    llvm::IRBuilder<> tmpBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, name);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Function Emission
// ═══════════════════════════════════════════════════════════════════════════════

void CodeGenerator::emitFunctionSignature(MirFunction &func) {
    // Build LLVM function type.
    std::vector<llvm::Type *> paramTypes;
    for (auto &p : func.params) {
        paramTypes.push_back(toLLVMType(p.typeInfo));
    }
    llvm::FunctionType *funcType = llvm::FunctionType::get(
        toLLVMType(func.returnTypeInfo), paramTypes, false);
    llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, func.name, module_.get());
}

void CodeGenerator::emitFunctionBody(MirFunction &func) {
    if (func.isExtern) return; // Extern functions only have a declaration, no body.

    llvm::Function *llvmFunc = module_->getFunction(func.name);
    if (!llvmFunc) return;

    // Set param names.
    unsigned idx = 0;
    for (auto &arg : llvmFunc->args()) {
        if (idx < func.params.size()) {
            arg.setName(func.params[idx].name);
        }
        idx++;
    }

    // Create LLVM basic blocks for all MIR blocks.
    std::unordered_map<MirBlockId, llvm::BasicBlock *> blockMap;
    for (auto &bb : func.blocks) {
        blockMap[bb.id] = llvm::BasicBlock::Create(*context_, bb.label, llvmFunc);
    }

    // Allocate locals.
    localAllocas_.clear();
    builder_->SetInsertPoint(blockMap[0]); // entry block
    for (auto &local : func.locals) {
        if (local.typeInfo.kind == TypeKind::Void) {
            continue; // Skip allocating for void types
        }
        llvm::AllocaInst *alloca = createEntryBlockAlloca(
            llvmFunc, local.name.empty() ? ("_" + std::to_string(local.id)) : local.name,
            toLLVMType(local.typeInfo));
        localAllocas_[local.id] = alloca;
    }

    // Store function args into their local allocas.
    idx = 0;
    for (auto &arg : llvmFunc->args()) {
        if (idx < func.params.size()) {
            MirLocalId paramLocal = func.params[idx].id;
            if (localAllocas_.count(paramLocal)) {
                builder_->CreateStore(&arg, localAllocas_[paramLocal]);
            }
        }
        idx++;
    }

    // Emit each basic block.
    for (auto &bb : func.blocks) {
        emitBlock(bb, llvmFunc, blockMap);
    }

    // Safety: if any LLVM block still lacks a terminator, add unreachable.
    for (auto &bb : func.blocks) {
        llvm::BasicBlock *llvmBB = blockMap[bb.id];
        if (!llvmBB->getTerminator()) {
            builder_->SetInsertPoint(llvmBB);
            builder_->CreateUnreachable();
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Basic Block Emission
// ═══════════════════════════════════════════════════════════════════════════════

void CodeGenerator::emitBlock(MirBasicBlock &bb, llvm::Function *func,
                               std::unordered_map<MirBlockId, llvm::BasicBlock *> &blockMap) {
    builder_->SetInsertPoint(blockMap[bb.id]);

    for (auto &stmt : bb.statements) {
        emitStatement(stmt, func);
    }

    emitTerminator(bb.terminator, func, blockMap);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Statement Emission
// ═══════════════════════════════════════════════════════════════════════════════

void CodeGenerator::emitStatement(const MirStatement &stmt, llvm::Function *func) {
    if (auto *assign = std::get_if<MirAssign>(&stmt)) {
        llvm::Value *rval = emitRvalue(assign->rvalue, func);
        if (!rval) return;

        auto it = localAllocas_.find(assign->destination.local);
        if (it != localAllocas_.end()) {
            llvm::Type *destType = it->second->getAllocatedType();
            // Handle array element type mismatch (e.g. [3 x i64] -> [3 x i32])
            if (destType->isArrayTy() && rval->getType()->isArrayTy() &&
                destType != rval->getType() &&
                destType->getArrayNumElements() == rval->getType()->getArrayNumElements()) {
                llvm::Type *destElemType = destType->getArrayElementType();
                unsigned numElems = destType->getArrayNumElements();
                llvm::Value *newArr = llvm::UndefValue::get(destType);
                for (unsigned i = 0; i < numElems; ++i) {
                    llvm::Value *elem = builder_->CreateExtractValue(rval, i);
                    if (elem->getType() != destElemType) {
                        if (elem->getType()->isIntegerTy() && destElemType->isIntegerTy()) {
                            elem = builder_->CreateIntCast(elem, destElemType, true, "arrcast");
                        } else if (elem->getType()->isFloatingPointTy() && destElemType->isFloatingPointTy()) {
                            elem = builder_->CreateFPCast(elem, destElemType, "arrcast");
                        }
                    }
                    newArr = builder_->CreateInsertValue(newArr, elem, i);
                }
                builder_->CreateStore(newArr, it->second);
            } else {
                // Ensure scalar types match (e.g. i32 -> i64)
                if (destType->isIntegerTy() && rval->getType()->isIntegerTy() && destType != rval->getType()) {
                    rval = builder_->CreateIntCast(rval, destType, true, "assign_cast");
                } else if (destType->isFloatingPointTy() && rval->getType()->isFloatingPointTy() && destType != rval->getType()) {
                    rval = builder_->CreateFPCast(rval, destType, "assign_cast");
                }
                builder_->CreateStore(rval, it->second);
            }
        }
    } else if (auto *idxAssign = std::get_if<MirIndexAssign>(&stmt)) {
        llvm::Value *index = emitOperand(idxAssign->index);
        llvm::Value *rval = emitOperand(idxAssign->value);
        if (!index || !rval) return;

        if (idxAssign->base.typeInfo.isSlice) {
            llvm::Value *slice = emitOperand(idxAssign->base);
            llvm::Value *ptr = builder_->CreateExtractValue(slice, 0, "sliceptr_assign");
            llvm::Value *gep = builder_->CreateGEP(toLLVMType(idxAssign->elementTypeInfo), ptr, index, "slicegep_assign");
            builder_->CreateStore(rval, gep);
            return;
        }

        if (idxAssign->base.typeInfo.pointerDepth > 0) {
            llvm::Value *ptr = emitOperand(idxAssign->base);
            llvm::Value *gep = builder_->CreateGEP(toLLVMType(idxAssign->elementTypeInfo), ptr, index, "ptrgep_assign");
            builder_->CreateStore(rval, gep);
            return;
        }
        
        if (idxAssign->base.kind != MirOperand::Kind::Copy) return;
        
        auto it = localAllocas_.find(idxAssign->base.place.local);
        if (it == localAllocas_.end()) return;
        
        llvm::AllocaInst *baseAlloca = it->second;
        
        llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0);
        std::vector<llvm::Value *> indices = {zero, index};
        llvm::Value *gep = builder_->CreateGEP(baseAlloca->getAllocatedType(), baseAlloca, indices, "idxassign");
        builder_->CreateStore(rval, gep);
    } else if (auto *fieldAssign = std::get_if<MirFieldAssign>(&stmt)) {
        if (fieldAssign->base.kind != MirOperand::Kind::Copy) return;

        auto it = localAllocas_.find(fieldAssign->base.place.local);
        if (it == localAllocas_.end()) return;

        llvm::AllocaInst *baseAlloca = it->second;
        llvm::Value *rval = emitOperand(fieldAssign->value);
        if (!rval) return;

        // If base is a pointer-to-struct, load the pointer first then GEP into the struct
        if (fieldAssign->base.typeInfo.pointerDepth > 0 && fieldAssign->base.typeInfo.isStruct) {
            llvm::Value *ptr = builder_->CreateLoad(baseAlloca->getAllocatedType(), baseAlloca, "deref_ptr");
            auto sit = structTypes_.find(fieldAssign->base.typeInfo.structName);
            if (sit == structTypes_.end()) return;
            llvm::Value *fieldPtr = builder_->CreateStructGEP(sit->second, ptr, fieldAssign->fieldIndex, "fieldptr");
            builder_->CreateStore(rval, fieldPtr);
        } else {
            llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0);
            llvm::Value *idxValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), fieldAssign->fieldIndex);
            std::vector<llvm::Value *> indices = {zero, idxValue};
            llvm::Value *gep = builder_->CreateGEP(baseAlloca->getAllocatedType(), baseAlloca, indices, "fieldassign");
            builder_->CreateStore(rval, gep);
        }
    } else if (auto *derefAssign = std::get_if<MirDerefAssign>(&stmt)) {
        llvm::Value *ptrVal = emitOperand(derefAssign->pointer);
        llvm::Value *rval = emitOperand(derefAssign->value);
        if (!ptrVal || !rval) return;
        builder_->CreateStore(rval, ptrVal);
    } else if (auto *heapFree = std::get_if<MirHeapFree>(&stmt)) {
        llvm::Value *ptr = emitOperand(heapFree->pointer);
        if (!ptr) return;

        // Declare free: void free(i8*)
        llvm::FunctionCallee freeFunc = module_->getOrInsertFunction("free",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*context_),
            {llvm::PointerType::getUnqual(*context_)}, false));

        builder_->CreateCall(freeFunc, {ptr});
    } else if (auto *zoneBegin = std::get_if<MirZoneBegin>(&stmt)) {
        // Declare agam_zone_create: i8* agam_zone_create()
        llvm::FunctionCallee createFunc = module_->getOrInsertFunction("agam_zone_create",
            llvm::FunctionType::get(llvm::PointerType::getUnqual(*context_), {}, false));
        
        llvm::Value *arena = builder_->CreateCall(createFunc, {}, "arena_" + zoneBegin->zoneName);
        activeZones_[zoneBegin->zoneName] = arena;
    } else if (auto *zoneEnd = std::get_if<MirZoneEnd>(&stmt)) {
        auto it = activeZones_.find(zoneEnd->zoneName);
        if (it != activeZones_.end()) {
            // Declare agam_zone_destroy: void agam_zone_destroy(i8*)
            llvm::FunctionCallee destroyFunc = module_->getOrInsertFunction("agam_zone_destroy",
                llvm::FunctionType::get(llvm::Type::getVoidTy(*context_),
                {llvm::PointerType::getUnqual(*context_)}, false));
            
            builder_->CreateCall(destroyFunc, {it->second});
            activeZones_.erase(it);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Operand Emission
// ═══════════════════════════════════════════════════════════════════════════════

llvm::Value *CodeGenerator::emitOperand(const MirOperand &op) {
    if (op.kind == MirOperand::Kind::Copy) {
        auto it = localAllocas_.find(op.place.local);
        if (it != localAllocas_.end()) {
            return builder_->CreateLoad(it->second->getAllocatedType(), it->second);
        }
        return nullptr;
    }

    // Constant
    return std::visit([this, &op](auto &&c) -> llvm::Value * {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, MirConstInt>) {
            return llvm::ConstantInt::get(toLLVMType(op.typeInfo), c.value);
        } else if constexpr (std::is_same_v<T, MirConstFloat>) {
            return llvm::ConstantFP::get(toLLVMType(op.typeInfo), c.value);
        } else if constexpr (std::is_same_v<T, MirConstBool>) {
            return llvm::ConstantInt::get(toLLVMType(op.typeInfo), c.value ? 1 : 0);
        } else if constexpr (std::is_same_v<T, MirConstString>) {
            return builder_->CreateGlobalString(c.value, "str");
        } else if constexpr (std::is_same_v<T, MirConstNull>) {
            return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(toLLVMType(op.typeInfo)));
        }
        return nullptr;
    }, op.constant);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Rvalue Emission
// ═══════════════════════════════════════════════════════════════════════════════

llvm::Value *CodeGenerator::emitRvalue(const MirRvalue &rv, llvm::Function *func) {
    return std::visit([this, func](auto &&r) -> llvm::Value * {
        using T = std::decay_t<decltype(r)>;

        if constexpr (std::is_same_v<T, MirRvalueUse>) {
            return emitOperand(r.operand);
        }
        else if constexpr (std::is_same_v<T, MirRvalueBinaryOp>) {
            llvm::Value *lhs = emitOperand(r.lhs);
            llvm::Value *rhs = emitOperand(r.rhs);
            if (!lhs || !rhs) return nullptr;

            bool isFloat = lhs->getType()->isDoubleTy();

            switch (r.op) {
            case BinaryOp::Add: return isFloat ? builder_->CreateFAdd(lhs, rhs, "addtmp") : builder_->CreateAdd(lhs, rhs, "addtmp");
            case BinaryOp::Sub: return isFloat ? builder_->CreateFSub(lhs, rhs, "subtmp") : builder_->CreateSub(lhs, rhs, "subtmp");
            case BinaryOp::Mul: return isFloat ? builder_->CreateFMul(lhs, rhs, "multmp") : builder_->CreateMul(lhs, rhs, "multmp");
            case BinaryOp::Div: return isFloat ? builder_->CreateFDiv(lhs, rhs, "divtmp") : builder_->CreateSDiv(lhs, rhs, "divtmp");
            case BinaryOp::Mod: return isFloat ? builder_->CreateFRem(lhs, rhs, "remtmp") : builder_->CreateSRem(lhs, rhs, "remtmp");
            case BinaryOp::Eq:  return isFloat ? builder_->CreateFCmpOEQ(lhs, rhs, "eqtmp") : builder_->CreateICmpEQ(lhs, rhs, "eqtmp");
            case BinaryOp::Neq: return isFloat ? builder_->CreateFCmpONE(lhs, rhs, "neqtmp") : builder_->CreateICmpNE(lhs, rhs, "neqtmp");
            case BinaryOp::Lt:  return isFloat ? builder_->CreateFCmpOLT(lhs, rhs, "lttmp") : builder_->CreateICmpSLT(lhs, rhs, "lttmp");
            case BinaryOp::Gt:  return isFloat ? builder_->CreateFCmpOGT(lhs, rhs, "gttmp") : builder_->CreateICmpSGT(lhs, rhs, "gttmp");
            case BinaryOp::Lte: return isFloat ? builder_->CreateFCmpOLE(lhs, rhs, "letmp") : builder_->CreateICmpSLE(lhs, rhs, "letmp");
            case BinaryOp::Gte: return isFloat ? builder_->CreateFCmpOGE(lhs, rhs, "getmp") : builder_->CreateICmpSGE(lhs, rhs, "getmp");
            case BinaryOp::And: return builder_->CreateAnd(lhs, rhs, "andtmp");
            case BinaryOp::Or:  return builder_->CreateOr(lhs, rhs, "ortmp");
            }
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, MirRvalueUnaryOp>) {
            if (r.op == UnaryOp::AddressOf) {
                if (r.operand.kind == MirOperand::Kind::Copy) {
                    auto it = localAllocas_.find(r.operand.place.local);
                    if (it != localAllocas_.end()) {
                        llvm::Value *ptr = it->second;
                        // Array-to-slice conversion
                        if (r.operand.typeInfo.isArray && r.operand.typeInfo.elementType != TypeKind::Unknown) {
                            llvm::Type *sliceTy = toLLVMType(TypeInfo::slice(r.operand.typeInfo.elementType));
                            llvm::Value *slice = llvm::UndefValue::get(sliceTy);
                            
                            // Pointer to first element
                            llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0);
                            std::vector<llvm::Value *> indices = {zero, zero};
                            llvm::Value *firstElemPtr = builder_->CreateGEP(toLLVMType(r.operand.typeInfo), ptr, indices, "slice_ptr");
                            
                            llvm::Value *lenVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), r.operand.typeInfo.arraySize);
                            slice = builder_->CreateInsertValue(slice, firstElemPtr, 0, "slice_ptr_init");
                            slice = builder_->CreateInsertValue(slice, lenVal, 1, "slice_len_init");
                            return slice;
                        }
                        return ptr;
                    }
                }
                return nullptr;
            }
            
            llvm::Value *operand = emitOperand(r.operand);
            if (!operand) return nullptr;
            
            switch (r.op) {
            case UnaryOp::Dereference: {
                TypeInfo pointeeType = r.operand.typeInfo;
                if (pointeeType.pointerDepth > 0) pointeeType.pointerDepth--;
                return builder_->CreateLoad(toLLVMType(pointeeType), operand, "dereftmp");
            }
            case UnaryOp::Negate:
                return operand->getType()->isDoubleTy()
                    ? builder_->CreateFNeg(operand, "negtmp")
                    : builder_->CreateNeg(operand, "negtmp");
            case UnaryOp::Not:
                return builder_->CreateNot(operand, "nottmp");
            default:
                break;
            }
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, MirRvalueCast>) {
            llvm::Value *operand = emitOperand(r.operand);
            if (!operand) return nullptr;
            
            llvm::Type *destType = toLLVMType(r.toTypeInfo);
            llvm::Type *srcType = operand->getType();

            if (srcType->isIntegerTy() && destType->isIntegerTy()) {
                return builder_->CreateIntCast(operand, destType, true, "casttmp");
            }
            if (srcType->isFloatingPointTy() && destType->isFloatingPointTy()) {
                return builder_->CreateFPCast(operand, destType, "casttmp");
            }
            if (srcType->isIntegerTy() && destType->isFloatingPointTy()) {
                return builder_->CreateSIToFP(operand, destType, "casttmp");
            }
            if (srcType->isFloatingPointTy() && destType->isIntegerTy()) {
                return builder_->CreateFPToSI(operand, destType, "casttmp");
            }
            if (srcType->isPointerTy() && destType->isIntegerTy()) {
                return builder_->CreatePtrToInt(operand, destType, "ptrtoint");
            }
            if (srcType->isIntegerTy() && destType->isPointerTy()) {
                return builder_->CreateIntToPtr(operand, destType, "inttoptr");
            }
            if (srcType->isPointerTy() && destType->isPointerTy()) {
                return operand; // Opaque pointers
            }
            
            return operand;
        }
        else if constexpr (std::is_same_v<T, MirRvalueCall>) {
            llvm::Function *callee = module_->getFunction(r.callee);
            if (!callee) return nullptr;
            std::vector<llvm::Value *> args;
            for (unsigned i = 0; i < r.args.size(); ++i) {
                llvm::Value *v = emitOperand(r.args[i]);
                if (!v) return nullptr;
                if (i < callee->arg_size()) {
                    llvm::Type *paramType = callee->getFunctionType()->getParamType(i);
                    if (v->getType()->isPointerTy() && !paramType->isPointerTy()) {
                        v = builder_->CreateLoad(paramType, v, "autoderef");
                    }
                }
                args.push_back(v);
            }
            if (callee->getReturnType()->isVoidTy()) {
                builder_->CreateCall(callee, args);
                return nullptr;
            }
            return builder_->CreateCall(callee, args, "calltmp");
        }
        else if constexpr (std::is_same_v<T, MirRvalueArrayInit>) {
            llvm::Type *llvmArrayType = toLLVMType(r.arrayTypeInfo);
            llvm::Type *elemType = llvmArrayType->getArrayElementType();
            llvm::Value *arr = llvm::UndefValue::get(llvmArrayType);
            for (unsigned i = 0; i < r.elements.size(); ++i) {
                llvm::Value *elemVal = emitOperand(r.elements[i]);
                if (!elemVal) return nullptr;
                // Cast element to target element type if needed
                if (elemVal->getType() != elemType) {
                    if (elemVal->getType()->isIntegerTy() && elemType->isIntegerTy()) {
                        elemVal = builder_->CreateIntCast(elemVal, elemType, true, "elemcast");
                    } else if (elemVal->getType()->isFloatingPointTy() && elemType->isFloatingPointTy()) {
                        elemVal = builder_->CreateFPCast(elemVal, elemType, "elemcast");
                    }
                }
                arr = builder_->CreateInsertValue(arr, elemVal, i, "arrinit");
            }
            return arr;
        }
        else if constexpr (std::is_same_v<T, MirRvalueIndex>) {
            if (r.base.kind != MirOperand::Kind::Copy) return nullptr;
            
            // Handle Slices
            if (r.base.typeInfo.isSlice) {
                llvm::Value *slice = emitOperand(r.base);
                llvm::Value *index = emitOperand(r.index);
                llvm::Value *ptr = builder_->CreateExtractValue(slice, 0, "sliceptr");
                llvm::Value *gep = builder_->CreateGEP(toLLVMType(r.elementTypeInfo), ptr, index, "slicegep");
                return builder_->CreateLoad(toLLVMType(r.elementTypeInfo), gep, "idxval");
            }
            if (r.base.typeInfo.pointerDepth > 0) {
                llvm::Value *ptr = emitOperand(r.base);
                llvm::Value *index = emitOperand(r.index);
                llvm::Value *gep = builder_->CreateGEP(toLLVMType(r.elementTypeInfo), ptr, index, "ptrgep");
                return builder_->CreateLoad(toLLVMType(r.elementTypeInfo), gep, "ptrload");
            }

            auto it = localAllocas_.find(r.base.place.local);
            if (it == localAllocas_.end()) return nullptr;
            
            llvm::AllocaInst *baseAlloca = it->second;
            llvm::Value *idxValue = emitOperand(r.index);
            if (!idxValue) return nullptr;
            
            llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0);
            std::vector<llvm::Value *> indices = {zero, idxValue};
            llvm::Value *gep = builder_->CreateGEP(baseAlloca->getAllocatedType(), baseAlloca, indices, "idxrval");
            return builder_->CreateLoad(toLLVMType(r.elementTypeInfo), gep, "idxload");
        }
        else if constexpr (std::is_same_v<T, MirRvalueStructInit>) {
            llvm::Type *llvmStructType = toLLVMType(r.structTypeInfo);
            llvm::Value *structVal = llvm::UndefValue::get(llvmStructType);
            for (auto &f : r.fields) {
                llvm::Value *elemVal = emitOperand(f.value);
                if (!elemVal) return nullptr;
                structVal = builder_->CreateInsertValue(structVal, elemVal, f.fieldIndex, "structinit");
            }
            return structVal;
        }
        else if constexpr (std::is_same_v<T, MirRvalueFieldAccess>) {
            // If base is a pointer-to-struct, load the pointer and GEP into the struct
            if (r.base.typeInfo.pointerDepth > 0 && r.base.typeInfo.isStruct) {
                llvm::Value *ptr = emitOperand(r.base);
                if (!ptr) return nullptr;
                auto sit = structTypes_.find(r.base.typeInfo.structName);
                if (sit == structTypes_.end()) return nullptr;
                llvm::Value *fieldPtr = builder_->CreateStructGEP(sit->second, ptr, r.fieldIndex, "fieldptr");
                return builder_->CreateLoad(toLLVMType(r.fieldTypeInfo), fieldPtr, "fieldload");
            }
            llvm::Value *baseVal = emitOperand(r.base); 
            if (!baseVal) return nullptr;
            return builder_->CreateExtractValue(baseVal, r.fieldIndex, "fieldaccess");
        }
        else if constexpr (std::is_same_v<T, MirRvalueEnumInit>) {
            llvm::Type *enumTy = toLLVMType(r.enumTypeInfo);
            llvm::AllocaInst *tmpAlloca = createEntryBlockAlloca(func, "enum_init_tmp", enumTy);
            
            llvm::Value *tagVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), r.variantIndex);
            llvm::Value *tagPtr = builder_->CreateStructGEP(enumTy, tmpAlloca, 0, "enum_tag_ptr");
            builder_->CreateStore(tagVal, tagPtr);
            
            if (r.hasPayload) {
                llvm::Value *payloadVal = emitOperand(r.payload);
                if (!payloadVal) return nullptr;
                
                llvm::Value *payloadFieldPtr = builder_->CreateStructGEP(enumTy, tmpAlloca, 1, "enum_payload_ptr");
                builder_->CreateStore(payloadVal, payloadFieldPtr);
            }
            
            return builder_->CreateLoad(enumTy, tmpAlloca, "enum_val");
        }
        else if constexpr (std::is_same_v<T, MirRvalueEnumPayload>) {
            llvm::Value *enumVal = emitOperand(r.enumOperand);
            if (!enumVal) return nullptr;
            
            llvm::Type *enumTy = enumVal->getType();
            llvm::AllocaInst *tmpAlloca = createEntryBlockAlloca(func, "enum_ext_tmp", enumTy);
            builder_->CreateStore(enumVal, tmpAlloca);
            
            llvm::Value *payloadFieldPtr = builder_->CreateStructGEP(enumTy, tmpAlloca, 1, "ext_payload_ptr");
            llvm::Type *payloadTy = toLLVMType(r.payloadTypeInfo);
            
            return builder_->CreateLoad(payloadTy, payloadFieldPtr, "extracted_payload");
        }
        else if constexpr (std::is_same_v<T, MirRvalueEnumTag>) {
            llvm::Value *enumVal = emitOperand(r.enumOperand);
            if (!enumVal) return nullptr;
            return builder_->CreateExtractValue(enumVal, 0, "extracted_tag");
        }
        else if constexpr (std::is_same_v<T, MirRvalueSliceLen>) {
            llvm::Value *sliceVal = emitOperand(r.slice);
            if (!sliceVal) return nullptr;
            return builder_->CreateExtractValue(sliceVal, 1, "slice_len");
        }
        else if constexpr (std::is_same_v<T, MirRvalueSlicePtr>) {
            llvm::Value *sliceVal = emitOperand(r.slice);
            if (!sliceVal) return nullptr;
            return builder_->CreateExtractValue(sliceVal, 0, "slice_ptr");
        }
        else if constexpr (std::is_same_v<T, MirRvalueHeapAlloc>) {
            llvm::Type *elemType = toLLVMType(TypeInfo::scalar(r.allocatedType.kind));
            llvm::Value *sizeVal;
            if (r.isDynamicArray) {
                llvm::Value *numElems = emitOperand(r.size);
                if (numElems->getType() != llvm::Type::getInt64Ty(*context_)) {
                    numElems = builder_->CreateIntCast(numElems, llvm::Type::getInt64Ty(*context_), false, "size_cast");
                }
                uint64_t elemSize = module_->getDataLayout().getTypeAllocSize(elemType);
                llvm::Value *elemSizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), elemSize);
                sizeVal = builder_->CreateMul(numElems, elemSizeVal, "alloc_size");
            } else {
                uint64_t size = module_->getDataLayout().getTypeAllocSize(elemType);
                sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), size);
            }

            // Declare malloc: i8* malloc(i64)
            llvm::FunctionCallee mallocFunc = module_->getOrInsertFunction("malloc",
                llvm::FunctionType::get(llvm::PointerType::getUnqual(*context_),
                {llvm::Type::getInt64Ty(*context_)}, false));

            llvm::Value *ptr = builder_->CreateCall(mallocFunc, {sizeVal}, "malloc_tmp");
            // With opaque pointers, no bitcast needed — ptr is already opaque ptr type

            if (r.isDynamicArray) {
                // Return a fat pointer (slice): { T*, i64 }
                TypeInfo sliceTI = TypeInfo::slice(r.allocatedType.kind);
                llvm::Type *sliceType = toLLVMType(sliceTI);
                llvm::Value *slice = llvm::UndefValue::get(sliceType);
                llvm::Value *lenVal = emitOperand(r.size);
                if (lenVal->getType() != llvm::Type::getInt64Ty(*context_)) {
                    lenVal = builder_->CreateIntCast(lenVal, llvm::Type::getInt64Ty(*context_), false, "len_cast");
                }
                slice = builder_->CreateInsertValue(slice, ptr, 0);
                slice = builder_->CreateInsertValue(slice, lenVal, 1);
                return slice;
            }
            return ptr;
        }
        else if constexpr (std::is_same_v<T, MirRvalueSlice>) {
            llvm::Value *base = emitOperand(r.base);   // Could be array pointer or another slice
            llvm::Value *start = emitOperand(r.start);
            llvm::Value *end = emitOperand(r.end);     // Note: Codegen needs to handle "end" logic

            llvm::Value *ptr;
            if (r.base.typeInfo.isArray) {
                ptr = base;  // With opaque pointers, no bitcast needed
            } else {
                ptr = builder_->CreateExtractValue(base, 0, "slice_ptr");
            }

            llvm::Value *subPtr = builder_->CreateGEP(toLLVMType(TypeInfo::scalar(r.base.typeInfo.elementType)), ptr, {start}, "sub_ptr");

            llvm::Value *len = builder_->CreateSub(end, start, "slice_len");

            TypeInfo sliceTI = TypeInfo::slice(r.base.typeInfo.elementType);
            llvm::Type *sliceType = toLLVMType(sliceTI);
            llvm::Value *slice = llvm::UndefValue::get(sliceType);
            slice = builder_->CreateInsertValue(slice, subPtr, 0);
            slice = builder_->CreateInsertValue(slice, len, 1);
            return slice;
        }
        else if constexpr (std::is_same_v<T, MirRvalueStringLen>) {
            llvm::Value *strVal = emitOperand(r.operand);
            if (!strVal) return nullptr;
            // Declare strlen: i64 strlen(i8*)
            llvm::FunctionCallee strlenFunc = module_->getOrInsertFunction("strlen",
                llvm::FunctionType::get(llvm::Type::getInt64Ty(*context_),
                {llvm::PointerType::getUnqual(*context_)}, false));
            return builder_->CreateCall(strlenFunc, {strVal}, "str_len");
        }
        else if constexpr (std::is_same_v<T, MirRvalueZoneAlloc>) {
            llvm::Type *elemType = toLLVMType(r.allocatedType);
            llvm::Value *count = emitOperand(r.count);
            if (count->getType() != llvm::Type::getInt64Ty(*context_)) {
                count = builder_->CreateIntCast(count, llvm::Type::getInt64Ty(*context_), false, "count_cast");
            }
            uint64_t elemSize = module_->getDataLayout().getTypeAllocSize(elemType);
            llvm::Value *elemSizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), elemSize);
            llvm::Value *totalSize = builder_->CreateMul(count, elemSizeVal, "alloc_size");

            // Get the arena pointer for the zone
            llvm::Value *arena = nullptr;
            auto it = activeZones_.find(r.zoneName);
            if (it != activeZones_.end()) {
                arena = it->second;
            } else {
                // Fallback: if zone not active, use a global/null arena or error?
                // For now, assume it's active or return null
                arena = llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(*context_));
            }

            // Declare agam_zone_alloc: i8* agam_zone_alloc(i8*, i64)
            llvm::FunctionCallee allocFunc = module_->getOrInsertFunction("agam_zone_alloc",
                llvm::FunctionType::get(llvm::PointerType::getUnqual(*context_),
                {llvm::PointerType::getUnqual(*context_), llvm::Type::getInt64Ty(*context_)}, false));

            return builder_->CreateCall(allocFunc, {arena, totalSize}, "zone_alloc_tmp");
        }
        else if constexpr (std::is_same_v<T, MirRvalueBorrow>) {
            return emitOperand(r.target);
        }
        else if constexpr (std::is_same_v<T, MirRvalueEscape>) {
            return emitOperand(r.target);
        }
        else if constexpr (std::is_same_v<T, MirRvaluePtrOffset>) {
            llvm::Value *base = emitOperand(r.base);
            llvm::Value *offset = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), r.byteOffset);
            
            // In LLVM with opaque pointers, we just use GEP with i8 as element type
            llvm::Value *offsetted = builder_->CreateGEP(llvm::Type::getInt8Ty(*context_), base, offset);
            return builder_->CreateBitCast(offsetted, toLLVMType(r.resultType));
        }
        else {
            return nullptr;
        }
    }, rv);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Terminator Emission
// ═══════════════════════════════════════════════════════════════════════════════

void CodeGenerator::emitTerminator(const MirTerminator &term, llvm::Function *func,
                                    std::unordered_map<MirBlockId, llvm::BasicBlock *> &blockMap) {
    std::visit([this, func, &blockMap](auto &&t) {
        using T = std::decay_t<decltype(t)>;

        if constexpr (std::is_same_v<T, MirGoto>) {
            builder_->CreateBr(blockMap[t.target]);
        }
        else if constexpr (std::is_same_v<T, MirSwitchInt>) {
            llvm::Value *cond = emitOperand(t.discriminant);
            if (cond) {
                // Convert to i1 if not already
                if (cond->getType()->isIntegerTy(32)) {
                    cond = builder_->CreateICmpNE(
                        cond, llvm::ConstantInt::get(builder_->getContext(), llvm::APInt(32, 0)), "tobool");
                }
                builder_->CreateCondBr(cond, blockMap[t.thenBlock], blockMap[t.elseBlock]);
            }
        }
        else if constexpr (std::is_same_v<T, MirReturn>) {
            llvm::Value *val = emitOperand(t.value);
            if (val) {
                builder_->CreateRet(val);
            }
        }
        else if constexpr (std::is_same_v<T, MirReturnVoid>) {
            builder_->CreateRetVoid();
        }
        else if constexpr (std::is_same_v<T, MirUnreachable>) {
            builder_->CreateUnreachable();
        }
        else if constexpr (std::is_same_v<T, MirSwitchValue>) {
            llvm::Value *cond = emitOperand(t.discriminant);
            if (cond) {
                llvm::SwitchInst *switchInst = builder_->CreateSwitch(cond, blockMap[t.defaultBlock], t.targets.size());
                for (auto &pair : t.targets) {
                    llvm::ConstantInt *onVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), pair.first);
                    switchInst->addCase(onVal, blockMap[pair.second]);
                }
            }
        }
    }, term);
}

} // namespace agam
