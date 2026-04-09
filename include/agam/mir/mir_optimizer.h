#pragma once

#include "agam/mir/mir.h"

namespace agam {

class MirOptimizer {
public:
    static void optimize(MirProgram &module);

private:
    static int64_t getTypeSize(const TypeInfo &ti);
    static int64_t align(int64_t size, int64_t alignment = 8);
    static void fuseZoneAllocations(MirFunction &func);
};

} // namespace agam
