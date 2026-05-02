#include "agam/ast/ast.h"

namespace agam {

// ── Expression accept() implementations ──────────────────────────────────────

void IntLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> IntLiteralExpr::clone() const {
    auto copy = std::make_unique<IntLiteralExpr>(value);
    copy->loc = loc;
    return copy;
}
void FloatLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> FloatLiteralExpr::clone() const {
    auto copy = std::make_unique<FloatLiteralExpr>(value);
    copy->loc = loc;
    return copy;
}
void StringLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> StringLiteralExpr::clone() const {
    auto copy = std::make_unique<StringLiteralExpr>(value);
    copy->loc = loc;
    return copy;
}
void BoolLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> BoolLiteralExpr::clone() const {
    auto copy = std::make_unique<BoolLiteralExpr>(value);
    copy->loc = loc;
    return copy;
}
void NullLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> NullLiteralExpr::clone() const {
    auto copy = std::make_unique<NullLiteralExpr>();
    copy->loc = loc;
    return copy;
}
void VariableExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> VariableExpr::clone() const {
    auto copy = std::make_unique<VariableExpr>(name, genericArgs);
    copy->loc = loc;
    return copy;
}
void BinaryExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> BinaryExpr::clone() const {
    auto copy = std::make_unique<BinaryExpr>(
        op, std::unique_ptr<Expr>(static_cast<Expr *>(lhs->clone().release())),
        std::unique_ptr<Expr>(static_cast<Expr *>(rhs->clone().release())));
    copy->loc = loc;
    return copy;
}
void UnaryExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> UnaryExpr::clone() const {
    auto copy = std::make_unique<UnaryExpr>(
        op, std::unique_ptr<Expr>(static_cast<Expr *>(operand->clone().release())));
    copy->loc = loc;
    return copy;
}
void CallExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> CallExpr::clone() const {
    std::vector<std::unique_ptr<Expr>> clonedArgs;
    for (const auto &arg : args) {
        clonedArgs.push_back(std::unique_ptr<Expr>(static_cast<Expr *>(arg->clone().release())));
    }
    auto copy = std::make_unique<CallExpr>(callee, std::move(clonedArgs), genericArgs);
    copy->loc = loc;
    return copy;
}
void AssignExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> AssignExpr::clone() const {
    auto copy = std::make_unique<AssignExpr>(
        name, std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())));
    copy->loc = loc;
    return copy;
}
void ArrayLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ArrayLiteralExpr::clone() const {
    std::vector<std::unique_ptr<Expr>> clonedElems;
    for (const auto &e : elements) {
        clonedElems.push_back(std::unique_ptr<Expr>(static_cast<Expr *>(e->clone().release())));
    }
    auto copy = std::make_unique<ArrayLiteralExpr>(std::move(clonedElems));
    copy->loc = loc;
    return copy;
}
void IndexExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> IndexExpr::clone() const {
    auto copy = std::make_unique<IndexExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(base->clone().release())),
        std::unique_ptr<Expr>(static_cast<Expr *>(index->clone().release())),
        endIndex ? std::unique_ptr<Expr>(static_cast<Expr *>(endIndex->clone().release()))
                 : nullptr,
        isRange);
    copy->loc = loc;
    return copy;
}
void IndexAssignExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> IndexAssignExpr::clone() const {
    auto copy = std::make_unique<IndexAssignExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(base->clone().release())),
        std::unique_ptr<Expr>(static_cast<Expr *>(index->clone().release())),
        std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())));
    copy->loc = loc;
    return copy;
}
void StructLiteralExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> StructLiteralExpr::clone() const {
    std::vector<StructFieldInit> clonedFields;
    for (const auto &f : fields) {
        clonedFields.push_back(
            {f.name, std::unique_ptr<Expr>(static_cast<Expr *>(f.value->clone().release()))});
    }
    auto copy = std::make_unique<StructLiteralExpr>(structType, std::move(clonedFields));
    copy->loc = loc;
    return copy;
}
void FieldAccessExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> FieldAccessExpr::clone() const {
    auto copy = std::make_unique<FieldAccessExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(base->clone().release())), field);
    copy->loc = loc;
    return copy;
}
void FieldAssignExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> FieldAssignExpr::clone() const {
    auto copy = std::make_unique<FieldAssignExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(base->clone().release())), field,
        std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())));
    copy->loc = loc;
    return copy;
}
void EnumVariantExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> EnumVariantExpr::clone() const {
    auto copy = std::make_unique<EnumVariantExpr>(
        enumName, variantName, genericArgs,
        payload ? std::unique_ptr<Expr>(static_cast<Expr *>(payload->clone().release())) : nullptr);
    copy->loc = loc;
    return copy;
}
void MatchExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> MatchExpr::clone() const {
    std::vector<MatchArm> clonedArms;
    for (const auto &arm : arms) {
        clonedArms.push_back({arm.variantName, arm.bindingName, arm.hasBinding, arm.body->clone()});
    }
    auto copy = std::make_unique<MatchExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())),
        std::move(clonedArms));
    copy->loc = loc;
    return copy;
}
void DerefAssignExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> DerefAssignExpr::clone() const {
    auto copy = std::make_unique<DerefAssignExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(pointer->clone().release())),
        std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())));
    copy->loc = loc;
    return copy;
}
void CastExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> CastExpr::clone() const {
    auto copy = std::make_unique<CastExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(expr->clone().release())), targetType);
    copy->loc = loc;
    return copy;
}
void NewExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> NewExpr::clone() const {
    auto copy = std::make_unique<NewExpr>(
        allocatedType, sizeExpr
                           ? std::unique_ptr<Expr>(static_cast<Expr *>(sizeExpr->clone().release()))
                           : nullptr);
    copy->loc = loc;
    return copy;
}
void MethodCallExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> MethodCallExpr::clone() const {
    std::vector<std::unique_ptr<Expr>> clonedArgs;
    for (const auto &arg : args) {
        clonedArgs.push_back(std::unique_ptr<Expr>(static_cast<Expr *>(arg->clone().release())));
    }
    auto copy = std::make_unique<MethodCallExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(base->clone().release())), methodName,
        std::move(clonedArgs), genericArgs);
    copy->loc = loc;
    copy->resolvedMangledName = resolvedMangledName;
    return copy;
}
void ZoneExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ZoneExpr::clone() const {
    auto copy = std::make_unique<ZoneExpr>(
        zoneName, std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(body->clone().release())));
    copy->loc = loc;
    return copy;
}
void AllocExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> AllocExpr::clone() const {
    auto copy = std::make_unique<AllocExpr>(
        type,
        count ? std::unique_ptr<Expr>(static_cast<Expr *>(count->clone().release())) : nullptr);
    copy->loc = loc;
    return copy;
}
void BorrowExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> BorrowExpr::clone() const {
    auto copy = std::make_unique<BorrowExpr>(
        isMutable, std::unique_ptr<Expr>(static_cast<Expr *>(target->clone().release())), viewName);
    copy->loc = loc;
    return copy;
}
void EscapeExpr::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> EscapeExpr::clone() const {
    auto copy = std::make_unique<EscapeExpr>(
        std::unique_ptr<Expr>(static_cast<Expr *>(target->clone().release())), destinationZone);
    copy->loc = loc;
    return copy;
}

