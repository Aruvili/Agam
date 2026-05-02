#include "agam/semantic/type_checker.h"

#include <string>
#include <vector>

namespace agam {

bool TypeChecker::check(Program &program, DiagnosticEngine &diag) {
    diag_ = &diag;
    diag_->clear();
    symbols_ = SymbolTable();
    activeTypeParams_.clear();
    visit(program);
    return !diag_->hasErrors();
}

void TypeChecker::error(const SourceLocation &loc, const std::string &message) {
    if (diag_)
        diag_->error(loc, message);
}

TypeInfo TypeChecker::resolveType(const TypeInfo &type, const SourceLocation &loc) {
    if (type.pointerDepth > 0) {
        TypeInfo elem = type;
        elem.pointerDepth = 0;
        TypeInfo resolvedElem = resolveType(elem, loc);
        resolvedElem.pointerDepth = type.pointerDepth;
        resolvedElem.pulseTag = type.pulseTag;
        resolvedElem.isMutable = type.isMutable;
        return resolvedElem;
    }
    if (type.isArray || type.isSlice) {
        TypeInfo elem = type.getElementTypeInfo();
        TypeInfo resolvedElem = resolveType(elem, loc);
        TypeInfo result = type;
        result.elementType = resolvedElem.kind;
        result.isStruct = resolvedElem.isStruct;
        result.structName = resolvedElem.structName;
        result.isGeneric = resolvedElem.isGeneric;
        result.genericName = resolvedElem.genericName;
        result.isEnum = resolvedElem.isEnum;
        result.enumName = resolvedElem.enumName;
        return result;
    }

    if (type.isGeneric) {
        return type;
    }
    if ((type.isStruct || type.kind == TypeKind::Unknown) && !type.isGeneric) {
        std::string name = type.isStruct ? type.structName : type.genericName;
        if (name.empty() && type.kind == TypeKind::Unknown)
            return type;

        if (!name.empty() && symbols_.lookupEnum(name)) {
            TypeInfo resolved = type;
            resolved.isStruct = false;
            resolved.isEnum = true;
            resolved.enumName = name;
            resolved.structName = "";
            return resolved;
        }
        if (!name.empty() && activeTypeParams_.find(name) != activeTypeParams_.end()) {
            TypeInfo resolved = type;
            resolved.isGeneric = true;
            resolved.genericName = name;
            resolved.isStruct = false;
            resolved.structName = "";
            resolved.kind = TypeKind::Generic;
            return resolved;
        }
        if (type.isStruct && !symbols_.lookupStruct(type.structName)) {
            error(loc, "அறியப்படாத வகை (Unknown type): '" + type.structName + "'");
        }
    }

    // ZPM Pulse Tag Validation
    if (!type.pulseTag.empty()) {
        if (!symbols_.isZoneInScope(type.pulseTag)) {
            error(loc, "அறியப்படாத அல்லது செயலற்ற மண்டலம் (Unknown region) '" + type.pulseTag +
                           "' துடிப்பு அடையாளமாக (pulse tag) பயன்படுத்தப்பட்டுள்ளது");
        }
    }

    return type;
}

void TypeChecker::visit(ZoneExpr &node) {
    symbols_.enterZone(node.zoneName);
    symbols_.enterScope();

    node.body->accept(*this);

    symbols_.exitScope();
    symbols_.exitZone();
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Void); // For now, zones are void-valued
}

void TypeChecker::visit(AllocExpr &node) {
    TypeInfo base = resolveType(node.type, node.loc);
    if (node.count) {
        node.count->accept(*this);
        if (!isNumeric(lastExprInfo_.kind)) {
            error(node.loc, "ஒதுக்கீடு எண்ணிக்கை (Allocation count) எண்ணாக இருக்க வேண்டும்");
        }
    }

    lastExprInfo_ = base;
    lastExprInfo_.pointerDepth++;
    lastExprInfo_.isMutable = true; // Owned allocation is always mutable
    if (lastExprInfo_.pulseTag.empty()) {
        lastExprInfo_.pulseTag = symbols_.currentZone();
    }
    lastExprMutable_ = true;
}

void TypeChecker::visit(BorrowExpr &node) {
    node.target->accept(*this);

    // In Agam, we can borrow anything that has an address (variables, fields, dereferences)
    if (lastExprInfo_.pointerDepth == 0 && !lastExprInfo_.isStruct && !lastExprInfo_.isEnum &&
        lastExprInfo_.kind == TypeKind::Unknown) {
        error(node.loc, "இந்தக் கோவையிலிருந்து (expression) கடன் (borrow) பெற முடியாது");
    }

    // The result of a borrow is always a pointer to the target type.
    // FIX: If the target is already a pointer, we "re-borrow" it (maintain same depth).
    TypeInfo resultType = lastExprInfo_;
    if (resultType.pointerDepth == 0) {
        resultType.pointerDepth = 1;
    }

    // Determine the symbol name if it's a variable or field access
    std::string varName = getRootVariable(node.target.get());

    if (node.isMutable) {
        if (!lastExprMutable_) {
            error(node.loc,
                  "மாற்ற முடியாத (immutable) மூலத்திலிருந்து மாற்றக்கூடிய (mutable) கடனைப் பெற முடியாது");
        }
        if (!varName.empty()) {
            auto info = symbols_.lookup(varName);
            if (info && (info->sharedBorrowCount > 0 || info->mutablyBorrowed)) {
                error(node.loc, "'" + varName +
                                    "' ஏற்கனவே கடன் வாங்கப்பட்டிருக்கும் நிலையில் மாற்றக்கூடிய வகையில் "
                                    "(mutably) கடன் வாங்க முடியாது");
            }
            symbols_.addBorrow(varName, true);
        }
    } else {
        if (!varName.empty()) {
            auto info = symbols_.lookup(varName);
            if (info && info->mutablyBorrowed) {
                error(node.loc, "'" + varName +
                                    "' மாற்றக்கூடிய வகையில் (mutably) கடன் வாங்கப்பட்டிருக்கும் நிலையில் கடன் "
                                    "வாங்க முடியாது");
            }
            symbols_.addBorrow(varName, false);
        }
    }

    resultType.isMutable = node.isMutable;
    lastExprInfo_ = resultType;
    lastExprMutable_ =
        node.isMutable; // A pointer itself is an L-value, but here it's an R-value result
}

void TypeChecker::visit(EscapeExpr &node) {
    node.target->accept(*this);
    TypeInfo type = lastExprInfo_;

    if (type.pointerDepth == 0) {
        error(node.loc, "சுட்டியால் (pointer) மட்டுமே தப்பிக்க (escape) முடியும்");
    }

    // Check if it's safe to escape (no active borrows if it's a variable or field)
    std::string varName = getRootVariable(node.target.get());
    if (!varName.empty()) {
        auto info = symbols_.lookup(varName);
        if (info && (info->sharedBorrowCount > 0 || info->mutablyBorrowed)) {
            error(node.loc,
                  "நினைவக பாதுகாப்பு மீறல் (Memory safety violation): '" + varName +
                      "' (அல்லது அதன் புலங்கள்) கடன் வாங்கப்பட்டிருக்கும் நிலையில் தப்பிக்க (escape) முடியாது");
        }
    }

    if (!symbols_.isZoneInScope(node.destinationZone)) {
        error(node.loc,
              "இலக்கு மண்டலம் (Destination zone) '" + node.destinationZone + "' எல்லைக்குள் இல்லை");
    }

    // Update pulse tag
    type.pulseTag = node.destinationZone;
    lastExprInfo_ = type;
}

