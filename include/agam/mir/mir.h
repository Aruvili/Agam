#pragma once

#include "agam/ast/ast.h" // TypeKind, BinaryOp, UnaryOp
#include "agam/hir/hir.h" // HirId

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Local — every variable and temporary in a function
// ═══════════════════════════════════════════════════════════════════════════════

using MirLocalId = uint32_t;
using MirBlockId = uint32_t;

/// A local variable or compiler-generated temporary.
struct MirLocal {
    MirLocalId id;
    TypeInfo typeInfo = TypeInfo::scalar(TypeKind::Unknown);
    std::string name;    // empty for compiler temporaries
    bool isTemp = false; // true if compiler-generated
};

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Operand — input to an instruction
// ═══════════════════════════════════════════════════════════════════════════════

/// Reference to a local (a "place" you can read from or write to).
struct MirPlace {
    MirLocalId local;
};

/// A constant value.
struct MirConstInt {
    int64_t value;
};
struct MirConstFloat {
    double value;
};
struct MirConstBool {
    bool value;
};
struct MirConstString {
    std::string value;
};
struct MirConstNull {};

using MirConstant =
    std::variant<MirConstInt, MirConstFloat, MirConstBool, MirConstString, MirConstNull>;

/// An operand is either a copy of a local or a constant.
struct MirOperand {
    enum class Kind { Copy, Constant };
    Kind kind;
    MirPlace place;       // valid when kind == Copy
    MirConstant constant; // valid when kind == Constant
    TypeInfo typeInfo = TypeInfo::scalar(TypeKind::Unknown);

