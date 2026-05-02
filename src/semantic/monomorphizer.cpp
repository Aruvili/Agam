#include "agam/semantic/monomorphizer.h"

#include <algorithm>
#include <sstream>

namespace agam {

void Monomorphizer::monomorphize(Program &program) {
    currentProgram_ = &program;
    visit(program);
}

void Monomorphizer::visit(Program &node) {
    for (size_t i = 0; i < node.functions.size(); ++i) {
        auto &fn = node.functions[i];
        if (fn->typeParams.empty())
            fn->accept(*this);
    }
    for (auto &impl : node.impls) {
        impl->accept(*this);
    }
}

void Monomorphizer::visit(VarDeclStmt &node) {
    mangleType(node.typeInfo);
    if (node.initializer)
        node.initializer->accept(*this);
}

void Monomorphizer::visit(CallExpr &node) {
    // Check if it's a call to a generic function
    // Use index-based loop because specializeFunction can add new functions
    for (size_t i = 0; i < currentProgram_->functions.size(); ++i) {
        auto &fn = currentProgram_->functions[i];
        if (fn->name == node.callee && !fn->typeParams.empty()) {
            // Found generic function!
            if (node.genericArgs.size() == fn->typeParams.size()) {
                std::string mangled = getMangledName(node.callee, node.genericArgs);
                Instantiation inst = {node.callee, node.genericArgs};
                if (specializedFunctions_.find(inst) == specializedFunctions_.end()) {
                    specializeFunction(node.callee, node.genericArgs);
                }
                node.callee = mangled;
                node.genericArgs.clear();
                break; // Found it
            }
        }
    }
    for (auto &arg : node.args)
        arg->accept(*this);
}

void Monomorphizer::visit(StructLiteralExpr &node) {
    if (!node.structType.genericArgs.empty()) {
        specializeStruct(node.structType.structName, node.structType.genericArgs);
        node.structType.structName =
            getMangledName(node.structType.structName, node.structType.genericArgs);
        node.structType.genericArgs.clear();
    }
    for (auto &f : node.fields)
        f.value->accept(*this);
}

void Monomorphizer::visit(BlockStmt &node) {
    for (auto &s : node.statements)
        s->accept(*this);
}

void Monomorphizer::visit(ReturnStmt &node) {
    if (node.value)
        node.value->accept(*this);
}
void Monomorphizer::visit(NewExpr &node) {
    if (!node.allocatedType.genericArgs.empty()) {
        if (node.allocatedType.isStruct)
            specializeStruct(node.allocatedType.structName, node.allocatedType.genericArgs);
        node.allocatedType.structName =
            getMangledName(node.allocatedType.structName, node.allocatedType.genericArgs);
        node.allocatedType.genericArgs.clear();
    }
}

void Monomorphizer::visit(FunctionDecl &node) {
    mangleType(node.returnTypeInfo);
    for (auto &p : node.params) {
        mangleType(p.typeInfo);
    }
    if (node.body)
        node.body->accept(*this);
}

void Monomorphizer::visit(StructDecl &node) {}
void Monomorphizer::visit(EnumDecl &node) {}
void Monomorphizer::visit(TraitDecl &node) {}
void Monomorphizer::visit(ImplDecl &node) {
    for (auto &fn : node.methods) {
        fn->accept(*this);
    }
}
void Monomorphizer::visit(ConstDecl &node) {
    if (node.value)
        node.value->accept(*this);
}

void Monomorphizer::visit(EnumVariantExpr &node) {
    if (!node.genericArgs.empty()) {
        specializeEnum(node.enumName, node.genericArgs);
        node.enumName = getMangledName(node.enumName, node.genericArgs);
        node.genericArgs.clear();
    }
    if (node.payload)
        node.payload->accept(*this);
}

void Monomorphizer::visit(MethodCallExpr &node) {
    node.base->accept(*this);
    for (auto &arg : node.args)
        arg->accept(*this);
}

void Monomorphizer::visit(CastExpr &node) {
    mangleType(node.targetType);
    node.expr->accept(*this);
}

std::string Monomorphizer::getMangledName(const std::string &base,
                                          const std::vector<TypeInfo> &args) {
    std::string name = base + "_";
    for (const auto &arg : args) {
        if (arg.isStruct)
            name += arg.structName;
        else if (arg.isEnum)
            name += arg.enumName;
        else
            name += typeKindToString(arg.kind);
        for (int i = 0; i < arg.pointerDepth; ++i)
            name += "Ptr";
    }
    return name;
}

void Monomorphizer::mangleType(TypeInfo &ti) {
    if (!ti.genericArgs.empty()) {
        for (auto &arg : ti.genericArgs)
            mangleType(arg);

        if (ti.isStruct || ti.isEnum) {
            if (ti.isStruct)
                specializeStruct(ti.structName, ti.genericArgs);
            else if (ti.isEnum)
                specializeEnum(ti.enumName, ti.genericArgs);

            ti.structName =
                getMangledName(ti.isStruct ? ti.structName : ti.enumName, ti.genericArgs);
            if (ti.isEnum)
                ti.enumName = ti.structName;
            ti.genericArgs.clear();
        }
    }
}

TypeInfo Monomorphizer::substitute(const TypeInfo &type, const std::vector<std::string> &params,
                                   const std::vector<TypeInfo> &args) {
    // Handle array/slice with generic element type: [T; n] or &[T]
    // The parser might not mark these as isGeneric, so we check both isGeneric and structName
    if (type.isArray || type.isSlice) {
        for (size_t i = 0; i < params.size(); ++i) {
            bool matches = false;
            if (type.isGeneric && params[i] == type.genericName)
                matches = true;
            else if (type.isStruct && params[i] == type.structName)
                matches = true;

            if (matches && i < args.size()) {
                TypeInfo result = type;
                result.isGeneric = args[i].isGeneric;
                result.genericName = args[i].genericName;
                result.kind = args[i].kind;
                result.elementType = args[i].kind;
                result.isStruct = args[i].isStruct;
                result.structName = args[i].structName;
                result.isEnum = args[i].isEnum;
                result.enumName = args[i].enumName;
                result.genericArgs = args[i].genericArgs;
                return result;
            }
        }
    }
    if (type.isGeneric) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == type.genericName) {
                if (i < args.size()) {
                    TypeInfo substituted = args[i];
                    substituted.pointerDepth += type.pointerDepth;
                    return substituted;
                }
            }
        }
    }
    // Also match struct names that are actually type params (parser doesn't mark them isGeneric)
    if (type.isStruct) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == type.structName && i < args.size()) {
                TypeInfo substituted = args[i];
                substituted.pointerDepth += type.pointerDepth;
                return substituted;
            }
        }
    }
    TypeInfo result = type;
    for (auto &arg : result.genericArgs) {
        arg = substitute(arg, params, args);
    }
    return result;
}

