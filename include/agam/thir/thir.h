#pragma once

#include "agam/ast/ast.h"  // TypeKind, BinaryOp, UnaryOp, SourceLocation
#include "agam/hir/hir.h"  // HirId
#include <memory>
#include <string>
#include <vector>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  THIR Expressions — every expression carries a resolved TypeKind
// ═══════════════════════════════════════════════════════════════════════════════

class ThirBlock;

class ThirExpr {
public:
    SourceLocation loc;
    TypeInfo typeInfo = TypeInfo::scalar(TypeKind::Unknown);  // resolved type
    virtual ~ThirExpr() = default;
};

class ThirIntLiteral : public ThirExpr {
public:
    int64_t value;
    explicit ThirIntLiteral(int64_t v) : value(v) { typeInfo = TypeInfo::scalar(TypeKind::Int); }
};

class ThirFloatLiteral : public ThirExpr {
public:
    double value;
    explicit ThirFloatLiteral(double v) : value(v) { typeInfo = TypeInfo::scalar(TypeKind::Float); }
};

class ThirStringLiteral : public ThirExpr {
public:
    std::string value;
    explicit ThirStringLiteral(std::string v) : value(std::move(v)) { typeInfo = TypeInfo::scalar(TypeKind::String); }
};

class ThirBoolLiteral : public ThirExpr {
public:
    bool value;
    explicit ThirBoolLiteral(bool v) : value(v) { typeInfo = TypeInfo::scalar(TypeKind::Bool); }
};

class ThirNullLiteral : public ThirExpr {
public:
    ThirNullLiteral() { typeInfo = TypeInfo::scalar(TypeKind::Unknown); /* will be updated during type checking if needed */ }
};

class ThirVarRef : public ThirExpr {
public:
    HirId defId;
    std::string name;
    ThirVarRef(HirId id, std::string name, TypeInfo t)
        : defId(id), name(std::move(name)) { typeInfo = t; }
};

class ThirBinaryExpr : public ThirExpr {
public:
    BinaryOp op;
    std::unique_ptr<ThirExpr> lhs;
    std::unique_ptr<ThirExpr> rhs;
    ThirBinaryExpr(BinaryOp op, std::unique_ptr<ThirExpr> l, std::unique_ptr<ThirExpr> r, TypeInfo t)
        : op(op), lhs(std::move(l)), rhs(std::move(r)) { typeInfo = t; }
};

class ThirUnaryExpr : public ThirExpr {
public:
    UnaryOp op;
    std::unique_ptr<ThirExpr> operand;
    ThirUnaryExpr(UnaryOp op, std::unique_ptr<ThirExpr> operand, TypeInfo t)
        : op(op), operand(std::move(operand)) { typeInfo = t; }
};

class ThirCallExpr : public ThirExpr {
public:
    HirId calleeId;
    std::string calleeName;
    std::vector<std::unique_ptr<ThirExpr>> args;
    ThirCallExpr(HirId id, std::string name, std::vector<std::unique_ptr<ThirExpr>> args, TypeInfo retType)
        : calleeId(id), calleeName(std::move(name)), args(std::move(args)) { typeInfo = retType; }
};

class ThirAssignExpr : public ThirExpr {
public:
    HirId targetId;
    std::string targetName;
    std::unique_ptr<ThirExpr> value;
    ThirAssignExpr(HirId id, std::string name, std::unique_ptr<ThirExpr> val, TypeInfo t)
        : targetId(id), targetName(std::move(name)), value(std::move(val)) { typeInfo = t; }
};

/// Explicit type cast (inserted by THIR builder for int→float promotion).
class ThirCastExpr : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> operand;
    TypeInfo fromTypeInfo;
    ThirCastExpr(std::unique_ptr<ThirExpr> op, TypeInfo from, TypeInfo to)
        : operand(std::move(op)), fromTypeInfo(from) { typeInfo = to; }
};

