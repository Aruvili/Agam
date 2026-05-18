cmake_minimum_required(VERSION 3.20)
project(Agam
    VERSION 1.0.1
    DESCRIPTION "Agam Programming Language Compiler (Production)"
    LANGUAGES C CXX
)
add_definitions(-DAGAM_VERSION="${Agam_VERSION}")

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ── AddressSanitizer (optional, for memory safety testing) ────────────────────
option(AGAM_ENABLE_ASAN "Enable AddressSanitizer for memory safety checks" OFF)
if(AGAM_ENABLE_ASAN)
    message(STATUS "AddressSanitizer ENABLED")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    # Disable container annotations to avoid mismatch with non-ASan LLVM
    add_definitions(-D_DISABLE_VECTOR_ANNOTATION -D_DISABLE_STRING_ANNOTATION)
endif()

# ── Output directories ───────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) 

# ── Find LLVM ─────────────────────────────────────────────────────────────────
# Force CMake to prefer static libraries by avoiding the shared 'LLVM' component
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# LLVM includes are added per-target as SYSTEM to suppress their warnings
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
            if(APPLE)
                list(APPEND LLVM_SYSTEM_LIBS z zstd dl pthread m)
            else()
                list(APPEND LLVM_SYSTEM_LIBS z zstd tinfo dl pthread m)
            endif()
        elseif(WIN32)
            list(APPEND LLVM_SYSTEM_LIBS psapi shell32 ole32 uuid advapi32)
        endif()
    endif()
endif()
message(STATUS "LLVM System Libs: ${LLVM_SYSTEM_LIBS}")

if(WIN32 AND LLVM_SYSTEM_LIBS)
    # Fix for diaguids.lib: LLVM sometimes hardcodes the absolute path based on the VS edition 
    # it was compiled with (e.g. Enterprise vs BuildTools). We strip the path to rely on the LIB env var.
    set(CLEANED_LIBS "")
    foreach(LIB IN LISTS LLVM_SYSTEM_LIBS)
        if(LIB MATCHES "diaguids\\.lib$")
            list(APPEND CLEANED_LIBS "diaguids.lib")
        else()
            list(APPEND CLEANED_LIBS "${LIB}")
        endif()
    endforeach()
    set(LLVM_SYSTEM_LIBS ${CLEANED_LIBS})
endif()

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

# Mark LLVM includes as SYSTEM to suppress their warnings
target_include_directories(agam_lib SYSTEM PUBLIC
    ${LLVM_INCLUDE_DIRS}
)

# ── Strict warnings for our own code only ────────────────────────────────────
if(MSVC)
    target_compile_options(agam_lib PRIVATE /W4 /WX)
else()
    target_compile_options(agam_lib PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)
endif()

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

if(MSVC)
    target_compile_options(agamc PRIVATE /W4 /WX)
endif()

# Fix LLVM imported targets containing absolute paths to diaguids.lib
if(WIN32)
    set(PROPS_TO_CHECK
        INTERFACE_LINK_LIBRARIES
        IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
        IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
        IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO
        IMPORTED_LINK_INTERFACE_LIBRARIES_MINSIZEREL
        IMPORTED_LINK_INTERFACE_LIBRARIES
    )
    foreach(tgt ${LLVM_LIBS})
        if(TARGET ${tgt})
            foreach(prop IN LISTS PROPS_TO_CHECK)
                get_target_property(il ${tgt} ${prop})
                if(il)
                    set(new_il "")
                    foreach(lib IN LISTS il)
                        if(lib MATCHES "diaguids\\.lib$")
                            list(APPEND new_il "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/DIA SDK/lib/amd64/diaguids.lib")
                        else()
                            list(APPEND new_il "${lib}")
                        endif()
                    endforeach()
                    set_target_properties(${tgt} PROPERTIES ${prop} "${new_il}")
                endif()
            endforeach()
        endif()
    endforeach()
endif()

# ── Agam LSP Server ──────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(agam-lsp
    src/lsp/main.cpp
)

target_link_libraries(agam-lsp PRIVATE
    agam_lib
    nlohmann_json::nlohmann_json
)

if(MSVC)
    target_compile_options(agam-lsp PRIVATE /W4 /WX)
endif()

# ── Testing ──────────────────────────────────────────────────────────────────
option(AGAM_BUILD_TESTS "Build the Agam test suite" ON)

if(AGAM_BUILD_TESTS)
    enable_testing()

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

# Install the compiler and LSP
install(TARGETS agamc agam-lsp
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
