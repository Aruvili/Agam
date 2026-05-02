#include "agam/mir/mir_printer.h"

#include <sstream>

namespace agam {

static std::string typeStr(const TypeInfo &ti) {
    std::string s = "";
    for (int i = 0; i < ti.pointerDepth; ++i)
        s += "*";
    if (ti.isArray) {
        s += std::string("[") + typeKindToString(ti.elementType) + "; " +
             std::to_string(ti.arraySize) + "]";
    } else if (ti.isStruct) {
        s += ti.structName;
    } else if (ti.isEnum) {
        s += ti.enumName;
    } else {
        s += typeKindToString(ti.kind);
    }
    return s;
}

static std::string operandStr(const MirOperand &op) {
    if (op.kind == MirOperand::Kind::Copy) {
        return "_" + std::to_string(op.place.local);
    }
    // Constant
    return std::visit(
        [](auto &&c) -> std::string {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, MirConstInt>)
                return std::to_string(c.value);
            else if constexpr (std::is_same_v<T, MirConstFloat>)
                return std::to_string(c.value);
            else if constexpr (std::is_same_v<T, MirConstBool>)
                return c.value ? "true" : "false";
            else if constexpr (std::is_same_v<T, MirConstString>)
                return "\"" + c.value + "\"";
            else
                return "?";
        },
        op.constant);
}

