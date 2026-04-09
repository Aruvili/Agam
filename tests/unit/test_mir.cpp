/*
 * Agam MIR Unit Tests
 *
 * Tests the THIR → MIR lowering: basic block generation,
 * expression flattening, and control flow graph construction.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/hir/hir_builder.h"
#include "agam/thir/thir_builder.h"
#include "agam/mir/mir.h"
#include "agam/mir/mir_builder.h"
#include "agam/mir/mir_printer.h"
#include "agam/utils/diagnostic.h"
#include <memory>
#include <string>

using namespace agam;

// Helper: source → AST → HIR → THIR → MIR
static std::unique_ptr<MirProgram> buildMir(const std::string &source) {
    SourceManager sm;
    DiagnosticEngine diag(sm);

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto ast = parser.parse();
    if (!ast) return nullptr;
    HirBuilder hirBuilder;
    auto hir = hirBuilder.build(*ast);
    if (!hir) return nullptr;
    ThirBuilder thirBuilder;
    auto thir = thirBuilder.build(*hir, diag);
    if (!thir) return nullptr;
    MirBuilder mirBuilder;
    return mirBuilder.build(*thir);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Basic Structure
// ═══════════════════════════════════════════════════════════════════════════════

TEST(MirTest, SimpleReturn) {
    auto mir = buildMir("செயல் மைய(): எண் { விடை 42; }");
    ASSERT_NE(mir, nullptr);
    ASSERT_EQ(mir->functions.size(), 1u);
    auto &fn = mir->functions[0];
    EXPECT_EQ(fn.name, "மைய");
    EXPECT_EQ(fn.returnTypeInfo.kind, TypeKind::Int);
    EXPECT_GE(fn.blocks.size(), 1u);
    // Entry block should have a return terminator
    auto &entry = fn.blocks[0];
    EXPECT_TRUE(std::holds_alternative<MirReturn>(entry.terminator));
}

TEST(MirTest, VarDeclCreatesLocal) {
    auto mir = buildMir("செயல் மைய(): எண் { மாறி x: எண் = 5; விடை x; }");
    auto &fn = mir->functions[0];
    // Should have at least one named local for 'x'
    bool foundX = false;
    for (auto &local : fn.locals) {
        if (local.name == "x") {
            EXPECT_EQ(local.typeInfo.kind, TypeKind::Int);
            foundX = true;
        }
    }
    EXPECT_TRUE(foundX);
}

TEST(MirTest, FunctionParams) {
    auto mir = buildMir("செயல் கூடு(a: எண், b: எண்): எண் { விடை a + b; }");
    auto &fn = mir->functions[0];
    ASSERT_EQ(fn.params.size(), 2u);
    EXPECT_EQ(fn.params[0].name, "a");
    EXPECT_EQ(fn.params[1].name, "b");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expression Flattening
// ═══════════════════════════════════════════════════════════════════════════════

TEST(MirTest, BinaryExprFlattened) {
    auto mir = buildMir("செயல் கூடு(a: எண், b: எண்): எண் { விடை a + b; }");
    auto &entry = mir->functions[0].blocks[0];
    // Should have at least one assignment (the add result) before the return
    EXPECT_GE(entry.statements.size(), 1u);
    auto &assign = std::get<MirAssign>(entry.statements.back());
    EXPECT_TRUE(std::holds_alternative<MirRvalueBinaryOp>(assign.rvalue));
    auto &binOp = std::get<MirRvalueBinaryOp>(assign.rvalue);
    EXPECT_EQ(binOp.op, BinaryOp::Add);
}

TEST(MirTest, NestedExprsFlattenToMultipleAssigns) {
    // (a + b) * (a - b) should produce 3 temps: t0=a+b, t1=a-b, t2=t0*t1
    auto mir = buildMir(
        "செயல் f(a: எண், b: எண்): எண் { விடை (a + b) * (a - b); }");
    auto &entry = mir->functions[0].blocks[0];
    EXPECT_GE(entry.statements.size(), 3u);
}

TEST(MirTest, FunctionCallInMir) {
    auto mir = buildMir(
        "செயல் பார்(): எண் { விடை 1; }\n"
        "செயல் மைய(): எண் { விடை பார்(); }");
    auto &mainFn = mir->functions[1];
    auto &entry = mainFn.blocks[0];
    // Should have a call assignment
    bool foundCall = false;
    for (auto &stmt : entry.statements) {
        auto &assign = std::get<MirAssign>(stmt);
        if (std::holds_alternative<MirRvalueCall>(assign.rvalue)) {
            foundCall = true;
            auto &call = std::get<MirRvalueCall>(assign.rvalue);
            EXPECT_EQ(call.callee, "பார்");
        }
    }
    EXPECT_TRUE(foundCall);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Control Flow → Basic Blocks
// ═══════════════════════════════════════════════════════════════════════════════

TEST(MirTest, IfCreatesBlocks) {
    auto mir = buildMir(
        "செயல் சோதனை(x: எண்): எண் { எனில் (x > 0) { விடை 1; } விடை 0; }");
    auto &fn = mir->functions[0];
    // Should have entry, then, merge blocks at minimum
    EXPECT_GE(fn.blocks.size(), 3u);
    // Entry should have SwitchInt terminator
    bool hasBranch = false;
    for (auto &bb : fn.blocks) {
        if (std::holds_alternative<MirSwitchInt>(bb.terminator)) {
            hasBranch = true;
        }
    }
    EXPECT_TRUE(hasBranch);
}

TEST(MirTest, WhileCreatesLoopBlocks) {
    auto mir = buildMir(
        "செயல் எண்ணு(n: எண்): எண் {\n"
        "    மாறி i: எண் = 0;\n"
        "    வரை (i < n) { i = i + 1; }\n"
        "    விடை i;\n"
        "}");
    auto &fn = mir->functions[0];
    // Should have entry, whilecond, whilebody, whileend blocks
    EXPECT_GE(fn.blocks.size(), 4u);

    bool hasCondBlock = false, hasBodyBlock = false;
    for (auto &bb : fn.blocks) {
        if (bb.label == "whilecond") hasCondBlock = true;
        if (bb.label == "whilebody") hasBodyBlock = true;
    }
    EXPECT_TRUE(hasCondBlock);
    EXPECT_TRUE(hasBodyBlock);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MIR Printer
// ═══════════════════════════════════════════════════════════════════════════════

TEST(MirTest, PrinterProducesOutput) {
    auto mir = buildMir("செயல் மைய(): எண் { விடை 42; }");
    std::string output = MirPrinter::print(*mir);
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("fn மைய"), std::string::npos);
    EXPECT_NE(output.find("return"), std::string::npos);
}

TEST(MirTest, PrinterShowsLocals) {
    auto mir = buildMir("செயல் மைய(): எண் { மாறி x: எண் = 5; விடை x; }");
    std::string output = MirPrinter::print(*mir);
    EXPECT_NE(output.find("/* x */"), std::string::npos);
}