void TypeChecker::visit(Program &node) {
    // Pass 1: Register all declarations
    for (size_t i = 0; i < node.structs.size(); ++i) {
        auto &sd = node.structs[i];
        std::vector<SymbolInfo> fields;
        for (auto &f : sd->fields) {
            SymbolInfo fi;
            fi.name = f.name;
            fi.typeInfo = f.typeInfo;
            fi.isMutable = f.isMutable;
            fields.push_back(fi);
        }
        symbols_.declareStruct(sd->name, sd->typeParams, fields);
        SymbolInfo info;
        info.name = sd->name;
        info.typeInfo = TypeInfo::namedStruct(sd->name);
        symbols_.declare(sd->name, info);
    }

    for (size_t i = 0; i < node.enums.size(); ++i) {
        auto &ed = node.enums[i];
        EnumInfo info;
        info.name = ed->name;
        info.typeParams = ed->typeParams;
        for (auto &v : ed->variants) {
            VariantInfo vi;
            vi.name = v.name;
            vi.hasPayload = v.hasPayload;
            vi.payloadType = v.payloadType;
            info.variants.push_back(vi);
        }
        symbols_.declareEnum(ed->name, ed->typeParams, info);
        SymbolInfo sinfo;
        sinfo.name = ed->name;
        sinfo.typeInfo = TypeInfo::namedEnum(ed->name);
        symbols_.declare(ed->name, sinfo);
    }

    for (size_t i = 0; i < node.functions.size(); ++i) {
        auto &fn = node.functions[i];
        SymbolInfo info;
        info.name = fn->name;
        info.typeInfo = fn->returnTypeInfo;
        info.isFunction = true;
        info.typeParams = fn->typeParams;
        for (auto &p : fn->params)
            info.paramTypes.push_back(p.typeInfo);
        info.isMutating = fn->isMutating;
        symbols_.declare(fn->name, info);
    }

    for (size_t i = 0; i < node.impls.size(); ++i) {
        auto &id = node.impls[i];
        ImplInfo info;
        info.traitName = id->traitName;
        info.targetType = id->targetType;
        for (auto &fn : id->methods) {
            SymbolInfo mi;
            mi.name = fn->name;
            mi.typeInfo = fn->returnTypeInfo;
            mi.isFunction = true;
            for (auto &p : fn->params)
                mi.paramTypes.push_back(p.typeInfo);
            mi.isMutating = fn->isMutating;
            info.methods.push_back(mi);
        }
        symbols_.declareImpl(info);
    }

    for (size_t i = 0; i < node.constants.size(); ++i) {
        auto &cn = node.constants[i];
        SymbolInfo info;
        info.name = cn->name;
        info.typeInfo = cn->typeInfo;
        info.isConstant = true;
        symbols_.declare(cn->name, info);
    }

    for (auto &cn : node.constants) {
        cn->accept(*this);
    }

    for (size_t i = 0; i < node.impls.size(); ++i) {
        node.impls[i]->accept(*this);
    }
    for (size_t i = 0; i < node.functions.size(); ++i) {
        auto &fn = node.functions[i];
        if (!fn->typeParams.empty())
            continue;
        if (fn->body) {
            TypeKind prevReturnType = currentReturnType_;
            currentReturnType_ = fn->returnTypeInfo.kind;

            symbols_.enterScope();
            for (auto &p : fn->params) {
                SymbolInfo pInfo;
                pInfo.name = p.name;
                pInfo.typeInfo = p.typeInfo;
                pInfo.isMutable = p.isMutable;
                if (p.isMutable && pInfo.typeInfo.pointerDepth > 0) {
                    pInfo.typeInfo.isMutable = true;
                }
                symbols_.declare(p.name, pInfo);
            }

            std::vector<std::unique_ptr<Stmt>> expanded;
            for (auto &stmt : fn->body->statements) {
                if (auto *es = dynamic_cast<ExprStmt *>(stmt.get())) {
                    if (auto *call = dynamic_cast<CallExpr *>(es->expr.get())) {
                        if (expandPrintCall(call, expanded)) {
                            continue;
                        }
                    } else if (auto *mc = dynamic_cast<MethodCallExpr *>(es->expr.get())) {
                        if (mc->methodName == "print" || mc->methodName == "println") {
                            mc->base->accept(*this);
                            TypeInfo baseTi = lastExprInfo_;
                            if (!symbols_.findImplForMethod(baseTi, mc->methodName)) {
                                if (expandStructuralPrinting(mc->base.get(), mc->methodName,
                                                             expanded)) {
                                    continue;
                                }
                            }
                        }
                    }
                }
                stmt->accept(*this);
                expanded.push_back(std::move(stmt));
            }
            fn->body->statements = std::move(expanded);

            symbols_.exitScope();
            currentReturnType_ = prevReturnType;
        }
    }
    for (size_t i = 0; i < node.constants.size(); ++i) {
        node.constants[i]->accept(*this);
    }
}

void TypeChecker::visit(StructDecl &node) {
    std::vector<SymbolInfo> fields;
    for (auto &f : node.fields) {
        resolveType(f.typeInfo, node.loc);
        SymbolInfo fi;
        fi.name = f.name;
        fi.typeInfo = f.typeInfo;
        fields.push_back(fi);
    }
    symbols_.declareStruct(node.name, node.typeParams, fields);
}

void TypeChecker::visit(EnumDecl &node) {
    // Enum registration is handled in Program pass
}

void TypeChecker::visit(TraitDecl &node) {
    TraitInfo info;
    info.name = node.name;
    for (auto &method : node.methods) {
        SymbolInfo mi;
        mi.name = method->name;
        mi.typeInfo = method->returnTypeInfo;
        mi.isFunction = true;
        for (auto &p : method->params)
            mi.paramTypes.push_back(p.typeInfo);
        info.methods.push_back(mi);
    }
    symbols_.declareTrait(node.name, info);
}

void TypeChecker::visit(ImplDecl &node) {
    // Skip generic impl blocks - they will be type-checked after
    // monomorphization produces concrete specialized versions.
    if (!node.typeParams.empty())
        return;

    // Bodies are checked here
    for (auto &fn : node.methods) {
        TypeKind prevReturnType = currentReturnType_;
        currentReturnType_ = fn->returnTypeInfo.kind;

        symbols_.enterScope();

        std::set<std::string> prevParams = activeTypeParams_;
        for (const auto &tp : node.typeParams) {
            SymbolInfo tpInfo;
            tpInfo.name = tp;
            tpInfo.typeInfo = TypeInfo::scalar(TypeKind::Unknown);
            tpInfo.typeInfo.isGeneric = true;
            tpInfo.typeInfo.genericName = tp;
            symbols_.declare(tp, tpInfo);
            activeTypeParams_.insert(tp);
        }

        for (auto &p : fn->params) {
            SymbolInfo pInfo;
            pInfo.name = p.name;
            pInfo.typeInfo = p.typeInfo;
            pInfo.isMutable = p.isMutable;
            // For 'mut' pointer parameters (e.g., mut self: *T), propagate
            // mutability into the type so field mutations through self work.
            if (p.isMutable && pInfo.typeInfo.pointerDepth > 0) {
                pInfo.typeInfo.isMutable = true;
            }
            // Substitute 'self' type if needed
            if (pInfo.typeInfo.kind == TypeKind::Self) {
                pInfo.typeInfo = node.targetType;
            }
            symbols_.declare(p.name, pInfo);
        }
        // Substitute 'self' in return type too
        if (fn->returnTypeInfo.kind == TypeKind::Self) {
            fn->returnTypeInfo = node.targetType;
            currentReturnType_ = fn->returnTypeInfo.kind;
        }

        if (fn->body) {
            std::vector<std::unique_ptr<Stmt>> expanded;
            for (auto &stmt : fn->body->statements) {
                if (auto *es = dynamic_cast<ExprStmt *>(stmt.get())) {
                    if (auto *call = dynamic_cast<CallExpr *>(es->expr.get())) {
                        if (expandPrintCall(call, expanded)) {
                            continue;
                        }
                    }
                }
                stmt->accept(*this);
                expanded.push_back(std::move(stmt));
            }
            fn->body->statements = std::move(expanded);
        }

        symbols_.exitScope();
        activeTypeParams_ = prevParams;
        currentReturnType_ = prevReturnType;
    }
}

void TypeChecker::visit(ConstDecl &node) {
    node.typeInfo = resolveType(node.typeInfo, node.loc);
    if (node.value) {
        node.value->accept(*this);
        TypeInfo initType = lastExprInfo_;
        if (auto reason = isCompatible(node.typeInfo, initType); !reason.empty()) {
            error(node.loc,
                  "நிலைமாறிலி (Constant) '" + node.name + "' ஐத் தொடங்க முடியாது: " + reason);
        }
    } else {
        error(node.loc,
              "நிலைமாறிலி (Constant) '" + node.name + "' ஒரு தொடக்க மதிப்பைக் கொண்டிருக்க வேண்டும்");
    }
    // Constants are always immutable
    SymbolInfo info;
    info.name = node.name;
    info.typeInfo = node.typeInfo;
    info.isConstant = true;
    symbols_.declare(node.name, info);
}

