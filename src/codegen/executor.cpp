#include "agam/codegen/executor.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorAddress.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

extern "C" int agam_printf(const char *fmt, const char *s) {
    return printf(fmt, s);
}

extern "C" int agam_printf_int(const char *fmt, int i) {
    return printf(fmt, i);
}

extern "C" int agam_printf_hex(const char *fmt, int64_t i) {
    return printf(fmt, (long long)i);
}

extern "C" void agam_printf_bin(int64_t val) {
    if (val == 0) {
        printf("0b0");
        return;
    }
    printf("0b");
    bool started = false;
    for (int i = 63; i >= 0; --i) {
        if ((val >> i) & 1) {
            printf("1");
            started = true;
        } else if (started) {
            printf("0");
        }
    }
}

extern "C" int agam_printf_float(const char *fmt, double f) {
    return printf(fmt, f);
}

// ── ZPM Runtime (High-Performance Linear Allocator) ──────────────────────────

const size_t ZPM_BLOCK_SIZE = 8 * 1024 * 1024; // 8MB Blocks

struct AgamZoneBlock {
    uint8_t *data;
    size_t used;
    size_t size;
    AgamZoneBlock *next;
};

struct AgamZone {
    AgamZoneBlock *head;
    AgamZoneBlock *current;
};

extern "C" void *agam_zone_create() {
    AgamZone *zone = new AgamZone();
    AgamZoneBlock *block = new AgamZoneBlock();
    block->data = (uint8_t *)malloc(ZPM_BLOCK_SIZE);
    block->used = 0;
    block->size = ZPM_BLOCK_SIZE;
    block->next = nullptr;
    zone->head = block;
    zone->current = block;
    return zone;
}

extern "C" void *agam_zone_alloc(void *zonePtr, size_t size) {
    if (!zonePtr)
        return malloc(size);
    AgamZone *zone = static_cast<AgamZone *>(zonePtr);

    // Aligmnent padding (8-byte)
    size = (size + 7) & ~7;

    if (zone->current->used + size > zone->current->size) {
        // Current block full, allocate new
        size_t newSize = size > ZPM_BLOCK_SIZE ? size : ZPM_BLOCK_SIZE;
        AgamZoneBlock *block = new AgamZoneBlock();
        block->data = (uint8_t *)malloc(newSize);
        block->used = 0;
        block->size = newSize;
        block->next = zone->head;
        zone->head = block;
        zone->current = block;
    }

    void *ptr = zone->current->data + zone->current->used;
    zone->current->used += size;
    return ptr;
}

extern "C" void agam_zone_destroy(void *zonePtr) {
    if (!zonePtr)
        return;
    AgamZone *zone = static_cast<AgamZone *>(zonePtr);
    AgamZoneBlock *block = zone->head;
    while (block) {
        AgamZoneBlock *next = block->next;
        free(block->data);
        delete block;
        block = next;
    }
    delete zone;
}

extern "C" void agam_print_int(int i) {
    printf("%d\n", i);
}

// ── Input helpers ────────────────────────────────────────────────────────────

extern "C" int agam_scanf_int(const char *fmt, int *p) {
    return scanf(fmt, p);
}

extern "C" int agam_scanf_int64(const char *fmt, long long *p) {
    return scanf(fmt, p);
}

extern "C" int agam_scanf_float(const char *fmt, double *p) {
    return scanf(fmt, p);
}

extern "C" int agam_getchar() {
    return getchar();
}

// Read one line from stdin, strip trailing newline, return pointer.
// Uses a static buffer — valid until next call.
static char agam_readline_buf[4096];
extern "C" const char *agam_readline() {
    if (!fgets(agam_readline_buf, sizeof(agam_readline_buf), stdin)) {
        agam_readline_buf[0] = '\0';
        return agam_readline_buf;
    }
    // Strip trailing newline
    agam_readline_buf[strcspn(agam_readline_buf, "\n")] = '\0';
    return agam_readline_buf;
}

// ── Additional output helpers ────────────────────────────────────────────────

extern "C" int agam_putchar(int c) {
    return putchar(c);
}

extern "C" int agam_fprintf_stderr(const char *fmt, const char *s) {
    return fprintf(stderr, fmt, s);
}

extern "C" int agam_fprintf_stderr_int(const char *fmt, int i) {
    return fprintf(stderr, fmt, i);
}

extern "C" int agam_fprintf_stderr_float(const char *fmt, double f) {
    return fprintf(stderr, fmt, f);
}

// ── OS Library JIT Wrappers ──

extern "C" void agam_os_exit(int64_t code) {
    exit((int)code);
}

extern "C" const char *agam_os_getenv(const char *name) {
    const char *val = getenv(name);
    return val ? val : "";
}

extern "C" int64_t agam_os_system(const char *cmd) {
    return system(cmd);
}

extern "C" const char *agam_os_name() {
#ifdef _WIN32
    return "windows";
#elif __APPLE__
    return "macos";
#else
    return "linux";
#endif
}

// ── Time Library JIT Wrappers ──

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif

extern "C" int64_t agam_time_epoch() {
    return (int64_t)time(nullptr);
}

