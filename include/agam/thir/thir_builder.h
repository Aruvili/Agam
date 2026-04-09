#pragma once

#include "agam/hir/hir.h"
#include "agam/thir/thir.h"
#include "agam/utils/diagnostic.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace agam {

/// Builds THIR from HIR by performing type checking.
///
/// Every HIR expression is annotated with its resolved type.
/// Type errors are collected and reported.
class ThirBuilder {
public:
    /// Build THIR from a lowered HIR program.
    std::unique_ptr<ThirProgram> build(HirProgram &program, DiagnosticEngine &diag);

    bool hasErrors() const { return diag_ && diag_->hasErrors(); }

private:
    /// HirId → TypeInfo mapping for variables and params.
    std::unordered_map<HirId, TypeInfo> typeMap_;

    /// HirId → function info for resolving call return types.
    struct FuncInfo {
        TypeInfo returnTypeInfo = TypeInfo::scalar(TypeKind::Void);
        std::vector<TypeInfo> paramTypes;
    };
    std::unordered_map<HirId, FuncInfo> funcMap_;

    /// Struct-name → HirStructDef* (borrowed, stored in ThirProgram).
    std::unordered_map<std::string, const HirStructDef*> structDefs_;

    /// Enum-name -> HirEnumDef*
    std::unordered_map<std::string, const HirEnumDef*> enumDefs_;

    /// Current function's return type (for checking return statements).
    TypeInfo currentReturnTypeInfo_ = TypeInfo::scalar(TypeKind::Void);
    DiagnosticEngine *diag_ = nullptr;

    // ── Lowering methods ────────────────────────────────────────────────────
    std::unique_ptr<ThirFuncDecl> lowerFunc(HirFuncDecl &node);
    std::unique_ptr<ThirBlock> lowerBlock(HirBlock &node);
    std::unique_ptr<ThirStmt> lowerStmt(HirStmt &node);
    std::unique_ptr<ThirExpr> lowerExpr(HirExpr &node);

    // ── Type helpers ────────────────────────────────────────────────────────
    TypeInfo resolveTypeInfo(const TypeInfo &ti);
    bool isNumeric(TypeKind t) const;
    bool isComparison(BinaryOp op) const;
    bool isLogical(BinaryOp op) const;

    void error(const SourceLocation &loc, const std::string &msg);
};

} // namespace agam
