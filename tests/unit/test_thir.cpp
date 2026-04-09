/*
 * Agam THIR Unit Tests
 *
 * Tests the HIR → THIR type-checking pass: type annotations,
 * type promotions, and type error detection.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/hir/hir_builder.h"
#include "agam/thir/thir_builder.h"
#include "agam/utils/diagnostic.h"
#include <memory>
#include <string>
#include <utility>

using namespace agam;

// Helper: source → AST → HIR → THIR
static std::pair<std::unique_ptr<ThirProgram>, DiagnosticEngine> buildThir(const std::string &source) {
    static SourceManager sm;
    static DiagnosticEngine diag(sm);
    diag.clear();

    Lexer lexer(source, "test.agam", diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, source, "test.agam", diag);
    auto ast = parser.parse();
    if (!ast) return {nullptr, std::move(diag)};
    HirBuilder hirBuilder;
    auto hir = hirBuilder.build(*ast);
    if (!hir) return {nullptr, std::move(diag)};
    ThirBuilder thirBuilder;
    auto thir = thirBuilder.build(*hir, diag);
    return {std::move(thir), std::move(diag)};
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Type Annotation
// ═══════════════════════════════════════════════════════════════════════════════

TEST(ThirTest, IntLiteralTyped) {
    auto [thir, diag] = buildThir("செயல் மைய(): எண் { விடை 42; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->value->typeInfo.kind, TypeKind::Int);
}

TEST(ThirTest, FloatLiteralTyped) {
    auto [thir, diag] = buildThir("செயல் மைய(): தசமம் { விடை 3.14; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[0].get());
    EXPECT_EQ(ret->value->typeInfo.kind, TypeKind::Float);
}

TEST(ThirTest, BoolLiteralTyped) {
    auto [thir, diag] = buildThir("செயல் மைய(): மெய்மை { விடை உண்மை; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[0].get());
    EXPECT_EQ(ret->value->typeInfo.kind, TypeKind::Bool);
}

TEST(ThirTest, VarRefInheritsType) {
    auto [thir, diag] = buildThir("செயல் மைய(): எண் { மாறி x: எண் = 5; விடை x; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[1].get());
    auto *ref = dynamic_cast<ThirVarRef *>(ret->value.get());
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->typeInfo, TypeInfo::scalar(TypeKind::Int));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Arithmetic Type Checking
// ═══════════════════════════════════════════════════════════════════════════════

TEST(ThirTest, IntPlusIntIsInt) {
    auto [thir, diag] = buildThir("செயல் மைய(): எண் { விடை 1 + 2; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[0].get());
    auto *bin = dynamic_cast<ThirBinaryExpr *>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->typeInfo, TypeInfo::scalar(TypeKind::Int));
}

TEST(ThirTest, FloatPlusFloatIsFloat) {
    auto [thir, diag] = buildThir("செயல் மைய(): தசமம் { விடை 1.0 + 2.0; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[0].get());
    auto *bin = dynamic_cast<ThirBinaryExpr *>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->typeInfo, TypeInfo::scalar(TypeKind::Float));
}

TEST(ThirTest, IntPlusFloatPromotesToFloat) {
    auto [thir, diag] = buildThir(
        "செயல் மைய(): தசமம் { மாறி a: எண் = 1; மாறி b: தசமம் = 2.0; விடை a + b; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[2].get());
    auto *bin = dynamic_cast<ThirBinaryExpr *>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->typeInfo, TypeInfo::scalar(TypeKind::Float));
    // LHS should be wrapped in a cast
    auto *cast = dynamic_cast<ThirCastExpr *>(bin->lhs.get());
    ASSERT_NE(cast, nullptr);
    EXPECT_EQ(cast->fromTypeInfo.kind, TypeKind::Int);
    EXPECT_EQ(cast->typeInfo, TypeInfo::scalar(TypeKind::Float));
}

TEST(ThirTest, ComparisonProducesBool) {
    auto [thir, diag] = buildThir(
        "செயல் மைய(): மெய்மை { மாறி x: எண் = 5; விடை x > 3; }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[0]->body->stmts[1].get());
    auto *bin = dynamic_cast<ThirBinaryExpr *>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->typeInfo, TypeInfo::scalar(TypeKind::Bool));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Function Call Type Checking
// ═══════════════════════════════════════════════════════════════════════════════

TEST(ThirTest, CallReturnTypeResolved) {
    auto [thir, diag] = buildThir(
        "செயல் பார்(): எண் { விடை 42; }\n"
        "செயல் மைய(): எண் { விடை பார்(); }");
    ASSERT_FALSE(diag.hasErrors());
    auto *ret = dynamic_cast<ThirReturn *>(thir->functions[1]->body->stmts[0].get());
    auto *call = dynamic_cast<ThirCallExpr *>(ret->value.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->typeInfo.kind, TypeKind::Int);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Type Error Detection
// ═══════════════════════════════════════════════════════════════════════════════

TEST(ThirTest, TypeMismatchOnVarInit) {
    auto [thir, diag] = buildThir("செயல் மைய(): வெற்று { மாறி x: எண் = உண்மை; }");
    EXPECT_TRUE(diag.hasErrors());
    if (diag.diagnostics()[0].message.find("தொடங்க") == std::string::npos) {
        std::cout << "DEBUG: Diagnostic message: " << diag.diagnostics()[0].message << std::endl;
    }
    EXPECT_NE(diag.diagnostics()[0].message.find("தொடங்க"), std::string::npos);
}

TEST(ThirTest, ReturnTypeMismatch) {
    auto [thir, diag] = buildThir("செயல் மைய(): எண் { விடை உண்மை; }");
    EXPECT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("திரும்பும் வகை"), std::string::npos);
}

TEST(ThirTest, InvalidBinaryOperands) {
    auto [thir, diag] = buildThir(
        "செயல் மைய(): எண் { விடை உண்மை + 1; }");
    EXPECT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("இருமை செயலி"), std::string::npos);
}

TEST(ThirTest, WrongArgCount) {
    auto [thir, diag] = buildThir(
        "செயல் பார்(a: எண், b: எண்): எண் { விடை a + b; }\n"
        "செயல் மைய(): எண் { விடை பார்(1); }");
    EXPECT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("அளபுருக்கள்"), std::string::npos);
}