/// Array literal: [1, 2, 3]
class ThirArrayLiteral : public ThirExpr {
public:
    std::vector<std::unique_ptr<ThirExpr>> elements;
    ThirArrayLiteral(std::vector<std::unique_ptr<ThirExpr>> elements, TypeInfo t)
        : elements(std::move(elements)) { typeInfo = t; }
};

/// Array indexing: arr[index]
class ThirIndexExpr : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> base;
    std::unique_ptr<ThirExpr> index;
    std::unique_ptr<ThirExpr> endIndex; // nullable
    bool isRange = false;

    ThirIndexExpr(std::unique_ptr<ThirExpr> base, std::unique_ptr<ThirExpr> index, TypeInfo t, std::unique_ptr<ThirExpr> end = nullptr, bool range = false)
        : base(std::move(base)), index(std::move(index)), endIndex(std::move(end)), isRange(range) { typeInfo = t; }
};

/// Array index assignment: arr[index] = value
class ThirIndexAssign : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> base;
    std::unique_ptr<ThirExpr> index;
    std::unique_ptr<ThirExpr> value;
    ThirIndexAssign(std::unique_ptr<ThirExpr> base, std::unique_ptr<ThirExpr> index, std::unique_ptr<ThirExpr> value, TypeInfo t)
        : base(std::move(base)), index(std::move(index)), value(std::move(value)) { typeInfo = t; }
};

struct ThirSliceLen : public ThirExpr {
    std::unique_ptr<ThirExpr> slice;
    ThirSliceLen(std::unique_ptr<ThirExpr> s) : slice(std::move(s)) {
        typeInfo = TypeInfo::scalar(TypeKind::Int64);
    }
};

struct ThirSlicePtr : public ThirExpr {
    std::unique_ptr<ThirExpr> slice;
    ThirSlicePtr(std::unique_ptr<ThirExpr> s, TypeInfo t) : slice(std::move(s)) {
        typeInfo = t;
    }
};

struct ThirStringLen : public ThirExpr {
    std::unique_ptr<ThirExpr> operand;
    ThirStringLen(std::unique_ptr<ThirExpr> s) : operand(std::move(s)) {
        typeInfo = TypeInfo::scalar(TypeKind::Int64);
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THIR Struct Expressions
// ═══════════════════════════════════════════════════════════════════════════════

/// Single field + typed value in a struct literal.
struct ThirStructFieldInit {
    std::string name;
    int fieldIndex;
    std::unique_ptr<ThirExpr> value;
};

/// Struct constructor: Point { x: 10, y: 20 }.
class ThirStructLiteral : public ThirExpr {
public:
    std::string structName;
    std::vector<ThirStructFieldInit> fields;
    ThirStructLiteral(std::string name, std::vector<ThirStructFieldInit> fields, TypeInfo t)
        : structName(std::move(name)), fields(std::move(fields)) { typeInfo = t; }
};

/// Field read: expr.field  (result type = field type).
class ThirFieldAccess : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> base;
    std::string field;
    int fieldIndex;
    ThirFieldAccess(std::unique_ptr<ThirExpr> base, std::string field, int idx, TypeInfo t)
        : base(std::move(base)), field(std::move(field)), fieldIndex(idx) { typeInfo = t; }
};

/// Field write: expr.field = value  (returns Void).
class ThirFieldAssign : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> base;
    std::string field;
    int fieldIndex;
    std::unique_ptr<ThirExpr> value;
    ThirFieldAssign(std::unique_ptr<ThirExpr> base, std::string field, int idx,
                    std::unique_ptr<ThirExpr> value)
        : base(std::move(base)), field(std::move(field)), fieldIndex(idx), value(std::move(value)) {
        typeInfo = TypeInfo::scalar(TypeKind::Void);
    }
};

