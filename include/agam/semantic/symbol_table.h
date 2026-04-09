#pragma once

#include "agam/ast/ast.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace agam {

/// Information stored for each symbol.
struct SymbolInfo {
    std::string name;
    TypeInfo typeInfo;
    bool isFunction = false;
    bool isMutable = false;
    bool isConstant = false;
    std::vector<std::string> typeParams; // For generic functions
    std::vector<TypeInfo> paramTypes; // Only for functions
    TypeInfo returnType = TypeInfo::scalar(TypeKind::Void); // Only for functions
    bool isMutating = false; // Only for methods (receiver mutability)

    // Borrow tracking
    int sharedBorrowCount = 0;
    bool mutablyBorrowed = false;
};

struct TraitInfo {
    std::string name;
    std::vector<SymbolInfo> methods;
};

struct ImplInfo {
    std::string traitName;
    TypeInfo targetType;
    std::vector<SymbolInfo> methods;
};

struct StructInfo {
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<SymbolInfo> fields;
};

struct VariantInfo {
    std::string name;
    bool hasPayload;
    TypeInfo payloadType;
};

struct EnumInfo {
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<VariantInfo> variants;
};

/// Scoped symbol table supporting nested lexical scopes.
class SymbolTable {
public:
    SymbolTable(); // Ensure default constructor
    /// Enter a new scope (e.g., function body, block).
    void enterScope();

    /// Exit the current scope.
    void exitScope();
 
    /// Add a borrow to the current scope.
    void addBorrow(const std::string &name, bool isMutable);
 
    /// Remove a borrow from the current scope.
    void removeBorrow(const std::string &name, bool isMutable);
 
    /// Helper to get current borrows for cleanup
    void releaseBorrows();

    /// Declare a symbol in the current scope.
    /// Returns false if the symbol already exists in the current scope.
    bool declare(const std::string &name, const SymbolInfo &info);

    /// Look up a symbol, searching from innermost to outermost scope.
    /// Returns nullopt if not found.
    std::optional<SymbolInfo> lookup(const std::string &name) const;

    /// Look up a symbol and return a pointer to it for modification.
    /// Returns nullptr if not found.
    SymbolInfo* lookupMutable(const std::string &name);

    /// Look up a symbol only in the current (innermost) scope.
    std::optional<SymbolInfo> lookupCurrent(const std::string &name) const;

    /// Get the current scope depth (0 = global).
    size_t depth() const { return scopes_.size(); }

    // Trait Management
    void declareTrait(const std::string &name, const TraitInfo &info);
    void declareImpl(const ImplInfo &info);
    const TraitInfo* lookupTrait(const std::string &name) const;
    const ImplInfo* findImpl(const std::string &traitName, const TypeInfo &targetType) const;
    const ImplInfo* findImplForMethod(const TypeInfo &targetType, const std::string &methodName) const;

    // Type management
    void declareStruct(const std::string &name, const std::vector<std::string> &typeParams, const std::vector<SymbolInfo> &fields);
    const StructInfo* lookupStruct(const std::string &name) const;
    void declareEnum(const std::string &name, const std::vector<std::string> &typeParams, const EnumInfo &info);
    const EnumInfo* lookupEnum(const std::string &name) const;

    // Region (Zone) Management
    void enterZone(const std::string &name);
    void exitZone();
    bool isZoneInScope(const std::string &name) const;
    std::string currentZone() const;
    const std::vector<std::string>& zoneStack() const { return zones_; }

private:
    using Scope = std::unordered_map<std::string, SymbolInfo>;
    std::vector<Scope> scopes_;
    std::unordered_map<std::string, TraitInfo> traits_;
    std::vector<ImplInfo> impls_;
    std::vector<std::string> zones_;
    std::vector<std::vector<std::pair<std::string, bool>>> borrowHistory_;
    std::unordered_map<std::string, StructInfo> structs_;
    std::unordered_map<std::string, EnumInfo> enums_;
};

} // namespace agam
