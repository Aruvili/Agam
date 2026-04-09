#pragma once

#include "agam/mir/mir.h"
#include <string>

namespace agam {

/// Produces a human-readable text dump of MIR (for --emit-mir).
class MirPrinter {
public:
    static std::string print(const MirProgram &program);
    static std::string printFunction(const MirFunction &func);
};

} // namespace agam
