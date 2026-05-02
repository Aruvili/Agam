#include "agam/semantic/scope_resolver.h"

namespace agam {

bool ScopeResolver::resolve(Program &program, DiagnosticEngine &diag) {
    diag_ = &diag;
    symbols_ = SymbolTable();
    symbols_.enterScope(); // Global scope
    program.accept(*this);
    symbols_.exitScope();
    return !diag_->hasErrors();
}

void ScopeResolver::error(const SourceLocation &loc, const std::string &msg) {
    if (diag_)
        diag_->error(loc, msg);
}

// ── Expressions ──────────────────────────────────────────────────────────────

void ScopeResolver::visit(IntLiteralExpr &) {}
void ScopeResolver::visit(FloatLiteralExpr &) {}
void ScopeResolver::visit(StringLiteralExpr &) {}
void ScopeResolver::visit(BoolLiteralExpr &) {}
void ScopeResolver::visit(NullLiteralExpr &) {}

void ScopeResolver::visit(VariableExpr &node) {
    if (!symbols_.lookup(node.name)) {
        error(node.loc, "அறிவிக்கப்படாத மாறி (Undeclared variable): '" + node.name + "'");
    }
}

void ScopeResolver::visit(BinaryExpr &node) {
    node.lhs->accept(*this);
    node.rhs->accept(*this);
}

void ScopeResolver::visit(UnaryExpr &node) {
    node.operand->accept(*this);
}

void ScopeResolver::visit(CastExpr &node) {
    node.expr->accept(*this);
}

void ScopeResolver::visit(CallExpr &node) {
    if (!symbols_.lookup(node.callee)) {
        error(node.loc, "அறிவிக்கப்படாத செயல் (Undeclared function): '" + node.callee + "'");
    }
    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void ScopeResolver::visit(AssignExpr &node) {
    if (!symbols_.lookup(node.name)) {
        error(node.loc, "அறிவிக்கப்படாத மாறி (Undeclared variable): '" + node.name + "'");
    }
    node.value->accept(*this);
}

void ScopeResolver::visit(DerefAssignExpr &node) {
    node.pointer->accept(*this);
    node.value->accept(*this);
}

void ScopeResolver::visit(ArrayLiteralExpr &node) {
    for (auto &e : node.elements)
        e->accept(*this);
}

void ScopeResolver::visit(IndexExpr &node) {
    node.base->accept(*this);
    node.index->accept(*this);
}

void ScopeResolver::visit(IndexAssignExpr &node) {
    node.base->accept(*this);
    node.index->accept(*this);
    node.value->accept(*this);
}

void ScopeResolver::visit(StructLiteralExpr &node) {
    for (auto &f : node.fields)
        f.value->accept(*this);
}

void ScopeResolver::visit(FieldAccessExpr &node) {
    node.base->accept(*this);
}

void ScopeResolver::visit(FieldAssignExpr &node) {
    node.base->accept(*this);
    node.value->accept(*this);
}

void ScopeResolver::visit(EnumVariantExpr &node) {
    if (node.payload)
        node.payload->accept(*this);
}

void ScopeResolver::visit(MethodCallExpr &node) {
    node.base->accept(*this);
    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void ScopeResolver::visit(MatchExpr &node) {
    node.value->accept(*this);
    for (auto &arm : node.arms) {
        symbols_.enterScope();
        if (arm.hasBinding) {
            SymbolInfo bindInfo;
            bindInfo.name = arm.bindingName;
            symbols_.declare(arm.bindingName, bindInfo);
        }
        arm.body->accept(*this);
        symbols_.exitScope();
    }
}

void ScopeResolver::visit(NewExpr &) {
    // No variables to resolve in a new expression
}

void ScopeResolver::visit(ZoneExpr &node) {
    node.body->accept(*this);
}

void ScopeResolver::visit(AllocExpr &node) {
    if (node.count)
        node.count->accept(*this);
}

void ScopeResolver::visit(BorrowExpr &node) {
    node.target->accept(*this);
}

void ScopeResolver::visit(EscapeExpr &node) {
    node.target->accept(*this);
}

void ScopeResolver::visit(DeleteStmt &node) {
    node.pointer->accept(*this);
}

void ScopeResolver::visit(ForStmt &node) {
    node.iterable->accept(*this);
    symbols_.enterScope();
    SymbolInfo info;
    info.name = node.varName;
    info.typeInfo = TypeInfo::scalar(TypeKind::Unknown);
    symbols_.declare(node.varName, info);
    node.body->accept(*this);
    symbols_.exitScope();
}

// ── Statements ───────────────────────────────────────────────────────────────

void ScopeResolver::visit(ExprStmt &node) {
    node.expr->accept(*this);
}

void ScopeResolver::visit(VarDeclStmt &node) {
    if (node.initializer) {
        node.initializer->accept(*this);
    }

    if (symbols_.lookupCurrent(node.name)) {
        error(node.loc, "மாறி (Variable) '" + node.name + "' ஏற்கனவே இந்த எல்லைக்குள் அறிவிக்கப்பட்டுள்ளது");
        return;
    }

    SymbolInfo info;
    info.name = node.name;
    info.typeInfo = node.typeInfo;
    symbols_.declare(node.name, info);
}

void ScopeResolver::visit(BlockStmt &node) {
    symbols_.enterScope();
    for (auto &stmt : node.statements) {
        stmt->accept(*this);
    }
    symbols_.exitScope();
}

void ScopeResolver::visit(ReturnStmt &node) {
    if (node.value) {
        node.value->accept(*this);
    }
}

void ScopeResolver::visit(IfStmt &node) {
    node.condition->accept(*this);
    node.thenBranch->accept(*this);
    if (node.elseBranch) {
        node.elseBranch->accept(*this);
    }
}

void ScopeResolver::visit(WhileStmt &node) {
    node.condition->accept(*this);
    node.body->accept(*this);
}

// ── Declarations ─────────────────────────────────────────────────────────────

void ScopeResolver::visit(FunctionDecl &node) {
    // Register function in current scope
    SymbolInfo funcInfo;
    funcInfo.name = node.name;
    funcInfo.typeInfo = node.returnTypeInfo;
    funcInfo.isFunction = true;
    funcInfo.returnType = node.returnTypeInfo;
    for (auto &p : node.params) {
        funcInfo.paramTypes.push_back(p.typeInfo);
    }

    if (!symbols_.declare(node.name, funcInfo)) {
        error(node.loc, "செயல் (Function) '" + node.name + "' ஏற்கனவே அறிவிக்கப்பட்டுள்ளது");
    }

    // Resolve body in new scope with parameters
    if (node.body) {
        symbols_.enterScope();
        for (auto &p : node.params) {
            SymbolInfo paramInfo;
            paramInfo.name = p.name;
            paramInfo.typeInfo = p.typeInfo;
            symbols_.declare(p.name, paramInfo);
        }

        for (auto &stmt : node.body->statements) {
            stmt->accept(*this);
        }
        symbols_.exitScope();
    }
}

void ScopeResolver::visit(ConstDecl &node) {
    if (node.value) {
        node.value->accept(*this);
    }
    if (symbols_.lookupCurrent(node.name)) {
        error(node.loc,
              "நிலைமாறிலி (Constant) '" + node.name + "' ஏற்கனவே இந்த எல்லைக்குள் அறிவிக்கப்பட்டுள்ளது");
        return;
    }
    SymbolInfo info;
    info.name = node.name;
    info.typeInfo = node.typeInfo;
    info.isConstant = true;
    symbols_.declare(node.name, info);
}

void ScopeResolver::visit(StructDecl &node) {
    // Current scope resolver only tracks variables/functions, not types.
}

void ScopeResolver::visit(EnumDecl &) {
    // Enums are resolved by the THIR/HIR builder
}

void ScopeResolver::visit(TraitDecl &) {
    // Traits only contain signatures, which don't have bodies to resolve.
}

void ScopeResolver::visit(ImplDecl &node) {
    for (auto &method : node.methods) {
        symbols_.enterScope();

        for (auto &p : method->params) {
            SymbolInfo paramInfo;
            paramInfo.name = p.name;
            symbols_.declare(p.name, paramInfo);
        }
        if (method->body) {
            for (auto &stmt : method->body->statements) {
                stmt->accept(*this);
            }
        }
        symbols_.exitScope();
    }
}

void ScopeResolver::visit(Program &node) {
    // Pass 1: Register signatures
    for ([[maybe_unused]] auto &sd : node.structs) {
        // Structs are just names in ScopeResolver
    }
    for (auto &fn : node.functions) {
        SymbolInfo info;
        info.name = fn->name;
        info.isFunction = true;
        symbols_.declare(fn->name, info);
    }
    for (auto &cn : node.constants) {
        SymbolInfo info;
        info.name = cn->name;
        info.typeInfo = cn->typeInfo;
        info.isConstant = true;
        symbols_.declare(cn->name, info);
    }

    // Pass 2: Resolve bodies
    for (auto &td : node.traits)
        td->accept(*this);
    for (auto &id : node.impls)
        id->accept(*this);
    for (auto &fn : node.functions) {
        if (fn->body) {
            symbols_.enterScope();
            for (auto &p : fn->params) {
                SymbolInfo pInfo;
                pInfo.name = p.name;
                symbols_.declare(p.name, pInfo);
            }
            for (auto &stmt : fn->body->statements)
                stmt->accept(*this);
            symbols_.exitScope();
        }
    }
}

} // namespace agam