/// Pointer dereference write: *ptr = value  (returns Void).
class ThirDerefAssign : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> pointer;
    std::unique_ptr<ThirExpr> value;
    ThirDerefAssign(std::unique_ptr<ThirExpr> ptr, std::unique_ptr<ThirExpr> val)
        : pointer(std::move(ptr)), value(std::move(val)) {
        typeInfo = TypeInfo::scalar(TypeKind::Void);
    }
};

/// Heap allocation: new T — returns a pointer to T
class ThirNewExpr : public ThirExpr {
public:
    TypeInfo allocatedType;
    std::unique_ptr<ThirExpr> sizeExpr; // for dynamic arrays

    ThirNewExpr(TypeInfo allocType, TypeInfo resultType, std::unique_ptr<ThirExpr> size = nullptr)
        : allocatedType(std::move(allocType)), sizeExpr(std::move(size)) { typeInfo = resultType; }
};

/// ZPM Zone block
class ThirZoneExpr : public ThirExpr {
public:
    std::string zoneName;
    std::unique_ptr<ThirBlock> body;
    ThirZoneExpr(std::string name, std::unique_ptr<ThirBlock> body, TypeInfo t)
        : zoneName(std::move(name)), body(std::move(body)) { typeInfo = t; }
};

/// ZPM Allocation
class ThirAllocExpr : public ThirExpr {
public:
    TypeInfo allocatedType;
    std::unique_ptr<ThirExpr> count;
    std::string zoneName;
    ThirAllocExpr(TypeInfo allocType, std::unique_ptr<ThirExpr> c, std::string z, TypeInfo resultType)
        : allocatedType(std::move(allocType)), count(std::move(c)), zoneName(std::move(z)) {
        typeInfo = resultType;
    }
};

/// ZPM Borrow
class ThirBorrowExpr : public ThirExpr {
public:
    bool isMutable;
    std::unique_ptr<ThirExpr> target;
    ThirBorrowExpr(bool mut, std::unique_ptr<ThirExpr> t, TypeInfo resType)
        : isMutable(mut), target(std::move(t)) { typeInfo = resType; }
};

/// ZPM Escape
class ThirEscapeExpr : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> target;
    std::string destinationZone;
    ThirEscapeExpr(std::unique_ptr<ThirExpr> t, std::string dest, TypeInfo resType)
        : target(std::move(t)), destinationZone(std::move(dest)) { typeInfo = resType; }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THIR Statements
// ═══════════════════════════════════════════════════════════════════════════════

class ThirStmt {
public:
    SourceLocation loc;
    virtual ~ThirStmt() = default;
};

class ThirExprStmt : public ThirStmt {
public:
    std::unique_ptr<ThirExpr> expr;
    explicit ThirExprStmt(std::unique_ptr<ThirExpr> e) : expr(std::move(e)) {}
};

class ThirVarDecl : public ThirStmt {
public:
    HirId id;
    std::string name;
    TypeInfo declaredTypeInfo;
    std::unique_ptr<ThirExpr> initializer;  // may be null
    ThirVarDecl(HirId id, std::string name, TypeInfo typeInfo, std::unique_ptr<ThirExpr> init = nullptr)
        : id(id), name(std::move(name)), declaredTypeInfo(typeInfo), initializer(std::move(init)) {}
};

class ThirBlock : public ThirStmt {
public:
    std::vector<std::unique_ptr<ThirStmt>> stmts;
    explicit ThirBlock(std::vector<std::unique_ptr<ThirStmt>> s) : stmts(std::move(s)) {}
};

struct ThirMatchArm {
    std::string variantName;
    int variantIndex = -1;
    bool hasBinding = false;
    HirId bindingId = INVALID_HIR_ID;
    std::string bindingName;
    TypeInfo bindingType;
    std::unique_ptr<ThirExpr> exprBody;
    std::unique_ptr<ThirBlock> blockBody;
};