void TypeChecker::visit(FunctionDecl &node) {
    activeTypeParams_.clear();
    for (const auto &p : node.typeParams)
        activeTypeParams_.insert(p);

    TypeKind prevReturnType = currentReturnType_;
    currentReturnType_ = node.returnTypeInfo.kind;

    symbols_.enterScope();
    for (auto &p : node.params) {
        SymbolInfo paramInfo;
        paramInfo.name = p.name;
        paramInfo.typeInfo = p.typeInfo;
        paramInfo.isMutable = p.isMutable;
        if (p.isMutable && paramInfo.typeInfo.pointerDepth > 0) {
            paramInfo.typeInfo.isMutable = true;
        }
        symbols_.declare(p.name, paramInfo);
    }

    if (node.body) {
        std::vector<std::unique_ptr<Stmt>> expanded;
        for (auto &stmt : node.body->statements) {
            if (auto *es = dynamic_cast<ExprStmt *>(stmt.get())) {
                if (auto *call = dynamic_cast<CallExpr *>(es->expr.get())) {
                    if (expandPrintCall(call, expanded)) {
                        continue;
                    }
                }
            }
            stmt->accept(*this);
            expanded.push_back(std::move(stmt));
        }
        node.body->statements = std::move(expanded);
    }

    currentReturnType_ = prevReturnType;
    for (const auto &p : node.typeParams)
        activeTypeParams_.erase(p);
    symbols_.exitScope();
}

void TypeChecker::visit(VarDeclStmt &node) {
    if (symbols_.lookupCurrent(node.name)) {
        error(node.loc, "மாறி (Variable) '" + node.name + "' மீண்டும் வரையறுக்கப்பட்டுள்ளது");
        return;
    }

    node.typeInfo = resolveType(node.typeInfo, node.loc);

    if (node.initializer) {
        node.initializer->accept(*this);
        TypeInfo initType = lastExprInfo_;
        if (auto reason = isCompatible(node.typeInfo, initType); !reason.empty()) {
            error(node.loc, "மாறி (Variable) '" + node.name + "' ஐத் தொடங்க முடியாது: " + reason);
        }

        // ZPM Safety check
        isZoneCompatible(initType.pulseTag, node.typeInfo.pulseTag, node.loc);

        // Propagate borrow mutability: if a pointer is initialized from a borrow expression,
        // carry the borrow's isMutable flag into the variable's stored type.
        if (node.typeInfo.pointerDepth > 0 && initType.pointerDepth > 0) {
            node.typeInfo.isMutable = initType.isMutable;
        }
    }

    SymbolInfo info;
    info.name = node.name;
    info.typeInfo = node.typeInfo;
    info.isMutable = node.isMutable;
    symbols_.declare(node.name, info);
}

void TypeChecker::visit(BlockStmt &node) {
    symbols_.enterScope();
    std::vector<std::unique_ptr<Stmt>> expanded;
    for (auto &stmt : node.statements) {
        if (auto *es = dynamic_cast<ExprStmt *>(stmt.get())) {
            if (auto *call = dynamic_cast<CallExpr *>(es->expr.get())) {
                if (expandPrintCall(call, expanded)) {
                    continue;
                }
            }
        }
        stmt->accept(*this);
        expanded.push_back(std::move(stmt));
    }
    node.statements = std::move(expanded);
    symbols_.exitScope();
}

void TypeChecker::visit(ReturnStmt &node) {
    if (node.value) {
        node.value->accept(*this);
        TypeInfo retType = lastExprInfo_;

        // Basic type compatibility check for return
        if (currentReturnType_ != TypeKind::Void) {
            if (auto reason = isCompatible(currentReturnType_, retType.kind); !reason.empty()) {
                error(node.loc, "திரும்பும் வகை பொருந்தவில்லை (Return type mismatch): " + reason);
            }
        }

        // ZPM Safety: Cannot return a tagged pointer from a function (local zone escape)
        if (!retType.pulseTag.empty()) {
            error(node.loc, "நினைவக பாதுகாப்பு மீறல் (Memory safety violation): உள்ளூர் மண்டலத்திலிருந்து "
                            "(local zone) '~" +
                                retType.pulseTag + "' சுட்டியைத் திரும்பப் பெற முடியாது");
        }
    } else if (currentReturnType_ != TypeKind::Void) {
        error(node.loc, "வெற்றிடமில்லாச் செயல் (Non-void function) ஒரு மதிப்பைத் திரும்பப் பெற வேண்டும்");
    }
}

void TypeChecker::visit(IfStmt &node) {
    node.condition->accept(*this);
    node.thenBranch->accept(*this);
    if (node.elseBranch)
        node.elseBranch->accept(*this);
}

void TypeChecker::visit(WhileStmt &node) {
    node.condition->accept(*this);
    node.body->accept(*this);
}

void TypeChecker::visit(ForStmt &node) {
    node.iterable->accept(*this);
    symbols_.enterScope();
    SymbolInfo itemInfo;
    itemInfo.name = node.varName;
    itemInfo.typeInfo = lastExprInfo_;
    itemInfo.isMutable = true; // for-loop variable is always a mutable binding
    symbols_.declare(node.varName, itemInfo);
    node.body->accept(*this);
    symbols_.exitScope();
}

void TypeChecker::visit(DeleteStmt &node) {
    node.pointer->accept(*this);
    if (lastExprInfo_.pointerDepth == 0 && lastExprInfo_.kind != TypeKind::Unknown) {
        error(node.loc,
              "delete requires a pointer operand, got '" + typeInfoToString(lastExprInfo_) + "'");
    }
}

void TypeChecker::visit(ExprStmt &node) {
    symbols_.enterScope();
    node.expr->accept(*this);
    symbols_.exitScope();
}

void TypeChecker::visit(IntLiteralExpr &node) {
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Int);
    lastExprMutable_ = false;
}

void TypeChecker::visit(FloatLiteralExpr &node) {
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Float64);
    lastExprMutable_ = false;
}

void TypeChecker::visit(StringLiteralExpr &node) {
    lastExprInfo_ = TypeInfo::scalar(TypeKind::String);
    lastExprMutable_ = false;
}

void TypeChecker::visit(BoolLiteralExpr &node) {
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Bool);
    lastExprMutable_ = false;
}

void TypeChecker::visit(NullLiteralExpr &node) {
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Null);
    lastExprInfo_.pointerDepth = 1; // It acts like a pointer
    lastExprMutable_ = false;
}

void TypeChecker::visit(VariableExpr &node) {
    auto info = symbols_.lookup(node.name);
    if (!info) {
        error(node.loc, "Undefined variable '" + node.name + "'");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        lastExprMutable_ = false;
        return;
    }

    if (info->isFunction) {
        lastExprInfo_ = info->typeInfo;
        lastExprMutable_ = false;
        return;
    }

    if (!canRead(node.name, node.loc)) {
        // Error already reported in canRead
    }

    lastExprInfo_ = info->typeInfo;
    // For pointer types, mutability is determined by the pointer type itself (*mut T vs *T)
    if (info->typeInfo.pointerDepth > 0) {
        lastExprMutable_ = info->typeInfo.isMutable;
    } else {
        lastExprMutable_ = info->isMutable;
    }
}

void TypeChecker::visit(BinaryExpr &node) {
    node.lhs->accept(*this);
    TypeInfo lhs = lastExprInfo_;
    node.rhs->accept(*this);
    TypeInfo rhs = lastExprInfo_;

    lastExprMutable_ = false; // Results of binary ops are not L-values

    if (auto reason = isCompatible(lhs.kind, rhs.kind); !reason.empty()) {
        error(node.loc,
              "இருமை செயலி வகைகள் பொருந்தவில்லை (Binary operator types mismatch): " + reason);
    }

    // Comparison operators always produce bool
    if (node.op == BinaryOp::Eq || node.op == BinaryOp::Neq || node.op == BinaryOp::Lt ||
        node.op == BinaryOp::Gt || node.op == BinaryOp::Lte || node.op == BinaryOp::Gte) {
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Bool);
    } else if (node.op == BinaryOp::And || node.op == BinaryOp::Or) {
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Bool);
    } else {
        // Arithmetic operators (+, -, *, /) require numeric types
        // Exception: string + string is string concatenation
        bool isStringConcat = (node.op == BinaryOp::Add && lhs.kind == TypeKind::String &&
                               rhs.kind == TypeKind::String);
        if (!isStringConcat && (!isNumeric(lhs.kind) || !isNumeric(rhs.kind))) {
            error(node.loc,
                  "கணித செயலிகளுக்கு எண் வகை தருமங்கள் (numeric operands) தேவை, ஆனால் கிடைத்துள்ளது '" +
                      std::string(typeKindToString(lhs.kind)) + "' மற்றும் '" +
                      typeKindToString(rhs.kind) + "'");
        }
        lastExprInfo_ = lhs;
    }
}

