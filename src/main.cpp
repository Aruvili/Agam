/// Agam Compiler Driver
///
/// Full pipeline: Source -> Lex -> Parse -> HIR -> THIR -> MIR -> LLVM IR -> [Optimize] ->
/// Codegen/Execute
///
/// Usage:
///   agamc <input.agam> [-o output] [--emit-ast] [--emit-hir] [--emit-thir] [--emit-mir]
///                      [--emit-llvm] [-O0|-O1|-O2|-O3] [--run]

#include "agam/ast/ast_printer.h"
#include "agam/codegen/codegen.h"
#include "agam/codegen/executor.h"
#include "agam/codegen/optimizer.h"
#include "agam/hir/hir_builder.h"
#include "agam/lexer/lexer.h"
#include "agam/mir/mir_builder.h"
#include "agam/mir/mir_optimizer.h"
#include "agam/mir/mir_printer.h"
#include "agam/parser/parser.h"
#include "agam/semantic/monomorphizer.h"
#include "agam/semantic/scope_resolver.h"
#include "agam/semantic/type_checker.h"
#include "agam/thir/thir_builder.h"
#include "agam/utils/diagnostic_renderer.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {
    static void agamSignalHandler(int sig) {
        std::cerr << "\n\033[1;31mமுக்கிய பிழை (fatal): கம்பைலரில் எதிர்பாராத அகப்பின்னடைவு பிழை "
                  << "(Compiler Internal Crash, signal " << sig << ")\033[0m\n"
                  << "உதவிக்குறிப்பு: இந்த கோளாறை https://github.com/Aruvili/Agam/issues இல் தெரிவிக்கவும்.\n";
        std::exit(128 + sig);
    }
}
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

#ifndef AGAM_VERSION
#define AGAM_VERSION "1.4.0"
#endif

using namespace agam;
namespace fs = std::filesystem;

namespace {
    template <typename F>
    static auto callHostFeatures(F fn, int) -> decltype(fn()) {
        return fn();
    }

    template <typename F>
    static auto callHostFeatures(F fn, long) -> llvm::StringMap<bool> {
        llvm::StringMap<bool> m;
        using fn_ptr_t = bool (*)(llvm::StringMap<bool>&);
        reinterpret_cast<fn_ptr_t>(fn)(m);
        return m;
    }

    inline llvm::StringMap<bool> fetchHostCPUFeatures() {
        return callHostFeatures(llvm::sys::getHostCPUFeatures, 0);
    }
}

// Global diagnostic infrastructure
SourceManager g_sourceManager;
DiagnosticEngine g_diagEngine(g_sourceManager);

static void printUsage(const char *prog) {
    std::cout << "அகம் (Agam) கம்பைலர் பதிப்பு " << AGAM_VERSION << "\n"
              << "பயன்பாடு: " << prog << " [விருப்பங்கள்] <input.agam>\n"
              << "          " << prog << " create <திட்டப்பொருள்_பெயர்> | .\n"
              << "          " << prog << " run [கோப்பு_பெயர்]\n"
              << "\nவிருப்பங்கள் (Options):\n"
              << "  -o, --output <கோப்பு>   வெளியீட்டு கோப்பினை குறிப்பிடுக (இயல்பு: <உள்ளீடு>.exe / <உள்ளீடு>)\n"
              << "  -v, --version         பதிப்பு தகவலை அச்சிடுக\n"
              << "  -h, --help            இந்த உதவிச் செய்தியை அச்சிடுக\n"
              << "  --emit-ast            AST-ஐ அச்சிட்டு வெளியேறுக\n"
              << "  --emit-hir            HIR-ஐ அச்சிட்டு வெளியேறுக\n"
              << "  --emit-thir           THIR வகைகளை அச்சிட்டு வெளியேறுக\n"
              << "  --emit-mir            MIR-ஐ அச்சிட்டு வெளியேறுக\n"
              << "  --emit-llvm           LLVM IR-ஐ அச்சிட்டு வெளியேறுக\n"
              << "  -O<n>                 மேம்படுத்தல் நிலை (Optimization level) (0-3, இயல்பு: 0)\n"
              << "  --run                 கம்பைல் செய்வதற்கு பதிலாக JIT மூலம் இயக்குக\n"
              << "  --lib-path <பாதை>     தனிப்பயன் தரநிலை நூலகப் பாதையைக் குறிப்பிடுக\n";
}

