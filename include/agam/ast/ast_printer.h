#pragma once

#include "ast.h"
#include <iostream>
#include <string>

namespace agam {

/// Pretty-prints an AST tree to an output stream (for debugging/testing).
class ASTPrinter : public ASTVisitor {
public:
    explicit ASTPrinter(std::ostream &os = std::cout) : os_(os) {}

    void print(ASTNode &node) {
        indent_ = 0;
        node.accept(*this);
    }

    // ── Expressions ──────────────────────────────────────────────────────────

    void visit(IntLiteralExpr &node) override {
        printIndent();
        os_ << "IntLiteral(" << node.value << ")\n";
    }

    void visit(FloatLiteralExpr &node) override {
        printIndent();
        os_ << "FloatLiteral(" << node.value << ")\n";
    }

    void visit(StringLiteralExpr &node) override {
        printIndent();
        os_ << "StringLiteral(\"" << node.value << "\")\n";
    }

    void visit(NullLiteralExpr &node) override {
        printIndent();
        os_ << "NullLiteral\n";
    }

    void visit(BoolLiteralExpr &node) override {
        printIndent();
        os_ << "BoolLiteral(" << (node.value ? "true" : "false") << ")\n";
    }

    void visit(VariableExpr &node) override {
        printIndent();
        os_ << "Variable(" << node.name << ")\n";
    }

    void visit(BinaryExpr &node) override {
        printIndent();
        os_ << "BinaryExpr(" << binaryOpToString(node.op) << ")\n";
        indent_++;
        node.lhs->accept(*this);
        node.rhs->accept(*this);
        indent_--;
    }

    void visit(UnaryExpr &node) override {
        printIndent();
        const char *opStr = "?";
        switch (node.op) {
            case UnaryOp::Negate: opStr = "-"; break;
            case UnaryOp::Not: opStr = "!"; break;
            case UnaryOp::AddressOf: opStr = "&"; break;
            case UnaryOp::Dereference: opStr = "*"; break;
        }
        os_ << "UnaryExpr(" << opStr << ")\n";
        indent_++;
        node.operand->accept(*this);
        indent_--;
    }

    void visit(CallExpr &node) override {
        printIndent();
        os_ << "CallExpr(" << node.callee << ")\n";
        indent_++;
        for (auto &arg : node.args) {
            arg->accept(*this);
        }
        indent_--;
    }

    void visit(CastExpr &node) override {
        printIndent();
        os_ << "CastExpr(as " << typeInfoToString(node.targetType) << ")\n";
        indent_++;
        node.expr->accept(*this);
        indent_--;
    }

    void visit(AssignExpr &node) override {
        printIndent();
        os_ << "Assign(" << node.name << ")\n";
        indent_++;
        node.value->accept(*this);
        indent_--;
    }

    void visit(ArrayLiteralExpr &node) override {
        printIndent();
        os_ << "ArrayLiteral(" << node.elements.size() << " elements)\n";
        indent_++;
        for (auto &e : node.elements) e->accept(*this);
        indent_--;
    }

    void visit(IndexExpr &node) override {
        printIndent();
        os_ << "Index\n";
        indent_++;
        node.base->accept(*this);
        node.index->accept(*this);
        indent_--;
    }

    void visit(IndexAssignExpr &node) override {
        printIndent();
        os_ << "IndexAssign\n";
        indent_++;
        node.base->accept(*this);
        node.index->accept(*this);
        node.value->accept(*this);
        indent_--;
    }

    void visit(StructLiteralExpr &node) override {
        printIndent();
        os_ << "StructLiteral(" << typeInfoToString(node.structType) << ")\n";
        indent_++;
        for (auto &f : node.fields) {
            printIndent();
            os_ << "FieldInit(" << f.name << ")\n";
            indent_++;
            f.value->accept(*this);
            indent_--;
        }
        indent_--;
    }

    void visit(FieldAccessExpr &node) override {
        printIndent();
        os_ << "FieldAccess(" << node.field << ")\n";
        indent_++;
        node.base->accept(*this);
        indent_--;
    }