void TypeChecker::visit(UnaryExpr &node) {
    node.operand->accept(*this);
    if (node.op == UnaryOp::AddressOf) {
        // Propagate mutability: &mut_var produces a mutable pointer
        lastExprInfo_.isMutable = lastExprMutable_;
        lastExprInfo_.pointerDepth++;
        lastExprMutable_ = false;
    } else if (node.op == UnaryOp::Dereference) {
        if (lastExprInfo_.pointerDepth > 0) {
            lastExprInfo_.pointerDepth--;
            lastExprMutable_ = true;
        } else {
            error(node.loc, "சுட்டியல்லாத (non-pointer) வகையை குறிநீக்கம் (dereference) செய்ய முடியாது");
        }
    } else {
        lastExprMutable_ = false;
    }
}

void TypeChecker::visit(CastExpr &node) {
    node.expr->accept(*this);
    TypeInfo from = lastExprInfo_;
    TypeInfo to = resolveType(node.targetType, node.loc);

    // Semantic validation for casts
    bool valid = false;
    if (isNumeric(from.kind) && isNumeric(to.kind))
        valid = true;
    else if (from.pointerDepth > 0 && to.pointerDepth > 0)
        valid = true;
    else if (from.pointerDepth > 0 && isNumeric(to.kind))
        valid = true; // pointer to int
    else if (isNumeric(from.kind) && to.pointerDepth > 0)
        valid = true; // int to pointer
    else if (isCompatible(to, from).empty())
        valid = true;

    if (!valid) {
        error(node.loc, "தவறான வகை மாற்றம் (invalid cast): '" + typeInfoToString(from) +
                            "' இலிருந்து '" + typeInfoToString(to) + "' க்கு மாற்ற முடியாது");
    }

    // ZPM Safety: Inherit pulse tag if not explicitly specified in target type
    if (to.pulseTag.empty() && !from.pulseTag.empty()) {
        to.pulseTag = from.pulseTag;
    }

    lastExprInfo_ = to;
    lastExprMutable_ = false;
}

