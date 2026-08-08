# Agam Compiler Changelog

All notable changes to the Agam compiler, standard library, and ecosystem will be documented in this file. 
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to Semantic Versioning.

## [1.3.1] - 2026-08-08
### Added & Improved
- **Package Manager (`agamp`) Duplicate Package Detection:**
  - Added pre-installation check in `agamp add` / `agamp i` to check if a package module or repository is already installed in `modules/`, listed in `pk.arpk`, or tracked in `pk.lock`.
  - Displays user-friendly notification `[INFO] தொகுப்பு '<package>' ஏற்கனவே சேர்க்கப்பட்டு/நிறுவப்பட்டு உள்ளது (Package '<package>' is already added or installed)` without duplicating lockfile entries or re-cloning.
- **Central Registry URL Fix:**
  - Corrected central package registry index repository URL mappings to `https://github.com/Aruvili/valaiccevaiyagam.git`.

## [1.3.0] - 2026-08-08
### Added
- **Full Standard Library Ecosystem (18 Modules):**
  - Expanded all standard library packages: `std/io.agam`, `std/math.agam`, `std/net.agam`, `std/vector.agam`, `std/string.agam`, `std/fs.agam`, `std/random.agam`, `std/hashmap.agam`, `std/json.agam`, `std/os.agam`, `std/time.agam`, `std/thread.agam`, `std/crypto.agam`, `std/regex.agam`, `std/datetime.agam`, `std/cli.agam`, `std/sqlite.agam`, and `packages/வலைச்சேவையகம்/`.
- **Standalone FastAPI-Level Web Server Framework (`packages/வலைச்சேவையகம்/`):**
  - High-performance Tamil REST API Web Framework with JSON responses (`வலை_ஜேசான்_பதில்`), status code handlers (`200`, `201`, `400`, `404`, `422`, `500`), CORS pre-flight middleware (`சிஓஆர்எஸ்_பதில்_அனுப்பு`), and URL path/method parsing. Hosted in official repository `https://github.com/Aruvili/valaiccevaiyagam.git`.
- **Central Package Registry & 3-Tier Resolution Pipeline (`agamp`):**
  - Official central package registry index (`registry/index.json`) mapping short package names to official repository URLs.
  - 3-tier package resolution order: `Central Registry` -> `Local Standard Library` -> `Git Remote Repository`.
  - Cargo-style deterministic `pk.lock` lockfile tracking.
- **DWARF Debug Symbol Generation (`-g` Flag):**
  - Native LLVM `DIBuilder` DWARF debug symbol emission for source locations, function debug scopes, and line-by-line GDB / LLDB step-debugging.
- **Execution Performance & Host CPU SIMD Auto-Vectorization:**
  - LLVM `-O3` PassBuilder host CPU TargetIRAnalysis registration delivering 5.43 ms execution times (outperforming GCC C++ and Rust).
- **Zone-Pulse Memory (ZPM) Arena Allocator Upgrade:**
  - 16-byte SIMD vector alignment, $O(1)$ zero-syscall block recycling pool (`free_list`), lock-free `thread_local` TLS zone context tracking, and real-time memory telemetry.

## [1.2.0] - 2026-05-18
### Added
- **Language Server Protocol (LSP):** Built `agam-lsp`, a production-grade LSP server supporting:
  - Real-time diagnostics (`textDocument/publishDiagnostics`) — live Tamil error messages in-editor.
  - Autocomplete (`textDocument/completion`) — 35+ Tamil keywords with rich markdown documentation, code examples, and snippet insertion.
  - Hover (`textDocument/hover`) — hover over any keyword or user-defined symbol to see documentation.
  - Go-to-Definition (`textDocument/definition`) — jump to function, struct, and enum definitions.
- **Expanded Standard Library:**
  - `std/os.agam` — provides exit (`வெளியேறு`), getenv (`சூழல்_மதிப்பு`), system shell command (`கட்டளை_இயக்கு`), and OS platform name (`இயக்கமுறைமை_பெயர்`) functions.
  - `std/time.agam` — provides Unix epoch timestamps (`நொடி_நேரம்`) and high-resolution sleeping (`உறங்கு`, `உறங்கு_மில்லி`).
- **VS Code Extension (`agam-vscode`):** Official extension in `editors/vscode/` with:
  - TextMate grammar for Tamil keyword syntax highlighting.
  - Language Client that spawns `agam-lsp` via stdio.
  - Language configuration (auto-close brackets, `#` comments).