    void visit(FieldAssignExpr &node) override {
        printIndent();
        os_ << "FieldAssign(" << node.field << ")\n";
        indent_++;
        node.base->accept(*this);
        node.value->accept(*this);
        indent_--;
    }

    void visit(EnumVariantExpr &node) override {
        printIndent();
        os_ << "EnumVariant(" << node.enumName;
        if (!node.genericArgs.empty()) {
            os_ << "<";
            for (size_t i = 0; i < node.genericArgs.size(); ++i) {
                if (i > 0) os_ << ", ";
                os_ << typeInfoToString(node.genericArgs[i]);
            }
            os_ << ">";
        }
        os_ << "::" << node.variantName << ")\n";
        if (node.payload) {
            indent_++;
            node.payload->accept(*this);
            indent_--;
        }
    }

    void visit(MatchExpr &node) override {
        printIndent();
        os_ << "Match\n";
        indent_++;
        node.value->accept(*this);
        for (auto &arm : node.arms) {
            printIndent();
            os_ << "Arm(" << arm.variantName;
            if (arm.hasBinding) os_ << ", bind: " << arm.bindingName;
            os_ << ")\n";
            indent_++;
            arm.body->accept(*this);
            indent_--;
        }
        indent_--;
    }

    void visit(DerefAssignExpr &node) override {
        printIndent();
        os_ << "DerefAssign\n";
        indent_++;
        node.pointer->accept(*this);
        node.value->accept(*this);
        indent_--;
    }

    void visit(NewExpr &node) override {
        printIndent();
        os_ << "New(" << typeInfoToString(node.allocatedType) << ")\n";
    }

    void visit(ZoneExpr &node) override {
        printIndent();
        os_ << "Zone(" << node.zoneName << ")\n";
        indent_++;
        node.body->accept(*this);
        indent_--;
    }

    void visit(AllocExpr &node) override {
        printIndent();
        os_ << "Alloc(" << typeInfoToString(node.type) << ")\n";
        if (node.count) {
            indent_++;
            node.count->accept(*this);
            indent_--;
        }
    }

    void visit(BorrowExpr &node) override {
        printIndent();
        os_ << "Borrow(" << (node.isMutable ? "mut" : "shared") << ")\n";
        indent_++;
        node.target->accept(*this);
        indent_--;
    }

    void visit(EscapeExpr &node) override {
        printIndent();
        os_ << "Escape(-> " << node.destinationZone << ")\n";
        indent_++;
        node.target->accept(*this);
        indent_--;
    }

    void visit(MethodCallExpr &node) override {
        printIndent();
        os_ << "MethodCall(" << node.methodName << ")\n";
        indent_++;
        node.base->accept(*this);
        for (auto &arg : node.args) arg->accept(*this);
        indent_--;
    }

    // ── Statements ───────────────────────────────────────────────────────────

    void visit(ExprStmt &node) override {
        printIndent();
        os_ << "ExprStmt\n";
        indent_++;
        node.expr->accept(*this);
        indent_--;
    }

    void visit(VarDeclStmt &node) override {
        printIndent();
        os_ << "VarDecl(" << node.name << ": " << typeInfoToString(node.typeInfo) << ")\n";
        if (node.initializer) {
            indent_++;
            node.initializer->accept(*this);
            indent_--;
        }
    }

    void visit(BlockStmt &node) override {
        printIndent();
        os_ << "Block\n";
        indent_++;
        for (auto &stmt : node.statements) {
            stmt->accept(*this);
        }
        indent_--;
    }

    void visit(ReturnStmt &node) override {
        printIndent();
        os_ << "Return\n";
        if (node.value) {
            indent_++;
            node.value->accept(*this);
            indent_--;
        }
    }

    void visit(IfStmt &node) override {
        printIndent();
        os_ << "If\n";
        indent_++;
        printIndent();
        os_ << "Condition:\n";
        indent_++;
        node.condition->accept(*this);
        indent_--;
        printIndent();
        os_ << "Then:\n";
        indent_++;
        node.thenBranch->accept(*this);
        indent_--;
        if (node.elseBranch) {
            printIndent();
            os_ << "Else:\n";
            indent_++;
            node.elseBranch->accept(*this);
            indent_--;
        }
        indent_--;
    }