class ThirMatchExpr : public ThirExpr {
public:
    std::unique_ptr<ThirExpr> value;
    std::vector<ThirMatchArm> arms;
    ThirMatchExpr(std::unique_ptr<ThirExpr> val, std::vector<ThirMatchArm> arms, TypeInfo t)
        : value(std::move(val)), arms(std::move(arms)) { typeInfo = t; }
};

class ThirEnumVariantExpr : public ThirExpr {
public:
    std::string enumName;
    std::string variantName;
    int variantIndex = -1;
    std::unique_ptr<ThirExpr> payload; // may be null
    ThirEnumVariantExpr(std::string ename, std::string vname, int vidx, std::unique_ptr<ThirExpr> p, TypeInfo t)
        : enumName(std::move(ename)), variantName(std::move(vname)), variantIndex(vidx), payload(std::move(p)) {
        typeInfo = t;
    }
};

class ThirReturn : public ThirStmt {
public:
    std::unique_ptr<ThirExpr> value;
    explicit ThirReturn(std::unique_ptr<ThirExpr> v = nullptr) : value(std::move(v)) {}
};

class ThirIf : public ThirStmt {
public:
    std::unique_ptr<ThirExpr> condition;
    std::unique_ptr<ThirBlock> thenBranch;
    std::unique_ptr<ThirBlock> elseBranch;
    ThirIf(std::unique_ptr<ThirExpr> cond, std::unique_ptr<ThirBlock> then,
           std::unique_ptr<ThirBlock> else_ = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(then)),
          elseBranch(std::move(else_)) {}
};

class ThirWhile : public ThirStmt {
public:
    std::unique_ptr<ThirExpr> condition;
    std::unique_ptr<ThirBlock> body;
    ThirWhile(std::unique_ptr<ThirExpr> cond, std::unique_ptr<ThirBlock> body)
        : condition(std::move(cond)), body(std::move(body)) {}
};

class ThirFor : public ThirStmt {
public:
    HirId varId;
    std::string varName;
    TypeInfo varType;
    std::unique_ptr<ThirExpr> iterable;
    std::unique_ptr<ThirBlock> body;
    ThirFor(HirId id, std::string name, TypeInfo vTy, std::unique_ptr<ThirExpr> iter, std::unique_ptr<ThirBlock> body)
        : varId(id), varName(std::move(name)), varType(vTy), iterable(std::move(iter)), body(std::move(body)) {}
};

/// Heap free: delete ptr
class ThirDeleteStmt : public ThirStmt {
public:
    std::unique_ptr<ThirExpr> pointer;
    explicit ThirDeleteStmt(std::unique_ptr<ThirExpr> ptr) : pointer(std::move(ptr)) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
//  THIR Declarations
// ═══════════════════════════════════════════════════════════════════════════════

struct ThirParam {
    HirId id;
    std::string name;
    TypeInfo typeInfo;
};

class ThirFuncDecl {
public:
    HirId id;
    std::string name;
    std::vector<ThirParam> params;
    TypeInfo returnTypeInfo;
    std::unique_ptr<ThirBlock> body;
    bool isExtern = false;

    ThirFuncDecl(HirId id, std::string name, std::vector<ThirParam> params,
                 TypeInfo retType, std::unique_ptr<ThirBlock> body, bool isExtern = false)
        : id(id), name(std::move(name)), params(std::move(params)),
          returnTypeInfo(retType), body(std::move(body)), isExtern(isExtern) {}
};

struct ThirConstDef {
    HirId id;
    std::string name;
    TypeInfo typeInfo;
    std::unique_ptr<ThirExpr> value;
};

class ThirProgram {
public:
    std::vector<std::unique_ptr<ThirFuncDecl>> functions;
    std::vector<std::unique_ptr<ThirConstDef>> constants;
    // Re-export struct defs so later passes don't need the HIR.
    std::vector<HirStructDef> structs;
    std::vector<HirEnumDef> enums;
};

} // namespace agam