void TypeChecker::visit(CallExpr &node) {
    auto info = symbols_.lookup(node.callee);
    if (!info) {
        if (node.callee == "printf" || node.callee == "printf_int" ||
            node.callee == "printf_float" || node.callee == "printf_hex" ||
            node.callee == "printf_bin" || node.callee == "scanf_int" ||
            node.callee == "scanf_int64" || node.callee == "scanf_float" ||
            node.callee == "agam_getchar" || node.callee == "agam_putchar" ||
            node.callee == "agam_readline" || node.callee == "fprintf_stderr" ||
            node.callee == "fprintf_stderr_int" || node.callee == "fprintf_stderr_float" ||
            node.callee == "safe_borrow_add" || node.callee == "safe_borrow_remove" ||
            node.callee == "பதிப்பி" || node.callee == "வரியிறக்கி_பதிப்பி" ||
            node.callee == "எண்_வாசி" || node.callee == "தசமம்_வாசி") {
            lastExprInfo_ = TypeInfo::scalar(TypeKind::Int32);
            return;
        }
        error(node.loc, "வரையறுக்கப்படாத செயல் (Undefined function): '" + node.callee + "'");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    // FIX: Simple Generic Inference
    if (!info->typeParams.empty() && node.genericArgs.empty()) {
        for (const auto &tp : info->typeParams) {
            [[maybe_unused]] bool inferred = false;
            for (size_t i = 0; i < info->paramTypes.size(); i++) {
                if (i < node.args.size() && info->paramTypes[i].isGeneric &&
                    info->paramTypes[i].genericName == tp) {
                    node.args[i]->accept(*this);
                    node.genericArgs.push_back(lastExprInfo_);
                    inferred = true;
                    break;
                }
            }
        }
    }

    TypeInfo resultType = info->typeInfo;
    if (!info->typeParams.empty() && !node.genericArgs.empty()) {
        resultType = substitute(resultType, info->typeParams, node.genericArgs);
    }

    // Validate argument count (skip check for generic functions — inference handles them)
    if (info->typeParams.empty() && !info->paramTypes.empty()) {
        if (node.args.size() != info->paramTypes.size()) {
            error(node.loc, "செயல் '" + node.callee + "' எதிர்பார்க்கும் அளபுருக்கள் (arguments) " +
                                std::to_string(info->paramTypes.size()) + ", ஆனால் கிடைத்துள்ளது " +
                                std::to_string(node.args.size()));
        }
    }

    lastExprInfo_ = resultType;
}

void TypeChecker::visit(MethodCallExpr &node) {
    node.base->accept(*this);
    TypeInfo baseType = lastExprInfo_;
    auto impl = symbols_.findImplForMethod(baseType, node.methodName);
    if (impl) {
        for (const auto &fn : impl->methods) {
            if (fn.name == node.methodName) {
                // Receiver Borrow Check
                std::string baseVar = getRootVariable(node.base.get());

                if (!baseVar.empty()) {
                    if (fn.isMutating) {
                        auto varInfo = symbols_.lookup(baseVar);
                        bool isPointerReceiver = varInfo && varInfo->typeInfo.pointerDepth > 0;

                        if (isPointerReceiver) {
                            bool hasMutableAccess =
                                varInfo->typeInfo.isMutable || varInfo->isMutable;
                            if (!hasMutableAccess) {
                                error(
                                    node.loc,
                                    "பகிர்வு கடனின் (shared borrow) வழியாக '" + baseVar +
                                        "' மாற்றக்கூடிய செயல்முறையினை (mutating method) '" +
                                        node.methodName +
                                        "' அழைக்க முடியாது; இதற்கு மாற்றக்கூடிய கடன் (borrow mut) தேவை");
                            }
                        } else {
                            // Value receiver: variable must be declared 'mut'
                            if (varInfo && !varInfo->isMutable) {
                                error(node.loc,
                                      "மாற்ற முடியாத மாறியின் (immutable variable) மீது '" + baseVar +
                                          "' மாற்றக்கூடிய செயல்முறையினை (mutating method) '" +
                                          node.methodName + "' அழைக்க முடியாது");
                            }
                        }

                        if (!canMutate(baseVar, node.loc)) {
                            // Error reported
                        }
                        symbols_.addBorrow(baseVar, true);
                    } else {
                        if (!canRead(baseVar, node.loc)) {
                            // Error reported
                        }
                        symbols_.addBorrow(baseVar, false);
                    }
                }

                // Process arguments while receiver is borrowed
                for (auto &arg : node.args)
                    arg->accept(*this);

                // Substitute return type if it's generic
                TypeInfo resultType = fn.typeInfo;
                auto sdName = baseType.structName;
                size_t undPos = sdName.find('_');
                if (undPos != std::string::npos)
                    sdName = sdName.substr(0, undPos);
                auto sd = symbols_.lookupStruct(sdName);
                if (sd && !sd->typeParams.empty()) {
                    auto args = baseType.genericArgs;
                    if (args.empty() && undPos != std::string::npos) {
                        // Recover args from mangled name (e.g. Vector_int64)
                        std::string suffix = baseType.structName.substr(undPos + 1);
                        size_t start = 0;
                        size_t end = suffix.find('_');
                        while (start < suffix.length()) {
                            std::string typeName =
                                suffix.substr(start, (end == std::string::npos) ? std::string::npos
                                                                                : end - start);
                            if (typeName == "int64")
                                args.push_back(TypeInfo::scalar(TypeKind::Int64));
                            else if (typeName == "int32")
                                args.push_back(TypeInfo::scalar(TypeKind::Int32));
                            else if (typeName == "float64")
                                args.push_back(TypeInfo::scalar(TypeKind::Float64));
                            else if (typeName == "bool")
                                args.push_back(TypeInfo::scalar(TypeKind::Bool));
                            else if (typeName == "string")
                                args.push_back(TypeInfo::scalar(TypeKind::String));
                            else {
                                // Fallback: look up as struct/enum
                                auto rti = TypeInfo::scalar(TypeKind::Unknown);
                                rti.structName = typeName;
                                rti.isStruct = true;
                                args.push_back(rti);
                            }

                            if (end == std::string::npos)
                                break;
                            start = end + 1;
                            end = suffix.find('_', start);
                        }
                    }

                    if (!args.empty()) {
                        resultType = substitute(resultType, sd->typeParams, args);
                    }
                }

                lastExprInfo_ = resultType;

                // Release Borrows after the call
                if (!baseVar.empty()) {
                    symbols_.removeBorrow(baseVar, fn.isMutating);
                }

                // Use actual base type name for mangling (handles monomorphized types)
                std::string typeStr;
                if (baseType.isStruct)
                    typeStr = baseType.structName;
                else if (baseType.isEnum)
                    typeStr = baseType.enumName;
                else if (impl->targetType.isStruct)
                    typeStr = impl->targetType.structName;
                else if (impl->targetType.isEnum)
                    typeStr = impl->targetType.enumName;
                else
                    typeStr = typeKindToString(baseType.kind);

                if (impl->traitName.empty()) {
                    node.resolvedMangledName = "_Inherent_" + typeStr + "_" + node.methodName;
                } else {
                    node.resolvedMangledName =
                        "_Impl_" + impl->traitName + "_for_" + typeStr + "_" + node.methodName;
                }
                return;
            }
        }
    }

    if (baseType.isGeneric && (node.methodName == "print" || node.methodName == "println")) {
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    // Special case: .len() on arrays, slices and strings
    if ((baseType.isArray || baseType.isSlice || baseType.kind == TypeKind::String) &&
        node.methodName == "len") {
        if (!node.args.empty()) {
            error(node.loc,
                  "அணி/சிறுதுண்டின் (array/slice) 'நீளம்' (len) செயல்முறை 0 அளபுருக்களை எதிர்பார்க்கிறது");
        }
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Int64);
        lastExprMutable_ = false;
        node.resolvedMangledName = "__len";
        return;
    }

    error(node.loc, "வகை (type) '" + baseType.structName + "' இல் செயல்முறை (method) '" +
                        node.methodName + "' காணப்படவில்லை");
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
}

void TypeChecker::visit(AssignExpr &node) {
    auto info = symbols_.lookup(node.name);
    if (!info) {
        error(node.loc, "வரையறுக்கப்படாத மாறி (undefined variable): '" + node.name + "'");
        return;
    }

    if (info) {
        if (!info->isMutable) {
            error(node.loc, "மாற்ற முடியாத மாறி (non-mutable variable) '" + node.name +
                                "' க்கு மதிப்பு ஒதுக்க முடியாது");
        }
    }
    if (!canMutate(node.name, node.loc)) {
        // Error already reported in canMutate
    }

    node.value->accept(*this);
    TypeInfo valType = lastExprInfo_;

    if (auto reason = isCompatible(info->typeInfo, valType); !reason.empty()) {
        error(node.loc,
              "'" + node.name + "' க்கான ஒதுக்கீட்டில் வகை பொருந்தவில்லை (type mismatch): " + reason);
    }

    // ZPM Safety Check
    if (!info->typeInfo.pulseTag.empty() && !valType.pulseTag.empty() &&
        info->typeInfo.pulseTag != valType.pulseTag) {
        // If assigning a fresh allocation (empty/any tag) to a tagged field, it's allowed.
        if (valType.pulseTag.find("~") != std::string::npos ||
            info->typeInfo.pulseTag == "Global") {
            // Allowed
        } else {
            isZoneCompatible(valType.pulseTag, info->typeInfo.pulseTag, node.loc);
        }
    }

    lastExprInfo_ = info->typeInfo;
    lastExprMutable_ = true;
}

void TypeChecker::visit(ArrayLiteralExpr &node) {
    if (node.elements.empty()) {
        // For empty arrays, we'll try to infer the type from context in the future.
        // For now, let's set it to Unknown and handle it in isCompatible/AssignExpr.
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        lastExprInfo_.isArray = true;
        lastExprInfo_.arraySize = 0;
        lastExprMutable_ = false;
        return;
    }

    node.elements[0]->accept(*this);
    TypeInfo elemType = lastExprInfo_;

    for (size_t i = 1; i < node.elements.size(); ++i) {
        node.elements[i]->accept(*this);
        // We use a simplified compatibility check for literals
        if (auto reason = isCompatible(elemType, lastExprInfo_); !reason.empty()) {
            error(node.elements[i]->loc,
                  "அணி மாறிலியில் (array literal) வகைகள் பொருந்தவில்லை: " + reason);
        }
    }

    lastExprInfo_ = TypeInfo();
    lastExprInfo_.kind =
        TypeKind::Unknown; // The kind of the array itself is technically Unknown/Not-a-Scalar
    lastExprInfo_.isArray = true;
    lastExprInfo_.elementType = elemType.kind;
    lastExprInfo_.arraySize = (int)node.elements.size();
    lastExprMutable_ = false;
}

void TypeChecker::visit(IndexExpr &node) {
    node.base->accept(*this);
    TypeInfo baseTI = lastExprInfo_;
    bool baseMutable = lastExprMutable_;

    if (!baseTI.isArray && !baseTI.isSlice && baseTI.pointerDepth == 0) {
        error(node.loc, "அணி/சுட்டியல்லாத வகை (non-array/non-pointer type) '" +
                            typeInfoToString(baseTI) + "' இல் அட்டவணை (index) பயன்படுத்த முடியாது");
    }

    node.index->accept(*this);
    if (!isNumericType(lastExprInfo_.kind)) {
        error(node.loc, "அணி அட்டவணை (array index) எண்ணாக இருக்க வேண்டும், ஆனால் '" +
                            typeInfoToString(lastExprInfo_) + "' உள்ளது");
    }

    if (node.isRange) {
        if (node.endIndex) {
            node.endIndex->accept(*this);
            if (!isNumericType(lastExprInfo_.kind)) {
                error(node.loc,
                      "அணி சிறுதுண்டின் இறுதி அட்டவணை (slice end index) எண்ணாக இருக்க வேண்டும், ஆனால் '" +
                          typeInfoToString(lastExprInfo_) + "' உள்ளது");
            }
        }
        // Slicing returns a slice of the same element type
        lastExprInfo_ = TypeInfo::slice(baseTI.elementType);
        lastExprMutable_ = false; // slices are typically immutable views in many designs, or
                                  // depends on base. For now, let's say they are like base.
        lastExprMutable_ = baseMutable;
    } else {
        // Simple indexing returns the element type
        lastExprInfo_ = baseTI.getElementTypeInfo();
        // If the base structure was mutable (or a pointer to it), its fields are mutable.
        lastExprMutable_ = baseMutable || (baseTI.pointerDepth > 0);
    }
}

void TypeChecker::visit(IndexAssignExpr &node) {
    node.base->accept(*this);
    TypeInfo baseTI = lastExprInfo_;
    bool baseMutable = lastExprMutable_;

    if (!baseMutable) {
        // Relax: If it's a pointer to an array/slice, allow mutation of the element.
        if (baseTI.pointerDepth == 0) {
            error(node.base->loc,
                  "அணி மாற்ற முடியாதது (immutable) என்பதால் அதன் உறுப்பிற்கு மதிப்பு ஒதுக்க முடியாது");
        }
    }

    TypeInfo targetElementType = baseTI.getElementTypeInfo();

    // If base is a variable, check its borrow status
    std::string baseVar = "";
    Expr *current = node.base.get();
    while (current) {
        if (auto *var = dynamic_cast<VariableExpr *>(current)) {
            baseVar = var->name;
            break;
        } else if (auto *fa = dynamic_cast<FieldAccessExpr *>(current)) {
            current = fa->base.get();
        } else if (auto *ua = dynamic_cast<UnaryExpr *>(current)) {
            if (ua->op == UnaryOp::Dereference || ua->op == UnaryOp::AddressOf) {
                current = ua->operand.get();
            } else
                break;
        } else
            break;
    }

    if (!baseVar.empty()) {
        if (!canMutate(baseVar, node.loc)) {
            // Error already reported
        }
    }

    node.index->accept(*this);
    if (!isNumericType(lastExprInfo_.kind)) {
        error(node.loc,
              "array index must be numeric, got '" + typeInfoToString(lastExprInfo_) + "'");
    }

    node.value->accept(*this);
    TypeInfo valType = lastExprInfo_;

    std::string compatError = isCompatible(targetElementType, valType);
    if (!compatError.empty()) {
        error(node.loc, "அணி ஒதுக்கீட்டில் வகை பொருந்தவில்லை (Type mismatch in array assignment): '" +
                            typeInfoToString(valType) + "' ஐ '" +
                            typeInfoToString(targetElementType) +
                            "' வகை உறுப்பிற்கு ஒதுக்க முடியாது. " + compatError);
    }
}

void TypeChecker::visit(StructLiteralExpr &node) {
    lastExprInfo_ = node.structType;
    lastExprInfo_.isStruct = true;
    lastExprMutable_ = false;
}

void TypeChecker::visit(FieldAccessExpr &node) {
    node.base->accept(*this);
    TypeInfo base = lastExprInfo_;
    if (base.isSlice) {
        if (node.field == "data") {
            // Correctly derive pointer type from element type info
            TypeInfo elemTI = base.getElementTypeInfo();
            elemTI.pointerDepth++;
            elemTI.isArray = false;
            elemTI.isSlice = false;
            lastExprInfo_ = elemTI;
            lastExprMutable_ = true;
            return;
        } else if (node.field == "len") {
            lastExprInfo_ = TypeInfo::scalar(TypeKind::Int64);
            lastExprMutable_ = false;
            return;
        }
    }

    auto sd = symbols_.lookupStruct(base.structName);
    if (sd) {
        // Find field
        for (auto &f : sd->fields) {
            if (f.name == node.field) {
                TypeInfo fieldType = f.typeInfo;
                // Perform substitution if the struct is generic
                auto sinfo = symbols_.lookupStruct(base.structName);
                if (sinfo && !sinfo->typeParams.empty() && !base.genericArgs.empty()) {
                    fieldType = substitute(fieldType, sinfo->typeParams, base.genericArgs);
                }
                lastExprInfo_ = fieldType;
                return;
            }
        }
    }
    lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
}

void TypeChecker::visit(FieldAssignExpr &node) {
    node.base->accept(*this);
    TypeInfo baseTI = lastExprInfo_;
    if (!lastExprMutable_) {
        error(node.loc,
              "மாற்ற முடியாத பொருளின் (non-mutable object) புலத்திற்கு (field) மதிப்பு ஒதுக்க முடியாது");
    }

    // If base is a variable, check its borrow status
    std::string baseVar = "";
    Expr *current = node.base.get();
    while (current) {
        if (auto *var = dynamic_cast<VariableExpr *>(current)) {
            baseVar = var->name;
            break;
        } else if (auto *fa = dynamic_cast<FieldAccessExpr *>(current)) {
            current = fa->base.get();
        } else if (auto *ua = dynamic_cast<UnaryExpr *>(current)) {
            if (ua->op == UnaryOp::Dereference || ua->op == UnaryOp::AddressOf) {
                current = ua->operand.get();
            } else
                break;
        } else
            break;
    }

    if (!baseVar.empty()) {
        if (!canMutate(baseVar, node.loc)) {
            // Error already reported
        }
    }

    if (!baseTI.isStruct) {
        error(node.loc, "அமைப்பல்லாத வகை (non-struct type) ஒன்றின் புலத்தினை (field) அணுக முடியாது");
        return;
    }

    auto sd = symbols_.lookupStruct(baseTI.structName);
    if (!sd) {
        error(node.loc, "அறியப்படாத அமைப்பு (Unknown struct): '" + baseTI.structName + "'");
        return;
    }

    TypeInfo fieldType;
    bool found = false;
    for (const auto &f : sd->fields) {
        if (f.name == node.field) {
            fieldType = f.typeInfo;
            // Handle generic struct fields
            if (!baseTI.genericArgs.empty() && !sd->typeParams.empty()) {
                fieldType = substitute(fieldType, sd->typeParams, baseTI.genericArgs);
            }
            found = true;
            break;
        }
    }

    if (!found) {
        error(node.loc, "அமைப்பு (Struct) '" + baseTI.structName + "' இல் '" + node.field +
                            "' என்ற புலம் (field) இல்லை");
        return;
    }

    node.value->accept(*this);
    TypeInfo valType = lastExprInfo_;

    if (auto reason = isCompatible(fieldType, valType); !reason.empty()) {
        error(node.loc, "'" + node.field +
                            "' புலத்திற்கான ஒதுக்கீட்டில் வகை பொருந்தவில்லை (Type mismatch): " + reason);
    }

    // ZPM Safety Check
    if (!fieldType.pulseTag.empty() && !valType.pulseTag.empty() &&
        fieldType.pulseTag != valType.pulseTag) {
        if (valType.pulseTag.find("~") != std::string::npos || fieldType.pulseTag == "Global" ||
            fieldType.pulseTag.empty()) {
            // Allowed
        } else {
            isZoneCompatible(valType.pulseTag, fieldType.pulseTag, node.loc);
        }
    }
}

void TypeChecker::visit(EnumVariantExpr &node) {
    auto en = symbols_.lookupEnum(node.enumName);
    if (!en) {
        error(node.loc, "வரையறுக்கப்படாத பட்டியல் (undefined enum): '" + node.enumName + "'");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    const VariantInfo *variant = nullptr;
    for (const auto &v : en->variants) {
        if (v.name == node.variantName) {
            variant = &v;
            break;
        }
    }

    if (!variant) {
        error(node.loc, "பட்டியல் (Enum) '" + node.enumName + "' இல் '" + node.variantName +
                            "' என்ற மாற்றின் (variant) காணப்படவில்லை");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    if (node.payload) {
        if (!variant->hasPayload) {
            error(node.loc,
                  "மாற்றின் (variant) '" + node.variantName + "' சுமை (payload) எதனையும் ஏற்காது");
        } else {
            node.payload->accept(*this);
            TypeInfo எதிர்பார்க்கிறோம்Payload = variant->payloadType;
            if (!node.genericArgs.empty()) {
                எதிர்பார்க்கிறோம்Payload =
                    substitute(எதிர்பார்க்கிறோம்Payload, en->typeParams, node.genericArgs);
            }
            if (auto reason = isCompatible(எதிர்பார்க்கிறோம்Payload, lastExprInfo_); !reason.empty()) {
                error(node.loc, "மாற்றின் (variant) '" + node.variantName +
                                    "' க்கான சுமை வகை (payload type) பொருந்தவில்லை: " + reason);
            }
        }
    } else if (variant->hasPayload) {
        error(node.loc,
              "மாற்றின் (variant) '" + node.variantName + "' ஒரு சுமையினை (payload) எதிர்பார்க்கிறது");
    }

    TypeInfo ti = TypeInfo::namedEnum(node.enumName);
    ti.genericArgs = node.genericArgs;
    lastExprInfo_ = ti;
    lastExprMutable_ = false;
}

void TypeChecker::visit(MatchExpr &node) {
    node.value->accept(*this);
    TypeInfo valType = lastExprInfo_;

    if (!valType.isEnum) {
        error(node.loc, "'பொருத்து' (match) கோவை ஒரு பட்டியலாக (enum) இருக்க வேண்டும், ஆனால் '" +
                            typeInfoToString(valType) + "' உள்ளது");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    auto en = symbols_.lookupEnum(valType.enumName);
    if (!en) {
        error(node.loc, "வரையறுக்கப்படாத பட்டியல் (undefined enum): '" + valType.enumName + "'");
        lastExprInfo_ = TypeInfo::scalar(TypeKind::Unknown);
        return;
    }

    TypeInfo resultType;
    bool first = true;

    for (auto &arm : node.arms) {
        const VariantInfo *variant = nullptr;
        for (const auto &v : en->variants) {
            if (v.name == arm.variantName) {
                variant = &v;
                break;
            }
        }

        if (!variant) {
            error(node.loc, "பட்டியல் (Enum) '" + valType.enumName + "' இல் '" + arm.variantName +
                                "' என்ற மாற்றின் (variant) காணப்படவில்லை");
            continue;
        }

        symbols_.enterScope();
        if (arm.hasBinding) {
            if (!variant->hasPayload) {
                error(arm.body->loc, "மாற்றின் (variant) '" + arm.variantName +
                                         "' சுமை (payload) எதனையும் கொண்டிருக்கவில்லை");
            } else {
                SymbolInfo binding;
                binding.name = arm.bindingName;
                binding.typeInfo = variant->payloadType;
                if (!valType.genericArgs.empty()) {
                    binding.typeInfo =
                        substitute(binding.typeInfo, en->typeParams, valType.genericArgs);
                }
                symbols_.declare(arm.bindingName, binding);
            }
        }

        arm.body->accept(*this);
        if (first) {
            resultType = lastExprInfo_;
            first = false;
        } else {
            if (auto reason = isCompatible(resultType, lastExprInfo_); !reason.empty()) {
                error(node.loc, "'பொருத்து' (match) பிரிவுகளின் வகைகள் பொருந்தவில்லை: " + reason);
            }
        }
        symbols_.exitScope();
    }

    lastExprInfo_ = resultType;
    lastExprMutable_ = false;
}

void TypeChecker::visit(DerefAssignExpr &node) {
    node.pointer->accept(*this);
    TypeInfo ptrTI = lastExprInfo_;
    if (ptrTI.pointerDepth == 0) {
        error(node.loc, "சுட்டியல்லாத (non-pointer) வகையை குறிநீக்கம் (dereference) செய்ய முடியாது");
        return;
    }

    TypeInfo targetType = ptrTI;
    targetType.pointerDepth--;

    node.value->accept(*this);
    TypeInfo valType = lastExprInfo_;

    if (auto reason = isCompatible(targetType, valType); !reason.empty()) {
        error(node.loc, "குறிநீக்க ஒதுக்கீட்டில் (dereference assignment) வகை பொருந்தவில்லை: " + reason);
    }

    // ZPM Safety Check
    isZoneCompatible(valType.pulseTag, targetType.pulseTag, node.loc);
}

void TypeChecker::visit(NewExpr &node) {
    node.allocatedType = resolveType(node.allocatedType, node.loc);
    if (node.sizeExpr) {
        node.sizeExpr->accept(*this);
        if (lastExprInfo_.kind != TypeKind::Int &&
            lastExprInfo_.kind != TypeKind::Float) { // allow float for now?
                                                     // Error handled by lastExprInfo_
        }
        // It's a dynamic array allocation, so the result is a slice.
        // We use getElementTypeInfo() to ensure we preserve struct/generic info.
        TypeInfo elemTI = node.allocatedType.getElementTypeInfo();
        lastExprInfo_ = TypeInfo::slice(elemTI);
    } else {
        lastExprInfo_ = node.allocatedType;
        lastExprInfo_.pointerDepth++;
        lastExprInfo_.isMutable = true; // Owned allocation is always mutable
    }
    lastExprMutable_ = true;
}

bool TypeChecker::isNumeric(TypeKind t) const {
    return isIntegerType(t) || isFloatType(t);
}

std::string TypeChecker::isCompatible(TypeKind எதிர்பார்க்கிறோம், TypeKind actual) const {
    if (எதிர்பார்க்கிறோம் == actual)
        return "";
    if (isNumeric(எதிர்பார்க்கிறோம்) && isNumeric(actual))
        return "";
    if (எதிர்பார்க்கிறோம் == TypeKind::Unknown || actual == TypeKind::Unknown)
        return "";
    if (எதிர்பார்க்கிறோம் == TypeKind::Generic || actual == TypeKind::Generic)
        return "";
    return "எதிர்பார்க்கிறோம் (Expected) '" + std::string(typeKindToString(எதிர்பார்க்கிறோம்)) +
           "', ஆனால் கிடைத்தது (got) '" + std::string(typeKindToString(actual)) + "'";
}

std::string TypeChecker::isCompatible(const TypeInfo &எதிர்பார்க்கிறோம், const TypeInfo &actual) const {
    if (எதிர்பார்க்கிறோம் == actual)
        return "";

    // If either side involves a generic parameter, accept — concrete types
    // will be verified after monomorphization.
    if (எதிர்பார்க்கிறோம்.isGeneric || actual.isGeneric)
        return "";

    if (எதிர்பார்க்கிறோம்.isArray != actual.isArray)
        return "எதிர்பார்க்கிறோம் array, got non-array type";
    if (எதிர்பார்க்கிறோம்.isSlice != actual.isSlice)
        return "எதிர்பார்க்கிறோம் slice, got non-slice type";
    if (எதிர்பார்க்கிறோம்.pointerDepth != actual.pointerDepth) {
        return "pointer depth mismatch: எதிர்பார்க்கிறோம் " +
               std::to_string(எதிர்பார்க்கிறோம்.pointerDepth) + ", got " +
               std::to_string(actual.pointerDepth);
    }

    if (actual.kind == TypeKind::Null && எதிர்பார்க்கிறோம்.pointerDepth > 0)
        return "";
    if (எதிர்பார்க்கிறோம்.kind == TypeKind::Null && actual.pointerDepth > 0)
        return "";

    if (எதிர்பார்க்கிறோம்.isStruct != actual.isStruct) {
        return "எதிர்பார்க்கிறோம் struct, got non-struct type";
    }
    if (எதிர்பார்க்கிறோம்.isEnum != actual.isEnum)
        return "எதிர்பார்க்கிறோம் enum, got non-enum type";

    if (எதிர்பார்க்கிறோம்.isStruct) {
        std::string e_base = எதிர்பார்க்கிறோம்.structName;
        std::string a_base = actual.structName;
        size_t e_pos = e_base.find('_');
        size_t a_pos = a_base.find('_');
        if (e_pos != std::string::npos)
            e_base = e_base.substr(0, e_pos);
        if (a_pos != std::string::npos)
            a_base = a_base.substr(0, a_pos);

        if (e_base != a_base) {
            return "struct mismatch: '" + normalizeTypeName(எதிர்பார்க்கிறோம்.structName) + "' vs '" +
                   normalizeTypeName(actual.structName) + "'";
        }

        if (எதிர்பார்க்கிறோம்.genericArgs.size() != actual.genericArgs.size()) {
            if (எதிர்பார்க்கிறோம்.genericArgs.empty() || actual.genericArgs.empty())
                return ""; // Template vs Instance
            return "generic argument count mismatch for " + e_base;
        }
        for (size_t i = 0; i < எதிர்பார்க்கிறோம்.genericArgs.size(); ++i) {
            auto subReason = isCompatible(எதிர்பார்க்கிறோம்.genericArgs[i], actual.genericArgs[i]);
            if (!subReason.empty()) {
                return "generic argument mismatch at index " + std::to_string(i) + ": " + subReason;
            }
        }
        return "";
    }

    if (எதிர்பார்க்கிறோம்.isEnum) {
        if (எதிர்பார்க்கிறோம்.enumName != actual.enumName) {
            return "பட்டியல் வகை பொருந்தவில்லை (enum mismatch): '" + எதிர்பார்க்கிறோம்.enumName + "' vs '" +
                   actual.enumName + "'";
        }
        return "";
    }

    if (எதிர்பார்க்கிறோம்.isArray || எதிர்பார்க்கிறோம்.isSlice) {
        auto e_elem = எதிர்பார்க்கிறோம்.getElementTypeInfo();
        auto a_elem = actual.getElementTypeInfo();
        auto subReason = isCompatible(e_elem, a_elem);
        if (!subReason.empty()) {
            return "உறுப்பு வகை பொருந்தவில்லை (element type mismatch): " + subReason;
        }
        return "";
    }

    if (எதிர்பார்க்கிறோம்.kind != actual.kind) {
        if (isNumeric(எதிர்பார்க்கிறோம்.kind) && isNumeric(actual.kind))
            return "";
        return "வகை பொருந்தவில்லை (type mismatch): எதிர்பார்க்கிறோம் (Expected) '" +
               typeInfoToString(எதிர்பார்க்கிறோம்) + "', ஆனால் கிடைத்தது (got) '" +
               typeInfoToString(actual) + "'";
    }

    return "";
}

bool TypeChecker::expandPrintCall(CallExpr *call, std::vector<std::unique_ptr<Stmt>> &expanded) {
    if (call->callee != "print" && call->callee != "println" && call->callee != "பதிப்பி" &&
        call->callee != "வரியிறக்கி_பதிப்பி")
        return false;

    if ((call->callee == "println" || call->callee == "வரியிறக்கி_பதிப்பி") && call->args.empty()) {
        auto subCall = std::make_unique<CallExpr>("printf", std::vector<std::unique_ptr<Expr>>());
        subCall->args.push_back(std::make_unique<StringLiteralExpr>("%s"));
        subCall->args.push_back(std::make_unique<StringLiteralExpr>("\n"));
        subCall->loc = call->loc;
        auto subStmt = std::make_unique<ExprStmt>(std::move(subCall));
        subStmt->accept(*this);
        expanded.push_back(std::move(subStmt));
        return true;
    }

    if (call->args.empty())
        return false;

    if (call->args.size() > 1) {
        for (size_t i = 0; i < call->args.size(); ++i) {
            std::string target;
            if (i == call->args.size() - 1) {
                target = call->callee;
            } else {
                target = (call->callee == "பதிப்பி" || call->callee == "வரியிறக்கி_பதிப்பி") ? "பதிப்பி"
                                                                                          : "print";
            }
            std::vector<std::unique_ptr<Expr>> subArgs;
            subArgs.push_back(std::move(call->args[i]));
            auto subCall = std::make_unique<CallExpr>(target, std::move(subArgs));
            subCall->loc = call->loc;
            if (!expandPrintCall(subCall.get(), expanded)) {
                auto subStmt = std::make_unique<ExprStmt>(std::move(subCall));
                subStmt->accept(*this);
                expanded.push_back(std::move(subStmt));
            }
        }
        return true;
    }

    // 1. Prefer direct Printable trait methods if it exists (for both scalars and structs)
    call->args[0]->accept(*this);
    TypeInfo ti = lastExprInfo_;
    if (symbols_.findImplForMethod(ti, call->callee)) {
        auto mc = std::make_unique<MethodCallExpr>(std::move(call->args[0]), call->callee,
                                                   std::vector<std::unique_ptr<Expr>>());
        mc->loc = call->loc;
        auto ms = std::make_unique<ExprStmt>(std::move(mc));
        ms->accept(*this);
        expanded.push_back(std::move(ms));
        return true;
    }

    return expandStructuralPrinting(call->args[0].get(), call->callee, expanded);
}

bool TypeChecker::expandStructuralPrinting(Expr *receiver, const std::string &methodName,
                                           std::vector<std::unique_ptr<Stmt>> &expanded) {
    receiver->accept(*this);
    TypeInfo ti = lastExprInfo_;
    SourceLocation loc = receiver->loc;

    if (ti.isStruct) {
        auto sd = symbols_.lookupStruct(ti.structName);
        if (sd) {
            auto p1 = std::make_unique<CallExpr>("print", std::vector<std::unique_ptr<Expr>>());
            p1->args.push_back(std::make_unique<StringLiteralExpr>(ti.structName + " { "));
            p1->loc = loc;
            if (!expandPrintCall(p1.get(), expanded)) {
                auto s1 = std::make_unique<ExprStmt>(std::move(p1));
                s1->accept(*this);
                expanded.push_back(std::move(s1));
            }

            for (size_t j = 0; j < sd->fields.size(); ++j) {
                auto pf = std::make_unique<CallExpr>("print", std::vector<std::unique_ptr<Expr>>());
                pf->args.push_back(std::make_unique<StringLiteralExpr>(sd->fields[j].name + ": "));
                pf->loc = loc;
                if (!expandPrintCall(pf.get(), expanded)) {
                    auto sf = std::make_unique<ExprStmt>(std::move(pf));
                    sf->accept(*this);
                    expanded.push_back(std::move(sf));
                }

                auto acc = std::make_unique<FieldAccessExpr>(
                    std::unique_ptr<Expr>(static_cast<Expr *>(receiver->clone().release())),
                    sd->fields[j].name);
                acc->loc = loc;
                auto pv = std::make_unique<CallExpr>("print", std::vector<std::unique_ptr<Expr>>());
                pv->args.push_back(std::move(acc));
                pv->loc = loc;

                if (!expandPrintCall(pv.get(), expanded)) {
                    auto sv = std::make_unique<ExprStmt>(std::move(pv));
                    sv->accept(*this);
                    expanded.push_back(std::move(sv));
                }

                if (j < sd->fields.size() - 1) {
                    auto pc =
                        std::make_unique<CallExpr>("print", std::vector<std::unique_ptr<Expr>>());
                    pc->args.push_back(std::make_unique<StringLiteralExpr>(", "));
                    pc->loc = loc;
                    if (!expandPrintCall(pc.get(), expanded)) {
                        auto sc = std::make_unique<ExprStmt>(std::move(pc));
                        sc->accept(*this);
                        expanded.push_back(std::move(sc));
                    }
                }
            }

            auto p2 = std::make_unique<CallExpr>(methodName, std::vector<std::unique_ptr<Expr>>());
            p2->args.push_back(std::make_unique<StringLiteralExpr>(" }"));
            p2->loc = loc;
            if (!expandPrintCall(p2.get(), expanded)) {
                auto s2 = std::make_unique<ExprStmt>(std::move(p2));
                s2->accept(*this);
                expanded.push_back(std::move(s2));
            }
            return true;
        }
    }
    return false;
}

TypeInfo TypeChecker::substitute(const TypeInfo &type, const std::vector<std::string> &params,
                                 const std::vector<TypeInfo> &args) {
    TypeInfo result = type;
    if (result.isGeneric) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == result.genericName && i < args.size()) {
                TypeInfo sub = args[i];
                sub.isArray = result.isArray;
                sub.isSlice = result.isSlice;
                sub.pointerDepth += result.pointerDepth;
                sub.isMutable = result.isMutable;
                sub.pulseTag = result.pulseTag;
                if (result.isSlice || result.isArray) {
                    sub.elementType = args[i].kind;
                    sub.kind = TypeKind::Unknown;
                }
                return sub;
            }
        }
    }
    for (auto &arg : result.genericArgs) {
        arg = substitute(arg, params, args);
    }

    // If the struct itself is generic and its name matches a parameter (shouldn't happen for
    // structs, but just in case)
    if (result.isStruct) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == result.structName && i < args.size()) {
                // This is a rare case where a generic T is used as a struct name
                return args[i];
            }
        }
    }

    return result;
}