class TypeSubstitutionVisitor : public ASTVisitor {
  public:
    TypeSubstitutionVisitor(Monomorphizer &monomorphizer, const std::vector<std::string> &params,
                            const std::vector<TypeInfo> &args)
        : monomorphizer_(monomorphizer), params_(params), args_(args) {}

    TypeInfo substitute(const TypeInfo &type) {
        // Handle array/slice with generic element type: [T; n] or &[T]
        if (type.isArray || type.isSlice) {
            for (size_t i = 0; i < params_.size(); ++i) {
                bool matches = false;
                if (type.isGeneric && params_[i] == type.genericName)
                    matches = true;
                else if (type.isStruct && params_[i] == type.structName)
                    matches = true;

                if (matches && i < args_.size()) {
                    TypeInfo res;
                    if (type.isSlice)
                        res = TypeInfo::slice(args_[i]);
                    else
                        res = TypeInfo::array(args_[i], type.arraySize);
                    monomorphizer_.mangleType(res);
                    return res;
                }
            }
        }

        if (type.isGeneric) {
            for (size_t i = 0; i < params_.size(); ++i) {
                if (params_[i] == type.genericName && i < args_.size()) {
                    TypeInfo result = args_[i];
                    result.pointerDepth += type.pointerDepth;
                    monomorphizer_.mangleType(result);
                    return result;
                }
            }
        }
        // Also match struct names that are actually type params
        if (type.isStruct) {
            for (size_t i = 0; i < params_.size(); ++i) {
                if (params_[i] == type.structName && i < args_.size()) {
                    TypeInfo result = args_[i];
                    result.pointerDepth += type.pointerDepth;
                    monomorphizer_.mangleType(result);
                    return result;
                }
            }
        }

        TypeInfo result = type;
        for (auto &arg : result.genericArgs) {
            arg = substitute(arg);
        }
        monomorphizer_.mangleType(result);
        return result;
    }