    void visit(WhileStmt &node) override {
        printIndent();
        os_ << "While(" << std::endl;
        indent_++;
        node.condition->accept(*this);
        os_ << std::endl;
        node.body->accept(*this);
        indent_--;
        os_ << std::endl;
        printIndent();
        os_ << ")";
    }

    void visit(ForStmt &node) override {
        printIndent();
        os_ << "For(" << node.varName << " in " << std::endl;
        indent_++;
        node.iterable->accept(*this);
        os_ << std::endl;
        node.body->accept(*this);
        indent_--;
        os_ << std::endl;
        printIndent();
        os_ << ")";
    }

    void visit(DeleteStmt &node) override {
        printIndent();
        os_ << "Delete\n";
        indent_++;
        node.pointer->accept(*this);
        indent_--;
    }

    // ── Declarations ─────────────────────────────────────────────────────────

    void visit(FunctionDecl &node) override {
        printIndent();
        os_ << "FunctionDecl(" << node.name;
        if (!node.typeParams.empty()) {
            os_ << "<" << genericParamsToString(node.typeParams) << ">";
        }
        os_ << " -> " << typeInfoToString(node.returnTypeInfo) << ")\n";
        indent_++;
        for (auto &p : node.params) {
            printIndent();
            os_ << "Param(" << p.name << ": " << typeInfoToString(p.typeInfo) << ")\n";
        }
        if (node.body) {
            node.body->accept(*this);
        }
        indent_--;
    }

    void visit(ConstDecl &node) override {
        printIndent();
        os_ << "ConstDecl(" << node.name << ": " << typeInfoToString(node.typeInfo) << ")\n";
        if (node.value) {
            indent_++;
            node.value->accept(*this);
            indent_--;
        }
    }

    void visit(Program &node) override {
        printIndent();
        os_ << "Program\n";
        indent_++;
        for (auto &sd : node.structs) sd->accept(*this);
        for (auto &cn : node.constants) cn->accept(*this);
        for (auto &ed : node.enums) ed->accept(*this);
        for (auto &td : node.traits) td->accept(*this);
        for (auto &id : node.impls) id->accept(*this);
        for (auto &fn : node.functions) fn->accept(*this);
        indent_--;
    }

    void visit(StructDecl &node) override {
        printIndent();
        os_ << "StructDecl(" << node.name;
        if (!node.typeParams.empty()) {
            os_ << "<" << genericParamsToString(node.typeParams) << ">";
        }
        os_ << ")\n";
        indent_++;
        for (auto &f : node.fields) {
            printIndent();
            os_ << "Field(" << f.name << ": " << typeInfoToString(f.typeInfo) << ")\n";
        }
        indent_--;
    }

    void visit(EnumDecl &node) override {
        printIndent();
        os_ << "EnumDecl(" << node.name;
        if (!node.typeParams.empty()) {
            os_ << "<" << genericParamsToString(node.typeParams) << ">";
        }
        os_ << ")\n";
        indent_++;
        for (auto &v : node.variants) {
            printIndent();
            os_ << "Variant(" << v.name;
            if (v.hasPayload) os_ << ": " << typeInfoToString(v.payloadType);
            os_ << ")\n";
        }
        indent_--;
    }

    void visit(TraitDecl &node) override {
        printIndent();
        os_ << "TraitDecl(" << node.name << ")\n";
        indent_++;
        for (auto &m : node.methods) m->accept(*this);
        indent_--;
    }

    void visit(ImplDecl &node) override {
        printIndent();
        os_ << "ImplDecl(" << node.traitName << " for " << typeInfoToString(node.targetType) << ")\n";
        indent_++;
        for (auto &m : node.methods) m->accept(*this);
        indent_--;
    }

private:
    std::ostream &os_;
    int indent_ = 0;

    std::string genericParamsToString(const std::vector<std::string> &params) {
        std::string s;
        for (size_t i = 0; i < params.size(); i++) {
            s += params[i];
            if (i < params.size() - 1) s += ", ";
        }
        return s;
    }

    void printIndent() {
        for (int i = 0; i < indent_; i++) {
            os_ << "  ";
        }
    }
};

} // namespace agam
