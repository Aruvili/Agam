#pragma once

#include "llvm/IR/Module.h"
#include <string>

namespace agam {

/// Wraps LLVM's new pass manager to optimize a module.
class Optimizer {
public:
    /// Optimization levels matching -O0 through -O3.
    enum class Level { O0, O1, O2, O3 };

    /// Run optimization passes on the module.
    static void optimize(llvm::Module &module, Level level = Level::O2);

    /// Parse a string like "0", "1", "2", "3" into a Level.
    static Level parseLevel(const std::string &s);
};

} // namespace agam
