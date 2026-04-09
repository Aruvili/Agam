#pragma once

#include "agam/ast/ast.h"
#include "agam/semantic/symbol_table.h"
#include "agam/utils/diagnostic.h"
#include <string>
#include <vector>
#include <set>

namespace agam {

/// Type checker: walks the AST and verifies type correctness.
class TypeChecker : public ASTVisitor {
public:
    /// Run type checking on the given program.
    /// Returns true if no errors were found.
    bool check(Program &program, DiagnosticEngine &diag);

    /// Get if any errors were found.
    bool hasErrors() const { return diag_ && diag_->hasErrors(); }
    
    // Legacy support: SemanticError is now just Diagnostic
    using SemanticError = Diagnostic;

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
    void visit(MethodCallExpr &node) override;
    void visit(AssignExpr &node) override;
    void visit(ArrayLiteralExpr &node) override;
    void visit(IndexExpr &node) override;
    void visit(IndexAssignExpr &node) override;
    void visit(CastExpr &node) override;

    void visit(StructLiteralExpr &node) override;
    void visit(FieldAccessExpr &node) override;
    void visit(FieldAssignExpr &node) override;
    void visit(EnumVariantExpr &node) override;
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
    TypeInfo lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
    bool lastExprMutable_ = false;
    TypeKind currentReturnType_ = TypeKind::Void;

    void error(const SourceLocation &loc, const std::string &msg);
    bool isNumeric(TypeKind t) const;
    std::string isCompatible(TypeKind expected, TypeKind actual) const;
    std::string isCompatible(const TypeInfo &expected, const TypeInfo &actual) const;

    /// Replaces generic placeholders with concrete types.
    TypeInfo substitute(const TypeInfo &type, const std::vector<std::string> &params, const std::vector<TypeInfo> &args);

    /// Validates and resolves a TypeInfo (e.g. checks if T is a valid generic parameter).
    TypeInfo resolveType(const TypeInfo &type, const SourceLocation &loc);

    /// Expands a print/println call into one or more calls (e.g. for variadic or automatic struct printing).
    /// Returns true if the call was expanded/handled.
    bool expandPrintCall(CallExpr *call, std::vector<std::unique_ptr<Stmt>> &expanded);
    bool expandStructuralPrinting(Expr* receiver, const std::string& methodName, std::vector<std::unique_ptr<Stmt>> &expanded);

    /// ZPM Lifetime/Zone Enforcements
    int getZoneDepth(const std::string &tag) const;
    bool isZoneCompatible(const std::string &srcTag, const std::string &tgtTag, const SourceLocation &loc);

    /// Borrow & Escape Safety
    std::string getRootVariable(Expr* expr);
    bool canMutate(const std::string &name, const SourceLocation &loc);
    bool canRead(const std::string &name, const SourceLocation &loc);

    std::set<std::string> activeTypeParams_;
};

} // namespace agam
