#pragma once

#include "agam/ast/ast.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace agam {

// ═══════════════════════════════════════════════════════════════════════════════
//  HIR ID — unique identifier for every definition
// ═══════════════════════════════════════════════════════════════════════════════

/// Every variable, function, and parameter gets a unique HirId.
using HirId = uint32_t;
constexpr HirId INVALID_HIR_ID = 0;

// ═══════════════════════════════════════════════════════════════════════════════
//  HIR Expressions
// ═══════════════════════════════════════════════════════════════════════════════

class HirBlock;

/// Base class for HIR expressions.
class HirExpr {
public:
    SourceLocation loc;
    virtual ~HirExpr() = default;
};

/// Integer literal: 42
class HirIntLiteral : public HirExpr {
public:
    int64_t value;
    explicit HirIntLiteral(int64_t v) : value(v) {}
};

/// Float literal: 3.14
class HirFloatLiteral : public HirExpr {
public:
    double value;
    explicit HirFloatLiteral(double v) : value(v) {}
};

/// String literal: "hello"
class HirStringLiteral : public HirExpr {
public:
    std::string value;
    explicit HirStringLiteral(std::string v) : value(std::move(v)) {}
};

/// Bool literal: true / false
class HirBoolLiteral : public HirExpr {
public:
    bool value;
    explicit HirBoolLiteral(bool v) : value(v) {}
};

/// Null literal: nil
class HirNullLiteral : public HirExpr {
public:
    HirNullLiteral() {}
};

/// Variable reference — resolved to HirId of its declaration.
class HirVarRef : public HirExpr {
public:
    HirId defId;       // points to the VarDecl or Param that defined this var
    std::string name;  // kept for diagnostics/printing
    HirVarRef(HirId id, std::string name) : defId(id), name(std::move(name)) {}
};

/// Binary operation
class HirBinaryExpr : public HirExpr {
public:
    BinaryOp op;
    std::unique_ptr<HirExpr> lhs;
    std::unique_ptr<HirExpr> rhs;
    HirBinaryExpr(BinaryOp op, std::unique_ptr<HirExpr> l, std::unique_ptr<HirExpr> r)
        : op(op), lhs(std::move(l)), rhs(std::move(r)) {}
};

/// Unary operation
class HirUnaryExpr : public HirExpr {
public:
    UnaryOp op;
    std::unique_ptr<HirExpr> operand;
    HirUnaryExpr(UnaryOp op, std::unique_ptr<HirExpr> operand)
        : op(op), operand(std::move(operand)) {}
};

/// Function call — callee resolved to HirId.
class HirCallExpr : public HirExpr {
public:
    HirId calleeId;
    std::string calleeName;  // for diagnostics
    std::vector<std::unique_ptr<HirExpr>> args;
    HirCallExpr(HirId id, std::string name, std::vector<std::unique_ptr<HirExpr>> args)
        : calleeId(id), calleeName(std::move(name)), args(std::move(args)) {}
};

/// Assignment: x = expr  (target resolved to HirId)
class HirAssignExpr : public HirExpr {
public:
    HirId targetId;
    std::string targetName;
    std::unique_ptr<HirExpr> value;
    HirAssignExpr(HirId id, std::string name, std::unique_ptr<HirExpr> val)
        : targetId(id), targetName(std::move(name)), value(std::move(val)) {}
};

/// Array literal: [1, 2, 3]
class HirArrayLiteral : public HirExpr {
public:
    std::vector<std::unique_ptr<HirExpr>> elements;
    explicit HirArrayLiteral(std::vector<std::unique_ptr<HirExpr>> elements)
        : elements(std::move(elements)) {}
};

/// Array indexing: arr[index]
class HirIndexExpr : public HirExpr {
public:
    std::unique_ptr<HirExpr> base;
    std::unique_ptr<HirExpr> index;
    std::unique_ptr<HirExpr> endIndex; // nullable
    bool isRange = false;

    HirIndexExpr(std::unique_ptr<HirExpr> base, std::unique_ptr<HirExpr> index, std::unique_ptr<HirExpr> end = nullptr, bool range = false)
        : base(std::move(base)), index(std::move(index)), endIndex(std::move(end)), isRange(range) {}
};

/// Array index assignment: arr[index] = value
class HirIndexAssign : public HirExpr {
public:
    std::unique_ptr<HirExpr> base;
    std::unique_ptr<HirExpr> index;
    std::unique_ptr<HirExpr> value;
    HirIndexAssign(std::unique_ptr<HirExpr> base, std::unique_ptr<HirExpr> index, std::unique_ptr<HirExpr> value)
        : base(std::move(base)), index(std::move(index)), value(std::move(value)) {}
};

/// Single field name + expr for a struct literal.
struct HirStructFieldInit {
    std::string name;
    std::unique_ptr<HirExpr> value;
};

