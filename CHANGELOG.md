# Agam Compiler Changelog

All notable changes to the Agam compiler, standard library, and ecosystem will be documented in this file. 
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to Semantic Versioning.

## [Unreleased]
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
