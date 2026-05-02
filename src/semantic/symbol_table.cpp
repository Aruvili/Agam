#include "agam/semantic/symbol_table.h"

#include <iostream>

namespace agam {

SymbolTable::SymbolTable() {
    enterScope(); // Global scope
}

void SymbolTable::enterScope() {
    scopes_.emplace_back();
    borrowHistory_.emplace_back();
}

void SymbolTable::exitScope() {
    if (!scopes_.empty()) {
        releaseBorrows();
        scopes_.pop_back();
        borrowHistory_.pop_back();
    }
}

void SymbolTable::addBorrow(const std::string &name, bool isMutable) {
    if (!borrowHistory_.empty()) {
        borrowHistory_.back().push_back({name, isMutable});

        // Update the symbol itself
        SymbolInfo *info = lookupMutable(name);
        if (info) {
            if (isMutable)
                info->mutablyBorrowed = true;
            else
                info->sharedBorrowCount++;
        }
    }
}

void SymbolTable::removeBorrow(const std::string &name, bool isMutable) {
    SymbolInfo *info = lookupMutable(name);
    if (info) {
        if (isMutable)
            info->mutablyBorrowed = false;
        else
            info->sharedBorrowCount = std::max(0, info->sharedBorrowCount - 1);
    }

    // Also remove from current scope history so it doesn't get double-released
    if (!borrowHistory_.empty()) {
        auto &history = borrowHistory_.back();
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            if (it->first == name && it->second == isMutable) {
                // We mark it as empty to "effectively" remove it without expensive vector erase
                it->first = "";
                break;
            }
        }
    }
}

void SymbolTable::releaseBorrows() {
    if (borrowHistory_.empty())
        return;

    auto &currentBorrows = borrowHistory_.back();
    for (const auto &borrow : currentBorrows) {
        if (borrow.first.empty())
            continue;
        SymbolInfo *info = lookupMutable(borrow.first);
        if (info) {
            if (borrow.second)
                info->mutablyBorrowed = false;
            else
                info->sharedBorrowCount = std::max(0, info->sharedBorrowCount - 1);
        }
    }
}

// ── Symbols ──────────────────────────────────────────────────────────────
bool SymbolTable::declare(const std::string &name, const SymbolInfo &info) {
    if (scopes_.empty()) {
        scopes_.emplace_back();
    }
    auto &currentScope = scopes_.back();
    if (currentScope.find(name) != currentScope.end()) {
        return false; // Already declared in this scope
    }
    currentScope[name] = info;
    return true;
}

std::optional<SymbolInfo> SymbolTable::lookup(const std::string &name) const {
    // Search from innermost scope to outermost
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

SymbolInfo *SymbolTable::lookupMutable(const std::string &name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &(found->second);
        }
    }
    return nullptr;
}

std::optional<SymbolInfo> SymbolTable::lookupCurrent(const std::string &name) const {
    if (scopes_.empty()) {
        return std::nullopt;
    }
    auto &currentScope = scopes_.back();
    auto found = currentScope.find(name);
    if (found != currentScope.end()) {
        return found->second;
    }
    return std::nullopt;
}

// ── Traits ───────────────────────────────────────────────────────────────

void SymbolTable::declareTrait(const std::string &name, const TraitInfo &info) {
    traits_[name] = info;
}

void SymbolTable::declareImpl(const ImplInfo &info) {
    impls_.push_back(info);
}

const TraitInfo *SymbolTable::lookupTrait(const std::string &name) const {
    auto it = traits_.find(name);
    if (it != traits_.end()) {
        return &it->second;
    }
    return nullptr;
}

const ImplInfo *SymbolTable::findImpl(const std::string &traitName,
                                      const TypeInfo &targetType) const {
    for (const auto &impl : impls_) {
        // targetType matches if the types are compatible (or identical)
        if (impl.traitName == traitName && impl.targetType == targetType) {
            return &impl;
        }
    }
    return nullptr;
}

const ImplInfo *SymbolTable::findImplForMethod(const TypeInfo &targetType,
                                               const std::string &methodName) const {
    auto typesMatch = [](const TypeInfo &pattern, const TypeInfo &target) -> bool {
        // Simple exact match
        if (pattern == target)
            return true;

        // Handle generic match: Vector<int64> (target) should match Vector<T> (pattern)
        // Also handle pointers: *Vector<int64> should match *Vector<T>
        if (pattern.kind == target.kind || (pattern.isStruct && target.isStruct) ||
            (pattern.isGeneric)) {
            // Check pointer depth: allow auto-ref (value→pointer) and auto-deref (pointer→value)
            int pdiff = pattern.pointerDepth - target.pointerDepth;
            if (pdiff != 0 && pdiff != 1 && pdiff != -1) {
                return false;
            }

            if (pattern.isStruct && target.isStruct) {
                if (pattern.structName == target.structName)
                    return true;
                // Match monomorphized names: Vector_int64 matches Vector
                if (target.structName.find(pattern.structName + "_") == 0)
                    return true;
                return false;
            }

            if (pattern.isGeneric)
                return true;
            return pattern.kind == target.kind && pattern.kind != TypeKind::Unknown;
        }
        return false;
    };

    for (const auto &impl : impls_) {
        if (typesMatch(impl.targetType, targetType)) {
            for (const auto &m : impl.methods) {
                if (m.name == methodName)
                    return &impl;
            }
        }
    }
    return nullptr;
}

void SymbolTable::declareStruct(const std::string &name, const std::vector<std::string> &typeParams,
                                const std::vector<SymbolInfo> &fields) {
    StructInfo info;
    info.name = name;
    info.typeParams = typeParams;
    info.fields = fields;
    structs_[name] = info;
}

const StructInfo *SymbolTable::lookupStruct(const std::string &name) const {
    auto it = structs_.find(name);
    if (it != structs_.end())
        return &it->second;
    return nullptr;
}

void SymbolTable::declareEnum(const std::string &name, const std::vector<std::string> &typeParams,
                              const EnumInfo &info) {
    EnumInfo updated = info;
    updated.typeParams = typeParams;
    enums_[name] = updated;
}

const EnumInfo *SymbolTable::lookupEnum(const std::string &name) const {
    auto it = enums_.find(name);
    if (it != enums_.end())
        return &it->second;
    return nullptr;
}

void SymbolTable::enterZone(const std::string &name) {
    zones_.push_back(name);
}

void SymbolTable::exitZone() {
    if (!zones_.empty()) {
        zones_.pop_back();
    }
}

bool SymbolTable::isZoneInScope(const std::string &name) const {
    if (name.empty() || name == "global")
        return true;
    for (const auto &z : zones_) {
        if (z == name)
            return true;
    }
    return false;
}

std::string SymbolTable::currentZone() const {
    if (zones_.empty())
        return "";
    return zones_.back();
}

} // namespace agam
