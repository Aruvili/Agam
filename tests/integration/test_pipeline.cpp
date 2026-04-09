/*
 * Agam Integration Tests (End-to-End Pipeline)
 *
 * Tests the full compilation pipeline by parsing source strings,
 * running semantic analysis, and generating LLVM IR.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/semantic/type_checker.h"
#include "agam/semantic/scope_resolver.h"
#include "agam/codegen/codegen.h"
#include "agam/utils/diagnostic.h"
#include "agam/utils/diagnostic_renderer.h"
#include <string>
#include <vector>

using namespace agam;

/// Pipeline result for testing.
struct PipelineResult {
    bool parseOk = false;
    bool scopeOk = false;
    bool typeOk = false;
    bool codegenOk = false;
    std::string ir;
    int errorCount = 0;
    std::vector<std::string> errorMessages;
};

/// Run the full pipeline on a source string.
static PipelineResult runPipeline(const std::string &source) {
    PipelineResult result;
    SourceManager sm;
    DiagnosticEngine diag(sm);

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    if (diag.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(diag);
        result.errorCount = diag.diagnostics().size();
        for (const auto& d : diag.diagnostics()) result.errorMessages.push_back(d.message);
        return result;
    }

    Parser parser(tokens, source, "test.agam", diag);
    auto program = parser.parse();
    result.parseOk = !diag.hasErrors() && program != nullptr;
    if (!result.parseOk) {
        DiagnosticRenderer::renderToTerminal(diag);
        result.errorCount = diag.diagnostics().size();
        for (const auto& d : diag.diagnostics()) result.errorMessages.push_back(d.message);
        return result;
    }

    ScopeResolver scopeResolver;
    result.scopeOk = scopeResolver.resolve(*program, diag);
    if (!result.scopeOk) {
        DiagnosticRenderer::renderToTerminal(diag);
        result.errorCount = diag.diagnostics().size();
        for (const auto& d : diag.diagnostics()) result.errorMessages.push_back(d.message);
        return result;
    }

    TypeChecker typeChecker;
    result.typeOk = typeChecker.check(*program, diag);
    if (!result.typeOk) {
        DiagnosticRenderer::renderToTerminal(diag);
        result.errorCount = diag.diagnostics().size();
        for (const auto& d : diag.diagnostics()) result.errorMessages.push_back(d.message);
        return result;
    }

    CodeGenerator codegen;
    codegen.generate(*program);
    result.ir = codegen.getIRString();
    result.codegenOk = true; 
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Successful Compilation Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(PipelineTest, ArithmeticProgram) {
    auto result = runPipeline(R"(
        செயல் மைய(): எண் {
            மாறி a: எண் = 10;
            மாறி b: எண் = 20;
            மாறி c: எண் = a + b * 2;
            விடை c;
        }
    )");

    EXPECT_TRUE(result.parseOk) << "Parse failed";
    EXPECT_TRUE(result.scopeOk) << "Scope resolution failed";
    EXPECT_TRUE(result.typeOk) << "Type checking failed";
    EXPECT_TRUE(result.codegenOk) << "Code generation failed";
    EXPECT_FALSE(result.ir.empty());
    if (result.ir.find("define i32 @மைய()") == std::string::npos) {
        std::cout << "DEBUG: IR does not contain @மைய(). Actual IR snippet:\n" << result.ir.substr(0, 500) << std::endl;
    }
    EXPECT_NE(result.ir.find("define i32 @\"\\E0\\AE\\AE\\E0\\AF\\88\\E0\\AE\\AF\"()"), std::string::npos);
}

TEST(PipelineTest, PointerDerefProgram) {
    auto result = runPipeline(R"(
        செயல் பார்(p: *எண்): வெற்று {
            *p = *p + 10;
        }

        செயல் மைய(): எண் {
            மாறி x: எண் = 5;
            பார்(&x);
            விடை x;
        }
    )");

    if (!result.parseOk && !result.errorMessages.empty()) {
        std::cerr << "Parse errors:\n";
        for (const auto& err : result.errorMessages) std::cerr << err << "\n";
    }
    EXPECT_TRUE(result.parseOk) << "Parse failed";
    EXPECT_TRUE(result.scopeOk) << "Scope resolution failed";
    EXPECT_TRUE(result.typeOk) << "Type checking failed";
    EXPECT_TRUE(result.codegenOk) << "Code generation failed";
    EXPECT_FALSE(result.ir.empty());
    EXPECT_NE(result.ir.find("define i32 @\"\\E0\\AE\\AE\\E0\\AF\\88\\E0\\AE\\AF\"()"), std::string::npos);
    EXPECT_NE(result.ir.find("define void @\"\\E0\\AE\\AA\\E0\\AE\\BE\\E0\\AE\\B0\\E0\\AF\\8D\"("), std::string::npos);
}

TEST(PipelineTest, ArraySliceForLoop) {
    auto result = runPipeline(R"(
        செயல் கூட்டு(துண்டு: &[எண்]): எண் {
            மாறி நிலை s: எண் = 0;
            சுற்று (x உள் துண்டு.len()) {
                s = s + x;
            }
            விடை s;
        }

        செயல் மைய(): எண் {
            மாறி உறுப்பு: [எண்; 3] = [1, 2, 3];
            விடை கூட்டு(&உறுப்பு);
        }
    )");

    if (!result.parseOk && !result.errorMessages.empty()) {
        std::cerr << "Parse errors in ArraySliceForLoop:\n";
        for (const auto& err : result.errorMessages) std::cerr << err << "\n";
    }
    if (!result.typeOk && !result.errorMessages.empty()) {
        std::cerr << "Type errors in ArraySliceForLoop:\n";
        for (const auto& err : result.errorMessages) std::cerr << err << "\n";
    }
    if (!result.codegenOk) {
        std::cerr << "Full LLVM IR on failure:\n" << result.ir << "\n";
    }
    EXPECT_TRUE(result.parseOk) << "Parse failed";
    EXPECT_TRUE(result.scopeOk) << "Scope resolution failed";
    EXPECT_TRUE(result.typeOk) << "Type checking failed";
    EXPECT_TRUE(result.codegenOk) << "Code generation failed";
    EXPECT_FALSE(result.ir.empty());
}

TEST(PipelineTest, MultiLevelPointerProgram) {
    auto result = runPipeline(R"(
        செயல் அமை_மதிப்பு(p: **எண்): வெற்று {
            **p = 42;
        }

        செயல் மைய(): எண் {
            மாறி x: எண் = 5;
            மாறி சுட்டி: *எண் = &x;
            அமை_மதிப்பு(&சுட்டி);
            விடை x;
        }
    )");

    EXPECT_TRUE(result.parseOk) << "Parse failed";
    EXPECT_TRUE(result.scopeOk) << "Scope resolution failed";
    EXPECT_TRUE(result.typeOk) << "Type checking failed";
    EXPECT_TRUE(result.codegenOk) << "Code generation failed";
    EXPECT_FALSE(result.ir.empty());
    EXPECT_NE(result.ir.find("define i32 @\"\\E0\\AE\\AE\\E0\\AF\\88\\E0\\AE\\AF\"()"), std::string::npos);
    EXPECT_NE(result.ir.find("define void @\"\\E0\\AE\\85\\E0\\AE\\AE\\E0\\AF\\88_\\E0\\AE\\AE\\E0\\AE\\A4\\E0\\AE\\BF\\E0\\AE\\AA\\E0\\AF\\8D\\E0\\AE\\AA\\E0\\AF\\81\"("), std::string::npos);
}

TEST(PipelineTest, FunctionCallProgram) {
    auto result = runPipeline(R"(
        செயல் கூடு(a: எண், b: எண்): எண் {
            விடை a + b;
        }

        செயல் மைய(): எண் {
            மாறி முடிவு: எண் = கூடு(3, 4);
            விடை முடிவு;
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_TRUE(result.typeOk);
    EXPECT_TRUE(result.codegenOk);
}

TEST(PipelineTest, ControlFlowProgram) {
    auto result = runPipeline(R"(
        செயல் தனி(x: எண்): எண் {
            எனில் (x < 0) {
                விடை -x;
            }
            விடை x;
        }

        செயல் மைய(): எண் {
            விடை தனி(-5);
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_TRUE(result.typeOk);
    EXPECT_TRUE(result.codegenOk);
}

TEST(PipelineTest, EnumProgram) {
    auto result = runPipeline(R"(
        பட்டியல் விருப்பம் { இல்லை, உள்ளது(எண்) }

        செயல் மைய(): எண் {
            மாறி விருப்பம்_: விருப்பம் = விருப்பம்::உள்ளது(42);
            மாறி முடிவு: எண் = பொருத்து (விருப்பம்_) {
                விருப்பம்::இல்லை => 0,
                விருப்பம்::உள்ளது(மதிப்பு) => மதிப்பு
            };
            விடை முடிவு;
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_TRUE(result.typeOk);
    EXPECT_TRUE(result.codegenOk);
}

TEST(PipelineTest, HeapAllocProgram) {
    auto result = runPipeline(R"(
        செயல் மைய(): எண் {
            மாறி p: *எண் = புதிய எண்;
            *p = 123;
            மாறி மதிப்பு: எண் = *p;
            நீக்கு p;
            விடை மதிப்பு;
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_TRUE(result.typeOk);
    EXPECT_TRUE(result.codegenOk);
}

TEST(PipelineTest, HeapAllocDeleteFails) {
    auto result = runPipeline(R"(
        செயல் மைய(): வெற்று {
            மாறி x: எண் = 10;
            நீக்கு x;  // Error: delete requires a pointer
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_FALSE(result.typeOk) << "Type checking should have failed for delete on non-pointer";
    EXPECT_GT(result.errorCount, 0);
}

TEST(PipelineTest, TypeMismatchFails) {
    auto result = runPipeline(R"(
        செயல் மைய(): எண் {
            மாறி x: எண் = "வணக்கம்";
            விடை x;
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_FALSE(result.typeOk);
    EXPECT_GT(result.errorCount, 0);
}

TEST(PipelineTest, UndeclaredVariableFails) {
    auto result = runPipeline(R"(
        செயல் மைய(): எண் {
            விடை x;
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_FALSE(result.scopeOk);
    EXPECT_GT(result.errorCount, 0);
}

TEST(PipelineTest, WrongArgCountFails) {
    auto result = runPipeline(R"(
        செயல் கூடு(a: எண், b: எண்): எண் {
            விடை a + b;
        }

        செயல் மைய(): எண் {
            விடை கூடு(1);
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_FALSE(result.typeOk);
    EXPECT_GT(result.errorCount, 0);
}

TEST(PipelineTest, ReturnTypeMismatchFails) {
    auto result = runPipeline(R"(
        செயல் மைய(): எண் {
            விடை "வணக்கம்";
        }
    )");

    EXPECT_TRUE(result.parseOk);
    EXPECT_TRUE(result.scopeOk);
    EXPECT_FALSE(result.typeOk);
    EXPECT_GT(result.errorCount, 0);
}