int TypeChecker::getZoneDepth(const std::string &tag) const {
    if (tag.empty())
        return -1; // Global/Eternal
    const auto &stack = symbols_.zoneStack();
    for (int i = 0; i < (int)stack.size(); ++i) {
        if (stack[i] == tag)
            return i;
    }
    return 9999; // Inactive or unknown zone
}

bool TypeChecker::isZoneCompatible(const std::string &srcTag, const std::string &tgtTag,
                                   const SourceLocation &loc) {
    if (srcTag == tgtTag)
        return true;

    int srcDepth = getZoneDepth(srcTag);
    int tgtDepth = getZoneDepth(tgtTag);

    // Source must live at least as long as target.
    // Smaller depth means outer/longer life. Global (-1) outlives everything.
    if (srcDepth <= tgtDepth) {
        return true;
    }

    std::string srcDesc = srcTag.empty() ? "Global" : "~" + srcTag;
    std::string tgtDesc = tgtTag.empty() ? "Global" : "~" + tgtTag;
    error(loc, "நினைவக பாதுகாப்பு மீறல் (Memory safety violation): குறுகிய கால மண்டலமான '" + srcDesc +
                   "' இலிருந்து நீண்ட கால மண்டலமான '" + tgtDesc + "' க்கு சுட்டியைச் சேமிக்க முடியாது");
    return false;
}

