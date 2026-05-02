#pragma once

#include "llvm/IR/Module.h"

#include <string>

namespace agam {

/// OrcJIT-based execution engine for running compiled code.
class Executor {
  public:
    /// JIT-compile and run a function, returning its exit code.
    /// The function must have signature: i32 main() or void main().
    static int run(llvm::Module &module, const std::string &entryPoint = "மைய");
};

} // namespace agam