    static MirOperand makeCopy(MirLocalId local, TypeInfo t) {
        MirOperand op;
        op.kind = Kind::Copy;
        op.place = {local};
        op.typeInfo = t;
        return op;
    }
    static MirOperand makeConstInt(int64_t v) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstInt{v};
        op.typeInfo = TypeInfo::scalar(TypeKind::Int64);
        return op;
    }
    static MirOperand makeConstInt(int64_t v, TypeInfo ti) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstInt{v};
        op.typeInfo = ti;
        return op;
    }
    static MirOperand makeConstInt64(int64_t v) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstInt{v};
        op.typeInfo = TypeInfo::scalar(TypeKind::Int64);
        return op;
    }
    static MirOperand makeConstFloat(double v) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstFloat{v};
        op.typeInfo = TypeInfo::scalar(TypeKind::Float);
        return op;
    }
    static MirOperand makeConstBool(bool v) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstBool{v};
        op.typeInfo = TypeInfo::scalar(TypeKind::Bool);
        return op;
    }
    static MirOperand makeConstString(std::string v) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstString{std::move(v)};
        op.typeInfo = TypeInfo::scalar(TypeKind::String);
        return op;
    }
    static MirOperand makeConstNull(TypeInfo t) {
        MirOperand op;
        op.kind = Kind::Constant;
        op.constant = MirConstNull{};
        op.typeInfo = t;
        return op;
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Rvalue — right-hand side of an assignment
// ═══════════════════════════════════════════════════════════════════════════════

struct MirRvalueUse {
    MirOperand operand;
};

struct MirRvalueBinaryOp {
    BinaryOp op;
    MirOperand lhs;
    MirOperand rhs;
};

struct MirRvalueUnaryOp {
    UnaryOp op;
    MirOperand operand;
};

struct MirRvalueCast {
    MirOperand operand;
    TypeInfo fromTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
    TypeInfo toTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueCall {
    std::string callee;
    std::vector<MirOperand> args;
    TypeInfo returnTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueArrayInit {
    std::vector<MirOperand> elements;
    TypeInfo arrayTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueIndex {
    MirOperand base;
    MirOperand index;
    TypeInfo elementTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirStructFieldInit {
    std::string name;
    int fieldIndex;
    MirOperand value;
};

struct MirRvalueStructInit {
    std::string structName;
    std::vector<MirStructFieldInit> fields;
    TypeInfo structTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueFieldAccess {
    MirOperand base;
    std::string field;
    int fieldIndex;
    TypeInfo fieldTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueEnumInit {
    std::string enumName;
    int variantIndex;
    bool hasPayload;
    MirOperand payload;
    TypeInfo enumTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueEnumPayload {
    MirOperand enumOperand;
    TypeInfo payloadTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirRvalueEnumTag {
    MirOperand enumOperand;
    TypeInfo tagTypeInfo = TypeInfo::scalar(TypeKind::Int32);
};

struct MirRvalueHeapAlloc {
    TypeInfo allocatedType;
    MirOperand size; // Optional: dynamic array size
    bool isDynamicArray = false;
};

struct MirRvalueSlice {
    MirOperand base;
    MirOperand start;
    MirOperand end;
};

struct MirRvalueSliceLen {
    MirOperand slice;
};

struct MirRvalueSlicePtr {
    MirOperand slice;
};

struct MirRvalueStringLen {
    MirOperand operand;
};

struct MirRvalueZoneAlloc {
    TypeInfo allocatedType;
    MirOperand count;
    std::string zoneName;
};

struct MirRvalueBorrow {
    bool isMutable;
    MirOperand target;
};

struct MirRvalueEscape {
    MirOperand target;
    std::string destinationZone;
};

struct MirRvaluePtrOffset {
    MirOperand base;
    int64_t byteOffset;
    TypeInfo resultType;
};

using MirRvalue =
    std::variant<MirRvalueUse, MirRvalueBinaryOp, MirRvalueUnaryOp, MirRvalueCast, MirRvalueCall,
                 MirRvalueArrayInit, MirRvalueIndex, MirRvalueStructInit, MirRvalueFieldAccess,
                 MirRvalueEnumInit, MirRvalueEnumPayload, MirRvalueEnumTag, MirRvalueHeapAlloc,
                 MirRvalueSlice, MirRvalueSliceLen, MirRvalueSlicePtr, MirRvalueStringLen,
                 MirRvalueZoneAlloc, MirRvalueBorrow, MirRvalueEscape, MirRvaluePtrOffset>;

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Statement — an instruction within a basic block
// ═══════════════════════════════════════════════════════════════════════════════

struct MirAssign {
    MirPlace destination;
    MirRvalue rvalue;
};

struct MirIndexAssign {
    MirOperand base;
    MirOperand index;
    MirOperand value;
    TypeInfo elementTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
};

struct MirFieldAssign {
    MirOperand base;
    std::string field;
    int fieldIndex;
    MirOperand value;
};

struct MirDerefAssign {
    MirOperand pointer;
    MirOperand value;
};

/// Heap free: free(ptr)
struct MirHeapFree {
    MirOperand pointer;
};

struct MirZoneBegin {
    std::string zoneName;
};

struct MirZoneEnd {
    std::string zoneName;
};

using MirStatement = std::variant<MirAssign, MirIndexAssign, MirFieldAssign, MirDerefAssign,
                                  MirHeapFree, MirZoneBegin, MirZoneEnd>;

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Terminator — ends a basic block
// ═══════════════════════════════════════════════════════════════════════════════

struct MirGoto {
    MirBlockId target;
};

struct MirSwitchInt {
    MirOperand discriminant;
    MirBlockId thenBlock; // if true
    MirBlockId elseBlock; // if false
};

struct MirSwitchValue {
    MirOperand discriminant;
    std::vector<std::pair<int, MirBlockId>> targets;
    MirBlockId defaultBlock;
};

struct MirReturn {
    MirOperand value;
    bool hasValue = true;
};

struct MirReturnVoid {};

/// Sentinel: block has not been terminated yet.
struct MirUnreachable {};

using MirTerminator =
    std::variant<MirGoto, MirSwitchInt, MirSwitchValue, MirReturn, MirReturnVoid, MirUnreachable>;

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Basic Block
// ═══════════════════════════════════════════════════════════════════════════════

struct MirBasicBlock {
    MirBlockId id;
    std::string label; // e.g., "entry", "then", "else", "whilecond"
    std::vector<MirStatement> statements;
    MirTerminator terminator;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Function + Program
// ═══════════════════════════════════════════════════════════════════════════════

struct MirFunction {
    std::string name;
    std::vector<MirLocal> params; // function parameters
    TypeInfo returnTypeInfo = TypeInfo::scalar(TypeKind::Unknown);
    std::vector<MirLocal> locals;      // all locals (params + vars + temps)
    std::vector<MirBasicBlock> blocks; // basic blocks (entry is blocks[0])
    bool isExtern = false;             // external C function (declared, not defined)
};

struct MirConstDef {
    std::string name;
    TypeInfo typeInfo;
    MirConstant value;
};

struct MirProgram {
    std::vector<MirFunction> functions;
    std::vector<MirConstDef> constants;
    std::vector<HirStructDef> structs;
    std::vector<HirEnumDef> enums;
};

} // namespace agam