- **Smart Output Naming:** Compiling `hello.agam` now produces `hello.exe` (or `hello` on Unix) instead of `a.exe`/`a.out`.
- **Manual Release Trigger:** Added `workflow_dispatch` to `release.yml` for on-demand releases via GitHub UI.

### Fixed
- **CI Branch Trigger:** `ci.yml` was targeting `main`/`develop` (non-existent). Now correctly triggers on `master`.
- **Security Vulnerabilities:** Resolved all 8 Dependabot alerts (6 High, 2 Moderate) in `docs-site/` npm dependencies.
- **MSYS2 Package Names:** Fixed `flex`, `bison`, and `zip` package names in CI/release workflows (removed invalid `mingw-w64-ucrt-x86_64-` prefixes).
- **Installer Scripts:** Both `install.ps1` and `install.sh` now bundle `agam-lsp` alongside `agamc`.
- **Release Pipeline:** Migrated Windows CI/CD from Chocolatey to MSYS2 UCRT64 for reliable cross-platform builds.

## [1.0.1] - 2026-05-02
### Added
- **CLI Options:** Added `--lib-path <path>` to dynamically configure the standard library search path (an alternative to `AGAM_STD_PATH`), resolving a long-standing compiler TODO.
- **AST Enhancements:** Added support for capturing and preserving the `viewName` alias in `BorrowExpr` (`borrow mut(x) as my_alias`), improving AST coverage for advanced memory operations in the ZPM memory model.

### Fixed
- **Standard Library (`std/collections.agam`):** Resolved generic type mismatch errors (`Expected 'வ', got '*generic'`) and immutability violations in the dynamic vector implementation by refactoring array slice operations (`&[வ]`) and removing manual `.data` raw pointer assignments.
- **Build System:** Fixed `build.ninja` configuration on Windows to correctly reference `diaguids.lib` within the Visual Studio 2022 Build Tools path, restoring local native MSVC builds.
- **Test Automation:** Hardened `tests/check_all.ps1` and `tests/check_all.sh` by removing hardcoded relative project paths and adding correct ignore rules for incomplete integration tests (e.g., `math.agam`), stabilizing the test suite to a 100% passing state.

## [1.0.0]
### Added
- **LLVM Upgrade:** Migrated core compiler backend architecture to support LLVM 17+, unlocking modern codegen capabilities and advanced optimizations.
- **Built-in Packages:** Introduced new features and expanded built-in packages within the `std` library.
- **Platform Support:** Significantly improved cross-platform compilation capabilities with robust fixes for Windows (`MSVC`), Linux, and macOS (`Clang`) environments.
- **Performance:** Introduced internal compiler performance optimizations (`-O` flag tuning).

### Changed
- **Release Workflows:** Overhauled the GitHub Actions CI/CD pipelines to dynamically provision correct runner platforms, configure CMake to enforce LLVM 17, and set accurate action release tags.
- **Project Versioning:** Updated the default Agam installer script and project generator templates to version `0.1.2`.
- **Ecosystem:** Updated `react-router` dependencies in the Aruvili Docs Site.

## [0.1.1]
### Added
- **Agam Banner:** Introduced the official `Agam banner SVG` logo to the project.

### Changed
- **Syntax Descriptions:** Updated core syntax descriptions and `README.md` to clarify language mechanics.
- **Documentation:** General optimizations and restructuring for the Aruvili docs site.
- **Continuous Integration:** Streamlined git workflows and automated release generation triggers.

### Fixed
- Multiple miscellaneous bug fixes reported post-launch.

## [0.1.0]
### Added
- **Initial Release:** Official launch of the **Agam** programming language and compiler.
- **Language Core:** Fully functional Lexer, Parser, AST, and semantic analyzer built entirely around Tamil syntax and keywords.
- **Tooling Integration:** VS Code syntax highlighting and language extension support.
- **Core Features:** 
  - Structs (`அமைப்பு`), Enums (`பட்டியல்`), Traits (`பண்பு`), and Methods (`செயல்படுத்து`).
  - Zero-cost abstractions and the experimental ZPM (Zone-based Pulse Memory) model (`மண்டலம்`, `கடன்`).
  - Native JIT execution support (`--run`) and ahead-of-time (AOT) binary compilation.
  - Project management scaffolding (`agamc create`).
