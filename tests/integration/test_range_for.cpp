#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/semantic/scope_resolver.h"
#include "agam/semantic/type_checker.h"
#include "agam/hir/hir_builder.h"
#include "agam/thir/thir_builder.h"
#include "agam/mir/mir_builder.h"
#include "agam/codegen/codegen.h"
#include "agam/codegen/executor.h"
#include "agam/utils/diagnostic.h"
#include "agam/utils/diagnostic_renderer.h"

using namespace agam;

TEST(PipelineTest, RangeForLoopWithLen) {
    std::string source = R"(
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
    )";

    SourceManager sm;
    DiagnosticEngine diag(sm);

    // Lex
    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    if (diag.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(diag);
    }
    ASSERT_FALSE(diag.hasErrors());

    // Parse
    Parser parser(tokens, source, "test.agam", diag);
    auto ast = parser.parse();
    if (diag.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(diag);
    }
    ASSERT_NE(ast, nullptr);
    ASSERT_FALSE(diag.hasErrors());

    // Semantic Analysis
    ScopeResolver scopeResolver;
    if (!scopeResolver.resolve(*ast, diag)) {
        DiagnosticRenderer::renderToTerminal(diag);
    }
    ASSERT_TRUE(diag.hasErrors() == false);

    TypeChecker typeChecker;
    if (!typeChecker.check(*ast, diag)) {
        DiagnosticRenderer::renderToTerminal(diag);
        EXPECT_TRUE(false) << "Type checking failed";
    }

    // HIR
    HirBuilder hirBuilder;
    auto hir = hirBuilder.build(*ast);
    ASSERT_NE(hir, nullptr);

    // THIR
    ThirBuilder thirBuilder;
    auto thir = thirBuilder.build(*hir, diag);
    if (diag.hasErrors()) {
        DiagnosticRenderer::renderToTerminal(diag);
    }
    ASSERT_NE(thir, nullptr);
    ASSERT_FALSE(diag.hasErrors());

    // MIR
    MirBuilder mirBuilder;
    auto mir = mirBuilder.build(*thir);
    ASSERT_NE(mir, nullptr);

    // Codegen
    CodeGenerator codegen;
    ASSERT_TRUE(codegen.generate(*mir));

    // Execute
    int result = Executor::run(*codegen.getModule(), "மைய");
    
    // sum(0+1+2) = 3
    EXPECT_EQ(result, 3);
}