// ── Statement accept() implementations ───────────────────────────────────────

void ExprStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ExprStmt::clone() const {
    auto copy = std::make_unique<ExprStmt>(
        std::unique_ptr<Expr>(static_cast<Expr *>(expr->clone().release())));
    copy->loc = loc;
    return copy;
}
void VarDeclStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> VarDeclStmt::clone() const {
    auto copy = std::make_unique<VarDeclStmt>(
        name, typeInfo,
        initializer ? std::unique_ptr<Expr>(static_cast<Expr *>(initializer->clone().release()))
                    : nullptr,
        isMutable);
    copy->loc = loc;
    return copy;
}
void BlockStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> BlockStmt::clone() const {
    std::vector<std::unique_ptr<Stmt>> clonedStmts;
    for (const auto &s : statements) {
        clonedStmts.push_back(std::unique_ptr<Stmt>(static_cast<Stmt *>(s->clone().release())));
    }
    auto copy = std::make_unique<BlockStmt>(std::move(clonedStmts));
    copy->loc = loc;
    return copy;
}
void ReturnStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ReturnStmt::clone() const {
    auto copy = std::make_unique<ReturnStmt>(
        value ? std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())) : nullptr);
    copy->loc = loc;
    return copy;
}
void IfStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> IfStmt::clone() const {
    auto copy = std::make_unique<IfStmt>(
        std::unique_ptr<Expr>(static_cast<Expr *>(condition->clone().release())),
        std::unique_ptr<Stmt>(static_cast<Stmt *>(thenBranch->clone().release())),
        elseBranch ? std::unique_ptr<Stmt>(static_cast<Stmt *>(elseBranch->clone().release()))
                   : nullptr);
    copy->loc = loc;
    return copy;
}
void WhileStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> WhileStmt::clone() const {
    auto copy = std::make_unique<WhileStmt>(
        std::unique_ptr<Expr>(static_cast<Expr *>(condition->clone().release())),
        std::unique_ptr<Stmt>(static_cast<Stmt *>(body->clone().release())));
    copy->loc = loc;
    return copy;
}
void ForStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ForStmt::clone() const {
    auto copy = std::make_unique<ForStmt>(
        varName, std::unique_ptr<Expr>(static_cast<Expr *>(iterable->clone().release())),
        std::unique_ptr<Stmt>(static_cast<Stmt *>(body->clone().release())));
    copy->loc = loc;
    return copy;
}
void DeleteStmt::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> DeleteStmt::clone() const {
    auto copy = std::make_unique<DeleteStmt>(
        std::unique_ptr<Expr>(static_cast<Expr *>(pointer->clone().release())));
    copy->loc = loc;
    return copy;
}

