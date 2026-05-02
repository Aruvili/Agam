#pragma once

#include "agam/ast/ast.h"
#include "agam/hir/hir.h"
#include "agam/semantic/symbol_table.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace agam {

/// Builds HIR from a semantically-analyzed AST.
class HirBuilder {
  public:
    std::unique_ptr<HirProgram> build(Program &program);

    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string> &errors() const { return errors_; }

  private:
    HirId nextId_ = 1;

    struct Scope {
        std::unordered_map<std::string, HirId> names;
    };
    std::vector<Scope> scopes_;

    std::vector<std::string> errors_;
    std::string currentZone_;

    HirId allocId() { return nextId_++; }

    void pushScope();
    void popScope();
    void define(const std::string &name, HirId id);
    HirId resolve(const std::string &name, const SourceLocation &loc);

    void visit(DerefAssignExpr &node);

    std::unique_ptr<HirFuncDecl> lowerFunc(FunctionDecl &node);
    std::unique_ptr<HirFuncDecl> lowerImplMethod(FunctionDecl &node, const std::string &mangledName,
                                                 HirId funcId, const TypeInfo &targetType);
    std::unique_ptr<HirBlock> lowerBlock(BlockStmt &node);
    std::unique_ptr<HirStmt> lowerStmt(Stmt &node);
    std::unique_ptr<HirExpr> lowerExpr(Expr &node);

    void error(const SourceLocation &loc, const std::string &msg);
};

} // namespace agam