bool TypeChecker::canMutate(const std::string &name, const SourceLocation &loc) {
    auto info = symbols_.lookup(name);
    if (!info)
        return true;
    if (info->mutablyBorrowed || info->sharedBorrowCount > 0) {
        error(loc, "கடன் வாங்கப்பட்ட மாறி (borrowed variable) '" + name + "' ஐ மாற்ற முடியாது");
        return false;
    }
    return true;
}

bool TypeChecker::canRead(const std::string &name, const SourceLocation &loc) {
    auto info = symbols_.lookup(name);
    if (!info)
        return true;
    if (info->mutablyBorrowed) {
        error(loc,
              "மாறி '" + name +
                  "' மாற்றக்கூடிய வகையில் (mutably) கடன் வாங்கப்பட்டிருக்கும் போது அதனைப் படிக்க முடியாது");
        return false;
    }
    return true;
}

std::string TypeChecker::getRootVariable(Expr *expr) {
    Expr *current = expr;
    while (current) {
        if (auto *var = dynamic_cast<VariableExpr *>(current)) {
            return var->name;
        } else if (auto *fa = dynamic_cast<FieldAccessExpr *>(current)) {
            current = fa->base.get();
        } else if (auto *ua = dynamic_cast<UnaryExpr *>(current)) {
            if (ua->op == UnaryOp::Dereference || ua->op == UnaryOp::AddressOf) {
                current = ua->operand.get();
            } else
                break;
        } else if (auto *ie = dynamic_cast<IndexExpr *>(current)) {
            current = ie->base.get();
        } else {
            break;
        }
    }
    return "";
}

} // namespace agam
