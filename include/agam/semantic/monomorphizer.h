#pragma once

#include "agam/ast/ast.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace agam {

/// Monomorphizer: identifies and generates specialized versions of generic declarations.
class Monomorphizer : public ASTVisitor {
  public:
    /// Processes the program and generates concrete versions of all generic uses.
    void monomorphize(Program &program);

    void mangleType(TypeInfo &ti);

    // ── Visitor methods ──────────────────────────────────────────────────────
    void visit(VarDeclStmt &node) override;
    void visit(FunctionDecl &node) override;
    void visit(StructDecl &node) override;
    void visit(EnumDecl &node) override;
    void visit(TraitDecl &node) override;
    void visit(ImplDecl &node) override;
    void visit(ConstDecl &node) override;
    void visit(Program &node) override;

    void visit(CallExpr &node) override;
    void visit(MethodCallExpr &node) override;
    void visit(StructLiteralExpr &node) override;
    void visit(EnumVariantExpr &node) override;
    void visit(NewExpr &node) override;

    // Expressions that don't need specialization (just traverse)
    void visit(IntLiteralExpr &) override {}
    void visit(FloatLiteralExpr &) override {}
    void visit(StringLiteralExpr &) override {}
    void visit(BoolLiteralExpr &) override {}
    void visit(NullLiteralExpr &) override {}
    void visit(VariableExpr &) override {}
    void visit(BinaryExpr &node) override {
        node.lhs->accept(*this);
        node.rhs->accept(*this);
    }
    void visit(UnaryExpr &node) override { node.operand->accept(*this); }
    void visit(AssignExpr &node) override { node.value->accept(*this); }
    void visit(ArrayLiteralExpr &node) override {
        for (auto &e : node.elements)
            e->accept(*this);
    }
    void visit(IndexExpr &node) override {
        node.base->accept(*this);
        node.index->accept(*this);
    }
    void visit(IndexAssignExpr &node) override {
        node.base->accept(*this);
        node.index->accept(*this);
        node.value->accept(*this);
    }
    void visit(FieldAccessExpr &node) override { node.base->accept(*this); }
    void visit(FieldAssignExpr &node) override {
        node.base->accept(*this);
        node.value->accept(*this);
    }
    void visit(MatchExpr &node) override { node.value->accept(*this); /* ... */ }
    void visit(DerefAssignExpr &node) override {
        node.pointer->accept(*this);
        node.value->accept(*this);
    }
    void visit(ExprStmt &node) override { node.expr->accept(*this); }
    void visit(BlockStmt &node) override;
    void visit(ReturnStmt &node) override;
    void visit(IfStmt &node) override {
        node.condition->accept(*this);
        node.thenBranch->accept(*this);
        if (node.elseBranch)
            node.elseBranch->accept(*this);
    }
    void visit(WhileStmt &node) override {
        node.condition->accept(*this);
        node.body->accept(*this);
    }
    void visit(ForStmt &node) override { /* ... */ }
    void visit(DeleteStmt &node) override { node.pointer->accept(*this); }
    void visit(CastExpr &node) override;

  private:
    Program *currentProgram_ = nullptr;

    struct Instantiation {
        std::string baseName;
        std::vector<TypeInfo> args;
        bool operator<(const Instantiation &other) const {
            if (baseName != other.baseName)
                return baseName < other.baseName;
            return args < other.args;
        }
    };

    std::map<Instantiation, std::string> specializedStructs_;
    std::map<Instantiation, std::string> specializedEnums_;
    std::map<Instantiation, std::string> specializedFunctions_;

    std::string getMangledName(const std::string &base, const std::vector<TypeInfo> &args);
    TypeInfo substitute(const TypeInfo &type, const std::vector<std::string> &params,
                        const std::vector<TypeInfo> &args);
    void specializeStruct(const std::string &name, const std::vector<TypeInfo> &args);
    void specializeEnum(const std::string &name, const std::vector<TypeInfo> &args);
    void specializeFunction(const std::string &name, const std::vector<TypeInfo> &args);
};

} // namespace agam
