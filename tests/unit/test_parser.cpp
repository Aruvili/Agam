/*
 * Agam Parser Unit Tests
 *
 * Tests the hand-written recursive descent parser by feeding source strings
 * and verifying the resulting AST structure.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/ast/ast.h"
#include "agam/ast/ast_printer.h"
#include <memory>
#include <sstream>
#include <string>

using namespace agam;

// Helper: lex + parse a source string into a Program AST
static std::unique_ptr<Program> parseSource(const std::string &source) {
    static SourceManager sm;
    static DiagnosticEngine diag(sm);
    diag.clear();

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    if (diag.hasErrors()) return nullptr;

    Parser parser(tokens, source, "test.agam", diag);
    auto program = parser.parse();
    return program;
}

// Helper: get AST print
static std::string getASTPrint(Program &prog) {
    std::ostringstream oss;
    ASTPrinter printer(oss);
    printer.print(prog);
    return oss.str();
}

// ── Basic Function Declaration ──────────────────────────────────────────────

TEST(ParserTest, ParsesEmptyFunction) {
    auto prog = parseSource("செயல் மைய(): வெற்று {}");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->functions.size(), 1u);
    EXPECT_EQ(prog->functions[0]->name, "மைய");
    EXPECT_EQ(prog->functions[0]->returnTypeInfo.kind, TypeKind::Void);
    EXPECT_TRUE(prog->functions[0]->params.empty());
    EXPECT_TRUE(prog->functions[0]->body->statements.empty());
}

TEST(ParserTest, ParsesFunctionWithParams) {
    auto prog = parseSource("செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->functions.size(), 1u);

    auto &fn = prog->functions[0];
    EXPECT_EQ(fn->name, "கூட்டு");
    EXPECT_EQ(fn->returnTypeInfo.kind, TypeKind::Int);
    ASSERT_EQ(fn->params.size(), 2u);
    EXPECT_EQ(fn->params[0].name, "அ");
    EXPECT_EQ(fn->params[0].typeInfo.kind, TypeKind::Int);
    EXPECT_EQ(fn->params[1].name, "ஆ");
    EXPECT_EQ(fn->params[1].typeInfo.kind, TypeKind::Int);
}

// ── Variable Declaration ────────────────────────────────────────────────────

TEST(ParserTest, ParsesVarDeclWithInit) {
    auto prog = parseSource("செயல் மைய(): வெற்று { மாறி x: எண் = 42; }");
    ASSERT_NE(prog, nullptr);
    auto &stmts = prog->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);

    auto *varDecl = dynamic_cast<VarDeclStmt *>(stmts[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->name, "x");
    EXPECT_EQ(varDecl->typeInfo.kind, TypeKind::Int);
    ASSERT_NE(varDecl->initializer, nullptr);

    auto *initExpr = dynamic_cast<IntLiteralExpr *>(varDecl->initializer.get());
    ASSERT_NE(initExpr, nullptr);
    EXPECT_EQ(initExpr->value, 42);
}

TEST(ParserTest, ParsesVarDeclWithoutInit) {
    auto prog = parseSource("செயல் மைய(): வெற்று { மாறி x: எண்; }");
    ASSERT_NE(prog, nullptr);
    auto *varDecl =
        dynamic_cast<VarDeclStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->name, "x");
    EXPECT_EQ(varDecl->initializer, nullptr);
}

// ── Expressions ─────────────────────────────────────────────────────────────

TEST(ParserTest, ParsesBinaryExprWithPrecedence) {
    // 2 + 3 * 4 should parse as 2 + (3 * 4)
    auto prog = parseSource("செயல் மைய(): எண் { விடை 2 + 3 * 4; }");
    ASSERT_NE(prog, nullptr);

    auto *retStmt =
        dynamic_cast<ReturnStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(retStmt, nullptr);

    auto *addExpr = dynamic_cast<BinaryExpr *>(retStmt->value.get());
    ASSERT_NE(addExpr, nullptr);
    EXPECT_EQ(addExpr->op, BinaryOp::Add);

    // LHS should be 2
    auto *lhs = dynamic_cast<IntLiteralExpr *>(addExpr->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->value, 2);

    // RHS should be 3 * 4
    auto *mulExpr = dynamic_cast<BinaryExpr *>(addExpr->rhs.get());
    ASSERT_NE(mulExpr, nullptr);
    EXPECT_EQ(mulExpr->op, BinaryOp::Mul);
}

TEST(ParserTest, ParsesFunctionCall) {
    auto prog = parseSource("செயல் ஒரு_செயல்(): எண் { விடை 1; } செயல் மைய(): எண் { விடை ஒரு_செயல்(); }");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->functions.size(), 2u);

    auto *retStmt =
        dynamic_cast<ReturnStmt *>(prog->functions[1]->body->statements[0].get());
    ASSERT_NE(retStmt, nullptr);

    auto *callExpr = dynamic_cast<CallExpr *>(retStmt->value.get());
    ASSERT_NE(callExpr, nullptr);
    EXPECT_EQ(callExpr->callee, "ஒரு_செயல்");
    EXPECT_TRUE(callExpr->args.empty());
}

TEST(ParserTest, ParsesFunctionCallWithArgs) {
    auto prog = parseSource(
        "செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }\n"
        "செயல் மைய(): எண் { விடை கூட்டு(1, 2); }");
    ASSERT_NE(prog, nullptr);

    auto *retStmt =
        dynamic_cast<ReturnStmt *>(prog->functions[1]->body->statements[0].get());
    auto *callExpr = dynamic_cast<CallExpr *>(retStmt->value.get());
    ASSERT_NE(callExpr, nullptr);
    EXPECT_EQ(callExpr->callee, "கூட்டு");
    ASSERT_EQ(callExpr->args.size(), 2u);
}

// ── Control Flow ────────────────────────────────────────────────────────────

TEST(ParserTest, ParsesIfStatement) {
    auto prog = parseSource("செயல் மைய(): வெற்று { எனில் (உண்மை) { விடை; } }");
    ASSERT_NE(prog, nullptr);
    auto *ifStmt =
        dynamic_cast<IfStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->condition, nullptr);
    EXPECT_NE(ifStmt->thenBranch, nullptr);
    EXPECT_EQ(ifStmt->elseBranch, nullptr);
}

TEST(ParserTest, ParsesIfElseStatement) {
    auto prog = parseSource("செயல் மைய(): எண் { எனில் (உண்மை) { விடை 1; } இல்லையெனில் { விடை 0; } }");
    ASSERT_NE(prog, nullptr);
    auto *ifStmt =
        dynamic_cast<IfStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->elseBranch, nullptr);
}

TEST(ParserTest, ParsesWhileStatement) {
    auto prog = parseSource("செயல் மைய(): வெற்று { வரை (உண்மை) { விடை; } }");
    ASSERT_NE(prog, nullptr);
    auto *whileStmt =
        dynamic_cast<WhileStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(whileStmt, nullptr);
    EXPECT_NE(whileStmt->condition, nullptr);
    EXPECT_NE(whileStmt->body, nullptr);
}

// ── Multiple Functions ──────────────────────────────────────────────────────

TEST(ParserTest, ParsesMultipleFunctions) {
    auto prog = parseSource(
        "செயல் அ(): வெற்று {}\n"
        "செயல் ஆ(): வெற்று {}\n"
        "செயல் இ(): வெற்று {}");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->functions.size(), 3u);
    EXPECT_EQ(prog->functions[0]->name, "அ");
    EXPECT_EQ(prog->functions[1]->name, "ஆ");
    EXPECT_EQ(prog->functions[2]->name, "இ");
}

// ── AST Printer ─────────────────────────────────────────────────────────────

TEST(ParserTest, ASTPrinterProducesOutput) {
    auto prog = parseSource("செயல் மைய(): எண் { விடை 42; }");
    ASSERT_NE(prog, nullptr);
    std::string output = getASTPrint(*prog);
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("FunctionDecl(மைய"), std::string::npos);
    EXPECT_NE(output.find("Return"), std::string::npos);
    EXPECT_NE(output.find("IntLiteral(42)"), std::string::npos);
}

// ── Unary Expressions ───────────────────────────────────────────────────────

TEST(ParserTest, ParsesUnaryNegation) {
    auto prog = parseSource("செயல் மைய(): எண் { விடை -5; }");
    ASSERT_NE(prog, nullptr);
    auto *retStmt =
        dynamic_cast<ReturnStmt *>(prog->functions[0]->body->statements[0].get());
    auto *unary = dynamic_cast<UnaryExpr *>(retStmt->value.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op, UnaryOp::Negate);
}

TEST(ParserTest, ParsesAssignment) {
    auto prog = parseSource("செயல் மைய(): வெற்று { மாறி x: எண் = 0; x = 5; }");
    ASSERT_NE(prog, nullptr);
    auto &stmts = prog->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 2u);

    auto *exprStmt = dynamic_cast<ExprStmt *>(stmts[1].get());
    ASSERT_NE(exprStmt, nullptr);
    auto *assign = dynamic_cast<AssignExpr *>(exprStmt->expr.get());
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->name, "x");
}

// ── Enums and Match Expressions ──────────────────────────────────────────────

TEST(ParserTest, ParsesEnumDecl) {
    auto prog = parseSource("பட்டியல் விருப்ப_எண் { இல்லை, உள்ளது(எண்) }");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->enums.size(), 1u);

    auto &ed = prog->enums[0];
    EXPECT_EQ(ed->name, "விருப்ப_எண்");
    ASSERT_EQ(ed->variants.size(), 2u);
    EXPECT_EQ(ed->variants[0].name, "இல்லை");
    EXPECT_FALSE(ed->variants[0].hasPayload);
    EXPECT_EQ(ed->variants[1].name, "உள்ளது");
    EXPECT_TRUE(ed->variants[1].hasPayload);
    EXPECT_EQ(ed->variants[1].payloadType.kind, TypeKind::Int);
}

TEST(ParserTest, ParsesEnumVariantExpr) {
    auto prog = parseSource("செயல் மைய(): வெற்று { மாறி x: எண் = விருப்ப_எண்::உள்ளது(42); }");
    ASSERT_NE(prog, nullptr);
    ASSERT_EQ(prog->functions[0]->body->statements.size(), 1u);
    auto *varDecl = dynamic_cast<VarDeclStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(varDecl, nullptr);
    auto *expr = dynamic_cast<EnumVariantExpr *>(varDecl->initializer.get());
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->enumName, "விருப்ப_எண்");
    EXPECT_EQ(expr->variantName, "உள்ளது");
    ASSERT_NE(expr->payload, nullptr);
}

TEST(ParserTest, ParsesMatchExpr) {
    auto prog = parseSource("செயல் மைய(): வெற்று { பொருத்து (x) { இல்லை => 0, உள்ளது(v) => v, }; }");
    ASSERT_NE(prog, nullptr);
    auto *exprStmt = dynamic_cast<ExprStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(exprStmt, nullptr);
    auto *matchExpr = dynamic_cast<MatchExpr *>(exprStmt->expr.get());
    ASSERT_NE(matchExpr, nullptr);
    ASSERT_EQ(matchExpr->arms.size(), 2u);
    EXPECT_EQ(matchExpr->arms[0].variantName, "இல்லை");
    EXPECT_FALSE(matchExpr->arms[0].hasBinding);
    EXPECT_EQ(matchExpr->arms[1].variantName, "உள்ளது");
    EXPECT_TRUE(matchExpr->arms[1].hasBinding);
    EXPECT_EQ(matchExpr->arms[1].bindingName, "v");
}

// ── Preservation Tests ───────────────────────────────────────────────────────
// These tests verify baseline behavior on UNFIXED code.
// They MUST PASS before and after any fix is applied.
// Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5, 3.7

// Property: for any single-variant enum source, parseEnumDecl returns exactly 1 variant.
// Validates: Requirements 3.3
TEST(ParserTest, PreservationSingleVariantEnum) {
    // Test multiple single-variant enum sources — none of these trigger the bug
    // (the bug only manifests with 2+ comma-separated variants).
    const std::vector<std::string> singleVariantSources = {
        "பட்டியல் நிலை { ஆம் }",
        "பட்டியல் வண்ணம் { சிவப்பு }",
        "பட்டியல் திசை { வடக்கு }",
        "பட்டியல் பதில் { உண்மை }",
        "பட்டியல் சுமை { மதிப்பு(எண்) }",
    };
    for (const auto &src : singleVariantSources) {
        auto prog = parseSource(src);
        ASSERT_NE(prog, nullptr) << "Failed to parse: " << src;
        ASSERT_EQ(prog->enums.size(), 1u) << "Expected 1 enum in: " << src;
        EXPECT_EQ(prog->enums[0]->variants.size(), 1u)
            << "Single-variant enum should have exactly 1 variant. Source: " << src;
    }
}

// Property: for any Tamil-keyword source that currently compiles, the compiler produces no errors.
// Validates: Requirements 3.1, 3.2, 3.5, 3.7
TEST(ParserTest, PreservationTamilKeywordSources) {
    // Known-good Tamil-keyword sources that parse correctly on unfixed code.
    const std::vector<std::string> goodSources = {
        "செயல் மைய(): வெற்று {}",
        "செயல் மைய(): எண் { விடை 42; }",
        "செயல் மைய(): வெற்று { மாறி x: எண் = 5; }",
        "செயல் மைய(): வெற்று { மாறி x: எண்; }",
        "செயல் மைய(): வெற்று { எனில் (உண்மை) { விடை; } }",
        "செயல் மைய(): வெற்று { வரை (உண்மை) { விடை; } }",
        "செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }",
    };
    for (const auto &src : goodSources) {
        auto prog = parseSource(src);
        EXPECT_NE(prog, nullptr)
            << "Tamil-keyword source should parse without errors. Source: " << src;
    }
}

// Property: match expressions with block-body arms parse correctly (unaffected by the
// trailing-comma bug which only affects expression-body arms).
// Validates: Requirements 3.4, 3.7
// NOTE: This test verifies that a single-arm block-body match parses correctly.
// The multi-arm block-body path shares the same loop bug as expression-body arms,
// so we test the single-arm case which is unambiguously unaffected.
TEST(ParserTest, PreservationMatchExprSingleBlockBodyArm) {
    // A single block-body arm has no comma/trailing-comma issue at all.
    auto prog = parseSource(
        "செயல் மைய(): வெற்று { "
        "பொருத்து (x) { "
        "  இல்லை => { விடை; } "
        "}; }");
    ASSERT_NE(prog, nullptr);
    auto *exprStmt = dynamic_cast<ExprStmt *>(prog->functions[0]->body->statements[0].get());
    ASSERT_NE(exprStmt, nullptr);
    auto *matchExpr = dynamic_cast<MatchExpr *>(exprStmt->expr.get());
    ASSERT_NE(matchExpr, nullptr);
    EXPECT_EQ(matchExpr->arms.size(), 1u)
        << "Single block-body arm match should parse as exactly 1 arm";
}

// ── Bug 5 Exploration: English-Keyword Sources Fail to Parse ────────────────
// These tests encode the EXPECTED (correct) behavior: source programs that use
// English keywords must fail to parse (because English words are plain
// identifiers, not keywords). They confirm Bug 5 exists on unfixed code.
// Validates: Requirements 1.5, 2.5

TEST(ParserTest, EnglishKeywordSourceFailsToParse) {
    // "func main(): int { return 42; }" uses English keywords.
    // The parser expects KW_FUNC but gets IDENTIFIER for "func",
    // so parse() should return nullptr or diag should have errors.
    // Validates: Requirements 2.5
    SourceManager sm;
    DiagnosticEngine diag(sm);
    const std::string source = "func main(): int { return 42; }";
    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto prog = parser.parse();
    EXPECT_TRUE(diag.hasErrors() || prog == nullptr || prog->functions.empty())
        << "Bug 5: English-keyword source should fail to parse";
}

TEST(ParserTest, EnglishKeywordLetSourceFailsToParse) {
    // "func main(): void { let x: int = 5; }" uses English keywords.
    // Validates: Requirements 2.5
    SourceManager sm;
    DiagnosticEngine diag(sm);
    const std::string source = "func main(): void { let x: int = 5; }";
    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto prog = parser.parse();
    EXPECT_TRUE(diag.hasErrors() || prog == nullptr || prog->functions.empty())
        << "Bug 5: English-keyword source should fail to parse";
}