static int handleCreate(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "பிழை: 'create' செய்வதற்கு திட்டப்பொருள் பெயர் (project name) தேவை\n";
        return 1;
    }

    std::string projNameArg = argv[2];
    fs::path projectRoot;

    if (projNameArg == ".") {
        projectRoot = fs::current_path();
    } else {
        projectRoot = fs::current_path() / projNameArg;
        if (!fs::exists(projectRoot)) {
            fs::create_directory(projectRoot);
        }
    }

    fs::path srcDir = projectRoot / "src";
    if (!fs::exists(srcDir))
        fs::create_directory(srcDir);

    // Create pk.arpk
    std::ofstream arpk(projectRoot / "pk.arpk");
    std::string name = (projNameArg == ".") ? projectRoot.filename().string() : projNameArg;
    arpk << "projectname: \"" << name << "\"\n"
         << "version: \"1.0.1\"\n"
         << "author: \"\"\n\n"
         << "#dependencies\n"
         << "[name]: [version]\n"
         << "std: 1.0.0,\n\n"
         << "#dev_dep\n"
         << "[name]: [version]\n"
         << "std: 1.0.0,\n";
    arpk.close();

    // Create src/main.agam
    std::ofstream mainAgam(srcDir / "main.agam");
    mainAgam << "செயல் மைய(): எண் {\n"
             << "    விடை 0;\n"
             << "}\n";
    mainAgam.close();

    // Create README.md
    std::ofstream readme(projectRoot / "README.md");
    readme << "# " << name << "\n\nGenerated by Agam Compiler.\n";
    readme.close();

    std::cout << "திட்டப்பொருள் (Project) '" << name << "' வெற்றிகரமாக உருவாக்கப்பட்டது.\n";
    return 0;
}

namespace agam {

static std::string trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start)))
        start++;
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(static_cast<unsigned char>(*end)));
    return std::string(start, end + 1);
}

static fs::path getExecutableDir(const char *argv0) {
#if defined(__linux__)
    char result[1024];
    ssize_t count = ::readlink("/proc/self/exe", result, sizeof(result) - 1);
    if (count != -1) {
        result[count] = '\0';
        return fs::path(result).parent_path();
    }
#elif defined(_WIN32)
    char result[MAX_PATH];
    DWORD ret = GetModuleFileNameA(NULL, result, MAX_PATH);
    if (ret > 0) {
        return fs::path(result).parent_path();
    }
#endif
    fs::path p(argv0);
    if (p.is_absolute()) return p.parent_path();
    if (p.has_parent_path()) return fs::absolute(p).parent_path();
    const char *pathEnv = std::getenv("PATH");
    if (pathEnv) {
        std::stringstream ss(pathEnv);
        std::string item;
#if defined(_WIN32)
        char delim = ';';
#else
        char delim = ':';
#endif
        while (std::getline(ss, item, delim)) {
            fs::path candidate = fs::path(item) / p;
            if (fs::exists(candidate)) {
                return fs::canonical(candidate).parent_path();
            }
        }
    }
    return fs::absolute(p).parent_path();
}

static fs::path g_exeDir;
static std::string g_stdEnvPath;

