#include "agam/mir/mir_builder.h"

#include <cassert>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  Block / Local Helpers
// ═══════════════════════════════════════════════════════════════════════════════

MirBlockId MirBuilder::newBlock(const std::string &label) {
    MirBlockId id = nextBlockId_++;
    currentFunc_->blocks.push_back({id, label, {}, MirUnreachable{}});
    return id;
}

MirBasicBlock &MirBuilder::block(MirBlockId id) {
    return currentFunc_->blocks[id];
}

void MirBuilder::setTerminator(MirBlockId blockId, MirTerminator term) {
    block(blockId).terminator = std::move(term);
}

MirLocalId MirBuilder::newLocal(TypeInfo typeInfo, const std::string &name, bool isTemp) {
    MirLocalId id = nextLocalId_++;
    currentFunc_->locals.push_back({id, typeInfo, name, isTemp});
    return id;
}

MirLocalId MirBuilder::newTemp(TypeInfo typeInfo) {
    return newLocal(typeInfo, "", true);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Top-Level Build
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<MirProgram> MirBuilder::build(ThirProgram &program) {
    auto mirProg = std::make_unique<MirProgram>();
    mirProg->structs = program.structs;
    mirProg->enums = program.enums;

    // ── Constants (must be before functions so VarRefs can find them) ─────
    for (auto &cn : program.constants) {
        MirConstDef mcn;
        mcn.name = cn->name;
        mcn.typeInfo = cn->typeInfo;

        ThirExpr *expr = cn->value.get();
        // Unwrap casts for constants
        while (auto *cast = dynamic_cast<ThirCastExpr *>(expr)) {
            expr = cast->operand.get();
        }

        if (auto *lit = dynamic_cast<ThirIntLiteral *>(expr)) {
            mcn.value = MirConstInt{lit->value};
        } else if (auto *flit = dynamic_cast<ThirFloatLiteral *>(expr)) {
            mcn.value = MirConstFloat{flit->value};
        } else if (auto *blit = dynamic_cast<ThirBoolLiteral *>(expr)) {
            mcn.value = MirConstBool{blit->value};
        } else if (auto *slit = dynamic_cast<ThirStringLiteral *>(expr)) {
            mcn.value = MirConstString{slit->value};
        } else {
            mcn.value = MirConstInt{0};
        }

        constantMap_[cn->id] = mcn.value;
        mirProg->constants.push_back(std::move(mcn));
    }

    // ── Functions ───────────────────────────────────────────────────────────
    for (auto &fn : program.functions) {
        MirFunction mirFunc;
        mirFunc.name = fn->name;
        mirFunc.returnTypeInfo = fn->returnTypeInfo;
        mirFunc.isExtern = fn->isExtern;
        mirProg->functions.push_back(std::move(mirFunc));
        currentFunc_ = &mirProg->functions.back();
        nextBlockId_ = 0;
        nextLocalId_ = 0;
        varMap_.clear();
        activeZones_.clear();
        lowerFunc(*fn);
    }

    return mirProg;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Function Lowering
// ═══════════════════════════════════════════════════════════════════════════════

void MirBuilder::lowerFunc(ThirFuncDecl &fn) {
    // Create locals for parameters.
    for (auto &p : fn.params) {
        MirLocalId localId = newLocal(p.typeInfo, p.name, false);
        varMap_[p.id] = localId;
        currentFunc_->params.push_back({localId, p.typeInfo, p.name, false});
    }

    // Lower body if not extern.
    if (!fn.isExtern) {
        // Create entry block.
        MirBlockId entryBlock = newBlock("entry");
        lowerBlock(*fn.body, entryBlock, nullptr);
    }

    // If the entry block or last block still has MirUnreachable, add appropriate return.
    for (auto &bb : currentFunc_->blocks) {
        if (std::holds_alternative<MirUnreachable>(bb.terminator)) {
            if (fn.returnTypeInfo.kind == TypeKind::Void) {
                bb.terminator = MirReturnVoid{};
            }
            // Non-void blocks with MirUnreachable are dead code — codegen will handle.
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Block / Statement Lowering
// ═══════════════════════════════════════════════════════════════════════════════

void MirBuilder::lowerBlock(ThirBlock &thirBlock, MirBlockId &currentBlock,
                            MirBlockId *continueBlockId) {
    for (auto &stmt : thirBlock.stmts) {
        lowerStmt(*stmt, currentBlock, continueBlockId);
    }
}

void MirBuilder::lowerStmt(ThirStmt &stmt, MirBlockId &currentBlock, MirBlockId *continueBlockId) {
    // VarDecl
    if (auto *vd = dynamic_cast<ThirVarDecl *>(&stmt)) {
        MirLocalId localId = newLocal(vd->declaredTypeInfo, vd->name, false);
        varMap_[vd->id] = localId;
        if (vd->initializer) {
            MirOperand val = lowerExpr(*vd->initializer, currentBlock);
            block(currentBlock).statements.push_back(MirAssign{{localId}, MirRvalueUse{val}});
        }
        return;
    }

    // Return
    if (auto *ret = dynamic_cast<ThirReturn *>(&stmt)) {
        if (ret->value) {
            MirOperand val = lowerExpr(*ret->value, currentBlock);

            // Cleanup active zones before return
            for (auto it = activeZones_.rbegin(); it != activeZones_.rend(); ++it) {
                block(currentBlock).statements.push_back(MirZoneEnd{*it});
            }

            setTerminator(currentBlock, MirReturn{val, true});
        } else {
            // Cleanup active zones before return void
            for (auto it = activeZones_.rbegin(); it != activeZones_.rend(); ++it) {
                block(currentBlock).statements.push_back(MirZoneEnd{*it});
            }
            setTerminator(currentBlock, MirReturnVoid{});
        }
        // Start a new unreachable block for any dead code after return.
        currentBlock = newBlock("post_return");
        return;
    }

    // If
    if (auto *ifS = dynamic_cast<ThirIf *>(&stmt)) {
        MirOperand cond = lowerExpr(*ifS->condition, currentBlock);

        MirBlockId thenBlock = newBlock("then");
        MirBlockId mergeBlock = newBlock("merge");
        MirBlockId elseBlock = ifS->elseBranch ? newBlock("else") : mergeBlock;

        setTerminator(currentBlock, MirSwitchInt{cond, thenBlock, elseBlock});

        // Then branch
        lowerBlock(*ifS->thenBranch, thenBlock, continueBlockId);
        // If then block hasn't been terminated, goto merge
        {
            auto &tb = block(thenBlock);
            if (std::holds_alternative<MirUnreachable>(tb.terminator)) {
                setTerminator(thenBlock, MirGoto{mergeBlock});
            }
        }

        // Else branch
        if (ifS->elseBranch) {
            lowerBlock(*ifS->elseBranch, elseBlock, continueBlockId);
            auto &eb = block(elseBlock);
            if (std::holds_alternative<MirUnreachable>(eb.terminator)) {
                setTerminator(elseBlock, MirGoto{mergeBlock});
            }
        }

        currentBlock = mergeBlock;
        return;
    }

    // While
    if (auto *whileS = dynamic_cast<ThirWhile *>(&stmt)) {
        MirBlockId condBlock = newBlock("whilecond");
        MirBlockId bodyBlock = newBlock("whilebody");
        MirBlockId endBlock = newBlock("whileend");

        setTerminator(currentBlock, MirGoto{condBlock});

        // Condition
        MirOperand cond = lowerExpr(*whileS->condition, condBlock);
        setTerminator(condBlock, MirSwitchInt{cond, bodyBlock, endBlock});

        // Body
        lowerBlock(*whileS->body, bodyBlock, &condBlock);
        auto &bb = block(bodyBlock);
        if (std::holds_alternative<MirUnreachable>(bb.terminator)) {
            setTerminator(bodyBlock, MirGoto{condBlock});
        }

        currentBlock = endBlock;
        return;
    }

    // For
    if (auto *forS = dynamic_cast<ThirFor *>(&stmt)) {
        MirBlockId condBlock = newBlock("forcond");
        MirBlockId bodyBlock = newBlock("forbody");
        MirBlockId incrBlock = newBlock("forincr");
        MirBlockId endBlock = newBlock("forend");

        // 1. Evaluate iterable
        MirOperand iterable = lowerExpr(*forS->iterable, currentBlock);

        // 2. Create index local
        MirLocalId indexLocal = newTemp(TypeInfo::scalar(TypeKind::Int64));
        block(currentBlock)
            .statements.push_back(
                MirAssign{{indexLocal}, MirRvalueUse{MirOperand::makeConstInt64(0)}});

        // 3. Get length
        MirLocalId lenLocal = newTemp(TypeInfo::scalar(TypeKind::Int64));
        if (iterable.typeInfo.isArray) {
            block(currentBlock)
                .statements.push_back(MirAssign{
                    {lenLocal},
                    MirRvalueUse{MirOperand::makeConstInt64(iterable.typeInfo.arraySize)}});
        } else if (iterable.typeInfo.isSlice) {
            block(currentBlock)
                .statements.push_back(MirAssign{{lenLocal}, MirRvalueSliceLen{iterable}});
        } else {
            // Integer range: len = iterable
            block(currentBlock).statements.push_back(MirAssign{{lenLocal}, MirRvalueUse{iterable}});
        }

        setTerminator(currentBlock, MirGoto{condBlock});

        // 4. Condition: index < len
        MirLocalId condLocal = newTemp(TypeInfo::scalar(TypeKind::Bool));
        block(condBlock).statements.push_back(MirAssign{
            {condLocal},
            MirRvalueBinaryOp{BinaryOp::Lt,
                              MirOperand::makeCopy(indexLocal, TypeInfo::scalar(TypeKind::Int64)),
                              MirOperand::makeCopy(lenLocal, TypeInfo::scalar(TypeKind::Int64))}});
        setTerminator(condBlock, MirSwitchInt{MirOperand::makeCopy(
                                                  condLocal, TypeInfo::scalar(TypeKind::Bool)),
                                              bodyBlock, endBlock});

        // 5. Body: x = iterable[index] (or x = index for ranges)
        MirLocalId loopVarLocal = newLocal(forS->varType, forS->varName, false);
        varMap_[forS->varId] = loopVarLocal;

        if (iterable.typeInfo.isArray || iterable.typeInfo.isSlice) {
            block(bodyBlock).statements.push_back(MirAssign{
                {loopVarLocal},
                MirRvalueIndex{iterable,
                               MirOperand::makeCopy(indexLocal, TypeInfo::scalar(TypeKind::Int64)),
                               TypeInfo::scalar(iterable.typeInfo.elementType)}});
        } else {
            // Integer range: x = index
            block(bodyBlock).statements.push_back(MirAssign{
                {loopVarLocal},
                MirRvalueUse{MirOperand::makeCopy(indexLocal, TypeInfo::scalar(TypeKind::Int64))}});
        }

        lowerBlock(*forS->body, bodyBlock, &incrBlock);
        if (std::holds_alternative<MirUnreachable>(block(bodyBlock).terminator)) {
            setTerminator(bodyBlock, MirGoto{incrBlock});
        }

        // 6. Increment: index = index + 1
        block(incrBlock).statements.push_back(MirAssign{
            {indexLocal},
            MirRvalueBinaryOp{BinaryOp::Add,
                              MirOperand::makeCopy(indexLocal, TypeInfo::scalar(TypeKind::Int64)),
                              MirOperand::makeConstInt64(1)}});
        setTerminator(incrBlock, MirGoto{condBlock});

        currentBlock = endBlock;
        return;
    }

    // Block (nested)
    if (auto *blk = dynamic_cast<ThirBlock *>(&stmt)) {
        lowerBlock(*blk, currentBlock, continueBlockId);
        return;
    }

    // ExprStmt
    if (auto *exprS = dynamic_cast<ThirExprStmt *>(&stmt)) {
        lowerExpr(*exprS->expr, currentBlock);
        return;
    }

    // DeleteStmt
    if (auto *delS = dynamic_cast<ThirDeleteStmt *>(&stmt)) {
        MirOperand ptr = lowerExpr(*delS->pointer, currentBlock);
        block(currentBlock).statements.push_back(MirHeapFree{ptr});
        return;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expression Lowering — flatten to temporaries
// ═══════════════════════════════════════════════════════════════════════════════

MirOperand MirBuilder::lowerExpr(ThirExpr &expr, MirBlockId &currentBlock) {
    // Int literal — use the typed kind from THIR (TypeKind::Int unless explicitly sized)
    if (auto *lit = dynamic_cast<ThirIntLiteral *>(&expr)) {
        return MirOperand::makeConstInt(lit->value, lit->typeInfo);
    }

    // Float literal
    if (auto *lit = dynamic_cast<ThirFloatLiteral *>(&expr)) {
        return MirOperand::makeConstFloat(lit->value);
    }

    // Bool literal
    if (auto *lit = dynamic_cast<ThirBoolLiteral *>(&expr)) {
        return MirOperand::makeConstBool(lit->value);
    }

    // Null literal
    if (auto *lit = dynamic_cast<ThirNullLiteral *>(&expr)) {
        return MirOperand::makeConstNull(lit->typeInfo);
    }

    // String literal
    if (auto *lit = dynamic_cast<ThirStringLiteral *>(&expr)) {
        return MirOperand::makeConstString(lit->value);
    }

    // Var ref → copy from the local, OR lookup constant
    if (auto *var = dynamic_cast<ThirVarRef *>(&expr)) {
        auto itLocal = varMap_.find(var->defId);
        if (itLocal != varMap_.end()) {
            return MirOperand::makeCopy(itLocal->second, var->typeInfo);
        }

        auto itConst = constantMap_.find(var->defId);
        if (itConst != constantMap_.end()) {
            MirOperand op;
            op.kind = MirOperand::Kind::Constant;
            op.constant = itConst->second;
            op.typeInfo = var->typeInfo;
            return op;
        }

        return MirOperand::makeConstInt(0);
    }

    // Binary expression → attempt folding, then lower operands, emit assign to temp
    if (auto *bin = dynamic_cast<ThirBinaryExpr *>(&expr)) {
        MirOperand lhs = lowerExpr(*bin->lhs, currentBlock);
        MirOperand rhs = lowerExpr(*bin->rhs, currentBlock);

        if (auto folded = foldBinaryOp(bin->op, lhs, rhs)) {
            return *folded;
        }

        MirLocalId tmp = newTemp(bin->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{{tmp}, MirRvalueBinaryOp{bin->op, lhs, rhs}});
        return MirOperand::makeCopy(tmp, bin->typeInfo);
    }

    // Unary expression
    if (auto *un = dynamic_cast<ThirUnaryExpr *>(&expr)) {
        MirOperand operand = lowerExpr(*un->operand, currentBlock);

        if (auto folded = foldUnaryOp(un->op, operand)) {
            return *folded;
        }

        MirLocalId tmp = newTemp(un->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{{tmp}, MirRvalueUnaryOp{un->op, operand}});
        return MirOperand::makeCopy(tmp, un->typeInfo);
    }

    // Cast expression
    if (auto *cast = dynamic_cast<ThirCastExpr *>(&expr)) {
        MirOperand operand = lowerExpr(*cast->operand, currentBlock);
        MirLocalId tmp = newTemp(cast->typeInfo);
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp}, MirRvalueCast{operand, cast->fromTypeInfo, cast->typeInfo}});
        return MirOperand::makeCopy(tmp, cast->typeInfo);
    }

    // Call expression
    if (auto *call = dynamic_cast<ThirCallExpr *>(&expr)) {
        std::vector<MirOperand> args;
        for (auto &arg : call->args) {
            args.push_back(lowerExpr(*arg, currentBlock));
        }
        MirLocalId tmp = newTemp(call->typeInfo);
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp}, MirRvalueCall{call->calleeName, std::move(args), call->typeInfo}});
        return MirOperand::makeCopy(tmp, call->typeInfo);
    }

    // Assign expression → store to the target's local
    if (auto *assign = dynamic_cast<ThirAssignExpr *>(&expr)) {
        MirOperand val = lowerExpr(*assign->value, currentBlock);
        auto it = varMap_.find(assign->targetId);
        if (it != varMap_.end()) {
            block(currentBlock).statements.push_back(MirAssign{{it->second}, MirRvalueUse{val}});
        }
        return val;
    }

    // Array literal -> emit MirRvalueArrayInit
    if (auto *arr = dynamic_cast<ThirArrayLiteral *>(&expr)) {
        std::vector<MirOperand> elems;
        for (auto &e : arr->elements) {
            elems.push_back(lowerExpr(*e, currentBlock));
        }
        MirLocalId tmp = newTemp(arr->typeInfo);
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp}, MirRvalueArrayInit{std::move(elems), arr->typeInfo}});
        return MirOperand::makeCopy(tmp, arr->typeInfo);
    }

    // Index expression -> emit MirRvalueIndex
    if (auto *idx = dynamic_cast<ThirIndexExpr *>(&expr)) {
        MirOperand baseOp = lowerExpr(*idx->base, currentBlock);
        MirOperand indexOp = lowerExpr(*idx->index, currentBlock);

        if (idx->isRange) {
            MirOperand endOp = idx->endIndex ? lowerExpr(*idx->endIndex, currentBlock)
                                             : MirOperand::makeConstInt(0);
            MirLocalId tmp = newTemp(idx->typeInfo);
            block(currentBlock)
                .statements.push_back(MirAssign{{tmp}, MirRvalueSlice{baseOp, indexOp, endOp}});
            return MirOperand::makeCopy(tmp, idx->typeInfo);
        } else {
            MirLocalId tmp = newTemp(idx->typeInfo);
            block(currentBlock)
                .statements.push_back(
                    MirAssign{{tmp}, MirRvalueIndex{baseOp, indexOp, idx->typeInfo}});
            return MirOperand::makeCopy(tmp, idx->typeInfo);
        }
    }

    if (auto *slen = dynamic_cast<ThirSliceLen *>(&expr)) {
        MirOperand sliceOp = lowerExpr(*slen->slice, currentBlock);
        MirLocalId tmp = newTemp(slen->typeInfo);
        block(currentBlock).statements.push_back(MirAssign{{tmp}, MirRvalueSliceLen{sliceOp}});
        return MirOperand::makeCopy(tmp, slen->typeInfo);
    }

    if (auto *sptr = dynamic_cast<ThirSlicePtr *>(&expr)) {
        MirOperand sliceOp = lowerExpr(*sptr->slice, currentBlock);
        MirLocalId tmp = newTemp(sptr->typeInfo);
        block(currentBlock).statements.push_back(MirAssign{{tmp}, MirRvalueSlicePtr{sliceOp}});
        return MirOperand::makeCopy(tmp, sptr->typeInfo);
    }

    if (auto *strlen = dynamic_cast<ThirStringLen *>(&expr)) {
        MirOperand op = lowerExpr(*strlen->operand, currentBlock);
        MirLocalId tmp = newTemp(strlen->typeInfo);
        block(currentBlock).statements.push_back(MirAssign{{tmp}, MirRvalueStringLen{op}});
        return MirOperand::makeCopy(tmp, strlen->typeInfo);
    }

    // IndexAssign -> emit MirIndexAssign
    if (auto *idxAssign = dynamic_cast<ThirIndexAssign *>(&expr)) {
        MirOperand baseOp = lowerExpr(*idxAssign->base, currentBlock);
        MirOperand indexOp = lowerExpr(*idxAssign->index, currentBlock);
        MirOperand valOp = lowerExpr(*idxAssign->value, currentBlock);
        block(currentBlock)
            .statements.push_back(MirIndexAssign{baseOp, indexOp, valOp, idxAssign->typeInfo});
        return valOp;
    }

    // Struct literal -> emit MirRvalueStructInit
    if (auto *slit = dynamic_cast<ThirStructLiteral *>(&expr)) {
        std::vector<MirStructFieldInit> fields;
        for (auto &f : slit->fields) {
            MirOperand val = lowerExpr(*f.value, currentBlock);
            fields.push_back({f.name, f.fieldIndex, val});
        }
        MirLocalId tmp = newTemp(slit->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{
                {tmp}, MirRvalueStructInit{slit->structName, std::move(fields), slit->typeInfo}});
        return MirOperand::makeCopy(tmp, slit->typeInfo);
    }

    // Field access -> emit MirRvalueFieldAccess or .len special handling
    if (auto *fa = dynamic_cast<ThirFieldAccess *>(&expr)) {
        MirOperand baseOp = lowerExpr(*fa->base, currentBlock);

        if (fa->fieldIndex == -1 && fa->field == "len") {
            if (fa->base->typeInfo.isArray) {
                return MirOperand::makeConstInt64(fa->base->typeInfo.arraySize);
            } else if (fa->base->typeInfo.isSlice) {
                MirLocalId tmp = newTemp(fa->typeInfo);
                block(currentBlock)
                    .statements.push_back(MirAssign{{tmp}, MirRvalueSliceLen{baseOp}});
                return MirOperand::makeCopy(tmp, fa->typeInfo);
            }
        }

        MirLocalId tmp = newTemp(fa->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{
                {tmp}, MirRvalueFieldAccess{baseOp, fa->field, fa->fieldIndex, fa->typeInfo}});
        return MirOperand::makeCopy(tmp, fa->typeInfo);
    }

    // Field assign -> emit MirFieldAssign
    if (auto *fassign = dynamic_cast<ThirFieldAssign *>(&expr)) {
        MirOperand baseOp = lowerExpr(*fassign->base, currentBlock);
        MirOperand valOp = lowerExpr(*fassign->value, currentBlock);
        block(currentBlock)
            .statements.push_back(
                MirFieldAssign{baseOp, fassign->field, fassign->fieldIndex, valOp});
        return valOp;
    }

    // Deref expression -> emit MirDerefAssign
    if (auto *derefAssign = dynamic_cast<ThirDerefAssign *>(&expr)) {
        MirOperand ptrOp = lowerExpr(*derefAssign->pointer, currentBlock);
        MirOperand valOp = lowerExpr(*derefAssign->value, currentBlock);
        block(currentBlock).statements.push_back(MirDerefAssign{ptrOp, valOp});
        return valOp;
    }

    // Enum variant initialization -> emit MirRvalueEnumInit
    if (auto *eve = dynamic_cast<ThirEnumVariantExpr *>(&expr)) {
        MirOperand payloadOp = MirOperand::makeConstInt(0); // default dummy
        bool hasPayload = false;
        if (eve->payload) {
            hasPayload = true;
            payloadOp = lowerExpr(*eve->payload, currentBlock);
        }
        MirLocalId tmp = newTemp(eve->typeInfo);
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp},
                          MirRvalueEnumInit{eve->enumName, eve->variantIndex, hasPayload, payloadOp,
                                            eve->typeInfo}});
        return MirOperand::makeCopy(tmp, eve->typeInfo);
    }

    // Match expression
    if (auto *me = dynamic_cast<ThirMatchExpr *>(&expr)) {
        // Evaluate the discriminant (the enum value)
        MirOperand valOp = lowerExpr(*me->value, currentBlock);

        // We need a temp to store the result of the match expression (if not void)
        MirLocalId resultTmp = 0;
        bool hasResult =
            (me->typeInfo.kind != TypeKind::Void && me->typeInfo.kind != TypeKind::Unknown);
        if (hasResult) {
            resultTmp = newTemp(me->typeInfo);
        }

        // Block to jump to after the match
        MirBlockId endBlock = newBlock("matchend");

        // Extract the tag using MirRvalueEnumTag
        MirLocalId tagTmp = newTemp(TypeInfo::scalar(TypeKind::Int32));
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tagTmp}, MirRvalueEnumTag{valOp, TypeInfo::scalar(TypeKind::Int32)}});
        MirOperand tagOp = MirOperand::makeCopy(tagTmp, TypeInfo::scalar(TypeKind::Int32));

        // Prepare MirSwitchValue
        MirSwitchValue switchVal;
        switchVal.discriminant = tagOp;

        // Default block: unreachable panics
        MirBlockId defaultBlock = newBlock("matchdefault");
        setTerminator(defaultBlock, MirUnreachable{});
        switchVal.defaultBlock = defaultBlock;

        // We must preserve the current block ID to assign the switch terminator
        MirBlockId switchBlock = currentBlock;

        for (auto &arm : me->arms) {
            MirBlockId armBlock = newBlock("matcharm_" + arm.variantName);
            switchVal.targets.push_back({arm.variantIndex, armBlock});

            // Generate code for the arm
            currentBlock = armBlock;

            // If the arm has a binding, extract the payload
            if (arm.hasBinding) {
                // New local for the binding
                MirLocalId bindingLocal = newLocal(arm.bindingType, arm.bindingName, false);
                varMap_[arm.bindingId] = bindingLocal;

                block(currentBlock)
                    .statements.push_back(
                        MirAssign{{bindingLocal}, MirRvalueEnumPayload{valOp, arm.bindingType}});
            }

            // Evaluate body
            MirOperand armResultOp;
            if (arm.exprBody) {
                armResultOp = lowerExpr(*arm.exprBody, currentBlock);
            } else if (arm.blockBody) {
                lowerBlock(*arm.blockBody, currentBlock, nullptr);
                if (hasResult) {
                    armResultOp = MirOperand::makeConstInt(0); // Dummy for blocks
                }
            }

            // If the arm evaluates to a value, assign it to resultTmp
            if (hasResult && arm.exprBody) {
                block(currentBlock)
                    .statements.push_back(MirAssign{{resultTmp}, MirRvalueUse{armResultOp}});
            }

            // Terminate arm block by jumping to endBlock
            setTerminator(currentBlock, MirGoto{endBlock});
        }

        // Terminate the block containing the switch
        setTerminator(switchBlock, switchVal);

        // Continue from endBlock
        currentBlock = endBlock;

        if (hasResult) {
            return MirOperand::makeCopy(resultTmp, me->typeInfo);
        } else {
            return MirOperand::makeConstInt(0); // Void dummy
        }
    }

    // New expression
    if (auto *newE = dynamic_cast<ThirNewExpr *>(&expr)) {
        MirLocalId tmp = newTemp(newE->typeInfo);
        MirOperand sizeOp;
        bool isDynamic = false;
        if (newE->sizeExpr) {
            sizeOp = lowerExpr(*newE->sizeExpr, currentBlock);
            isDynamic = true;
        }
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp}, MirRvalueHeapAlloc{newE->allocatedType, sizeOp, isDynamic}});
        return MirOperand::makeCopy(tmp, newE->typeInfo);
    }

    // ZoneExpr
    if (auto *zoneE = dynamic_cast<ThirZoneExpr *>(&expr)) {
        block(currentBlock).statements.push_back(MirZoneBegin{zoneE->zoneName});
        activeZones_.push_back(zoneE->zoneName);

        lowerBlock(*zoneE->body, currentBlock, nullptr);

        // Only pop and emit End if the block wasn't terminated (e.g. by return)
        if (!activeZones_.empty() && activeZones_.back() == zoneE->zoneName) {
            block(currentBlock).statements.push_back(MirZoneEnd{zoneE->zoneName});
            activeZones_.pop_back();
        }

        // Zone returns Void
        return MirOperand::makeConstInt(0, TypeInfo::scalar(TypeKind::Void));
    }

    // AllocExpr
    if (auto *allocE = dynamic_cast<ThirAllocExpr *>(&expr)) {
        MirOperand countOp = MirOperand::makeConstInt64(1);
        if (allocE->count)
            countOp = lowerExpr(*allocE->count, currentBlock);

        MirLocalId tmp = newTemp(allocE->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{
                {tmp}, MirRvalueZoneAlloc{allocE->allocatedType, countOp, allocE->zoneName}});
        return MirOperand::makeCopy(tmp, allocE->typeInfo);
    }

    // BorrowExpr
    if (auto *borrowE = dynamic_cast<ThirBorrowExpr *>(&expr)) {
        MirOperand targetOp = lowerExpr(*borrowE->target, currentBlock);
        MirLocalId tmp = newTemp(borrowE->typeInfo);
        block(currentBlock)
            .statements.push_back(MirAssign{{tmp}, MirRvalueBorrow{borrowE->isMutable, targetOp}});
        return MirOperand::makeCopy(tmp, borrowE->typeInfo);
    }

    // EscapeExpr
    if (auto *escapeE = dynamic_cast<ThirEscapeExpr *>(&expr)) {
        MirOperand targetOp = lowerExpr(*escapeE->target, currentBlock);
        MirLocalId tmp = newTemp(escapeE->typeInfo);
        block(currentBlock)
            .statements.push_back(
                MirAssign{{tmp}, MirRvalueEscape{targetOp, escapeE->destinationZone}});
        return MirOperand::makeCopy(tmp, escapeE->typeInfo);
    }

    assert(false && "unsupported THIR expression in MIR builder");
    // Fallback
    return MirOperand::makeConstInt(0);
}