extern "C" void agam_time_sleep(double seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000.0));
#else
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - ts.tv_sec) * 1e9);
    nanosleep(&ts, nullptr);
#endif
}

extern "C" void agam_time_sleep_ms(int64_t ms) {
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    usleep(ms * 1000);
#endif
}


namespace agam {

#ifdef __MINGW32__
extern "C" void agam_noop_main(void) {}
#endif

int Executor::run(llvm::Module &module, const std::string &entryPoint) {
    // Initialize native target.
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Create the JIT.
    auto jitExpected = llvm::orc::LLJITBuilder().create();
    if (!jitExpected) {
        std::cerr << "Error: failed to create JIT: " << llvm::toString(jitExpected.takeError())
                  << "\n";
        return -1;
    }
    auto &jit = *jitExpected;

    // Register host process symbols so CRT functions can be found.
    auto &dl = jit->getDataLayout();
    auto processSymbolsGenerator =
        llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix());
    if (processSymbolsGenerator) {
        jit->getMainJITDylib().addGenerator(std::move(*processSymbolsGenerator));
    }

    // On MinGW, GCC inserts a call to __main for C++ static initialization.
    // We define a no-op stub since JIT doesn't link the CRT.
#ifdef __MINGW32__
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("__main")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_noop_main)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define __main stub: " << llvm::toString(std::move(err))
                      << "\n";
        }
    }
#endif

    // Register printf_int helper
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("printf")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_printf)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("printf_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_printf_int)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("printf_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_printf_float)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("printf_hex")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_printf_hex)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("printf_bin")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_printf_bin)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_create")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_create)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_alloc")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_alloc)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_destroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_destroy)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("print_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_print_int)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define printf_int: " << llvm::toString(std::move(err))
                      << "\n";
        }
    }

    // Register printf_float helper
    // Register math helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("sqrt")] = symbols[jit->mangleAndIntern("வர்க்கமூலம்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(sqrt)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("sin")] = symbols[jit->mangleAndIntern("சைன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(sin)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("cos")] = symbols[jit->mangleAndIntern("கொசைன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(cos)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("tan")] = symbols[jit->mangleAndIntern("டேன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(tan)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("asin")] = symbols[jit->mangleAndIntern("தலைகீழ்_சைன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(asin)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("acos")] = symbols[jit->mangleAndIntern("தலைகீழ்_கொசைன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(acos)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("atan")] = symbols[jit->mangleAndIntern("தலைகீழ்_டேன்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(atan)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("atan2")] = symbols[jit->mangleAndIntern("தலைகீழ்_டேன்2")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double, double)>(atan2)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("exp")] = symbols[jit->mangleAndIntern("அடுக்கு_இ")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(exp)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("log")] = symbols[jit->mangleAndIntern("இயற்கை_மடக்கை")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(log)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("log10")] = symbols[jit->mangleAndIntern("மடக்கை10")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(log10)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("pow")] = symbols[jit->mangleAndIntern("அடுக்கு")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double, double)>(pow)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fabs")] = symbols[jit->mangleAndIntern("மட்டு_மதிப்பு")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(fabs)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("ceil")] = symbols[jit->mangleAndIntern("மேல்_எண்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(ceil)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("floor")] = symbols[jit->mangleAndIntern("கீழ்_எண்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(floor)),
                llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("round")] = symbols[jit->mangleAndIntern("முழு_எண்")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(static_cast<double (*)(double)>(round)),
                llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define math helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Register input helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("scanf_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_scanf_int)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("scanf_int64")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_scanf_int64)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("scanf_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_scanf_float)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_getchar")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_getchar)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_readline")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_readline)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define input helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Register additional output helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("agam_putchar")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_putchar)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fprintf_stderr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fprintf_stderr)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fprintf_stderr_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fprintf_stderr_int)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fprintf_stderr_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                reinterpret_cast<void (*)()>(agam_fprintf_stderr_float)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define output helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Register OS helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("agam_os_exit")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_os_exit)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_os_getenv")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_os_getenv)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_os_system")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_os_system)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_os_name")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_os_name)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define OS helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Register Time helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("agam_time_epoch")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_time_epoch)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_time_sleep")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_time_sleep)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_time_sleep_ms")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_time_sleep_ms)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define Time helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Transfer module ownership to the JIT.
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto clonedModule = llvm::CloneModule(module);
    auto tsm = llvm::orc::ThreadSafeModule(std::move(clonedModule), std::move(ctx));

    if (auto err = jit->addIRModule(std::move(tsm))) {
        std::cerr << "Error: failed to add module to JIT: " << llvm::toString(std::move(err))
                  << "\n";
        return -1;
    }

    // Look up the entry point.
    auto sym = jit->lookup(entryPoint);
    if (!sym) {
        std::cerr << "Error: could not find entry point '" << entryPoint
                  << "': " << llvm::toString(sym.takeError()) << "\n";
        return -1;
    }

    // Cast and call.
    auto *mainFn = sym->toPtr<int()>();
    return mainFn();
}

} // namespace agam
