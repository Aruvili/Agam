#include "agam/mir/mir_optimizer.h"

#include <map>
#include <variant>

namespace agam {

void MirOptimizer::optimize(MirProgram &module) {
    for (auto &func : module.functions) {
        if (func.isExtern)
            continue;
        fuseZoneAllocations(func);
    }
}

int64_t MirOptimizer::getTypeSize(const TypeInfo &ti) {
    if (ti.pointerDepth > 0)
        return 8;
    if (ti.isSlice)
        return 16;
    if (ti.isArray)
        return getTypeSize(TypeInfo::scalar(ti.elementType)) * ti.arraySize;

    switch (ti.kind) {
    case TypeKind::Int8:
    case TypeKind::UInt8:
        return 1;
    case TypeKind::Int16:
    case TypeKind::UInt16:
        return 2;
    case TypeKind::Int32:
    case TypeKind::UInt32:
        return 4;
    case TypeKind::Int64:
    case TypeKind::UInt64:
        return 8;
    case TypeKind::Float32:
        return 4;
    case TypeKind::Float64:
        return 8;
    case TypeKind::Bool:
        return 1;
    default:
        return 8; // Fallback
    }
}

int64_t MirOptimizer::align(int64_t size, int64_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

void MirOptimizer::fuseZoneAllocations(MirFunction &func) {
    for (auto &block : func.blocks) {
        std::map<std::string, std::vector<size_t>> blockAllocations;
        for (size_t i = 0; i < block.statements.size(); ++i) {
            if (auto *assign = std::get_if<MirAssign>(&block.statements[i])) {
                if (auto *alloc = std::get_if<MirRvalueZoneAlloc>(&assign->rvalue)) {
                    if (alloc->count.kind == MirOperand::Kind::Constant) {
                        blockAllocations[alloc->zoneName].push_back(i);
                    }
                }
            }
        }

        std::vector<MirStatement> finalStmts;
        std::map<size_t, MirStatement> replacements;
        std::map<size_t, std::vector<MirStatement>> insertions;

        for (auto &pair : blockAllocations) {
            auto &indices = pair.second;
            if (indices.size() <= 1)
                continue;

            const std::string &zone = pair.first;
            int64_t totalSize = 0;

            MirLocalId masterLocal = (MirLocalId)func.locals.size();
            TypeInfo masterType = TypeInfo::scalar(TypeKind::Int8);
            masterType.pointerDepth = 1;
            func.locals.push_back({masterLocal, masterType, "_fused_" + zone, true});

            MirAssign masterAssign;
            masterAssign.destination = {masterLocal};
            masterAssign.rvalue = MirRvalueZoneAlloc{TypeInfo::scalar(TypeKind::Int8),
                                                     MirOperand::makeConstInt64(0), zone};

            for (size_t idx : indices) {
                auto *assign = std::get_if<MirAssign>(&block.statements[idx]);
                auto *alloc = std::get_if<MirRvalueZoneAlloc>(&assign->rvalue);
                int64_t count = std::get<MirConstInt>(alloc->count.constant).value;
                int64_t elementSize = getTypeSize(alloc->allocatedType);
                int64_t allocSize = align(elementSize * count);

                MirAssign offsetAssign;
                offsetAssign.destination = assign->destination;
                if (totalSize == 0) {
                    offsetAssign.rvalue =
                        MirRvalueCast{MirOperand::makeCopy(masterLocal, masterType), masterType,
                                      func.locals[assign->destination.local].typeInfo};
                } else {
                    offsetAssign.rvalue =
                        MirRvaluePtrOffset{MirOperand::makeCopy(masterLocal, masterType), totalSize,
                                           func.locals[assign->destination.local].typeInfo};
                }

                replacements[idx] = offsetAssign;
                totalSize += allocSize;
            }

            std::get<MirRvalueZoneAlloc>(masterAssign.rvalue).count =
                MirOperand::makeConstInt64(totalSize);
            insertions[indices[0]].push_back(masterAssign);
        }

        for (size_t i = 0; i < block.statements.size(); ++i) {
            if (insertions.count(i)) {
                for (auto &s : insertions[i])
                    finalStmts.push_back(s);
            }
            if (replacements.count(i)) {
                finalStmts.push_back(replacements[i]);
            } else {
                finalStmts.push_back(block.statements[i]);
            }
        }
        block.statements = std::move(finalStmts);
    }
}

} // namespace agam