std::string MirPrinter::printFunction(const MirFunction &func) {
    std::ostringstream out;
    out << "fn " << func.name << "(";
    for (size_t i = 0; i < func.params.size(); ++i) {
        if (i > 0)
            out << ", ";
        out << "%" << func.params[i].id << ": " << typeStr(func.params[i].typeInfo);
    }
    out << ") -> " << typeStr(func.returnTypeInfo) << " {\n";

    // Print locals declarations (excluding params since they are in signature)
    for (const auto &local : func.locals) {
        if (local.id < func.params.size())
            continue; // simple heuristic if params are first
        out << "    let ";
        if (local.isTemp)
            out << "mut ";
        out << "%" << local.id;
        if (!local.name.empty())
            out << " /* " << local.name << " */";
        out << ": " << typeStr(local.typeInfo) << ";\n";
    }
    out << "\n";

    // Print blocks
    for (auto &bb : func.blocks) {
        out << "    " << bb.label << " (bb" << bb.id << "):\n";

        for (auto &stmt : bb.statements) {
            std::visit(
                [&](auto &&s) {
                    using StatementT = std::decay_t<decltype(s)>;
                    if constexpr (std::is_same_v<StatementT, MirAssign>) {
                        out << "        _" << s.destination.local << " = ";
                        std::visit(
                            [&](auto &&rv) {
                                using RValueT = std::decay_t<decltype(rv)>;
                                if constexpr (std::is_same_v<RValueT, MirRvalueUse>) {
                                    out << operandStr(rv.operand);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueBinaryOp>) {
                                    out << operandStr(rv.lhs) << " " << binaryOpToString(rv.op)
                                        << " " << operandStr(rv.rhs);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueUnaryOp>) {
                                    out << (rv.op == UnaryOp::Negate
                                                ? "-"
                                                : (rv.op == UnaryOp::Not
                                                       ? "!"
                                                       : (rv.op == UnaryOp::AddressOf ? "&" : "*")))
                                        << operandStr(rv.operand);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueCast>) {
                                    out << operandStr(rv.operand) << " as "
                                        << typeStr(rv.toTypeInfo);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueCall>) {
                                    out << rv.callee << "(";
                                    for (size_t i = 0; i < rv.args.size(); i++) {
                                        if (i > 0)
                                            out << ", ";
                                        out << operandStr(rv.args[i]);
                                    }
                                    out << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueArrayInit>) {
                                    out << "[";
                                    for (size_t i = 0; i < rv.elements.size(); i++) {
                                        if (i > 0)
                                            out << ", ";
                                        out << operandStr(rv.elements[i]);
                                    }
                                    out << "]";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueIndex>) {
                                    out << operandStr(rv.base) << "[" << operandStr(rv.index)
                                        << "]";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueStructInit>) {
                                    out << rv.structName << " { ";
                                    for (size_t i = 0; i < rv.fields.size(); i++) {
                                        if (i > 0)
                                            out << ", ";
                                        out << rv.fields[i].name << ": "
                                            << operandStr(rv.fields[i].value);
                                    }
                                    out << " }";
                                } else if constexpr (std::is_same_v<RValueT,
                                                                    MirRvalueFieldAccess>) {
                                    out << operandStr(rv.base) << "." << rv.field;
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueEnumInit>) {
                                    out << rv.enumName << "::Variant(" << rv.variantIndex << ")";
                                } else if constexpr (std::is_same_v<RValueT,
                                                                    MirRvalueEnumPayload>) {
                                    out << "payload(" << operandStr(rv.enumOperand) << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueEnumTag>) {
                                    out << "tag(" << operandStr(rv.enumOperand) << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueHeapAlloc>) {
                                    out << "new " << typeKindToString(rv.allocatedType.kind);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueSlice>) {
                                    out << operandStr(rv.base) << "[" << operandStr(rv.start)
                                        << ".." << operandStr(rv.end) << "]";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueSliceLen>) {
                                    out << "slice_len(" << operandStr(rv.slice) << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueSlicePtr>) {
                                    out << "slice_ptr(" << operandStr(rv.slice) << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueStringLen>) {
                                    out << "strlen(" << operandStr(rv.operand) << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueZoneAlloc>) {
                                    out << "alloc<" << typeKindToString(rv.allocatedType.kind)
                                        << ">~" << rv.zoneName << "(" << operandStr(rv.count)
                                        << ")";
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueBorrow>) {
                                    out << (rv.isMutable ? "borrow mut " : "borrow ")
                                        << operandStr(rv.target);
                                } else if constexpr (std::is_same_v<RValueT, MirRvalueEscape>) {
                                    out << "escape " << operandStr(rv.target) << " to "
                                        << rv.destinationZone;
                                } else if constexpr (std::is_same_v<RValueT, MirRvaluePtrOffset>) {
                                    out << "ptr_offset(" << operandStr(rv.base) << ", "
                                        << rv.byteOffset << ") as " << typeStr(rv.resultType);
                                }
                            },
                            s.rvalue);
                        out << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirIndexAssign>) {
                        out << "        " << operandStr(s.base) << "[" << operandStr(s.index)
                            << "] = " << operandStr(s.value) << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirFieldAssign>) {
                        out << "        " << operandStr(s.base) << "." << s.field << " = "
                            << operandStr(s.value) << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirDerefAssign>) {
                        out << "        *" << operandStr(s.pointer) << " = " << operandStr(s.value)
                            << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirHeapFree>) {
                        out << "        delete " << operandStr(s.pointer) << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirZoneBegin>) {
                        out << "        zone_begin " << s.zoneName << "\n";
                    } else if constexpr (std::is_same_v<StatementT, MirZoneEnd>) {
                        out << "        zone_end " << s.zoneName << "\n";
                    }
                },
                stmt);
        }

        // Print terminator
        out << "        ";
        std::visit(
            [&](auto &&term) {
                using T = std::decay_t<decltype(term)>;
                if constexpr (std::is_same_v<T, MirGoto>) {
                    out << "goto -> bb" << term.target;
                } else if constexpr (std::is_same_v<T, MirSwitchInt>) {
                    out << "switchInt(" << operandStr(term.discriminant) << ") -> [true: bb"
                        << term.thenBlock << ", false: bb" << term.elseBlock << "]";
                } else if constexpr (std::is_same_v<T, MirReturn>) {
                    out << "return " << operandStr(term.value);
                } else if constexpr (std::is_same_v<T, MirReturnVoid>) {
                    out << "return void";
                } else if constexpr (std::is_same_v<T, MirUnreachable>) {
                    out << "unreachable";
                }
            },
            bb.terminator);
        out << "\n\n";
    }

    out << "}\n";
    return out.str();
}

std::string MirPrinter::print(const MirProgram &program) {
    std::ostringstream out;
    for (auto &func : program.functions) {
        out << printFunction(func) << "\n";
    }
    return out.str();
}

} // namespace agam