    void visit(VarDeclStmt &node) override {
        node.typeInfo = substitute(node.typeInfo);
        monomorphizer_.mangleType(node.typeInfo);
        if (node.initializer)
            node.initializer->accept(*this);
    }

    void visit(NewExpr &node) override {
        node.allocatedType = substitute(node.allocatedType);
        monomorphizer_.mangleType(node.allocatedType);
        if (node.sizeExpr)
            node.sizeExpr->accept(*this);
    }

    void visit(StructLiteralExpr &node) override {
        node.structType = substitute(node.structType);
        monomorphizer_.mangleType(node.structType);
        for (auto &f : node.fields)
            f.value->accept(*this);
    }

    void visit(CallExpr &node) override {
        for (auto &arg : node.args)
            arg->accept(*this);
        for (auto &ga : node.genericArgs) {
            ga = substitute(ga);
            monomorphizer_.mangleType(ga);
        }
    }

    // Boilerplate traversal
    void visit(IntLiteralExpr &) override {}
    void visit(FloatLiteralExpr &) override {}
    void visit(StringLiteralExpr &) override {}
    void visit(BoolLiteralExpr &) override {}
    void visit(NullLiteralExpr &node) override {}
    void visit(VariableExpr &node) override {
        for (auto &ga : node.genericArgs) {
            ga = substitute(ga);
            monomorphizer_.mangleType(ga);
        }
    }
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
    void visit(BlockStmt &node) override {
        for (auto &s : node.statements)
            s->accept(*this);
    }
    void visit(ReturnStmt &node) override {
        if (node.value)
            node.value->accept(*this);
    }
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
    void visit(CastExpr &node) override {
        node.targetType = substitute(node.targetType);
        monomorphizer_.mangleType(node.targetType);
        node.expr->accept(*this);
    }

    void visit(FunctionDecl &node) override {
        if (node.body)
            node.body->accept(*this);
    }
    void visit(StructDecl &node) override {}
    void visit(EnumDecl &node) override {}
    void visit(TraitDecl &node) override {}
    void visit(ImplDecl &node) override {}
    void visit(Program &node) override {}
    void visit(MethodCallExpr &node) override {
        node.base->accept(*this);
        for (auto &arg : node.args)
            arg->accept(*this);
    }
    void visit(EnumVariantExpr &node) override {
        if (node.payload)
            node.payload->accept(*this);
    }
    void visit(ConstDecl &node) override {
        if (node.value)
            node.value->accept(*this);
    }

  private:
    Monomorphizer &monomorphizer_;
    const std::vector<std::string> &params_;
    const std::vector<TypeInfo> &args_;
};

