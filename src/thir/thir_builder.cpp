#include "agam/thir/thir_builder.h"

#include <iostream>
#include <sstream>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════════

bool ThirBuilder::isNumeric(TypeKind t) const {
    return isNumericType(t);
}

bool ThirBuilder::isComparison(BinaryOp op) const {
    return op == BinaryOp::Eq || op == BinaryOp::Neq || op == BinaryOp::Lt || op == BinaryOp::Gt ||
           op == BinaryOp::Lte || op == BinaryOp::Gte;
}

bool ThirBuilder::isLogical(BinaryOp op) const {
    return op == BinaryOp::And || op == BinaryOp::Or;
}

void ThirBuilder::error(const SourceLocation &loc, const std::string &msg) {
    if (diag_)
        diag_->error(loc, msg);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Top-Level Build
// ═══════════════════════════════════════════════════════════════════════════════

TypeInfo ThirBuilder::resolveTypeInfo(const TypeInfo &ti) {
    TypeInfo resolved = ti;
    if (resolved.isStruct && !resolved.isEnum) {
        if (enumDefs_.count(resolved.structName)) {
            resolved.isEnum = true;
            resolved.isStruct = false;
            resolved.enumName = resolved.structName;
            resolved.structName = "";
        }
    }
    return resolved;
}

std::unique_ptr<ThirProgram> ThirBuilder::build(HirProgram &program, DiagnosticEngine &diag) {
    diag_ = &diag;
    typeMap_.clear();
    auto thirProg = std::make_unique<ThirProgram>();

    thirProg->structs = program.structs;
    thirProg->enums = program.enums;

    enumDefs_.clear();
    for (auto &ed : thirProg->enums) {
        enumDefs_[ed.name] = &ed;
    }

    structDefs_.clear();
    for (auto &sd : thirProg->structs) {
        for (auto &f : sd.fields) {
            f.typeInfo = resolveTypeInfo(f.typeInfo);
        }
        structDefs_[sd.name] = &sd;
    }

    for (auto &ed : thirProg->enums) {
        for (auto &v : ed.variants) {
            if (v.hasPayload)
                v.payloadType = resolveTypeInfo(v.payloadType);
        }
    }

    // First pass: register all function signatures.
    for (auto &fn : program.functions) {
        FuncInfo info;
        info.returnTypeInfo = resolveTypeInfo(fn->returnTypeInfo);
        for (auto &p : fn->params) {
            info.paramTypes.push_back(resolveTypeInfo(p.typeInfo));
        }
        funcMap_[fn->id] = info;
    }

    for (auto &cn : program.constants) {
        cn->typeInfo = resolveTypeInfo(cn->typeInfo);
        typeMap_[cn->id] = cn->typeInfo;
    }

    // Second pass: type-check each function body.
    for (auto &fn : program.functions) {
        thirProg->functions.push_back(lowerFunc(*fn));
    }

    for (auto &cn : program.constants) {
        auto tcn = std::make_unique<ThirConstDef>();
        tcn->id = cn->id;
        tcn->name = cn->name;
        tcn->typeInfo = cn->typeInfo;
        if (cn->value) {
            tcn->value = lowerExpr(*cn->value);
        }
        thirProg->constants.push_back(std::move(tcn));
    }

    return thirProg;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Function Lowering
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<ThirFuncDecl> ThirBuilder::lowerFunc(HirFuncDecl &node) {
    node.returnTypeInfo = resolveTypeInfo(node.returnTypeInfo);
    currentReturnTypeInfo_ = node.returnTypeInfo;

    // Register params in type map.
    std::vector<ThirParam> thirParams;
    for (auto &p : node.params) {
        p.typeInfo = resolveTypeInfo(p.typeInfo);
        typeMap_[p.id] = p.typeInfo;
        thirParams.push_back({p.id, p.name, p.typeInfo});
    }

    std::unique_ptr<ThirBlock> body = nullptr;
    if (node.body) {
        body = lowerBlock(*node.body);
    }

    return std::make_unique<ThirFuncDecl>(node.id, node.name, std::move(thirParams),
                                          node.returnTypeInfo, std::move(body), node.isExtern);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Block / Statement Lowering
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<ThirBlock> ThirBuilder::lowerBlock(HirBlock &node) {
    std::vector<std::unique_ptr<ThirStmt>> stmts;
    for (auto &s : node.stmts) {
        auto thir = lowerStmt(*s);
        if (thir)
            stmts.push_back(std::move(thir));
    }
    return std::make_unique<ThirBlock>(std::move(stmts));
}

std::unique_ptr<ThirStmt> ThirBuilder::lowerStmt(HirStmt &node) {
    // VarDecl
    if (auto *vd = dynamic_cast<HirVarDecl *>(&node)) {
        vd->typeInfo = resolveTypeInfo(vd->typeInfo);
        typeMap_[vd->id] = vd->typeInfo;
        std::unique_ptr<ThirExpr> init;
        if (vd->initializer) {
            init = lowerExpr(*vd->initializer);
            // Check type compatibility
            if (init && init->typeInfo.kind != TypeKind::Unknown &&
                init->typeInfo != vd->typeInfo) {
                // Allow numeric conversions (int↔int, int→float, float32→float64)
                if (!vd->typeInfo.isArray && isNumericType(vd->typeInfo.kind) &&
                    isNumericType(init->typeInfo.kind)) {
                    TypeInfo originalType = init->typeInfo;
                    init =
                        std::make_unique<ThirCastExpr>(std::move(init), originalType, vd->typeInfo);
                }
                // Array element type mismatch: cast each element
                else if (vd->typeInfo.isArray && init->typeInfo.isArray &&
                         vd->typeInfo.elementType != init->typeInfo.elementType &&
                         isNumericType(vd->typeInfo.elementType) &&
                         isNumericType(init->typeInfo.elementType)) {
                    auto *arrLit = dynamic_cast<ThirArrayLiteral *>(init.get());
                    if (arrLit) {
                        TypeInfo fromElem = TypeInfo::scalar(init->typeInfo.elementType);
                        TypeInfo toElem = TypeInfo::scalar(vd->typeInfo.elementType);
                        for (auto &elem : arrLit->elements) {
                            elem =
                                std::make_unique<ThirCastExpr>(std::move(elem), fromElem, toElem);
                        }
                        arrLit->typeInfo = vd->typeInfo;
                    }
                } else if (init->typeInfo != vd->typeInfo) {
                    error(node.loc, "மாறி (Variable) '" + vd->name +
                                        "' ஐத் தொடங்க முடியாது: வகை பொருந்தவில்லை (type mismatch): "
                                        "எதிர்பார்க்கிறோம் (Expected) '" +
                                        typeInfoToString(vd->typeInfo) +
                                        "', ஆனால் கிடைத்தது (got) '" +
                                        typeInfoToString(init->typeInfo) + "'");
                }
            }
        }
        auto stmt = std::make_unique<ThirVarDecl>(vd->id, vd->name, vd->typeInfo, std::move(init));
        stmt->loc = node.loc;
        return stmt;
    }

    // Return
    if (auto *ret = dynamic_cast<HirReturn *>(&node)) {
        std::unique_ptr<ThirExpr> val;
        if (ret->value) {
            val = lowerExpr(*ret->value);
            if (val->typeInfo != currentReturnTypeInfo_ &&
                val->typeInfo.kind != TypeKind::Unknown) {
                if (isNumericType(currentReturnTypeInfo_.kind) &&
                    isNumericType(val->typeInfo.kind)) {
                    TypeInfo originalType = val->typeInfo;
                    val = std::make_unique<ThirCastExpr>(std::move(val), originalType,
                                                         currentReturnTypeInfo_);
                } else {
                    error(node.loc, "திரும்பும் வகை பொருந்தவில்லை (Return type mismatch): "
                                    "எதிர்பார்க்கிறோம் (Expected) '" +
                                        std::string(typeKindToString(currentReturnTypeInfo_.kind)) +
                                        "', ஆனால் கிடைத்தது (got) '" +
                                        typeKindToString(val->typeInfo.kind) + "'");
                }
            }
        }
        auto stmt = std::make_unique<ThirReturn>(std::move(val));
        stmt->loc = node.loc;
        return stmt;
    }

    // If
    if (auto *ifS = dynamic_cast<HirIf *>(&node)) {
        auto cond = lowerExpr(*ifS->condition);
        auto thenB = lowerBlock(*ifS->thenBranch);
        std::unique_ptr<ThirBlock> elseB;
        if (ifS->elseBranch) {
            elseB = lowerBlock(*ifS->elseBranch);
        }
        auto stmt = std::make_unique<ThirIf>(std::move(cond), std::move(thenB), std::move(elseB));
        stmt->loc = node.loc;
        return stmt;
    }

    // While
    if (auto *whileS = dynamic_cast<HirWhile *>(&node)) {
        auto cond = lowerExpr(*whileS->condition);
        auto body = lowerBlock(*whileS->body);
        auto stmt = std::make_unique<ThirWhile>(std::move(cond), std::move(body));
        stmt->loc = node.loc;
        return stmt;
    }

    // HirFor
    if (auto *hf = dynamic_cast<HirFor *>(&node)) {
        auto iter = lowerExpr(*hf->iterable);
        TypeInfo elemType;
        if (iter->typeInfo.isArray || iter->typeInfo.isSlice) {
            elemType = TypeInfo::scalar(iter->typeInfo.elementType);
        } else {
            // Integer range
            elemType = iter->typeInfo;
        }
        typeMap_[hf->varId] = elemType;

        auto body = lowerBlock(*hf->body);
        auto stmt = std::make_unique<ThirFor>(hf->varId, hf->varName, elemType, std::move(iter),
                                              std::move(body));
        stmt->loc = node.loc;
        return stmt;
    }

    // Block
    if (auto *block = dynamic_cast<HirBlock *>(&node)) {
        return lowerBlock(*block);
    }

    // DeleteStmt
    if (auto *delS = dynamic_cast<HirDeleteStmt *>(&node)) {
        auto pointer = lowerExpr(*delS->pointer);
        if (pointer && pointer->typeInfo.pointerDepth == 0) {
            error(node.loc,
                  "'நீக்கு' (delete) செயல்முறைக்கு ஒரு சுட்டி (pointer) தேவை, ஆனால் கிடைத்தது '" +
                      typeInfoToString(pointer->typeInfo) + "'");
        }
        auto stmt = std::make_unique<ThirDeleteStmt>(std::move(pointer));
        stmt->loc = node.loc;
        return stmt;
    }

    // ExprStmt
    if (auto *exprS = dynamic_cast<HirExprStmt *>(&node)) {
        auto expr = lowerExpr(*exprS->expr);
        auto stmt = std::make_unique<ThirExprStmt>(std::move(expr));
        stmt->loc = node.loc;
        return stmt;
    }

    error(node.loc, "THIR மாற்றத்தில் அறியப்படாத கூற்று (unknown statement)");
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expression Lowering (type checking happens here)
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<ThirExpr> ThirBuilder::lowerExpr(HirExpr &node) {
    // Literals — type is known
    if (auto *lit = dynamic_cast<HirIntLiteral *>(&node)) {
        auto e = std::make_unique<ThirIntLiteral>((int64_t)lit->value);
        e->loc = node.loc;
        return e;
    }
    if (auto *lit = dynamic_cast<HirFloatLiteral *>(&node)) {
        auto e = std::make_unique<ThirFloatLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }
    if (auto *lit = dynamic_cast<HirStringLiteral *>(&node)) {
        auto e = std::make_unique<ThirStringLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }
    if (auto *lit = dynamic_cast<HirBoolLiteral *>(&node)) {
        auto e = std::make_unique<ThirBoolLiteral>(lit->value);
        e->loc = node.loc;
        return e;
    }
    if (dynamic_cast<HirNullLiteral *>(&node)) {
        auto e = std::make_unique<ThirNullLiteral>();
        e->loc = node.loc;
        // nil is compatible with any pointer
        e->typeInfo.pointerDepth = 1;
        return e;
    }

    // Variable reference — look up type from typeMap
    if (auto *var = dynamic_cast<HirVarRef *>(&node)) {
        TypeInfo t = TypeInfo::scalar(TypeKind::Unknown);
        auto it = typeMap_.find(var->defId);
        if (it != typeMap_.end()) {
            t = it->second;
        } else {
            error(node.loc, "'" + var->name + "' மாறியின் வகை அறியப்படவில்லை");
        }
        auto e = std::make_unique<ThirVarRef>(var->defId, var->name, t);
        e->loc = node.loc;
        return e;
    }

    // Binary expression — type check operands
    if (auto *bin = dynamic_cast<HirBinaryExpr *>(&node)) {
        auto lhs = lowerExpr(*bin->lhs);
        auto rhs = lowerExpr(*bin->rhs);
        if (!lhs || !rhs)
            return nullptr;
        TypeInfo lt = lhs->typeInfo, rt = rhs->typeInfo;
        TypeInfo resultType = TypeInfo::scalar(TypeKind::Unknown);

        if (isComparison(bin->op)) {
            // Comparisons produce bool
            if (isNumericType(lt.kind) && isNumericType(rt.kind)) {
                // Promote if mixed
                if (lt != rt) {
                    TypeKind common =
                        isFloatType(lt.kind) || isFloatType(rt.kind)
                            ? TypeKind::Float64
                            : (typeSizeInBits(lt.kind) >= typeSizeInBits(rt.kind) ? lt.kind
                                                                                  : rt.kind);
                    if (lt.kind != common)
                        lhs = std::make_unique<ThirCastExpr>(std::move(lhs), lt,
                                                             TypeInfo::scalar(common));
                    if (rt.kind != common)
                        rhs = std::make_unique<ThirCastExpr>(std::move(rhs), rt,
                                                             TypeInfo::scalar(common));
                }
                resultType = TypeInfo::scalar(TypeKind::Bool);
            } else if (lt == rt) {
                resultType = TypeInfo::scalar(TypeKind::Bool);
            } else {
                error(node.loc, std::string("வகை பொருந்தவில்லை (type mismatch): '") +
                                    typeKindToString(lt.kind) + "' மற்றும் '" +
                                    typeKindToString(rt.kind) + "' ஆகியவற்றை ஒப்பிட முடியாது");
            }
        } else if (isLogical(bin->op)) {
            // Logical ops require bool operands, produce bool
            if (lt.kind == TypeKind::Bool && rt.kind == TypeKind::Bool) {
                resultType = TypeInfo::scalar(TypeKind::Bool);
            } else {
                error(node.loc,
                      "தருக்க செயலிகளுக்கு (logical operators) மெய்மை (bool) வகை இயக்கிகள் தேவை");
            }
        } else {
            // Arithmetic: +, -, *, /
            if (isNumericType(lt.kind) && isNumericType(rt.kind)) {
                if (isFloatType(lt.kind) || isFloatType(rt.kind)) {
                    // If either is float, promote to the wider float
                    resultType = TypeInfo::scalar(
                        (lt.kind == TypeKind::Float64 || rt.kind == TypeKind::Float64)
                            ? TypeKind::Float64
                            : TypeKind::Float32);
                    if (lt != resultType)
                        lhs = std::make_unique<ThirCastExpr>(std::move(lhs), lt, resultType);
                    if (rt != resultType)
                        rhs = std::make_unique<ThirCastExpr>(std::move(rhs), rt, resultType);
                } else {
                    // Both integers — use the wider type
                    resultType = TypeInfo::scalar(
                        typeSizeInBits(lt.kind) >= typeSizeInBits(rt.kind) ? lt.kind : rt.kind);
                    if (lt != resultType)
                        lhs = std::make_unique<ThirCastExpr>(std::move(lhs), lt, resultType);
                    if (rt != resultType)
                        rhs = std::make_unique<ThirCastExpr>(std::move(rhs), rt, resultType);
                }
            } else if (bin->op == BinaryOp::Add && lt.kind == TypeKind::String &&
                       rt.kind == TypeKind::String) {
                resultType = TypeInfo::scalar(TypeKind::String);
            } else {
                error(node.loc, std::string("இருமை செயலி (binary operator) '") +
                                    binaryOpToString(bin->op) + "' -க்கு பொருந்தாத இயக்கிகள்: '" +
                                    typeKindToString(lt.kind) + "' மற்றும் '" +
                                    typeKindToString(rt.kind) + "'");
            }
        }

        auto e =
            std::make_unique<ThirBinaryExpr>(bin->op, std::move(lhs), std::move(rhs), resultType);
        e->loc = node.loc;
        return e;
    }

    // Unary expression
    if (auto *un = dynamic_cast<HirUnaryExpr *>(&node)) {
        auto operand = lowerExpr(*un->operand);
        if (!operand)
            return nullptr;
        TypeInfo t = operand->typeInfo;

        if (un->op == UnaryOp::AddressOf) {
            if (t.isArray) {
                t = TypeInfo::slice(t.elementType);
            } else {
                t.pointerDepth++;
            }
        } else if (un->op == UnaryOp::Dereference) {
            if (t.pointerDepth == 0) {
                error(node.loc, "சுட்டி (pointer) அல்லாத வகை '" + typeInfoToString(t) +
                                    "' -ஐ குறிநீக்கம் (dereference) செய்ய முடியாது");
            } else {
                t.pointerDepth--;
            }
        } else if (un->op == UnaryOp::Negate && !isNumericType(t.kind)) {
            error(node.loc, "எண் அல்லாத வகை '" + std::string(typeKindToString(t.kind)) +
                                "' -ஐ எதிர்மறையாக்க (negate) முடியாது");
        } else if (un->op == UnaryOp::Not && t.kind != TypeKind::Bool) {
            error(node.loc, "தருக்க எதிர்மறைக்கு (logical NOT) மெய்மை (bool) வகை இயக்கி தேவை");
        }

        auto e = std::make_unique<ThirUnaryExpr>(un->op, std::move(operand), t);
        e->loc = node.loc;
        return e;
    }

    // Call expression — get return type from funcMap
    if (auto *call = dynamic_cast<HirCallExpr *>(&node)) {
        if (call->calleeName == "__len") {
            auto base = lowerExpr(*call->args[0]);
            if (!base)
                return nullptr;
            // String: call strlen at runtime
            if (base->typeInfo.kind == TypeKind::String) {
                auto e = std::make_unique<ThirStringLen>(std::move(base));
                e->loc = node.loc;
                return e;
            }
            // Fixed-size array: compile-time constant
            if (base->typeInfo.isArray) {
                auto e = std::make_unique<ThirIntLiteral>(base->typeInfo.arraySize);
                e->typeInfo = TypeInfo::scalar(TypeKind::Int);
                e->loc = node.loc;
                return e;
            }
            // Slice: extract len from fat pointer
            auto e = std::make_unique<ThirSliceLen>(std::move(base));
            e->loc = node.loc;
            return e;
        }

        TypeInfo retTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
        auto it = funcMap_.find(call->calleeId);
        if (it != funcMap_.end()) {
            retTypeInfo = it->second.returnTypeInfo;
            // Check arg count
            if (call->args.size() != it->second.paramTypes.size()) {
                error(node.loc, std::string("செயல்முறை (function) '") + call->calleeName +
                                    "' எதிர்பார்க்கும் அளபுருக்கள் " +
                                    std::to_string(it->second.paramTypes.size()) +
                                    ", ஆனால் கிடைத்துள்ளது " + std::to_string(call->args.size()));
            }
        }

        std::vector<std::unique_ptr<ThirExpr>> thirArgs;
        for (size_t i = 0; i < call->args.size(); i++) {
            auto arg = lowerExpr(*call->args[i]);
            if (!arg)
                return nullptr;
            // Check arg type if we have func info
            if (it != funcMap_.end() && i < it->second.paramTypes.size()) {
                TypeInfo expected = it->second.paramTypes[i];

                // Auto-take address if expected is a pointer and we have a value
                if (expected.pointerDepth == arg->typeInfo.pointerDepth + 1) {
                    TypeInfo addrType = arg->typeInfo;
                    addrType.pointerDepth++;
                    // Only auto-address if kinds match (to avoid addressing random things)
                    if (expected.kind == addrType.kind &&
                        expected.structName == addrType.structName) {
                        auto loc = arg->loc;
                        arg = std::make_unique<ThirUnaryExpr>(UnaryOp::AddressOf, std::move(arg),
                                                              expected);
                        arg->loc = loc;
                    }
                }

                if (arg->typeInfo != expected && arg->typeInfo.kind != TypeKind::Unknown) {
                    if (isNumericType(expected.kind) && isNumericType(arg->typeInfo.kind)) {
                        TypeInfo originalType = arg->typeInfo;
                        arg =
                            std::make_unique<ThirCastExpr>(std::move(arg), originalType, expected);
                    } else {
                        error(node.loc, std::string("செயல்முறை '") + call->calleeName +
                                            "' -ன் அளபுரு " + std::to_string(i + 1) + ": '" +
                                            typeKindToString(expected.kind) +
                                            "' எதிர்பார்க்கிறோம், ஆனால் கிடைத்துள்ளது '" +
                                            typeKindToString(arg->typeInfo.kind) + "'");
                    }
                }
            }
            thirArgs.push_back(std::move(arg));
        }

        auto e = std::make_unique<ThirCallExpr>(call->calleeId, call->calleeName,
                                                std::move(thirArgs), retTypeInfo);
        e->loc = node.loc;
        return e;
    }

    // Assign expression
    if (auto *assign = dynamic_cast<HirAssignExpr *>(&node)) {
        auto val = lowerExpr(*assign->value);
        if (!val)
            return nullptr;
        TypeInfo targetType = TypeInfo::scalar(TypeKind::Unknown);
        auto it = typeMap_.find(assign->targetId);
        if (it != typeMap_.end()) {
            targetType = it->second;
            if (val->typeInfo != targetType && val->typeInfo.kind != TypeKind::Unknown) {
                if (isNumericType(targetType.kind) && isNumericType(val->typeInfo.kind)) {
                    TypeInfo originalType = val->typeInfo;
                    val = std::make_unique<ThirCastExpr>(std::move(val), originalType, targetType);
                } else {
                    error(node.loc, std::string("மாறியின் வகை '") +
                                        typeKindToString(targetType.kind) + "' -க்கு '" +
                                        typeKindToString(val->typeInfo.kind) +
                                        "' -ஐ ஒதுக்கீடு செய்ய முடியாது");
                }
            }
        }
        auto e = std::make_unique<ThirAssignExpr>(assign->targetId, assign->targetName,
                                                  std::move(val), targetType);
        e->loc = node.loc;
        return e;
    }

    // ArrayLiteralExpr
    if (auto *arr = dynamic_cast<HirArrayLiteral *>(&node)) {
        std::vector<std::unique_ptr<ThirExpr>> thirElems;
        TypeKind elemKind = TypeKind::Unknown;
        if (!arr->elements.empty()) {
            // Check first element
            thirElems.push_back(lowerExpr(*arr->elements[0]));
            elemKind = thirElems[0]->typeInfo.kind;
            // Iterate remaining
            for (size_t i = 1; i < arr->elements.size(); i++) {
                auto elem = lowerExpr(*arr->elements[i]);
                if (!elem)
                    return nullptr;
                if (elem->typeInfo.kind != elemKind) {
                    error(node.loc,
                          std::string("அணி உறுப்புகள் ஒரே வகையாக இருக்க வேண்டும். எதிர்பார்க்கிறோம் '") +
                              typeKindToString(elemKind) + "', ஆனால் கிடைத்தது '" +
                              typeKindToString(elem->typeInfo.kind) + "'");
                }
                thirElems.push_back(std::move(elem));
            }
        }
        TypeInfo arrType = TypeInfo::array(elemKind, thirElems.size());
        arrType.arraySize = thirElems.size();
        auto e = std::make_unique<ThirArrayLiteral>(std::move(thirElems), arrType);
        e->loc = node.loc;
        return e;
    }

    // IndexExpr
    if (auto *idx = dynamic_cast<HirIndexExpr *>(&node)) {
        auto base = lowerExpr(*idx->base);
        auto index = lowerExpr(*idx->index);
        if (!base || !index)
            return nullptr;
        std::unique_ptr<ThirExpr> endIndex = nullptr;
        if (idx->endIndex) {
            endIndex = lowerExpr(*idx->endIndex);
        }

        TypeInfo resType;
        if (idx->isRange) {
            resType = TypeInfo::slice(base->typeInfo.elementType);
        } else {
            if (base->typeInfo.pointerDepth > 0) {
                resType = base->typeInfo;
                resType.pointerDepth--;
            } else {
                resType = TypeInfo::scalar(base->typeInfo.elementType);
                resType.isStruct = base->typeInfo.isStruct;
                resType.structName = base->typeInfo.structName;
                resType.isEnum = base->typeInfo.isEnum;
                resType.enumName = base->typeInfo.enumName;
                resType.isGeneric = base->typeInfo.isGeneric;
                resType.genericName = base->typeInfo.genericName;
            }
        }

        auto e = std::make_unique<ThirIndexExpr>(std::move(base), std::move(index), resType,
                                                 std::move(endIndex), idx->isRange);
        e->loc = node.loc;
        return e;
    }

    // IndexAssign
    if (auto *idxAssign = dynamic_cast<HirIndexAssign *>(&node)) {
        auto base = lowerExpr(*idxAssign->base);
        auto index = lowerExpr(*idxAssign->index);
        auto val = lowerExpr(*idxAssign->value);

        if (!base || !index || !val)
            return nullptr;

        if (!base->typeInfo.isArray && !base->typeInfo.isSlice &&
            base->typeInfo.pointerDepth == 0) {
            error(node.loc, "அணி அல்லது சுட்டி அல்லாத வகையில் குறியீட்டை (index) பயன்படுத்த முடியாது");
            return nullptr;
        }

        TypeInfo elemType;
        if (base->typeInfo.pointerDepth > 0) {
            elemType = base->typeInfo;
            elemType.pointerDepth--;
        } else {
            elemType = TypeInfo::scalar(base->typeInfo.elementType);
            elemType.isStruct = base->typeInfo.isStruct;
            elemType.structName = base->typeInfo.structName;
            elemType.isGeneric = base->typeInfo.isGeneric;
            elemType.genericName = base->typeInfo.genericName;
        }

        if (val->typeInfo.kind != elemType.kind && val->typeInfo.kind != TypeKind::Unknown) {
            error(node.loc, "குறியீட்டு மதிப்பிற்கு ஒதுக்கீடு செய்ய முடியாது (வகை பொருந்தவில்லை)");
        }

        auto e = std::make_unique<ThirIndexAssign>(std::move(base), std::move(index),
                                                   std::move(val), elemType);
        e->loc = node.loc;
        return e;
    }

    // HirStructLiteral
    if (auto *slit = dynamic_cast<HirStructLiteral *>(&node)) {
        auto it = structDefs_.find(slit->structName);
        if (it == structDefs_.end()) {
            error(node.loc,
                  std::string("அறியப்படாத அமைப்பு (struct) வகை '") + slit->structName + "'");
            return nullptr;
        }
        const HirStructDef *sdef = it->second;
        std::vector<ThirStructFieldInit> tfields;
        for (auto &f : slit->fields) {
            int idx = sdef->fieldIndex(f.name);
            if (idx < 0) {
                error(node.loc, std::string("அமைப்பு '") + slit->structName + "' -ல் '" + f.name +
                                    "' என்ற புலம் (field) இல்லை");
                idx = 0;
            }
            auto val = lowerExpr(*f.value);
            if (!val) {
                tfields.push_back({f.name, -1, nullptr});
                continue;
            }
            // Type-check field value against declared type.
            const TypeInfo &declTI = sdef->fields[idx].typeInfo;
            if (!declTI.isStruct && !declTI.isArray && val->typeInfo.kind != declTI.kind &&
                val->typeInfo.kind != TypeKind::Unknown) {
                if (isNumericType(declTI.kind) && isNumericType(val->typeInfo.kind)) {
                    TypeInfo originalType = val->typeInfo;
                    val = std::make_unique<ThirCastExpr>(std::move(val), originalType, declTI);
                } else {
                    error(node.loc, std::string("புலம் (field) '") + f.name + "' '" +
                                        typeInfoToString(declTI) +
                                        "' -ஐ எதிர்பார்க்கிறது, ஆனால் கிடைத்துள்ளது '" +
                                        typeInfoToString(val->typeInfo) + "'");
                }
            }
            ThirStructFieldInit tfi;
            tfi.name = f.name;
            tfi.fieldIndex = idx;
            tfi.value = std::move(val);
            tfields.push_back(std::move(tfi));
        }
        TypeInfo stype = TypeInfo::namedStruct(slit->structName);
        auto e = std::make_unique<ThirStructLiteral>(slit->structName, std::move(tfields), stype);
        e->loc = node.loc;
        return e;
    }

    // HirFieldAccess
    if (auto *fa = dynamic_cast<HirFieldAccess *>(&node)) {

        auto base = lowerExpr(*fa->base);
        if (!base)
            return nullptr;

        // Special case: .len and .data on arrays and slices
        if (base->typeInfo.isSlice && fa->field == "data") {
            TypeInfo ptrType = TypeInfo::scalar(base->typeInfo.elementType);
            ptrType.pointerDepth = 1;
            auto e = std::make_unique<ThirFieldAccess>(std::move(base), "data", 0, ptrType);
            e->loc = node.loc;
            return e;
        }
        if ((base->typeInfo.isArray || base->typeInfo.isSlice) && fa->field == "len") {
            int idx = base->typeInfo.isSlice ? 1 : -1;
            auto e = std::make_unique<ThirFieldAccess>(std::move(base), "len", idx,
                                                       TypeInfo::scalar(TypeKind::Int));
            e->loc = node.loc;
            return e;
        }

        if (!base->typeInfo.isStruct) {
            error(node.loc, "அமைப்பு (struct) அல்லாத வகை '" + typeInfoToString(base->typeInfo) +
                                "' -ல் புலத்தை அணுக முடியாது");
            return nullptr;
        }
        auto it = structDefs_.find(base->typeInfo.structName);
        if (it == structDefs_.end()) {
            error(node.loc, "unknown struct type '" + base->typeInfo.structName + "'");
            return nullptr;
        }
        const HirStructDef *sdef = it->second;
        int idx = sdef->fieldIndex(fa->field);
        if (idx < 0) {
            error(node.loc, std::string("அமைப்பு '") + base->typeInfo.structName + "' -ல் '" +
                                fa->field + "' என்ற புலம் இல்லை");
            idx = 0;
        }
        TypeInfo fieldType = sdef->fields[idx].typeInfo;
        auto e = std::make_unique<ThirFieldAccess>(std::move(base), fa->field, idx, fieldType);
        e->loc = node.loc;
        return e;
    }

    // HirFieldAssign
    if (auto *fassign = dynamic_cast<HirFieldAssign *>(&node)) {
        auto base = lowerExpr(*fassign->base);
        if (!base)
            return nullptr;
        if (!base->typeInfo.isStruct) {
            error(node.loc, "அமைப்பு (struct) அல்லாத வகை '" + typeInfoToString(base->typeInfo) +
                                "' -ல் புலத்திற்கு மதிப்பை ஒதுக்க முடியாது");
            return nullptr;
        }
        auto it = structDefs_.find(base->typeInfo.structName);
        if (it == structDefs_.end()) {
            error(node.loc, "unknown struct type '" + base->typeInfo.structName + "'");
            return nullptr;
        }
        const HirStructDef *sdef = it->second;
        int idx = sdef->fieldIndex(fassign->field);
        if (idx < 0) {
            error(node.loc, std::string("அமைப்பு '") + base->typeInfo.structName + "' -ல் '" +
                                fassign->field + "' என்ற புலம் இல்லை");
            idx = 0;
        }
        TypeInfo fieldType = sdef->fields[idx].typeInfo;
        auto val = lowerExpr(*fassign->value);
        if (!fieldType.isStruct && !fieldType.isArray && val->typeInfo.kind != fieldType.kind &&
            val->typeInfo.kind != TypeKind::Unknown) {
            if (isNumericType(fieldType.kind) && isNumericType(val->typeInfo.kind)) {
                TypeInfo originalType = val->typeInfo;
                val = std::make_unique<ThirCastExpr>(std::move(val), originalType, fieldType);
            } else {
                error(node.loc, std::string("புலத்தின் வகை '") + typeInfoToString(fieldType) +
                                    "' -க்கு '" + typeInfoToString(val->typeInfo) +
                                    "' -ஐ ஒதுக்கீடு செய்ய முடியாது (" + fassign->field + ")");
            }
        }
        auto e =
            std::make_unique<ThirFieldAssign>(std::move(base), fassign->field, idx, std::move(val));
        e->loc = node.loc;
        return e;
    }

    // HirDerefAssign -> ThirDerefAssign
    if (auto *da = dynamic_cast<HirDerefAssign *>(&node)) {
        auto pointer = lowerExpr(*da->pointer);
        auto val = lowerExpr(*da->value);
        if (!pointer || !val)
            return nullptr;

        TypeInfo ptrType = pointer->typeInfo;
        if (ptrType.pointerDepth == 0) {
            error(node.loc, "ஒதுக்கீட்டில் சுட்டி (pointer) அல்லாத வகையை குறிநீக்க (dereference) முடியாது");
        }

        TypeInfo expectedValType = ptrType;
        expectedValType.pointerDepth--;

        if (val->typeInfo != expectedValType && val->typeInfo.kind != TypeKind::Unknown &&
            expectedValType.kind != TypeKind::Unknown) {
            // Allow numeric coercion (e.g. int64 literal assigned to *int32 target)
            if (expectedValType != val->typeInfo && isNumericType(expectedValType.kind) &&
                isNumericType(val->typeInfo.kind)) {
                TypeInfo originalType = val->typeInfo;
                val = std::make_unique<ThirCastExpr>(std::move(val), originalType, expectedValType);
            } else {
                error(node.loc, std::string("குறிநீக்க ஒதுக்கீட்டில் வகை பொருந்தவில்லை. எதிர்பார்க்கிறோம் '") +
                                    typeInfoToString(expectedValType) + "', ஆனால் கிடைத்தது '" +
                                    typeInfoToString(val->typeInfo) + "'");
            }
        }

        auto e = std::make_unique<ThirDerefAssign>(std::move(pointer), std::move(val));
        e->loc = node.loc;
        return e;
    }

    // HirEnumVariantExpr
    if (auto *eve = dynamic_cast<HirEnumVariantExpr *>(&node)) {
        auto it = enumDefs_.find(eve->enumName);
        if (it == enumDefs_.end()) {
            error(node.loc, "அறியப்படாத பட்டியல் (enum) வகை '" + eve->enumName + "'");
            return nullptr;
        }
        const HirEnumDef *edef = it->second;
        int vIdx = edef->variantIndex(eve->variantName);
        if (vIdx < 0) {
            error(node.loc, std::string("பட்டியல் '") + eve->enumName + "' -ல் '" + eve->variantName +
                                "' என்ற உருப்படி (variant) இல்லை");
            return nullptr;
        }

        std::unique_ptr<ThirExpr> payloadNode = nullptr;
        const auto &vdef = edef->variants[vIdx];
        if (vdef.hasPayload) {
            if (!eve->payload) {
                error(node.loc, std::string("உருப்படி '") + eve->variantName + "' '" +
                                    typeInfoToString(vdef.payloadType) +
                                    "' வகை சுமையை (payload) எதிர்பார்க்கிறது");
            } else {
                payloadNode = lowerExpr(*eve->payload);
                if (payloadNode && payloadNode->typeInfo.kind != vdef.payloadType.kind &&
                    payloadNode->typeInfo.kind != TypeKind::Unknown) {
                    // Allow numeric coercion (e.g. int64 literal for int32 payload)
                    if (isNumericType(payloadNode->typeInfo.kind) &&
                        isNumericType(vdef.payloadType.kind)) {
                        TypeInfo originalType = payloadNode->typeInfo;
                        payloadNode = std::make_unique<ThirCastExpr>(
                            std::move(payloadNode), originalType, vdef.payloadType);
                    } else {
                        error(node.loc, std::string("உருப்படி '") + eve->variantName + "' '" +
                                            typeInfoToString(vdef.payloadType) +
                                            "' வகை சுமையை எதிர்பார்க்கிறது, ஆனால் கிடைத்தது '" +
                                            typeInfoToString(payloadNode->typeInfo) + "'");
                    }
                }
            }
        } else {
            if (eve->payload) {
                error(node.loc,
                      std::string("உருப்படி '") + eve->variantName + "' சுமையை ஏற்றுக்கொள்வதில்லை");
            }
        }

        TypeInfo enumType = TypeInfo::namedEnum(eve->enumName);
        auto e = std::make_unique<ThirEnumVariantExpr>(eve->enumName, eve->variantName, vIdx,
                                                       std::move(payloadNode), enumType);
        e->loc = node.loc;
        return e;
    }

    // HirMatchExpr
    if (auto *me = dynamic_cast<HirMatchExpr *>(&node)) {
        auto valNode = lowerExpr(*me->value);
        if (!valNode)
            return nullptr;
        if (!valNode->typeInfo.isEnum) {
            error(me->value->loc,
                  "பொருத்து (match) கோவையின் மதிப்பு பட்டியலாக (enum) இருக்க வேண்டும், ஆனால் கிடைத்தது '" +
                      typeInfoToString(valNode->typeInfo) + "'");
            return nullptr;
        }
        std::string enumName = valNode ? valNode->typeInfo.enumName : "";
        auto it = enumDefs_.find(enumName);
        if (it == enumDefs_.end()) {
            if (valNode)
                error(node.loc, "அறியப்படாத பட்டியல் வகை '" + enumName + "'");
            return nullptr;
        }
        const HirEnumDef *edef = it->second;

        std::vector<ThirMatchArm> thirArms;
        TypeInfo matchTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
        bool firstArm = true;

        for (auto &arm : me->arms) {
            ThirMatchArm tarm;
            tarm.variantName = arm.variantName;

            int vIdx = edef->variantIndex(arm.variantName);
            if (vIdx < 0) {
                error(node.loc, std::string("பட்டியல் '") + enumName + "' -ல் '" + arm.variantName +
                                    "' என்ற உருப்படி இல்லை");
                continue;
            }
            tarm.variantIndex = vIdx;

            tarm.hasBinding = arm.hasBinding;
            const auto &vdef = edef->variants[vIdx];

            if (arm.hasBinding && !vdef.hasPayload) {
                error(node.loc,
                      std::string("உருப்படி '") + arm.variantName + "' -ல் இணைப்பதற்கு சுமை இல்லை");
            }

            if (arm.hasBinding) {
                tarm.bindingId = arm.bindingId;
                tarm.bindingName = arm.bindingName;
                tarm.bindingType = vdef.payloadType;
                typeMap_[arm.bindingId] = vdef.payloadType;
            }

            TypeInfo armType = TypeInfo::scalar(TypeKind::Void);
            if (arm.exprBody) {
                tarm.exprBody = lowerExpr(*arm.exprBody);
                if (tarm.exprBody)
                    armType = tarm.exprBody->typeInfo;
            } else if (arm.blockBody) {
                tarm.blockBody = lowerBlock(*arm.blockBody);
                // Blocks evaluate to Void
                armType = TypeInfo::scalar(TypeKind::Void);
            }

            if (firstArm && armType.kind != TypeKind::Unknown && armType.kind != TypeKind::Void) {
                matchTypeInfo = armType;
                firstArm = false;
            } else if (!firstArm && matchTypeInfo != armType && armType.kind != TypeKind::Unknown &&
                       armType.kind != TypeKind::Void) {
                error(node.loc, std::string("பொருத்து கிளைகளின் வகைகள் பொருந்தவில்லை: எதிர்பார்க்கிறோம் '") +
                                    typeInfoToString(matchTypeInfo) + "', ஆனால் கிடைத்தது '" +
                                    typeInfoToString(armType) + "'");
            }

            thirArms.push_back(std::move(tarm));
        }

        auto e =
            std::make_unique<ThirMatchExpr>(std::move(valNode), std::move(thirArms), matchTypeInfo);
        e->loc = node.loc;
        return e;
    }
    // HirNewExpr → ThirNewExpr
    if (auto *newE = dynamic_cast<HirNewExpr *>(&node)) {
        TypeInfo allocType = resolveTypeInfo(newE->allocatedType);
        // Result type is a pointer to the allocated type, or a slice if size is provided
        TypeInfo resultType;
        if (newE->sizeExpr) {
            resultType = TypeInfo::slice(allocType);
        } else {
            resultType = allocType;
            resultType.pointerDepth++;
        }
        std::unique_ptr<ThirExpr> sizeExpr = nullptr;
        if (newE->sizeExpr) {
            sizeExpr = lowerExpr(*newE->sizeExpr);
            if (!sizeExpr)
                return nullptr;
        }
        auto e = std::make_unique<ThirNewExpr>(allocType, resultType, std::move(sizeExpr));
        e->loc = node.loc;
        return e;
    }

    // HirZoneExpr → ThirZoneExpr
    if (auto *zoneE = dynamic_cast<HirZoneExpr *>(&node)) {
        auto body = lowerBlock(*zoneE->body);
        auto e = std::make_unique<ThirZoneExpr>(zoneE->zoneName, std::move(body),
                                                TypeInfo::scalar(TypeKind::Void));
        e->loc = node.loc;
        return e;
    }

    // HirAllocExpr → ThirAllocExpr
    if (auto *allocE = dynamic_cast<HirAllocExpr *>(&node)) {
        TypeInfo allocType = resolveTypeInfo(allocE->type);
        TypeInfo resultType = allocType;
        resultType.pointerDepth++;
        resultType.pulseTag = allocE->zoneName;

        std::unique_ptr<ThirExpr> count = nullptr;
        if (allocE->count)
            count = lowerExpr(*allocE->count);

        auto e = std::make_unique<ThirAllocExpr>(allocType, std::move(count), allocE->zoneName,
                                                 resultType);
        e->loc = node.loc;
        return e;
    }

    // HirBorrowExpr → ThirBorrowExpr
    if (auto *borrowE = dynamic_cast<HirBorrowExpr *>(&node)) {
        auto target = lowerExpr(*borrowE->target);
        TypeInfo resultType = target->typeInfo;
        resultType.pointerDepth++; // Borrowing makes it a pointer
        auto e =
            std::make_unique<ThirBorrowExpr>(borrowE->isMutable, std::move(target), resultType);
        e->loc = node.loc;
        return e;
    }

    // HirEscapeExpr → ThirEscapeExpr
    if (auto *escapeE = dynamic_cast<HirEscapeExpr *>(&node)) {
        auto target = lowerExpr(*escapeE->target);
        TypeInfo resultType = target->typeInfo;
        resultType.pulseTag = escapeE->destinationZone; // Change the region tag
        auto e = std::make_unique<ThirEscapeExpr>(std::move(target), escapeE->destinationZone,
                                                  resultType);
        e->loc = node.loc;
        return e;
    }

    // HirCastExpr → ThirCastExpr
    if (auto *castE = dynamic_cast<HirCastExpr *>(&node)) {
        auto operand = lowerExpr(*castE->expr);
        if (!operand)
            return nullptr;

        TypeInfo from = operand->typeInfo;
        TypeInfo to = resolveTypeInfo(castE->targetType);

        // Semantic validation for casts
        bool valid = false;
        if (isNumericType(from.kind) && isNumericType(to.kind))
            valid = true;
        else if (from.pointerDepth > 0 && to.pointerDepth > 0)
            valid = true;
        else if (from.pointerDepth > 0 && isNumericType(to.kind))
            valid = true; // pointer to int
        else if (isNumericType(from.kind) && to.pointerDepth > 0)
            valid = true; // int to pointer (e.g., 0 as *int)
        else if (from == to)
            valid = true;

        if (!valid) {
            error(node.loc, "தவறான வகை நிலைமாற்றம் (cast): '" + typeInfoToString(from) +
                                "' என்பதிலிருந்து '" + typeInfoToString(to) + "' -க்கு மாற்ற முடியாது");
        }

        auto e = std::make_unique<ThirCastExpr>(std::move(operand), from, to);
        e->loc = node.loc;
        return e;
    }

    error(node.loc, "THIR மாற்றத்தில் அறியப்படாத கோவை (unknown expression)");
    return nullptr;
}

} // namespace agam
