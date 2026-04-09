#pragma once

#include "agam/ast/ast.h"
#include "agam/semantic/symbol_table.h"
#include "agam/semantic/type_checker.h"
#include "agam/utils/diagnostic.h"
#include <vector>
#include <string>

namespace agam {

/// Resolves variable and function references to their declarations.
/// Reports undeclared identifiers.
class ScopeResolver : public ASTVisitor {
public:
    /// Run scope resolution on the given program.
    /// Returns true if no errors were found.
    bool resolve(Program &program, DiagnosticEngine &diag);

    /// Get if any errors were found.
    bool hasErrors() const { return diag_ && diag_->hasErrors(); }

    // ── Visitor methods ──────────────────────────────────────────────────────
    void visit(IntLiteralExpr &node) override;
    void visit(FloatLiteralExpr &node) override;
    void visit(StringLiteralExpr &node) override;
    void visit(BoolLiteralExpr &node) override;
    void visit(NullLiteralExpr &node) override;
    void visit(VariableExpr &node) override;
    void visit(BinaryExpr &node) override;
    void visit(UnaryExpr &node) override;
    void visit(CallExpr &node) override;
    void visit(AssignExpr &node) override;
    void visit(ArrayLiteralExpr &node) override;
    void visit(IndexExpr &node) override;
    void visit(IndexAssignExpr &node) override;
    void visit(CastExpr &node) override;

    void visit(StructLiteralExpr &node) override;
    void visit(FieldAccessExpr &node) override;
    void visit(FieldAssignExpr &node) override;
    void visit(EnumVariantExpr &node) override;
    void visit(MethodCallExpr &node) override;
    void visit(MatchExpr &node) override;
    void visit(DerefAssignExpr &node) override;
    void visit(NewExpr &node) override;
    void visit(ZoneExpr &node) override;
    void visit(AllocExpr &node) override;
    void visit(BorrowExpr &node) override;
    void visit(EscapeExpr &node) override;

    void visit(ExprStmt &node) override;
    void visit(VarDeclStmt &node) override;
    void visit(BlockStmt &node) override;
    void visit(ReturnStmt &node) override;
    void visit(IfStmt &node) override;
    void visit(WhileStmt &node) override;
    void visit(ForStmt &node) override;
    void visit(DeleteStmt &node) override;

    void visit(FunctionDecl &node) override;
    void visit(ConstDecl &node) override;
    void visit(StructDecl &node) override;
    void visit(EnumDecl &node) override;
    void visit(TraitDecl &node) override;
    void visit(ImplDecl &node) override;
    void visit(Program &node) override;

private:
    SymbolTable symbols_;
    DiagnosticEngine *diag_ = nullptr;

    void error(const SourceLocation &loc, const std::string &msg);
};

} // namespace agam