void Monomorphizer::specializeStruct(const std::string &name, const std::vector<TypeInfo> &args) {
    Instantiation inst = {name, args};
    if (specializedStructs_.count(inst))
        return;

    std::vector<std::string> structTypeParams;

    for (auto &sd : currentProgram_->structs) {
        if (sd->name == name) {
            std::string mangled = getMangledName(name, args);
            specializedStructs_[inst] = mangled;
            structTypeParams = sd->typeParams;

            auto specialized =
                std::unique_ptr<StructDecl>(static_cast<StructDecl *>(sd->clone().release()));
            specialized->name = mangled;
            specialized->typeParams.clear();
            for (auto &f : specialized->fields) {
                f.typeInfo = substitute(f.typeInfo, sd->typeParams, args);
                mangleType(f.typeInfo);
            }
            currentProgram_->structs.push_back(std::move(specialized));
            break;
        }
    }

    // Also specialize any generic impl blocks that target this struct
    if (!structTypeParams.empty()) {
        std::string mangled = getMangledName(name, args);
        size_t implCount = currentProgram_->impls.size();
        for (size_t i = 0; i < implCount; ++i) {
            auto &impl = currentProgram_->impls[i];
            // Match impl<T> blocks where targetType.structName matches our struct name
            if (!impl->typeParams.empty() && impl->targetType.structName == name) {
                TypeSubstitutionVisitor sub(*this, structTypeParams, args);

                auto specializedImpl =
                    std::unique_ptr<ImplDecl>(static_cast<ImplDecl *>(impl->clone().release()));
                specializedImpl->typeParams.clear();
                specializedImpl->targetType.structName = mangled;
                specializedImpl->targetType.genericArgs.clear();

                for (auto &method : specializedImpl->methods) {
                    // Substitute return type
                    method->returnTypeInfo = sub.substitute(method->returnTypeInfo);
                    // Substitute parameter types
                    for (auto &param : method->params) {
                        param.typeInfo = sub.substitute(param.typeInfo);
                    }
                    // Substitute types within method body
                    if (method->body) {
                        method->body->accept(sub);
                    }
                    // Recursively monomorphize inside the specialized method
                    if (method->body) {
                        method->body->accept(*this);
                    }
                }

                currentProgram_->impls.push_back(std::move(specializedImpl));
            }
        }
    }
}

void Monomorphizer::specializeFunction(const std::string &name, const std::vector<TypeInfo> &args) {
    Instantiation inst = {name, args};
    if (specializedFunctions_.count(inst))
        return;

    for (size_t i = 0; i < currentProgram_->functions.size(); ++i) {
        auto &fn = currentProgram_->functions[i];
        if (fn->name == name) {
            std::string mangled = getMangledName(name, args);
            specializedFunctions_[inst] = mangled;

            auto specialized =
                std::unique_ptr<FunctionDecl>(static_cast<FunctionDecl *>(fn->clone().release()));
            specialized->name = mangled;
            specialized->typeParams.clear();

            TypeSubstitutionVisitor sub(*this, fn->typeParams, args);

            // Substitute parameter types
            for (auto &param : specialized->params) {
                param.typeInfo = sub.substitute(param.typeInfo);
            }
            // Substitute return type
            specialized->returnTypeInfo = sub.substitute(specialized->returnTypeInfo);

            if (specialized->body) {
                specialized->body->accept(sub);
            }

            // After substitution, we MUST mangle all types in the specialized function
            specialized->returnTypeInfo = substitute(fn->returnTypeInfo, fn->typeParams, args);
            mangleType(specialized->returnTypeInfo);
            for (auto &p : specialized->params) {
                p.typeInfo = substitute(p.typeInfo, fn->typeParams, args);
                mangleType(p.typeInfo);
            }

            // Important: we need to recursively monomorphize inside the specialized function!
            specialized->body->accept(*this);

            currentProgram_->functions.push_back(std::move(specialized));
            break;
        }
    }
}

void Monomorphizer::specializeEnum(const std::string &name, const std::vector<TypeInfo> &args) {
    Instantiation inst = {name, args};
    if (specializedEnums_.count(inst))
        return;

    for (size_t i = 0; i < currentProgram_->enums.size(); ++i) {
        auto &ed = currentProgram_->enums[i];
        if (ed->name == name) {
            std::string mangled = getMangledName(name, args);
            specializedEnums_[inst] = mangled;

            auto specialized =
                std::unique_ptr<EnumDecl>(static_cast<EnumDecl *>(ed->clone().release()));
            specialized->name = mangled;
            specialized->typeParams.clear();

            for (auto &v : specialized->variants) {
                if (v.hasPayload) {
                    v.payloadType = substitute(v.payloadType, ed->typeParams, args);
                    mangleType(v.payloadType);
                }
            }
            currentProgram_->enums.push_back(std::move(specialized));
            break;
        }
    }
}

} // namespace agam
