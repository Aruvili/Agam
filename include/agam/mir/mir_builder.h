#pragma once

#include "agam/mir/mir.h"
#include "agam/thir/thir.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace agam {

/// Builds MIR from THIR by flattening the tree into basic blocks.
///
/// Nested expressions become sequences of tmp assignments.
/// Control flow (if/while) becomes basic blocks with goto/switch terminators.
class MirBuilder {
  public:
    std::unique_ptr<MirProgram> build(ThirProgram &program);

  private:
    // ── Per-function state ──────────────────────────────────────────────────
    MirFunction *currentFunc_ = nullptr;
    MirBlockId nextBlockId_ = 0;
    MirLocalId nextLocalId_ = 0;

    /// HirId → MirLocalId for named variables.
    std::unordered_map<HirId, MirLocalId> varMap_;

    /// HirId → MirConstant for global constants.
    std::unordered_map<HirId, MirConstant> constantMap_;

    /// Tracking active zones for cleanup (Return/Goto).
    std::vector<std::string> activeZones_;

    // ── Block management ────────────────────────────────────────────────────
    MirBlockId newBlock(const std::string &label);
    MirBasicBlock &block(MirBlockId id);
    void setTerminator(MirBlockId blockId, MirTerminator term);

    // ── Local management ────────────────────────────────────────────────────
    MirLocalId newLocal(TypeInfo typeInfo, const std::string &name = "", bool isTemp = true);
    MirLocalId newTemp(TypeInfo typeInfo);

    // ── Lowering ────────────────────────────────────────────────────────────
    void lowerFunc(ThirFuncDecl &fn);
    void lowerBlock(ThirBlock &block, MirBlockId &blockId, MirBlockId *continueBlockId);
    void lowerStmt(ThirStmt &stmt, MirBlockId &currentBlock, MirBlockId *continueBlockId);

    /// Lower an expression into a series of assignments and return the
    /// MirOperand that holds the result.
    MirOperand lowerExpr(ThirExpr &expr, MirBlockId &currentBlock);

    // ── Constant Folding ────────────────────────────────────────────────────
    std::optional<MirOperand> foldBinaryOp(BinaryOp op, const MirOperand &lhs,
                                           const MirOperand &rhs);
    std::optional<MirOperand> foldUnaryOp(UnaryOp op, const MirOperand &operand);
};

} // namespace agam