std::optional<MirOperand> MirBuilder::foldBinaryOp(BinaryOp op, const MirOperand &lhs,
                                                   const MirOperand &rhs) {
    if (lhs.kind != MirOperand::Kind::Constant || rhs.kind != MirOperand::Kind::Constant) {
        return std::nullopt;
    }

    // Integer folding
    if (std::holds_alternative<MirConstInt>(lhs.constant) &&
        std::holds_alternative<MirConstInt>(rhs.constant)) {
        int64_t l = std::get<MirConstInt>(lhs.constant).value;
        int64_t r = std::get<MirConstInt>(rhs.constant).value;
        TypeInfo resType = lhs.typeInfo;

        switch (op) {
        case BinaryOp::Add:
            return MirOperand::makeConstInt(l + r, resType);
        case BinaryOp::Sub:
            return MirOperand::makeConstInt(l - r, resType);
        case BinaryOp::Mul:
            return MirOperand::makeConstInt(l * r, resType);
        case BinaryOp::Div:
            if (r != 0)
                return MirOperand::makeConstInt(l / r, resType);
            break;
        case BinaryOp::Mod:
            if (r != 0)
                return MirOperand::makeConstInt(l % r, resType);
            break;
        case BinaryOp::Eq:
            return MirOperand::makeConstBool(l == r);
        case BinaryOp::Neq:
            return MirOperand::makeConstBool(l != r);
        case BinaryOp::Lt:
            return MirOperand::makeConstBool(l < r);
        case BinaryOp::Gt:
            return MirOperand::makeConstBool(l > r);
        case BinaryOp::Lte:
            return MirOperand::makeConstBool(l <= r);
        case BinaryOp::Gte:
            return MirOperand::makeConstBool(l >= r);
        default:
            break;
        }
    }

    // Float folding
    if (std::holds_alternative<MirConstFloat>(lhs.constant) &&
        std::holds_alternative<MirConstFloat>(rhs.constant)) {
        double l = std::get<MirConstFloat>(lhs.constant).value;
        double r = std::get<MirConstFloat>(rhs.constant).value;

        switch (op) {
        case BinaryOp::Add:
            return MirOperand::makeConstFloat(l + r);
        case BinaryOp::Sub:
            return MirOperand::makeConstFloat(l - r);
        case BinaryOp::Mul:
            return MirOperand::makeConstFloat(l * r);
        case BinaryOp::Div:
            if (r != 0.0)
                return MirOperand::makeConstFloat(l / r);
            break;
        case BinaryOp::Eq:
            return MirOperand::makeConstBool(l == r);
        case BinaryOp::Neq:
            return MirOperand::makeConstBool(l != r);
        case BinaryOp::Lt:
            return MirOperand::makeConstBool(l < r);
        case BinaryOp::Gt:
            return MirOperand::makeConstBool(l > r);
        case BinaryOp::Lte:
            return MirOperand::makeConstBool(l <= r);
        case BinaryOp::Gte:
            return MirOperand::makeConstBool(l >= r);
        default:
            break;
        }
    }

    // Bool folding
    if (std::holds_alternative<MirConstBool>(lhs.constant) &&
        std::holds_alternative<MirConstBool>(rhs.constant)) {
        bool l = std::get<MirConstBool>(lhs.constant).value;
        bool r = std::get<MirConstBool>(rhs.constant).value;

        switch (op) {
        case BinaryOp::And:
            return MirOperand::makeConstBool(l && r);
        case BinaryOp::Or:
            return MirOperand::makeConstBool(l || r);
        case BinaryOp::Eq:
            return MirOperand::makeConstBool(l == r);
        case BinaryOp::Neq:
            return MirOperand::makeConstBool(l != r);
        default:
            break;
        }
    }

    return std::nullopt;
}

std::optional<MirOperand> MirBuilder::foldUnaryOp(UnaryOp op, const MirOperand &operand) {
    if (operand.kind != MirOperand::Kind::Constant) {
        return std::nullopt;
    }

    if (std::holds_alternative<MirConstInt>(operand.constant)) {
        int64_t v = std::get<MirConstInt>(operand.constant).value;
        if (op == UnaryOp::Negate)
            return MirOperand::makeConstInt(-v, operand.typeInfo);
    }

    if (std::holds_alternative<MirConstFloat>(operand.constant)) {
        double v = std::get<MirConstFloat>(operand.constant).value;
        if (op == UnaryOp::Negate)
            return MirOperand::makeConstFloat(-v);
    }

    if (std::holds_alternative<MirConstBool>(operand.constant)) {
        bool v = std::get<MirConstBool>(operand.constant).value;
        if (op == UnaryOp::Not)
            return MirOperand::makeConstBool(!v);
    }

    return std::nullopt;
}

} // namespace agam
