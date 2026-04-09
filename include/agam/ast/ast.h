#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace agam {

// Forward declarations for visitor
class ASTVisitor;
class Stmt;
class ConstDecl;

// ═══════════════════════════════════════════════════════════════════════════════
//  Base Node
// ═══════════════════════════════════════════════════════════════════════════════

/// Location information for error reporting.
struct SourceLocation {
    std::string filename;
    int line = 0;
    int column = 0;

    SourceLocation() = default;
    SourceLocation(const std::string& f, int l, int c) 
        : filename(f), line(l), column(c) {}
};

/// Base class for all AST nodes.
class ASTNode {
public:
    SourceLocation loc;
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
    virtual std::unique_ptr<ASTNode> clone() const = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Type Representation

class BlockStmt;

enum class TypeKind {
    // Signed integers
    Int8, Int16, Int32, Int64, Int128,
    // Unsigned integers
    UInt8, UInt16, UInt32, UInt64, UInt128,
    // Floating point
    Float32, Float64,
    // Other
    String, Bool, Void, Null, Unknown, Self, Generic,

    // Aliases (Int = Int32, Float = Float64) — for backward compat
    Int = Int32,
    Float = Float64,
};

inline const char *typeKindToString(TypeKind t) {
    switch (t) {
    case TypeKind::Int8:    return "int8";
    case TypeKind::Int16:   return "int16";
    case TypeKind::Int32:   return "int32";
    case TypeKind::Int64:   return "int64";
    case TypeKind::Int128:  return "int128";
    case TypeKind::UInt8:   return "uint8";
    case TypeKind::UInt16:  return "uint16";
    case TypeKind::UInt32:  return "uint32";
    case TypeKind::UInt64:  return "uint64";
    case TypeKind::UInt128: return "uint128";
    case TypeKind::Float32: return "float32";
    case TypeKind::Float64: return "float64";
    case TypeKind::String:  return "string";
    case TypeKind::Bool:    return "bool";
    case TypeKind::Void:    return "void";
    case TypeKind::Unknown: return "unknown";
    case TypeKind::Self:    return "self";
    case TypeKind::Generic: return "generic";
    case TypeKind::Null:    return "null";
    }
    return "unknown";
}

/// True for any signed or unsigned integer type.
inline bool isIntegerType(TypeKind t) {
    switch (t) {
    case TypeKind::Int8:  case TypeKind::Int16:  case TypeKind::Int32:
    case TypeKind::Int64: case TypeKind::Int128:
    case TypeKind::UInt8: case TypeKind::UInt16: case TypeKind::UInt32:
    case TypeKind::UInt64: case TypeKind::UInt128:
        return true;
    default: return false;
    }
}

/// True for unsigned integer types.
inline bool isUnsignedType(TypeKind t) {
    switch (t) {
    case TypeKind::UInt8: case TypeKind::UInt16: case TypeKind::UInt32:
    case TypeKind::UInt64: case TypeKind::UInt128:
        return true;
    default: return false;
    }
}

/// True for float32 or float64.
inline bool isFloatType(TypeKind t) {
    return t == TypeKind::Float32 || t == TypeKind::Float64;
}

/// True for any numeric type (integer or float).
inline bool isNumericType(TypeKind t) {
    return isIntegerType(t) || isFloatType(t);
}

/// Bit width of the type (for integers and floats).
inline int typeSizeInBits(TypeKind t) {
    switch (t) {
    case TypeKind::Bool:    return 1;
    case TypeKind::Int8:    case TypeKind::UInt8:   return 8;
    case TypeKind::Int16:   case TypeKind::UInt16:  return 16;
    case TypeKind::Int32:   case TypeKind::UInt32:  case TypeKind::Float32: return 32;
    case TypeKind::Int64:   case TypeKind::UInt64:  case TypeKind::Float64: return 64;
    case TypeKind::Int128:  case TypeKind::UInt128: return 128;
    default: return 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Compound Type — wraps TypeKind + optional array info
// ═══════════════════════════════════════════════════════════════════════════════

struct TypeInfo {
    TypeKind kind;
    bool isArray = false;
    TypeKind elementType = TypeKind::Unknown;  // only when isArray
    int arraySize = 0;                         // only when isArray
    bool isStruct = false;
    std::string structName;                    // only when isStruct
    bool isEnum = false;
    std::string enumName;                      // only when isEnum
    int pointerDepth = 0;
    bool isSlice = false;
    bool isGeneric = false;                    // Placeholder like T
    std::string genericName;                   // only when isGeneric
    std::vector<TypeInfo> genericArgs;         // for Name<T1, T2>
    std::string internalId;                    // Internal identifier for anonymous types
    std::string pulseTag;                     // ZPM Pulse Tag (e.g., "A" for ~A)
    bool isMutable = false;                   // Pointer/Type mutability (e.g., *mut T vs *T)

    bool operator==(const TypeInfo &other) const {
        return kind == other.kind && isArray == other.isArray &&
               elementType == other.elementType && arraySize == other.arraySize &&
               isStruct == other.isStruct && structName == other.structName &&
               isEnum == other.isEnum && enumName == other.enumName &&
               pointerDepth == other.pointerDepth && isSlice == other.isSlice &&
               isGeneric == other.isGeneric && genericName == other.genericName &&
               genericArgs == other.genericArgs && pulseTag == other.pulseTag &&
               isMutable == other.isMutable;
    }

    bool operator!=(const TypeInfo &other) const { return !(*this == other); }

    bool operator<(const TypeInfo &other) const {
        if (kind != other.kind) return kind < other.kind;
        if (isArray != other.isArray) return isArray < other.isArray;
        if (elementType != other.elementType) return elementType < other.elementType;
        if (arraySize != other.arraySize) return arraySize < other.arraySize;
        if (isStruct != other.isStruct) return isStruct < other.isStruct;
        if (structName != other.structName) return structName < other.structName;
        if (isEnum != other.isEnum) return isEnum < other.isEnum;
        if (enumName != other.enumName) return enumName < other.enumName;
        if (pointerDepth != other.pointerDepth) return pointerDepth < other.pointerDepth;
        if (isSlice != other.isSlice) return isSlice < other.isSlice;
        if (isGeneric != other.isGeneric) return isGeneric < other.isGeneric;
        if (genericName != other.genericName) return genericName < other.genericName;
        if (genericArgs != other.genericArgs) return genericArgs < other.genericArgs;
        if (pulseTag != other.pulseTag) return pulseTag < other.pulseTag;
        return isMutable < other.isMutable;
    }

    /// Returns a TypeInfo representing the element of this array, slice, or pointer.
    TypeInfo getElementTypeInfo() const {
        if (pointerDepth > 0) {
            TypeInfo ti = *this;
            ti.pointerDepth--;
            return ti;
        }
        
        TypeInfo ti;
        ti.kind = elementType;
        ti.isGeneric = isGeneric;
        ti.genericName = genericName;
        ti.isStruct = isStruct;
        ti.structName = structName;
        ti.isEnum = isEnum;
        ti.enumName = enumName;
        ti.genericArgs = genericArgs;
        ti.pointerDepth = pointerDepth; // preserve the base pointer depth of the element
        ti.pulseTag = pulseTag;
        return ti;
    }

    /// Create a scalar (non-array) type.
    static TypeInfo scalar(TypeKind k) { return {k, false, TypeKind::Unknown, 0, false, "", false, "", 0}; }
    /// Create a fixed-size array type: [elementType; size]
    static TypeInfo array(TypeKind elem, int size) {
        return array(scalar(elem), size);
    }
    static TypeInfo array(const TypeInfo &elem, int size) {
        TypeInfo ti;
        ti.kind = TypeKind::Unknown;
        ti.isArray = true;
        ti.elementType = elem.kind;
        ti.arraySize = size;
        ti.isGeneric = elem.isGeneric;
        ti.genericName = elem.genericName;
        ti.isStruct = elem.isStruct;
        ti.structName = elem.structName;
        ti.isEnum = elem.isEnum;
        ti.enumName = elem.enumName;
        ti.genericArgs = elem.genericArgs;
        ti.pointerDepth = elem.pointerDepth;
        return ti;
    }
    /// Create a named struct type.
    static TypeInfo namedStruct(std::string name) {
        TypeInfo ti;
        ti.kind = TypeKind::Unknown;
        ti.isStruct = true;
        ti.structName = std::move(name);
        ti.pointerDepth = 0;
        return ti;
    }
    /// Create a named enum type.
    static TypeInfo namedEnum(std::string name) {
        TypeInfo ti;
        ti.kind = TypeKind::Unknown;
        ti.isEnum = true;
        ti.enumName = std::move(name);
        ti.pointerDepth = 0;
        return ti;
    }
    /// Create a slice type: &[T]
    static TypeInfo slice(TypeKind elem) {
        return slice(scalar(elem));
    }
    static TypeInfo slice(const TypeInfo &elem) {
        TypeInfo ti;
        ti.kind = TypeKind::Unknown;
        ti.isSlice = true;
        ti.elementType = elem.kind;
        ti.isGeneric = elem.isGeneric;
        ti.genericName = elem.genericName;
        ti.isStruct = elem.isStruct;
        ti.structName = elem.structName;
        ti.isEnum = elem.isEnum;
        ti.enumName = elem.enumName;
        ti.genericArgs = elem.genericArgs;
        ti.pointerDepth = elem.pointerDepth;
        return ti;
    }
    /// Create a generic placeholder type (e.g. T)
    static TypeInfo generic(std::string name) {
        TypeInfo ti = scalar(TypeKind::Generic);
        ti.isGeneric = true;
        ti.genericName = std::move(name);
        return ti;
    }


};

inline std::string normalizeTypeName(const std::string &name) {
    size_t undPos = name.find('_');
    if (undPos == std::string::npos) return name;
    
    std::string base = name.substr(0, undPos);
    std::string suffix = name.substr(undPos + 1);
    std::string result = base + "<";
    
    size_t start = 0;
    size_t end = suffix.find('_');
    while (start < suffix.length()) {
        std::string typeName = suffix.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        result += normalizeTypeName(typeName);
        if (end == std::string::npos) break;
        result += ", ";
        start = end + 1;
        end = suffix.find('_', start);
    }
    result += ">";
    return result;
}

inline std::string typeInfoToString(const TypeInfo &t) {
    if (t.kind == TypeKind::Unknown && !t.isArray && !t.isSlice && !t.isStruct && !t.isEnum && !t.isGeneric) {
        return "Unknown";
    }

    std::string prefix = "";
    for (int i = 0; i < t.pointerDepth; ++i) prefix += "*";
    
    if (t.isSlice) {
        TypeInfo elemTI;
        elemTI.kind = t.elementType;
        // In simple cases where elementType belongs to a complex type that wasn't fully nested, 
        // this might still show 'Struct', but we try our best.
        if (t.elementType == TypeKind::Unknown) {
             return prefix + "&[" + (t.isGeneric ? t.genericName : normalizeTypeName(t.structName)) + "]";
        }
        return prefix + "&[" + typeInfoToString(elemTI) + "]";
    }
    if (t.isArray) {
        TypeInfo elemTI;
        elemTI.kind = t.elementType;
        return prefix + "[" + typeInfoToString(elemTI) + "; " + std::to_string(t.arraySize) + "]";
    }
    if (t.isStruct) {
        prefix += normalizeTypeName(t.structName);
    } else if (t.isEnum) {
        prefix += t.enumName;
    } else {
        prefix += typeKindToString(t.kind);
    }
    if (t.isGeneric) {
        prefix = t.genericName; // Overwrite prefix for simple T
    }
    if (!t.genericArgs.empty()) {
        prefix += "<";
        for (size_t i = 0; i < t.genericArgs.size(); ++i) {
            if (i > 0) prefix += ", ";
            prefix += typeInfoToString(t.genericArgs[i]);
        }
        prefix += ">";
    }
    if (!t.pulseTag.empty()) {
        prefix += "~" + t.pulseTag;
    }
    return prefix;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expressions
// ═══════════════════════════════════════════════════════════════════════════════

/// Base class for all expression nodes.
class Expr : public ASTNode {
public:
    virtual std::unique_ptr<ASTNode> clone() const override = 0;
};

/// Integer literal: 42
class IntLiteralExpr : public Expr {
public:
    int64_t value;
    explicit IntLiteralExpr(int64_t val) : value(val) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Float literal: 3.14
class FloatLiteralExpr : public Expr {
public:
    double value;
    explicit FloatLiteralExpr(double val) : value(val) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// String literal: "hello"
class StringLiteralExpr : public Expr {
public:
    std::string value;
    explicit StringLiteralExpr(std::string val) : value(std::move(val)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Boolean literal: true / false
class BoolLiteralExpr : public Expr {
public:
    bool value;
    explicit BoolLiteralExpr(bool val) : value(val) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Null pointer literal: nil
class NullLiteralExpr : public Expr {
public:
    NullLiteralExpr() {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Variable reference: x
class VariableExpr : public Expr {
public:
    std::string name;
    std::vector<TypeInfo> genericArgs;

    explicit VariableExpr(std::string name, std::vector<TypeInfo> genArgs = {})
        : name(std::move(name)), genericArgs(std::move(genArgs)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Binary operation: a + b, x == y
enum class BinaryOp {
    Add, Sub, Mul, Div, Mod,
    Eq, Neq, Lt, Gt, Lte, Gte,
    And, Or
};

inline const char *binaryOpToString(BinaryOp op) {
    switch (op) {
    case BinaryOp::Add: return "+";
    case BinaryOp::Sub: return "-";
    case BinaryOp::Mul: return "*";
    case BinaryOp::Div:  return "/";
    case BinaryOp::Mod:  return "%";
    case BinaryOp::Eq:   return "==";
    case BinaryOp::Neq: return "!=";
    case BinaryOp::Lt:  return "<";
    case BinaryOp::Gt:  return ">";
    case BinaryOp::Lte: return "<=";
    case BinaryOp::Gte: return ">=";
    case BinaryOp::And: return "&&";
    case BinaryOp::Or:  return "||";
    }
    return "?";
}

class BinaryExpr : public Expr {
public:
    BinaryOp op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Unary operation: !x, -x, &x, *x
enum class UnaryOp { Negate, Not, AddressOf, Dereference };

class UnaryExpr : public Expr {
public:
    UnaryOp op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand)
        : op(op), operand(std::move(operand)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Dereference assignment: *ptr = 42
class DerefAssignExpr : public Expr {
public:
    std::unique_ptr<Expr> pointer;
    std::unique_ptr<Expr> value;

    DerefAssignExpr(std::unique_ptr<Expr> ptr, std::unique_ptr<Expr> val)
        : pointer(std::move(ptr)), value(std::move(val)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Function call: add(1, 2)
class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    std::vector<TypeInfo> genericArgs;

    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args, 
             std::vector<TypeInfo> genArgs = {})
        : callee(std::move(callee)), args(std::move(args)), genericArgs(std::move(genArgs)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Method call: obj.print()
class MethodCallExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::string methodName;
    std::vector<std::unique_ptr<Expr>> args;
    std::vector<TypeInfo> genericArgs;
    std::string resolvedMangledName; // Set during type checking

    MethodCallExpr(std::unique_ptr<Expr> base, std::string methodName, 
                   std::vector<std::unique_ptr<Expr>> args, std::vector<TypeInfo> genArgs = {})
        : base(std::move(base)), methodName(std::move(methodName)), 
          args(std::move(args)), genericArgs(std::move(genArgs)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Assignment expression: x = 5
class AssignExpr : public Expr {
public:
    std::string name;
    std::unique_ptr<Expr> value;

    AssignExpr(std::string name, std::unique_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Array literal: [1, 2, 3]
class ArrayLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    explicit ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elems)
        : elements(std::move(elems)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Index expression: arr[i]
class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> base;   // the array
    std::unique_ptr<Expr> index;  // the index (or start of range)
    std::unique_ptr<Expr> endIndex; // end of range (nullable)
    bool isRange = false;

    IndexExpr(std::unique_ptr<Expr> base, std::unique_ptr<Expr> idx, std::unique_ptr<Expr> end = nullptr, bool range = false)
        : base(std::move(base)), index(std::move(idx)), endIndex(std::move(end)), isRange(range) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Index assignment: arr[i] = value
class IndexAssignExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::unique_ptr<Expr> index;
    std::unique_ptr<Expr> value;
    IndexAssignExpr(std::unique_ptr<Expr> base, std::unique_ptr<Expr> idx, std::unique_ptr<Expr> val)
        : base(std::move(base)), index(std::move(idx)), value(std::move(val)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// A single field name + value in a struct literal: field: expr
struct StructFieldInit {
    std::string name;
    std::unique_ptr<Expr> value;
};

/// Struct literal: Point { x: 10, y: 20 }
class StructLiteralExpr : public Expr {
public:
    TypeInfo structType;
    std::vector<StructFieldInit> fields;
    StructLiteralExpr(TypeInfo type, std::vector<StructFieldInit> fields)
        : structType(std::move(type)), fields(std::move(fields)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Field access: expr.field
class FieldAccessExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::string field;
    FieldAccessExpr(std::unique_ptr<Expr> base, std::string field)
        : base(std::move(base)), field(std::move(field)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Field assignment: expr.field = value
class FieldAssignExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::string field;
    std::unique_ptr<Expr> value;
    FieldAssignExpr(std::unique_ptr<Expr> base, std::string field, std::unique_ptr<Expr> value)
        : base(std::move(base)), field(std::move(field)), value(std::move(value)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

// ── Enum Variant Expression ───────────────────────────────────────────

class EnumVariantExpr : public Expr {
public:
    std::string enumName;
    std::string variantName;
    std::vector<TypeInfo> genericArgs;
    std::unique_ptr<Expr> payload; // optional
    EnumVariantExpr(std::string enumName, std::string variantName, 
                    std::vector<TypeInfo> genArgs = {}, std::unique_ptr<Expr> payload = nullptr)
        : enumName(std::move(enumName)), variantName(std::move(variantName)), 
          genericArgs(std::move(genArgs)), payload(std::move(payload)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

// ── Match Expression ──────────────────────────────────────────────────

struct MatchArm {
    std::string variantName;
    std::string bindingName; // optional payload binding
    bool hasBinding = false;
    std::unique_ptr<ASTNode> body; // Expression or Block
};

class MatchExpr : public Expr {
public:
    std::unique_ptr<Expr> value;
    std::vector<MatchArm> arms;
    MatchExpr(std::unique_ptr<Expr> value, std::vector<MatchArm> arms)
        : value(std::move(value)), arms(std::move(arms)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Cast expression: expr as Type
class CastExpr : public Expr {
public:
    std::unique_ptr<Expr> expr;
    TypeInfo targetType;
    CastExpr(std::unique_ptr<Expr> e, TypeInfo t)
        : expr(std::move(e)), targetType(std::move(t)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Heap allocation: new T
class NewExpr : public Expr {
public:
    TypeInfo allocatedType;
    std::unique_ptr<Expr> sizeExpr; // for dynamic arrays: new [T; n]

    explicit NewExpr(TypeInfo ti, std::unique_ptr<Expr> size = nullptr) 
        : allocatedType(std::move(ti)), sizeExpr(std::move(size)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// ZPM Zone block: zone A { ... }
class ZoneExpr : public Expr {
public:
    std::string zoneName;
    std::unique_ptr<BlockStmt> body;
    ZoneExpr(std::string name, std::unique_ptr<BlockStmt> body)
        : zoneName(std::move(name)), body(std::move(body)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// ZPM Allocation: alloc<T>(n)
class AllocExpr : public Expr {
public:
    TypeInfo type;
    std::unique_ptr<Expr> count;
    AllocExpr(TypeInfo ti, std::unique_ptr<Expr> cnt = nullptr)
        : type(std::move(ti)), count(std::move(cnt)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// ZPM Borrow: borrow shared/mut(x) as view
class BorrowExpr : public Expr {
public:
    bool isMutable;
    std::unique_ptr<Expr> target;
    BorrowExpr(bool mut, std::unique_ptr<Expr> tgt)
        : isMutable(mut), target(std::move(tgt)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// ZPM Escape: escape(x) -> A
class EscapeExpr : public Expr {
public:
    std::unique_ptr<Expr> target;
    std::string destinationZone;
    EscapeExpr(std::unique_ptr<Expr> tgt, std::string dest)
        : target(std::move(tgt)), destinationZone(std::move(dest)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Statements
// ═══════════════════════════════════════════════════════════════════════════════

/// Base class for all statement nodes.
class Stmt : public ASTNode {
public:
    virtual std::unique_ptr<ASTNode> clone() const override = 0;
};

/// Expression statement: expr;
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Variable declaration: let x: int = 5;
class VarDeclStmt : public Stmt {
public:
    std::string name;
    TypeInfo typeInfo;
    std::unique_ptr<Expr> initializer; // may be null
    bool isMutable = false;

    VarDeclStmt(std::string name, TypeInfo ti, std::unique_ptr<Expr> init = nullptr, bool isMut = false)
        : name(std::move(name)), typeInfo(std::move(ti)), initializer(std::move(init)), isMutable(isMut) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Block statement: { stmt1; stmt2; }
class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Return statement: return expr;
class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value; // may be null for void return
    explicit ReturnStmt(std::unique_ptr<Expr> value = nullptr)
        : value(std::move(value)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// If statement: if (cond) { ... } else { ... }
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // may be null

    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenBr,
           std::unique_ptr<Stmt> elseBr = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(thenBr)),
          elseBranch(std::move(elseBr)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// While statement: while (cond) { ... }
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body)
        : condition(std::move(cond)), body(std::move(body)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// For statement: for x in iterable { ... }
class ForStmt : public Stmt {
public:
    std::string varName;
    std::unique_ptr<Expr> iterable;
    std::unique_ptr<Stmt> body;

    ForStmt(std::string var, std::unique_ptr<Expr> iter, std::unique_ptr<Stmt> body)
        : varName(std::move(var)), iterable(std::move(iter)), body(std::move(body)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Delete statement: delete ptr;
class DeleteStmt : public Stmt {
public:
    std::unique_ptr<Expr> pointer;
    explicit DeleteStmt(std::unique_ptr<Expr> ptr) : pointer(std::move(ptr)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Declarations
// ═══════════════════════════════════════════════════════════════════════════════

/// Function parameter: name: type
struct Param {
    std::string name;
    TypeInfo typeInfo;
    bool isMutable = false;
};

/// Function declaration: func add(a: int, b: int): int { ... }
class FunctionDecl : public ASTNode {
public:
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<Param> params;
    TypeInfo returnTypeInfo;
    std::unique_ptr<BlockStmt> body;
    bool isExtern = false;
    bool isMutating = false;

    FunctionDecl(std::string name, std::vector<std::string> typeParams, std::vector<Param> params, TypeInfo retType,
                 std::unique_ptr<BlockStmt> body, bool isExtern = false, bool isMut = false)
        : name(std::move(name)), typeParams(std::move(typeParams)), params(std::move(params)), returnTypeInfo(std::move(retType)),
          body(std::move(body)), isExtern(isExtern), isMutating(isMut) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Constant declaration: const x: int = 5;
class ConstDecl : public ASTNode {
public:
    std::string name;
    TypeInfo typeInfo;
    std::unique_ptr<Expr> value;

    ConstDecl(std::string name, TypeInfo ti, std::unique_ptr<Expr> val)
        : name(std::move(name)), typeInfo(std::move(ti)), value(std::move(val)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// A field in a struct declaration: name: type
struct StructField {
    std::string name;
    TypeInfo typeInfo;
    bool isMutable = false;
};

/// Struct declaration: struct Point { x: int, y: int }
class StructDecl : public ASTNode {
public:
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<StructField> fields;
    StructDecl(std::string name, std::vector<std::string> typeParams, std::vector<StructField> fields)
        : name(std::move(name)), typeParams(std::move(typeParams)), fields(std::move(fields)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Enum Variant: VariantName(Type) or VariantName
struct EnumVariant {
    std::string name;
    bool hasPayload = false;
    TypeInfo payloadType = TypeInfo::scalar(TypeKind::Unknown);
};

/// Enum declaration: enum OptionInt { Some(int), None }
class EnumDecl : public ASTNode {
public:
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<EnumVariant> variants;
    EnumDecl(std::string name, std::vector<std::string> typeParams, std::vector<EnumVariant> variants)
        : name(std::move(name)), typeParams(std::move(typeParams)), variants(std::move(variants)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Trait declaration: trait Printable { func print(); }
class TraitDecl : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    TraitDecl(std::string name, std::vector<std::unique_ptr<FunctionDecl>> methods)
        : name(std::move(name)), methods(std::move(methods)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Implementation block: impl Printable for int { func print() { ... } }
class ImplDecl : public ASTNode {
public:
    std::string traitName;
    std::vector<std::string> typeParams;
    TypeInfo targetType;
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    ImplDecl(std::string traitName, std::vector<std::string> typeParams, TypeInfo targetType, std::vector<std::unique_ptr<FunctionDecl>> methods)
        : traitName(std::move(traitName)), typeParams(std::move(typeParams)), targetType(targetType), methods(std::move(methods)) {}
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

/// Top-level program.
class Program : public ASTNode {
public:
    std::vector<std::string> imports;
    std::vector<std::unique_ptr<ConstDecl>> constants;
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    std::vector<std::unique_ptr<StructDecl>> structs;
    std::vector<std::unique_ptr<EnumDecl>> enums;
    std::vector<std::unique_ptr<TraitDecl>> traits;
    std::vector<std::unique_ptr<ImplDecl>> impls;
    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Visitor Interface
// ═══════════════════════════════════════════════════════════════════════════════

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Expressions
    virtual void visit(IntLiteralExpr &node) = 0;
    virtual void visit(FloatLiteralExpr &node) = 0;
    virtual void visit(StringLiteralExpr &node) = 0;
    virtual void visit(BoolLiteralExpr &node) = 0;
    virtual void visit(NullLiteralExpr &node) = 0;
    virtual void visit(VariableExpr &node) = 0;
    virtual void visit(BinaryExpr &node) = 0;
    virtual void visit(UnaryExpr &node) = 0;
    virtual void visit(CallExpr &node) = 0;
    virtual void visit(MethodCallExpr &node) = 0;
    virtual void visit(AssignExpr &node) = 0;
    virtual void visit(ArrayLiteralExpr &node) = 0;
    virtual void visit(IndexExpr &node) = 0;
    virtual void visit(IndexAssignExpr &node) = 0;
    virtual void visit(StructLiteralExpr &node) = 0;
    virtual void visit(FieldAccessExpr &node) = 0;
    virtual void visit(FieldAssignExpr &node) = 0;
    virtual void visit(EnumVariantExpr &node) = 0;
    virtual void visit(MatchExpr &node) = 0;
    virtual void visit(DerefAssignExpr &node) = 0;
    virtual void visit(CastExpr &node) = 0;
    virtual void visit(NewExpr &node) = 0;
    virtual void visit(ZoneExpr &node) {}
    virtual void visit(AllocExpr &node) {}
    virtual void visit(BorrowExpr &node) {}
    virtual void visit(EscapeExpr &node) {}

    // Statements
    virtual void visit(ExprStmt &node) = 0;
    virtual void visit(VarDeclStmt &node) = 0;
    virtual void visit(BlockStmt &node) = 0;
    virtual void visit(ReturnStmt &node) = 0;
    virtual void visit(IfStmt &node) = 0;
    virtual void visit(WhileStmt &node) = 0;
    virtual void visit(ForStmt &node) = 0;
    virtual void visit(DeleteStmt &node) = 0;

    // Declarations
    virtual void visit(FunctionDecl &node) = 0;
    virtual void visit(ConstDecl &node) = 0;
    virtual void visit(StructDecl &node) = 0;
    virtual void visit(EnumDecl &node) = 0;
    virtual void visit(TraitDecl &node) = 0;
    virtual void visit(ImplDecl &node) = 0;
    virtual void visit(Program &node) = 0;
};

} // namespace agam
