/*
 * Agam HIR Unit Tests
 *
 * Tests the AST → HIR lowering: name resolution, HirId assignment,
 * and structural correctness.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/hir/hir.h"
#include "agam/hir/hir_builder.h"
#include <memory>
#include <string>

using namespace agam;

// Helper: parse source → AST → HIR
static std::unique_ptr<HirProgram> buildHir(const std::string &source) {
    static SourceManager sm;
    static DiagnosticEngine diag(sm);
    diag.clear();

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto ast = parser.parse();
    HirBuilder builder;
    return builder.build(*ast);
}

// Helper: parse + build, also return the builder for error checking
static std::pair<std::unique_ptr<HirProgram>, HirBuilder> buildHirWithErrors(const std::string &source) {
    static SourceManager sm;
    static DiagnosticEngine diag(sm);
    diag.clear();

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto ast = parser.parse();
    HirBuilder builder;
    auto hir = builder.build(*ast);
    return {std::move(hir), std::move(builder)};
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Basic HIR Generation
// ═══════════════════════════════════════════════════════════════════════════════

TEST(HirTest, EmptyFunction) {
    auto hir = buildHir("செயல் மைய(): வெற்று {}");
    ASSERT_NE(hir, nullptr);
    ASSERT_EQ(hir->functions.size(), 1u);
    EXPECT_EQ(hir->functions[0]->name, "மைய");
    EXPECT_NE(hir->functions[0]->id, INVALID_HIR_ID);
    EXPECT_TRUE(hir->functions[0]->body->stmts.empty());
}

TEST(HirTest, FunctionGetsUniqueId) {
    auto hir = buildHir(
        "செயல் ஒன்று(): வெற்று {}\n"
        "செயல் இரண்டு(): வெற்று {}");
    ASSERT_EQ(hir->functions.size(), 2u);
    EXPECT_NE(hir->functions[0]->id, hir->functions[1]->id);
    EXPECT_NE(hir->functions[0]->id, INVALID_HIR_ID);
    EXPECT_NE(hir->functions[1]->id, INVALID_HIR_ID);
}

TEST(HirTest, ParamsGetUniqueIds) {
    auto hir = buildHir("செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }");
    ASSERT_EQ(hir->functions.size(), 1u);
    auto &fn = hir->functions[0];
    ASSERT_EQ(fn->params.size(), 2u);
    EXPECT_NE(fn->params[0].id, fn->params[1].id);
    EXPECT_NE(fn->params[0].id, INVALID_HIR_ID);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Name Resolution
// ═══════════════════════════════════════════════════════════════════════════════

TEST(HirTest, VarRefResolvesToDecl) {
    auto hir = buildHir("செயல் மைய(): எண் { மாறி x: எண் = 42; விடை x; }");
    auto &stmts = hir->functions[0]->body->stmts;
    ASSERT_EQ(stmts.size(), 2u);

    // First stmt is VarDecl
    auto *decl = dynamic_cast<HirVarDecl *>(stmts[0].get());
    ASSERT_NE(decl, nullptr);
    HirId declId = decl->id;

    // Second stmt is Return with VarRef
    auto *ret = dynamic_cast<HirReturn *>(stmts[1].get());
    ASSERT_NE(ret, nullptr);
    auto *varRef = dynamic_cast<HirVarRef *>(ret->value.get());
    ASSERT_NE(varRef, nullptr);

    // The VarRef should point to the VarDecl's HirId
    EXPECT_EQ(varRef->defId, declId);
}

TEST(HirTest, ParamRefResolvesToParam) {
    auto hir = buildHir("செயல் அடையாளம்(x: எண்): எண் { விடை x; }");
    auto &fn = hir->functions[0];
    HirId paramId = fn->params[0].id;

    auto *ret = dynamic_cast<HirReturn *>(fn->body->stmts[0].get());
    auto *varRef = dynamic_cast<HirVarRef *>(ret->value.get());
    ASSERT_NE(varRef, nullptr);
    EXPECT_EQ(varRef->defId, paramId);
}

TEST(HirTest, FunctionCallResolvesToFunc) {
    auto hir = buildHir(
        "செயல் ஒன்று(): எண் { விடை 1; }\n"
        "செயல் மைய(): எண் { விடை ஒன்று(); }");
    HirId fooId = hir->functions[0]->id;

    auto *ret = dynamic_cast<HirReturn *>(hir->functions[1]->body->stmts[0].get());
    auto *call = dynamic_cast<HirCallExpr *>(ret->value.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->calleeId, fooId);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expressions
// ═══════════════════════════════════════════════════════════════════════════════

TEST(HirTest, BinaryExprPreserved) {
    auto hir = buildHir("செயல் மைய(): எண் { விடை 1 + 2; }");
    auto *ret = dynamic_cast<HirReturn *>(hir->functions[0]->body->stmts[0].get());
    auto *bin = dynamic_cast<HirBinaryExpr *>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op, BinaryOp::Add);

    auto *lhs = dynamic_cast<HirIntLiteral *>(bin->lhs.get());
    auto *rhs = dynamic_cast<HirIntLiteral *>(bin->rhs.get());
    ASSERT_NE(lhs, nullptr);
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(lhs->value, 1);
    EXPECT_EQ(rhs->value, 2);
}

TEST(HirTest, UnaryExprPreserved) {
    auto hir = buildHir("செயல் மைய(): எண் { விடை -5; }");
    auto *ret = dynamic_cast<HirReturn *>(hir->functions[0]->body->stmts[0].get());
    auto *unary = dynamic_cast<HirUnaryExpr *>(ret->value.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op, UnaryOp::Negate);
}

TEST(HirTest, AssignExprResolvesTarget) {
    auto hir = buildHir("செயல் மைய(): வெற்று { மாறி x: எண் = 0; x = 5; }");
    auto &stmts = hir->functions[0]->body->stmts;
    ASSERT_EQ(stmts.size(), 2u);

    auto *decl = dynamic_cast<HirVarDecl *>(stmts[0].get());
    auto *exprStmt = dynamic_cast<HirExprStmt *>(stmts[1].get());
    ASSERT_NE(exprStmt, nullptr);
    auto *assign = dynamic_cast<HirAssignExpr *>(exprStmt->expr.get());
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->targetId, decl->id);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Control Flow
// ═══════════════════════════════════════════════════════════════════════════════

TEST(HirTest, IfStmtLowered) {
    auto hir = buildHir("செயல் மைய(): வெற்று { எனில் (உண்மை) { விடை; } }");
    auto *ifStmt = dynamic_cast<HirIf *>(hir->functions[0]->body->stmts[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->condition, nullptr);
    EXPECT_NE(ifStmt->thenBranch, nullptr);
    EXPECT_EQ(ifStmt->elseBranch, nullptr);
}

TEST(HirTest, WhileStmtLowered) {
    auto hir = buildHir("செயல் மைய(): வெற்று { வரை (உண்மை) { விடை; } }");
    auto *whileStmt = dynamic_cast<HirWhile *>(hir->functions[0]->body->stmts[0].get());
    ASSERT_NE(whileStmt, nullptr);
    EXPECT_NE(whileStmt->condition, nullptr);
    EXPECT_NE(whileStmt->body, nullptr);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Forward References
// ═══════════════════════════════════════════════════════════════════════════════

TEST(HirTest, ForwardFunctionCall) {
    // main calls foo, which is declared AFTER main
    auto hir = buildHir(
        "செயல் மைய(): எண் { விடை பார்(); }\n"
        "செயல் பார்(): எண் { விடை 42; }");
    ASSERT_EQ(hir->functions.size(), 2u);

    // main should be able to call bar (forward reference)
    auto *ret = dynamic_cast<HirReturn *>(hir->functions[0]->body->stmts[0].get());
    auto *call = dynamic_cast<HirCallExpr *>(ret->value.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->calleeId, hir->functions[1]->id);
}