std::unique_ptr<Program> parseWithImports(const std::string &filename,
                                          std::set<std::string> &visited) {
    fs::path targetPath = fs::absolute(filename);

    // If not found, it might be a library import without full path
    if (!fs::exists(targetPath)) {
        std::cerr << "பிழை: எந்தத் தேடல் பாதையிலும் '" << filename << "' என்ற கோப்பினைத் தேட முடியவில்லை\n";
        return nullptr;
    }

    std::string absolutePath = targetPath.string();
    if (visited.count(absolutePath))
        return nullptr;
    visited.insert(absolutePath);

    std::ifstream file(absolutePath);
    if (!file.is_open()) {
        std::cerr << "பிழை: '" << absolutePath << "' என்ற கோப்பினைத் திறக்க முடியவில்லை\n";
        return nullptr;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    g_sourceManager.addSource(absolutePath, source);

    // Lex
    Lexer lexer(source, absolutePath, g_diagEngine);
    auto tokens = lexer.tokenize();
    if (g_diagEngine.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return nullptr;
    }

    // Parse
    Parser parser(tokens, source, absolutePath, g_diagEngine);
    auto ast = parser.parse();
    if (g_diagEngine.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return nullptr;
    }

    auto master = std::move(ast);
    auto baseDir = fs::path(absolutePath).parent_path();

    // Recursively handle imports
    std::vector<std::string> discoveredImports = master->imports;
    for (const auto &imp : discoveredImports) {
        fs::path impPath = baseDir / imp;
        bool found = false;

        // 1. Try relative to current file
        if (impPath.extension().empty())
            impPath.replace_extension(".agam");
        if (fs::exists(impPath)) {
            found = true;
        }

        // 2. Try AGAM_STD_PATH
        if (!found && !g_stdEnvPath.empty()) {
            fs::path envPath = fs::path(g_stdEnvPath) / imp;
            if (envPath.extension().empty())
                envPath.replace_extension(".agam");
            if (fs::exists(envPath)) {
                impPath = envPath;
                found = true;
            }
        }

        // 3. Try relative to executable
        if (!found) {
            std::vector<fs::path> sysPaths = {g_exeDir / ".." / "share" / "agam" / imp,
                                              g_exeDir / ".." / imp, g_exeDir / ".." / ".." / imp};
            for (auto &sp : sysPaths) {
                if (sp.extension().empty())
                    sp.replace_extension(".agam");
                if (fs::exists(sp)) {
                    impPath = sp;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cerr << "பிழை: இறக்குமதி செய்யப்பட வேண்டிய '" << imp << "' என்ற பெயரைக் காணவில்லை.\n";
            std::cerr << "  (அடிப்படைப் பாதை: " << baseDir << ")\n";
            if (!g_stdEnvPath.empty()) {
                fs::path envPath = fs::path(g_stdEnvPath) / imp;
                if (envPath.extension().empty())
                    envPath.replace_extension(".agam");
                std::cerr << "  (AGAM_STD_PATH check: " << envPath
                          << " exists=" << (fs::exists(envPath) ? "true" : "false") << ")\n";
            }
            return nullptr;
        }

        auto subProg = parseWithImports(impPath.string(), visited);
        if (subProg) {
            // Merge declarations into master (flat namespace for now)
            for (auto &f : subProg->functions)
                master->functions.push_back(std::move(f));
            for (auto &s : subProg->structs)
                master->structs.push_back(std::move(s));
            for (auto &e : subProg->enums)
                master->enums.push_back(std::move(e));
            for (auto &t : subProg->traits)
                master->traits.push_back(std::move(t));
            for (auto &i : subProg->impls)
                master->impls.push_back(std::move(i));
            for (auto &c : subProg->constants)
                master->constants.push_back(std::move(c));
        }
    }

    return master;
}

} // namespace agam

int main(int argc, char *argv[]) {
    std::signal(SIGSEGV, agamSignalHandler);
    std::signal(SIGABRT, agamSignalHandler);
    std::signal(SIGFPE,  agamSignalHandler);
    std::signal(SIGILL,  agamSignalHandler);

    g_exeDir = getExecutableDir(argv[0]);
    const char *envPath = std::getenv("AGAM_STD_PATH");
    if (envPath)
        g_stdEnvPath = trim(envPath);

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile;
    std::string outputFile;
    bool emitAst = false, emitHir = false, emitThir = false, emitMir = false, emitLlvm = false;
    bool runJit = false;
    bool generateDebugInfo = false;
    Optimizer::Level optLevel = Optimizer::Level::O0;

    int argStart = 1;

    // Pre-scan for -C flag
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-C" && i + 1 < argc) {
            fs::current_path(argv[i + 1]);
            // Re-order or skip? Let's just handle it here and we'll skip it in the main loop
        }
    }

    // Check for subcommands
    std::string firstArg = argv[1];
    if (firstArg == "create") {
        return handleCreate(argc, argv);
    } else if (firstArg == "run") {
        runJit = true;
        argStart = 2;
        // Re-check pk.arpk after possible -C
        if (fs::exists("pk.arpk")) {
            if (fs::exists("src/main.agam")) {
                inputFile = "src/main.agam";
            } else {
                std::cerr << "பிழை: 'pk.arpk' உள்ளது, ஆனால் 'src/main.agam' கோப்பைக் காணவில்லை.\n";
                return 1;
            }
        } else {
            // Check for file in arguments, skipping -C and its value
            for (int i = 2; i < argc; i++) {
                std::string arg = argv[i];
                if (arg == "-C") {
                    i++;
                    continue;
                }
                if (arg[0] != '-') {
                    inputFile = arg;
                    argStart = i + 1;
                    break;
                }
            }
            if (inputFile.empty()) {
                std::cerr << "பிழை: 'pk.arpk' கோப்பு இல்லை மற்றும் 'run' செய்ய வேண்டிய கோப்பும் "
                             "குறிப்பிடப்படவில்லை.\n";
                return 1;
            }
        }
    }

    for (int i = argStart; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-C") {
            i++;
            continue;
        }
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--emit-ast") {
            emitAst = true;
        } else if (arg == "--emit-hir") {
            emitHir = true;
        } else if (arg == "--emit-thir") {
            emitThir = true;
        } else if (arg == "--emit-mir") {
            emitMir = true;
        } else if (arg == "--emit-llvm") {
            emitLlvm = true;
        } else if (arg == "--run") {
            runJit = true;
        } else if (arg == "-g" || arg == "--debug") {
            generateDebugInfo = true;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "அகம் (Agam) கம்பைலர் பதிப்பு " << AGAM_VERSION << "\n";
            return 0;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--lib-path" && i + 1 < argc) {
            g_stdEnvPath = trim(argv[++i]);
        } else if (arg.substr(0, 2) == "-O" && arg.size() == 3) {
            optLevel = Optimizer::parseLevel(arg.substr(2));
        } else if (arg[0] != '-') {
            inputFile = arg;
        } else {
            std::cerr << "அறியப்படாத விருப்பம்: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "பிழை: உள்ளீட்டு கோப்பு (input file) இல்லை\n";
        return 1;
    }

    // ── Parse with Imports ──────────────────────────────────────────────────
    std::set<std::string> visited;
    auto ast = parseWithImports(inputFile, visited);
    if (!ast) {
        return 1;
    }

    if (emitAst) {
        ASTPrinter printer;
        ast->accept(printer);
        return 0;
    }

    // ── Semantic Analysis ──────────────────────────────────────────────────
    ScopeResolver scopeResolver;
    if (!scopeResolver.resolve(*ast, g_diagEngine)) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return 1;
    }

    TypeChecker typeChecker;
    if (!typeChecker.check(*ast, g_diagEngine)) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return 1;
    }

    // ── Monomorphization ──────────────────────────────────────────────────
    // Check for main function in the entry AST.
    bool hasMain = false;
    for (const auto &fn : ast->functions) {
        if (fn->name == "மைய") {
            hasMain = true;
            break;
        }
    }
    if (!hasMain) {
        std::cerr << "பிழை: " << inputFile << " இல் 'மைய' (main) செயல்பாடு காணப்படவில்லை\n";
        return 1;
    }

    Monomorphizer monomorphizer;
    monomorphizer.monomorphize(*ast);

    // Re-run semantic analysis on specialized nodes
    g_diagEngine.clear();
    if (!scopeResolver.resolve(*ast, g_diagEngine)) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return 1;
    }
    if (!typeChecker.check(*ast, g_diagEngine)) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return 1;
    }

    // ── HIR ─────────────────────────────────────────────────────────────────
    HirBuilder hirBuilder;
    auto hir = hirBuilder.build(*ast);
    if (hirBuilder.hasErrors()) {
        for (auto &e : hirBuilder.errors())
            std::cerr << e << "\n";
        return 1;
    }

    if (emitHir) {
        std::cout << "=== HIR ===" << std::endl;
        for (auto &fn : hir->functions) {
            std::cout << "fn " << fn->name << " (id=" << fn->id << ")";
            std::cout << " -> " << typeKindToString(fn->returnTypeInfo.kind) << std::endl;
        }
        return 0;
    }

    // ── Lower to THIR ──────────────────────────────────────────────────────
    ThirBuilder thirBuilder;
    auto thir = thirBuilder.build(*hir, g_diagEngine);
    if (!thir || g_diagEngine.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(g_diagEngine);
        return 1;
    }

    if (emitThir) {
        for (auto &fn : thir->functions) {
            std::cout << "fn " << fn->name << " -> " << typeKindToString(fn->returnTypeInfo.kind)
                      << std::endl;
        }
        return 0;
    }

    // ── MIR ─────────────────────────────────────────────────────────────────
    MirBuilder mirBuilder;
    auto mir = mirBuilder.build(*thir);

    // ── MIR Optimization ───────────────────────────────────────────────────
    if (optLevel != Optimizer::Level::O0) {
        MirOptimizer::optimize(*mir);
    }

    if (emitMir) {
        std::cout << MirPrinter::print(*mir);
        return 0;
    }

    // ── Codegen ─────────────────────────────────────────────────────────────
    CodeGenerator codegen;
    codegen.setDebugInfo(generateDebugInfo);
    if (!codegen.generate(*mir)) {
        std::cerr << "பிழை: குறியாக்கத்தில் (code generation) பிழை ஏற்பட்டது\n";
        return 1;
    }

    // ── Target Machine & Vectorization Setup ─────────────────────────────
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto targetTriple = llvm::Triple(llvm::sys::getDefaultTargetTriple());
#if LLVM_VERSION_MAJOR >= 22
    codegen.getModule()->setTargetTriple(targetTriple);
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
#else
    codegen.getModule()->setTargetTriple(targetTriple.getTriple());
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple.getTriple(), error);
#endif
    if (!target) {
        std::cerr << "பிழை: " << error << "\n";
        return 1;
    }

    std::string hostCPU = llvm::sys::getHostCPUName().str();
    llvm::StringMap<bool> hostFeatures = fetchHostCPUFeatures();
    std::string featureStr;
    for (auto &f : hostFeatures) {
        if (!featureStr.empty()) featureStr += ",";
        featureStr += (f.second ? "+" : "-");
        featureStr += f.first().str();
    }

    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
