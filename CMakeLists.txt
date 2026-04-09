cmake_minimum_required(VERSION 3.20)
project(Agam
    VERSION 1.0.0
    DESCRIPTION "Agam Programming Language Compiler (Production)"
    LANGUAGES C CXX
)
add_definitions(-DAGAM_VERSION="${Agam_VERSION}")

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Output directories ───────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Find LLVM ─────────────────────────────────────────────────────────────────
# Force CMake to prefer static libraries by avoiding the shared 'LLVM' component
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Map LLVM components to libraries
# We use individual components to ensure static linking
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
    Target
    ExecutionEngine
    OrcJIT
    native
    nativecodegen
)

# Extract system libraries required for static LLVM linking
# (e.g., zlib, tinfo, shell32, etc.)
if(NOT LLVM_SYSTEM_LIBS)
    if(COMMAND llvm_map_components_to_system_libs)
        llvm_map_components_to_system_libs(LLVM_SYSTEM_LIBS)
    else()
        # Fallback/Manual detection for common static LLVM dependencies
        set(LLVM_SYSTEM_LIBS "")
        if(UNIX)
            list(APPEND LLVM_SYSTEM_LIBS z zstd tinfo dl pthread m)
        elseif(WIN32)
            list(APPEND LLVM_SYSTEM_LIBS psapi shell32 ole32 uuid advapi32)
        endif()
    endif()
endif()
message(STATUS "LLVM System Libs: ${LLVM_SYSTEM_LIBS}")

# ── Agam compiler library (shared between compiler and tests) ────────────────
add_library(agam_lib STATIC
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast.cpp
    src/hir/hir_builder.cpp
    src/thir/thir_builder.cpp
    src/mir/mir_builder.cpp
    src/mir/mir_printer.cpp
    src/mir/mir_optimizer.cpp
    src/semantic/symbol_table.cpp
    src/semantic/type_checker.cpp
    src/semantic/scope_resolver.cpp
    src/semantic/monomorphizer.cpp
    src/codegen/codegen.cpp
    src/codegen/optimizer.cpp
    src/codegen/executor.cpp
    src/utils/diagnostic_renderer.cpp
)

target_include_directories(agam_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(agam_lib PUBLIC
    ${LLVM_LIBS}
    ${LLVM_SYSTEM_LIBS}
)

# ── Agam compiler executable ────────────────────────────────────────────────
add_executable(agamc
    src/main.cpp
)

target_link_libraries(agamc PRIVATE
    agam_lib
)

# ── Testing ──────────────────────────────────────────────────────────────────
option(AGAM_BUILD_TESTS "Build the Agam test suite" ON)

if(AGAM_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Prevent GoogleTest from overriding compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Lexer tests
    add_executable(agam_test_lexer tests/unit/test_lexer.cpp)
    target_link_libraries(agam_test_lexer PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME LexerTests COMMAND agam_test_lexer)

    # Parser tests
    add_executable(agam_test_parser tests/unit/test_parser.cpp)
    target_link_libraries(agam_test_parser PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME ParserTests COMMAND agam_test_parser)

    # Semantic tests
    add_executable(agam_test_semantic tests/unit/test_semantic.cpp)
    target_link_libraries(agam_test_semantic PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME SemanticTests COMMAND agam_test_semantic)

    # Codegen tests
    add_executable(agam_test_codegen tests/unit/test_codegen.cpp)
    target_link_libraries(agam_test_codegen PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME CodegenTests COMMAND agam_test_codegen)

    # Integration tests
    add_executable(agam_test_pipeline tests/integration/test_pipeline.cpp)
    target_link_libraries(agam_test_pipeline PRIVATE agam_lib GTest::gtest_main)
    target_compile_definitions(agam_test_pipeline PRIVATE
        AGAM_SAMPLES_DIR="${CMAKE_SOURCE_DIR}/tests/samples"
    )
    add_test(NAME PipelineTests COMMAND agam_test_pipeline)

    # HIR tests
    add_executable(agam_test_hir tests/unit/test_hir.cpp)
    target_link_libraries(agam_test_hir PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME HirTests COMMAND agam_test_hir)

    # THIR tests
    add_executable(agam_test_thir tests/unit/test_thir.cpp)
    target_link_libraries(agam_test_thir PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME ThirTests COMMAND agam_test_thir)

    # MIR tests
    add_executable(agam_test_mir tests/unit/test_mir.cpp)
    target_link_libraries(agam_test_mir PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME MirTests COMMAND agam_test_mir)

    # Range-based for loop tests
    add_executable(agam_test_range_for tests/integration/test_range_for.cpp)
    target_link_libraries(agam_test_range_for PRIVATE agam_lib GTest::gtest_main)
    add_test(NAME RangeForTests COMMAND agam_test_range_for)
endif()

# ── Installation ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)

# Install the compiler
install(TARGETS agamc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install the standard library
install(DIRECTORY ${CMAKE_SOURCE_DIR}/std/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/agam/std
)

# Install headers (for FFI/Extension development)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