/// Struct constructor literal: Point { x: 10, y: 20 }
class HirStructLiteral : public HirExpr {
public:
    std::string structName;
    std::vector<HirStructFieldInit> fields;
    HirStructLiteral(std::string name, std::vector<HirStructFieldInit> fields)
        : structName(std::move(name)), fields(std::move(fields)) {}
};

/// Field read: expr.fieldName
class HirFieldAccess : public HirExpr {
public:
    std::unique_ptr<HirExpr> base;
    std::string field;
    HirFieldAccess(std::unique_ptr<HirExpr> base, std::string field)
        : base(std::move(base)), field(std::move(field)) {}
};

class HirFieldAssign : public HirExpr {
public:
    std::unique_ptr<HirExpr> base;
    std::string field;
    std::unique_ptr<HirExpr> value;
    HirFieldAssign(std::unique_ptr<HirExpr> base, std::string field, std::unique_ptr<HirExpr> value)
        : base(std::move(base)), field(std::move(field)), value(std::move(value)) {}
};

/// Pointer dereference assignment: *ptr = value
class HirDerefAssign : public HirExpr {
public:
    std::unique_ptr<HirExpr> pointer;
    std::unique_ptr<HirExpr> value;
    HirDerefAssign(std::unique_ptr<HirExpr> ptr, std::unique_ptr<HirExpr> val)
        : pointer(std::move(ptr)), value(std::move(val)) {}
};

class HirEnumVariantExpr : public HirExpr {
public:
    std::string enumName;
    std::string variantName;
    std::unique_ptr<HirExpr> payload; // may be null
    HirEnumVariantExpr(std::string e, std::string v, std::unique_ptr<HirExpr> p)
        : enumName(std::move(e)), variantName(std::move(v)), payload(std::move(p)) {}
};

/// Explicit cast: expr as Type
class HirCastExpr : public HirExpr {
public:
    std::unique_ptr<HirExpr> expr;
    TypeInfo targetType;
    HirCastExpr(std::unique_ptr<HirExpr> e, TypeInfo t)
        : expr(std::move(e)), targetType(std::move(t)) {}
};

/// Heap allocation: new T
class HirNewExpr : public HirExpr {
public:
    TypeInfo allocatedType;
    std::unique_ptr<HirExpr> sizeExpr; // for dynamic arrays

    explicit HirNewExpr(TypeInfo ti, std::unique_ptr<HirExpr> size = nullptr)
        : allocatedType(std::move(ti)), sizeExpr(std::move(size)) {}
};

/// ZPM Zone block: zone A { ... }
class HirZoneExpr : public HirExpr {
public:
    std::string zoneName;
    std::unique_ptr<HirBlock> body;
    HirZoneExpr(std::string name, std::unique_ptr<HirBlock> body)
        : zoneName(std::move(name)), body(std::move(body)) {}
};

/// ZPM Allocation: alloc<T>(size)~A
class HirAllocExpr : public HirExpr {
public:
    TypeInfo type;
    std::unique_ptr<HirExpr> count;
    std::string zoneName;
    HirAllocExpr(TypeInfo ti, std::unique_ptr<HirExpr> c, std::string z)
        : type(std::move(ti)), count(std::move(c)), zoneName(std::move(z)) {}
};

/// ZPM Borrow: borrow shared/mut (target)
class HirBorrowExpr : public HirExpr {
public:
    bool isMutable;
    std::unique_ptr<HirExpr> target;
    HirBorrowExpr(bool mut, std::unique_ptr<HirExpr> t)
        : isMutable(mut), target(std::move(t)) {}
};

/// ZPM Escape: escape(target) -> destination
class HirEscapeExpr : public HirExpr {
public:
    std::unique_ptr<HirExpr> target;
    std::string destinationZone;
    HirEscapeExpr(std::unique_ptr<HirExpr> t, std::string dest)
        : target(std::move(t)), destinationZone(std::move(dest)) {}
};


// ═══════════════════════════════════════════════════════════════════════════════
//  HIR Statements
// ═══════════════════════════════════════════════════════════════════════════════

class HirStmt {
public:
    SourceLocation loc;
    virtual ~HirStmt() = default;
};

/// Expression statement
class HirExprStmt : public HirStmt {
public:
    std::unique_ptr<HirExpr> expr;
    explicit HirExprStmt(std::unique_ptr<HirExpr> e) : expr(std::move(e)) {}
};

/// Variable declaration — gets its own HirId.
class HirVarDecl : public HirStmt {
public:
    HirId id;
    std::string name;
    TypeInfo typeInfo;
    std::unique_ptr<HirExpr> initializer;  // may be null
    HirVarDecl(HirId id, std::string name, TypeInfo typeInfo, std::unique_ptr<HirExpr> init = nullptr)
        : id(id), name(std::move(name)), typeInfo(typeInfo), initializer(std::move(init)) {}
};

/// Block: { stmts }
class HirBlock : public HirStmt {
public:
    std::vector<std::unique_ptr<HirStmt>> stmts;
    explicit HirBlock(std::vector<std::unique_ptr<HirStmt>> s) : stmts(std::move(s)) {}
};