#if LLVM_VERSION_MAJOR >= 22
    auto targetMachine = target->createTargetMachine(targetTriple, hostCPU, featureStr, opt, RM);
#else
    auto targetMachine = target->createTargetMachine(targetTriple.getTriple(), hostCPU, featureStr, opt, RM);
#endif

    codegen.getModule()->setDataLayout(targetMachine->createDataLayout());

    if (optLevel != Optimizer::Level::O0) {
        Optimizer::optimize(*codegen.getModule(), optLevel, targetMachine);
    }

    if (emitLlvm) {
        std::cout << codegen.getIRString();
        return 0;
    }

    // ── JIT Execute ─────────────────────────────────────────────────────────
    if (runJit) {
        int exitCode = Executor::run(*codegen.getModule(), "மைய");
        return exitCode;
    }

    // ── Emit C `main` wrapper for AOT linking ───────────────────────────────
    // The system linker expects a `main` symbol. Agam's entry point is `மைய`.
    // We emit: int main() { return மைய(); }
    {
        auto *mod = codegen.getModule();
        auto &ctx = mod->getContext();
        llvm::Function *entryFn = mod->getFunction("மைய");
        if (entryFn && !mod->getFunction("main")) {
            llvm::FunctionType *mainTy = llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ctx), false);
            llvm::Function *mainFn = llvm::Function::Create(
                mainTy, llvm::Function::ExternalLinkage, "main", mod);
            llvm::BasicBlock *bb = llvm::BasicBlock::Create(ctx, "entry", mainFn);
            llvm::IRBuilder<> b(bb);
            llvm::Value *ret = b.CreateCall(entryFn);
            b.CreateRet(ret);
        }
    }

    if (outputFile.empty()) {
        // Default output to target/debug/ or target/release/ directory
        fs::path inputPath(inputFile);
        fs::path stem = inputPath.stem(); // "main" from "main.agam"
        fs::path targetSubDir = fs::current_path() / "target" / (optLevel != Optimizer::Level::O0 ? "release" : "debug");
        
        if (!fs::exists(targetSubDir)) {
            fs::create_directories(targetSubDir);
        }

#ifdef _WIN32
        outputFile = (targetSubDir / (stem.string() + ".exe")).string();
#else
        outputFile = (targetSubDir / stem.string()).string();
#endif
    } else {
        // Ensure parent output directory exists if custom -o path provided
        fs::path outPath(outputFile);
        if (outPath.has_parent_path() && !fs::exists(outPath.parent_path())) {
            fs::create_directories(outPath.parent_path());
        }
    }

    // ── Emit object file to a temp path ─────────────────────────────────────
    std::string objFile = outputFile + ".o";

    {
        std::error_code ec;
        llvm::raw_fd_ostream dest(objFile, ec, llvm::sys::fs::OF_None);

        if (ec) {
            std::cerr << "கோப்பினைத் திறக்க முடியவில்லை: " << ec.message() << std::endl;
            return 1;
        }

        llvm::legacy::PassManager pass;
#if LLVM_VERSION_MAJOR >= 18
        auto FileType = llvm::CodeGenFileType::ObjectFile;
#else
        auto FileType = llvm::CGFT_ObjectFile;
#endif

        if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            std::cerr << "இந்த வகை கோப்பினை உருவாக்க முடியாது" << std::endl;
            fs::remove(objFile);
            return 1;
        }

        pass.run(*codegen.getModule());
        dest.flush();
    }

    // ── Locate runtime library (libagam_rt.a) ───────────────────────────────
    fs::path rtLib;
    std::vector<fs::path> rtSearchPaths = {
        g_exeDir / ".." / "lib" / "agam" / "libagam_rt.a",  // Installed layout (Unix)
        g_exeDir / ".." / "lib" / "libagam_rt.a",            // Build-tree layout
        g_exeDir / "lib" / "libagam_rt.a",                   // Flat layout
        g_exeDir / ".." / "lib" / "agam" / "agam_rt.lib",    // Installed layout (Windows/MSVC)
    };
    for (auto& p : rtSearchPaths) {
        if (fs::exists(p)) {
            rtLib = fs::canonical(p);
            break;
        }
    }

    // ── Link into executable ────────────────────────────────────────────────
    std::string linkCmd;

#ifdef _WIN32
    linkCmd = "gcc";
#else
    linkCmd = "cc";
#endif

    // Object file
    linkCmd += " \"" + objFile + "\"";

    // Runtime library
    if (!rtLib.empty()) {
        linkCmd += " \"" + rtLib.string() + "\"";
    } else {
        std::cerr << "எச்சரிக்கை: libagam_rt.a கண்டுபிடிக்கப்படவில்லை. இணைப்பு தோல்வியடையலாம்.\n";
    }

    // System libraries
    linkCmd += " -lm -lpthread";

#ifdef _WIN32
    linkCmd += " -lws2_32";
#endif

    // SQLite3 (if available on system; linker will only pull symbols actually referenced)
    linkCmd += " -lsqlite3";

    // Output
    linkCmd += " -o \"" + outputFile + "\"";

    int linkResult = std::system(linkCmd.c_str());

    // Clean up temp object file
    fs::remove(objFile);

    if (linkResult != 0) {
        std::cerr << "பிழை: இணைப்பு (linking) தோல்வியடைந்தது. "
                  << "C கம்பைலர் (cc/gcc) நிறுவப்பட்டுள்ளதா எனச் சரிபார்க்கவும்.\n";
        return 1;
    }

    return 0;
}
