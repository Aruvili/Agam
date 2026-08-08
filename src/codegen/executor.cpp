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

// ── Runtime Function Declarations (Implemented in agam_runtime.c) ──────────────

extern "C" {
void *agam_zone_create();
void *agam_zone_alloc(void *zonePtr, size_t size);
void agam_zone_destroy(void *zonePtr);
void agam_zone_reset(void *zonePtr);
void agam_zone_push(void *zonePtr);
void agam_zone_pop();
size_t agam_zone_allocated_bytes(void *zonePtr);
size_t agam_zone_block_count(void *zonePtr);
void print_int(int32_t i);
int printf_int(const char *fmt, int64_t i);
int printf_float(const char *fmt, double f);
int printf_hex(const char *fmt, int64_t i);
void printf_bin(int64_t i);
int scanf_int(const char *fmt, int *p);
int scanf_int64(const char *fmt, int64_t *p);
int scanf_float(const char *fmt, double *p);
int agam_getchar();
char *agam_readline();
int agam_putchar(int c);
int fprintf_stderr(const char *fmt, const char *s);
int fprintf_stderr_int(const char *fmt, int64_t i);
int fprintf_stderr_float(const char *fmt, double f);
void agam_os_exit(int64_t code);
const char *agam_os_getenv(const char *name);
int64_t agam_os_system(const char *cmd);
const char *agam_os_name();
int64_t agam_time_epoch();
void agam_time_sleep(double seconds);
void agam_time_sleep_ms(int64_t ms);
int64_t agam_net_listen(int64_t port);
int64_t agam_net_accept(int64_t server_fd);
int64_t agam_net_send(int64_t client_fd, const char *data);
const char *agam_net_recv(int64_t client_fd);
void agam_net_close(int64_t fd);
const char* agam_str_substring(const char* s, int64_t start, int64_t len);
const char* agam_str_trim(const char* s);
const char* agam_str_to_upper(const char* s);
const char* agam_str_to_lower(const char* s);
int64_t agam_str_contains(const char* s, const char* sub);
int64_t agam_fs_mkdir(const char* path);
int64_t agam_fs_exists(const char* path);
int64_t agam_fs_is_dir(const char* path);
int64_t agam_fs_size(const char* path);
int64_t agam_rand_range(int64_t min_val, int64_t max_val);
double agam_rand_float(void);
int64_t agam_str_len(const char* s);
const char* agam_str_replace(const char* s, const char* old_sub, const char* new_sub);
const char* agam_fs_read_all(const char* path);
int64_t agam_fs_write_all(const char* path, const char* content);
int64_t agam_thread_spawn(void* fn_ptr, void* arg);
int64_t agam_thread_join(int64_t thread_id);
const char* agam_base64_encode(const char* data);
const char* agam_base64_decode(const char* data);
const char* agam_crypto_sha256(const char* data);
int64_t agam_regex_match(const char* text, const char* pattern);
const char* agam_datetime_now(void);
const char* agam_datetime_format(int64_t timestamp, const char* format);
int64_t agam_db_open(const char* db_name);
int64_t agam_db_exec(int64_t handle, const char* query);
const char* agam_str_concat(const char* s1, const char* s2);
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
        symbols[jit->mangleAndIntern("agam_zone_reset")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_reset)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_push")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_push)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_pop")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_pop)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_allocated_bytes")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_allocated_bytes)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_zone_block_count")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_zone_block_count)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("print_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(print_int)),
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
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(scanf_int)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("scanf_int64")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(scanf_int64)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("scanf_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(scanf_float)),
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
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(fprintf_stderr)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fprintf_stderr_int")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(fprintf_stderr_int)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("fprintf_stderr_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                reinterpret_cast<void (*)()>(fprintf_stderr_float)),
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

    // Register Network helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("agam_net_listen")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_net_listen)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_net_accept")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_net_accept)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_net_send")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_net_send)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_net_recv")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_net_recv)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_net_close")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_net_close)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define Network helpers: "
                      << llvm::toString(std::move(err)) << "\n";
        }
    }

    // Register Extended String, FS, and Random helpers
    {
        llvm::orc::SymbolMap symbols;
        symbols[jit->mangleAndIntern("agam_str_substring")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_substring)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_trim")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_trim)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_to_upper")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_to_upper)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_to_lower")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_to_lower)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_contains")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_contains)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_mkdir")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_mkdir)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_exists")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_exists)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_is_dir")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_is_dir)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_size")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_size)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_rand_range")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_rand_range)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_rand_float")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_rand_float)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_len")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_len)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_replace")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_replace)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_read_all")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_read_all)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_fs_write_all")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_fs_write_all)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_thread_spawn")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_thread_spawn)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_thread_join")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_thread_join)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_base64_encode")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_base64_encode)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_base64_decode")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_base64_decode)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_crypto_sha256")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_crypto_sha256)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_regex_match")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_regex_match)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_datetime_now")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_datetime_now)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_datetime_format")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_datetime_format)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_db_open")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_db_open)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_db_exec")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_db_exec)),
            llvm::JITSymbolFlags::Exported);
        symbols[jit->mangleAndIntern("agam_str_concat")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(reinterpret_cast<void (*)()>(agam_str_concat)),
            llvm::JITSymbolFlags::Exported);
        if (auto err =
                jit->getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(symbols)))) {
            std::cerr << "Warning: failed to define extended std helpers: "
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