struct HirMatchArm {
    std::string variantName;
    bool hasBinding = false;
    HirId bindingId = INVALID_HIR_ID;
    std::string bindingName;
    TypeInfo bindingType;
    std::unique_ptr<HirExpr> exprBody;   // If it's a single expression
    std::unique_ptr<HirBlock> blockBody; // If it's a block
};

class HirMatchExpr : public HirExpr {
public:
    std::unique_ptr<HirExpr> value;
    std::vector<HirMatchArm> arms;
    HirMatchExpr(std::unique_ptr<HirExpr> val, std::vector<HirMatchArm> arms)
        : value(std::move(val)), arms(std::move(arms)) {}
};

/// Return statement
class HirReturn : public HirStmt {
public:
    std::unique_ptr<HirExpr> value;  // may be null
    explicit HirReturn(std::unique_ptr<HirExpr> v = nullptr) : value(std::move(v)) {}
};

/// If statement
class HirIf : public HirStmt {
public:
    std::unique_ptr<HirExpr> condition;
    std::unique_ptr<HirBlock> thenBranch;
    std::unique_ptr<HirBlock> elseBranch;  // may be null
    HirIf(std::unique_ptr<HirExpr> cond, std::unique_ptr<HirBlock> then,
          std::unique_ptr<HirBlock> else_ = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(then)),
          elseBranch(std::move(else_)) {}
};

/// While loop
class HirWhile : public HirStmt {
public:
    std::unique_ptr<HirExpr> condition;
    std::unique_ptr<HirBlock> body;
    HirWhile(std::unique_ptr<HirExpr> cond, std::unique_ptr<HirBlock> body)
        : condition(std::move(cond)), body(std::move(body)) {}
};

/// For loop: for x in iterable { body }
class HirFor : public HirStmt {
public:
    HirId varId;
    std::string varName;
    std::unique_ptr<HirExpr> iterable;
    std::unique_ptr<HirBlock> body;
    HirFor(HirId id, std::string name, std::unique_ptr<HirExpr> iter, std::unique_ptr<HirBlock> body)
        : varId(id), varName(std::move(name)), iterable(std::move(iter)), body(std::move(body)) {}
};

/// Heap free: delete ptr
class HirDeleteStmt : public HirStmt {
public:
    std::unique_ptr<HirExpr> pointer;
    explicit HirDeleteStmt(std::unique_ptr<HirExpr> ptr) : pointer(std::move(ptr)) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
//  HIR Declarations
// ═══════════════════════════════════════════════════════════════════════════════

/// Function parameter with HirId
struct HirParam {
    HirId id;
    std::string name;
    TypeInfo typeInfo;
};

/// Function declaration
class HirFuncDecl {
public:
    HirId id;
    std::string name;
    std::vector<HirParam> params;
    TypeInfo returnTypeInfo;
    std::unique_ptr<HirBlock> body;
    bool isExtern = false;

    HirFuncDecl(HirId id, std::string name, std::vector<HirParam> params,
                TypeInfo retType, std::unique_ptr<HirBlock> body, bool isExtern = false)
        : id(id), name(std::move(name)), params(std::move(params)),
          returnTypeInfo(retType), body(std::move(body)), isExtern(isExtern) {}
};

/// A resolved struct definition (stored in HIR for type queries).
struct HirStructField {
    std::string name;
    TypeInfo typeInfo;
};

struct HirStructDef {
    std::string name;
    std::vector<HirStructField> fields;
    /// Returns field index, or -1 if not found.
    int fieldIndex(const std::string &fname) const {
        for (int i = 0; i < (int)fields.size(); ++i)
            if (fields[i].name == fname) return i;
        return -1;
    }
};

struct HirEnumVariantDef {
    std::string name;
    bool hasPayload;
    TypeInfo payloadType;
};

struct HirEnumDef {
    std::string name;
    std::vector<HirEnumVariantDef> variants;
    
    int variantIndex(const std::string &vname) const {
        for (int i = 0; i < (int)variants.size(); ++i)
            if (variants[i].name == vname) return i;
        return -1;
    }
};

struct HirConstDef {
    HirId id;
    std::string name;
    TypeInfo typeInfo;
    std::unique_ptr<HirExpr> value;
};

/// Top-level HIR program
class HirProgram {
public:
    std::vector<std::unique_ptr<HirFuncDecl>> functions;
    std::vector<std::unique_ptr<HirConstDef>> constants;
    std::vector<HirStructDef> structs;
    std::vector<HirEnumDef> enums;

    /// Find a struct def by name.
    const HirStructDef *findStruct(const std::string &name) const {
        for (auto &s : structs)
            if (s.name == name) return &s;
        return nullptr;
    }

    /// Find an enum def by name.
    const HirEnumDef *findEnum(const std::string &name) const {
        for (auto &e : enums)
            if (e.name == name) return &e;
        return nullptr;
    }
};

} // namespace agam