// ── Declaration accept() implementations ─────────────────────────────────────

void FunctionDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> FunctionDecl::clone() const {
    auto copy = std::make_unique<FunctionDecl>(
        name, typeParams, params, returnTypeInfo,
        body ? std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(body->clone().release()))
             : nullptr,
        isExtern);
    copy->loc = loc;
    return copy;
}

void ConstDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ConstDecl::clone() const {
    auto copy = std::make_unique<ConstDecl>(
        name, typeInfo,
        value ? std::unique_ptr<Expr>(static_cast<Expr *>(value->clone().release())) : nullptr);
    copy->loc = loc;
    return copy;
}
void StructDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> StructDecl::clone() const {
    auto copy = std::make_unique<StructDecl>(name, typeParams, fields);
    copy->loc = loc;
    return copy;
}
void EnumDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> EnumDecl::clone() const {
    auto copy = std::make_unique<EnumDecl>(name, typeParams, variants);
    copy->loc = loc;
    return copy;
}
void TraitDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> TraitDecl::clone() const {
    std::vector<std::unique_ptr<FunctionDecl>> clonedMethods;
    for (const auto &m : methods) {
        clonedMethods.push_back(
            std::unique_ptr<FunctionDecl>(static_cast<FunctionDecl *>(m->clone().release())));
    }
    auto copy = std::make_unique<TraitDecl>(name, std::move(clonedMethods));
    copy->loc = loc;
    return copy;
}
void ImplDecl::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> ImplDecl::clone() const {
    std::vector<std::unique_ptr<FunctionDecl>> clonedMethods;
    for (const auto &m : methods) {
        clonedMethods.push_back(
            std::unique_ptr<FunctionDecl>(static_cast<FunctionDecl *>(m->clone().release())));
    }
    auto copy =
        std::make_unique<ImplDecl>(traitName, typeParams, targetType, std::move(clonedMethods));
    copy->loc = loc;
    return copy;
}
void Program::accept(ASTVisitor &v) {
    v.visit(*this);
}
std::unique_ptr<ASTNode> Program::clone() const {
    auto copy = std::make_unique<Program>();
    copy->loc = loc;
    copy->imports = imports;
    for (const auto &f : functions)
        copy->functions.push_back(
            std::unique_ptr<FunctionDecl>(static_cast<FunctionDecl *>(f->clone().release())));
    for (const auto &c : constants)
        copy->constants.push_back(
            std::unique_ptr<ConstDecl>(static_cast<ConstDecl *>(c->clone().release())));
    for (const auto &s : structs)
        copy->structs.push_back(
            std::unique_ptr<StructDecl>(static_cast<StructDecl *>(s->clone().release())));
    for (const auto &e : enums)
        copy->enums.push_back(
            std::unique_ptr<EnumDecl>(static_cast<EnumDecl *>(e->clone().release())));
    for (const auto &t : traits)
        copy->traits.push_back(
            std::unique_ptr<TraitDecl>(static_cast<TraitDecl *>(t->clone().release())));
    for (const auto &i : impls)
        copy->impls.push_back(
            std::unique_ptr<ImplDecl>(static_cast<ImplDecl *>(i->clone().release())));
    return copy;
}

} // namespace agam
