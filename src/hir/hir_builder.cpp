#include "agam/hir/hir_builder.h"
#include <cassert>
#include <iostream>
#include <sstream>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  Scope Management
// ═══════════════════════════════════════════════════════════════════════════════

void HirBuilder::pushScope() {
    scopes_.push_back({});
}

void HirBuilder::popScope() {
    assert(!scopes_.empty());
    scopes_.pop_back();
}

void HirBuilder::define(const std::string &name, HirId id) {
    assert(!scopes_.empty());
    scopes_.back().names[name] = id;
}

HirId HirBuilder::resolve(const std::string &name, const SourceLocation &loc) {
    // Walk scopes from innermost to outermost.
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->names.find(name);
    if (found != it->names.end()) {
        return found->second;
    }
}
error(loc, "undeclared identifier '" + name + "'");
return INVALID_HIR_ID;
}

void HirBuilder::error(const SourceLocation &loc, const std::string &msg) {
    std::ostringstream oss;
    oss << "HIR error (line " << loc.line << ", col " << loc.column << "): " << msg;
    errors_.push_back(oss.str());
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Top-Level Build
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<HirProgram> HirBuilder::build(Program &program) {
    auto hirProg = std::make_unique<HirProgram>();
    errors_.clear();
    nextId_ = 1;
    scopes_.clear();

    // Global scope: register all functions first (forward declarations).
    pushScope();
    for (auto &fn : program.functions) {
        if (!fn->typeParams.empty()) {
            continue;
        }
        HirId fnId = allocId();
        define(fn->name, fnId);
    }

    for (auto &cn : program.constants) {
        HirId cnId = allocId();
        define(cn->name, cnId);
    }

    // Register all methods in impl blocks (skip generic impls)
    for (auto &impl : program.impls) {
        if (!impl->typeParams.empty()) continue; // Skip generic impl templates
        std::string typeStr;
        if (impl->targetType.isStruct) typeStr = impl->targetType.structName;
        else if (impl->targetType.isEnum) typeStr = impl->targetType.enumName;
        else typeStr = typeKindToString(impl->targetType.kind);

        for (auto &method : impl->methods) {
            std::string mangledName;
            if (impl->traitName.empty()) {
                mangledName = "_Inherent_" + typeStr + "_" + method->name;
            } else {
                mangledName = "_Impl_" + impl->traitName + "_for_" + typeStr + "_" + method->name;
            }
            HirId mId = allocId();
            define(mangledName, mId);
        }
    }

    // Now lower each function.
    // We need to re-iterate because IDs were allocated in order above.
    HirId fnId = 1;  // first allocated ID
    for (auto &fn : program.functions) {
        if (!fn->typeParams.empty()) continue; // Skip generic
        // The fnId was already allocated in the loop above, re-resolve it.
        HirId resolvedId = resolve(fn->name, fn->loc);
        hirProg->functions.push_back(lowerFunc(*fn));
        hirProg->functions.back()->id = resolvedId;
    }

    for (auto &impl : program.impls) {
        if (!impl->typeParams.empty()) continue; // Skip generic impl templates
        std::string typeStr;
        if (impl->targetType.isStruct) typeStr = impl->targetType.structName;
        else if (impl->targetType.isEnum) typeStr = impl->targetType.enumName;
        else typeStr = typeKindToString(impl->targetType.kind);

        for (auto &method : impl->methods) {
            std::string mangledName;
            if (impl->traitName.empty()) {
                mangledName = "_Inherent_" + typeStr + "_" + method->name;
            } else {
                mangledName = "_Impl_" + impl->traitName + "_for_" + typeStr + "_" + method->name;
            }
            HirId resolvedId = resolve(mangledName, method->loc);
            
            // Correctly lower dengan 'self'
            hirProg->functions.push_back(lowerImplMethod(*method, mangledName, resolvedId, impl->targetType));
        }
    }

    for (auto &cn : program.constants) {
        auto hcn = std::make_unique<HirConstDef>();
        hcn->id = resolve(cn->name, cn->loc);
        hcn->name = cn->name;
        hcn->typeInfo = cn->typeInfo;
        if (cn->value) {
            hcn->value = lowerExpr(*cn->value);
        }
        hirProg->constants.push_back(std::move(hcn));
    }

    // Lower struct definitions into HIR (register in hirProg->structs).
    for (auto &sd : program.structs) {
        if (!sd->typeParams.empty()) continue; // Skip generic
        HirStructDef hsd;
        hsd.name = sd->name;
        for (auto &f : sd->fields) {
            hsd.fields.push_back({f.name, f.typeInfo});
        }
        hirProg->structs.push_back(std::move(hsd));
    }

    // Lower enum definitions into HIR
    for (auto &ed : program.enums) {
        if (!ed->typeParams.empty()) continue; // Skip generic
        HirEnumDef hed;
        hed.name = ed->name;
        for (auto &v : ed->variants) {
            hed.variants.push_back({v.name, v.hasPayload, v.payloadType});
        }
        hirProg->enums.push_back(std::move(hed));
    }

    popScope();
    return hirProg;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Function Lowering
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<HirFuncDecl> HirBuilder::lowerFunc(FunctionDecl &node) {
    HirId funcId = resolve(node.name, node.loc);

    // Function body scope: register parameters.
    pushScope();
    std::vector<HirParam> hirParams;
    for (auto &p : node.params) {
        HirId paramId = allocId();
        define(p.name, paramId);
        hirParams.push_back({paramId, p.name, p.typeInfo});
    }

    std::unique_ptr<HirBlock> body = nullptr;
    if (node.body) {
        body = lowerBlock(*node.body);
    }

    popScope();

    return std::make_unique<HirFuncDecl>(
        funcId, node.name, std::move(hirParams), node.returnTypeInfo, std::move(body), node.isExtern);
}

std::unique_ptr<HirFuncDecl> HirBuilder::lowerImplMethod(FunctionDecl &node, const std::string &mangledName, HirId funcId, const TypeInfo &targetType) {
    // Implementation body scope: register 'self' + parameters.
    pushScope();
    std::vector<HirParam> hirParams;
    
    for (auto &p : node.params) {
        HirId paramId = allocId();
        define(p.name, paramId);
        hirParams.push_back({paramId, p.name, p.typeInfo});
    }

    std::unique_ptr<HirBlock> body = nullptr;
    if (node.body) {
        body = lowerBlock(*node.body);
    }

    popScope();

    return std::make_unique<HirFuncDecl>(
        funcId, mangledName, std::move(hirParams), node.returnTypeInfo, std::move(body), node.isExtern);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Block / Statement Lowering
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<HirBlock> HirBuilder::lowerBlock(BlockStmt &node) {
    pushScope();
    std::vector<std::unique_ptr<HirStmt>> stmts;
    for (auto &s : node.statements) {
        auto hir = lowerStmt(*s);
        if (hir) stmts.push_back(std::move(hir));
    }
    popScope();
    return std::make_unique<HirBlock>(std::move(stmts));
}

std::unique_ptr<HirStmt> HirBuilder::lowerStmt(Stmt &node) {
    // VarDeclStmt
    if (auto *vd = dynamic_cast<VarDeclStmt *>(&node)) {
        HirId id = allocId();
        define(vd->name, id);
        std::unique_ptr<HirExpr> init;
        if (vd->initializer) {
            init = lowerExpr(*vd->initializer);
        }
        auto decl = std::make_unique<HirVarDecl>(id, vd->name, vd->typeInfo, std::move(init));
        decl->loc = node.loc;
        return decl;
    }

    // ReturnStmt
    if (auto *ret = dynamic_cast<ReturnStmt *>(&node)) {
        std::unique_ptr<HirExpr> val;
        if (ret->value) {
            val = lowerExpr(*ret->value);
        }
        auto stmt = std::make_unique<HirReturn>(std::move(val));
        stmt->loc = node.loc;
        return stmt;
    }

    // IfStmt
    if (auto *ifS = dynamic_cast<IfStmt *>(&node)) {
        auto cond = lowerExpr(*ifS->condition);

        // Then branch: if it's a BlockStmt, lower it as a block.
        std::unique_ptr<HirBlock> thenBlock;
        if (auto *block = dynamic_cast<BlockStmt *>(ifS->thenBranch.get())) {
            thenBlock = lowerBlock(*block);
        } else {
            // Single statement → wrap in a block
            std::vector<std::unique_ptr<HirStmt>> stmts;
            stmts.push_back(lowerStmt(*ifS->thenBranch));
            thenBlock = std::make_unique<HirBlock>(std::move(stmts));
        }

        std::unique_ptr<HirBlock> elseBlock;
        if (ifS->elseBranch) {
            if (auto *block = dynamic_cast<BlockStmt *>(ifS->elseBranch.get())) {
                elseBlock = lowerBlock(*block);
            } else {
                std::vector<std::unique_ptr<HirStmt>> stmts;
                stmts.push_back(lowerStmt(*ifS->elseBranch));
                elseBlock = std::make_unique<HirBlock>(std::move(stmts));
            }
        }

        auto stmt = std::make_unique<HirIf>(
            std::move(cond), std::move(thenBlock), std::move(elseBlock));
        stmt->loc = node.loc;
        return stmt;
    }

    // WhileStmt
    if (auto *whileS = dynamic_cast<WhileStmt *>(&node)) {
        auto cond = lowerExpr(*whileS->condition);
        std::unique_ptr<HirBlock> body;
        if (auto *block = dynamic_cast<BlockStmt *>(whileS->body.get())) {
            body = lowerBlock(*block);
        } else {
            std::vector<std::unique_ptr<HirStmt>> stmts;
            stmts.push_back(lowerStmt(*whileS->body));
            body = std::make_unique<HirBlock>(std::move(stmts));
        }
        auto stmt = std::make_unique<HirWhile>(std::move(cond), std::move(body));
        stmt->loc = node.loc;
        return stmt;
    }

    // ForStmt
    if (auto *forS = dynamic_cast<ForStmt *>(&node)) {
        auto iter = lowerExpr(*forS->iterable);
        pushScope();
        HirId varId = allocId();
        define(forS->varName, varId);
        
        std::unique_ptr<HirBlock> hirBody;
        if (auto *block = dynamic_cast<BlockStmt*>(forS->body.get())) {
            hirBody = lowerBlock(*block);
        } else {
            std::vector<std::unique_ptr<HirStmt>> stmts;
            auto s = lowerStmt(*forS->body);
            if (s) stmts.push_back(std::move(s));
            hirBody = std::make_unique<HirBlock>(std::move(stmts));
        }

        popScope();
        auto stmt = std::make_unique<HirFor>(varId, forS->varName, std::move(iter), std::move(hirBody));
        stmt->loc = node.loc;
        return stmt;
    }

    // BlockStmt (nested)
    if (auto *block = dynamic_cast<BlockStmt *>(&node)) {
        return lowerBlock(*block);
    }

    // DeleteStmt
    if (auto *delS = dynamic_cast<DeleteStmt *>(&node)) {
        auto pointer = lowerExpr(*delS->pointer);
        auto stmt = std::make_unique<HirDeleteStmt>(std::move(pointer));
        stmt->loc = node.loc;
        return stmt;
    }

    // ExprStmt
    if (auto *exprS = dynamic_cast<ExprStmt *>(&node)) {
        auto expr = lowerExpr(*exprS->expr);
        auto stmt = std::make_unique<HirExprStmt>(std::move(expr));
        stmt->loc = node.loc;
        return stmt;
    }

    error(node.loc, "unknown statement type during HIR lowering");
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expression Lowering
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<HirExpr> HirBuilder::lowerExpr(Expr &node) {
    // IntLiteralExpr
    if (auto *lit = dynamic_cast<IntLiteralExpr *>(&node)) {
        auto e = std::make_unique<HirIntLiteral>((int64_t)lit->value);
        e->loc = node.loc;
        return e;
    }

    // FloatLiteralExpr
    if (auto *lit = dynamic_cast<FloatLiteralExpr *>(&node)) {
        auto e = std::make_unique<HirFloatLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }

    // StringLiteralExpr
    if (auto *lit = dynamic_cast<StringLiteralExpr *>(&node)) {
        auto e = std::make_unique<HirStringLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }

    // BoolLiteralExpr
    if (auto *lit = dynamic_cast<BoolLiteralExpr *>(&node)) {
        auto e = std::make_unique<HirBoolLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }

    // NullLiteralExpr
    if (auto *lit = dynamic_cast<NullLiteralExpr *>(&node)) {
        auto e = std::make_unique<HirNullLiteral>();
        e->loc = node.loc;
        return e;
    }

    // VariableExpr → HirVarRef (resolve name to HirId)
    if (auto *var = dynamic_cast<VariableExpr *>(&node)) {
        HirId id = resolve(var->name, node.loc);
        auto e = std::make_unique<HirVarRef>(id, var->name);
        e->loc = node.loc;
        return e;
    }

    // BinaryExpr
    if (auto *bin = dynamic_cast<BinaryExpr *>(&node)) {
        auto lhs = lowerExpr(*bin->lhs);
        auto rhs = lowerExpr(*bin->rhs);
        auto e = std::make_unique<HirBinaryExpr>(bin->op, std::move(lhs), std::move(rhs));
        e->loc = node.loc;
        return e;
    }

    // UnaryExpr
    if (auto *un = dynamic_cast<UnaryExpr *>(&node)) {
        auto operand = lowerExpr(*un->operand);
        auto e = std::make_unique<HirUnaryExpr>(un->op, std::move(operand));
        e->loc = node.loc;
        return e;
    }

    // CallExpr → resolve callee name
    if (auto *call = dynamic_cast<CallExpr *>(&node)) {
        HirId calleeId = resolve(call->callee, node.loc);
        std::vector<std::unique_ptr<HirExpr>> hirArgs;
        for (auto &arg : call->args) {
            hirArgs.push_back(lowerExpr(*arg));
        }
        auto e = std::make_unique<HirCallExpr>(calleeId, call->callee, std::move(hirArgs));
        e->loc = node.loc;
        return e;
    }

    // AssignExpr → resolve target name
    if (auto *assign = dynamic_cast<AssignExpr *>(&node)) {
        HirId targetId = resolve(assign->name, node.loc);
        auto val = lowerExpr(*assign->value);
        auto e = std::make_unique<HirAssignExpr>(targetId, assign->name, std::move(val));
        e->loc = node.loc;
        return e;
    }

    // ArrayLiteralExpr
    if (auto *arr = dynamic_cast<ArrayLiteralExpr *>(&node)) {
        std::vector<std::unique_ptr<HirExpr>> elems;
        for (auto &elem : arr->elements) {
            elems.push_back(lowerExpr(*elem));
        }
        auto e = std::make_unique<HirArrayLiteral>(std::move(elems));
        e->loc = node.loc;
        return e;
    }

    // IndexExpr
    if (auto *idx = dynamic_cast<IndexExpr *>(&node)) {
        auto base = lowerExpr(*idx->base);
        auto index = lowerExpr(*idx->index);
        std::unique_ptr<HirExpr> endIndex = nullptr;
        if (idx->endIndex) {
            endIndex = lowerExpr(*idx->endIndex);
        }
        auto e = std::make_unique<HirIndexExpr>(std::move(base), std::move(index), std::move(endIndex), idx->isRange);
        e->loc = node.loc;
        return e;
    }

    // IndexAssignExpr
    if (auto *idxAssign = dynamic_cast<IndexAssignExpr *>(&node)) {
        auto base = lowerExpr(*idxAssign->base);
        auto index = lowerExpr(*idxAssign->index);
        auto val = lowerExpr(*idxAssign->value);
        auto e = std::make_unique<HirIndexAssign>(std::move(base), std::move(index), std::move(val));
        e->loc = node.loc;
        return e;
    }

    // StructLiteralExpr → HirStructLiteral
    if (auto *slit = dynamic_cast<StructLiteralExpr *>(&node)) {
        std::vector<HirStructFieldInit> hfields;
        for (auto &f : slit->fields) {
            auto val = lowerExpr(*f.value);
            hfields.push_back({f.name, std::move(val)});
        }
        auto e = std::make_unique<HirStructLiteral>(slit->structType.structName, std::move(hfields));
        e->loc = node.loc;
        return e;
    }

    // FieldAccessExpr → HirFieldAccess
    if (auto *fa = dynamic_cast<FieldAccessExpr *>(&node)) {
        auto base = lowerExpr(*fa->base);
        auto e = std::make_unique<HirFieldAccess>(std::move(base), fa->field);
        e->loc = node.loc;
        return e;
    }

    // MethodCallExpr → HirCallExpr (with mangled name)
    if (auto *mc = dynamic_cast<MethodCallExpr *>(&node)) {
        // Special case: len() on arrays and slices
        if (mc->resolvedMangledName == "__len") {
            std::vector<std::unique_ptr<HirExpr>> hirArgs;
            hirArgs.push_back(lowerExpr(*mc->base));
            auto e = std::make_unique<HirCallExpr>(0, "__len", std::move(hirArgs));
            e->loc = node.loc;
            return e;
        }

        HirId calleeId = resolve(mc->resolvedMangledName, node.loc);
        std::vector<std::unique_ptr<HirExpr>> hirArgs;
        // Prepend receiver as first argument
        hirArgs.push_back(lowerExpr(*mc->base));
        for (auto &arg : mc->args) {
            hirArgs.push_back(lowerExpr(*arg));
        }
        auto e = std::make_unique<HirCallExpr>(calleeId, mc->resolvedMangledName, std::move(hirArgs));
        e->loc = node.loc;
        return e;
    }

    // FieldAssignExpr → HirFieldAssign
    if (auto *fassign = dynamic_cast<FieldAssignExpr *>(&node)) {
        auto base = lowerExpr(*fassign->base);
        auto val  = lowerExpr(*fassign->value);
        auto e = std::make_unique<HirFieldAssign>(std::move(base), fassign->field, std::move(val));
        e->loc = node.loc;
        return e;
    }

    // DerefAssignExpr → HirDerefAssign
    if (auto *derefAssign = dynamic_cast<DerefAssignExpr *>(&node)) {
        auto pointer = lowerExpr(*derefAssign->pointer);
        auto val = lowerExpr(*derefAssign->value);
        auto e = std::make_unique<HirDerefAssign>(std::move(pointer), std::move(val));
        e->loc = node.loc;
        return e;
    }

    // EnumVariantExpr → HirEnumVariantExpr
    if (auto *eve = dynamic_cast<EnumVariantExpr *>(&node)) {
        std::unique_ptr<HirExpr> payloadHir = nullptr;
        if (eve->payload) {
            payloadHir = lowerExpr(*eve->payload);
        }
        auto e = std::make_unique<HirEnumVariantExpr>(eve->enumName, eve->variantName, std::move(payloadHir));
        e->loc = node.loc;
        return e;
    }

    // MatchExpr → HirMatchExpr
    if (auto *me = dynamic_cast<MatchExpr *>(&node)) {
        auto valHir = lowerExpr(*me->value);
        std::vector<HirMatchArm> arms;
        for (auto &arm : me->arms) {
            HirMatchArm harm;
            harm.variantName = arm.variantName;
            harm.hasBinding = arm.hasBinding;
            harm.bindingName = arm.bindingName;
            
            // Scope for bindings
            pushScope();
            if (arm.hasBinding) {
                harm.bindingId = allocId();
                define(arm.bindingName, harm.bindingId);
            }
            
            if (auto *b = dynamic_cast<BlockStmt *>(arm.body.get())) {
                harm.blockBody = lowerBlock(*b);
            } else if (auto *ex = dynamic_cast<Expr *>(arm.body.get())) {
                harm.exprBody = lowerExpr(*ex);
            } else {
                error(node.loc, "Match arm body is neither block nor expression");
            }
            
            popScope();
            arms.push_back(std::move(harm));
        }
        
        auto e = std::make_unique<HirMatchExpr>(std::move(valHir), std::move(arms));
        e->loc = node.loc;
        return e;
    }

    // NewExpr → HirNewExpr
    if (auto *newE = dynamic_cast<NewExpr *>(&node)) {
        std::unique_ptr<HirExpr> sizeExpr = nullptr;
        if (newE->sizeExpr) {
            sizeExpr = lowerExpr(*newE->sizeExpr);
        }
        auto e = std::make_unique<HirNewExpr>(newE->allocatedType, std::move(sizeExpr));
        e->loc = node.loc;
        return e;
    }

    // ZoneExpr → HirZoneExpr
    if (auto *zoneE = dynamic_cast<ZoneExpr *>(&node)) {
        std::string savedZone = currentZone_;
        currentZone_ = zoneE->zoneName;
        auto body = lowerBlock(*zoneE->body);
        currentZone_ = savedZone;

        auto e = std::make_unique<HirZoneExpr>(zoneE->zoneName, std::move(body));
        e->loc = node.loc;
        return e;
    }

    // AllocExpr → HirAllocExpr
    if (auto *allocE = dynamic_cast<AllocExpr *>(&node)) {
        std::unique_ptr<HirExpr> count = nullptr;
        if (allocE->count) count = lowerExpr(*allocE->count);
        auto e = std::make_unique<HirAllocExpr>(allocE->type, std::move(count), currentZone_);
        e->loc = node.loc;
        return e;
    }

    // BorrowExpr → HirBorrowExpr
    if (auto *borrowE = dynamic_cast<BorrowExpr *>(&node)) {
        auto target = lowerExpr(*borrowE->target);
        auto e = std::make_unique<HirBorrowExpr>(borrowE->isMutable, std::move(target));
        e->loc = node.loc;
        return e;
    }

    // EscapeExpr → HirEscapeExpr
    if (auto *escapeE = dynamic_cast<EscapeExpr *>(&node)) {
        auto target = lowerExpr(*escapeE->target);
        auto e = std::make_unique<HirEscapeExpr>(std::move(target), escapeE->destinationZone);
        e->loc = node.loc;
        return e;
    }

    // CastExpr → HirCastExpr
    if (auto *castE = dynamic_cast<CastExpr *>(&node)) {
        auto expr = lowerExpr(*castE->expr);
        auto e = std::make_unique<HirCastExpr>(std::move(expr), castE->targetType);
        e->loc = node.loc;
        return e;
    }

    error(node.loc, "unknown expression type during HIR lowering");
    return nullptr;
}

} // namespace agam
