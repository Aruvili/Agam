/*
 * அகம் (Agam) சொற்பொருள் பகுப்பாய்வு (Semantic Analysis) அலகு சோதனைகள்
 *
 * வகை சரிபார்ப்பான் (type checker) மற்றும் எல்லை தீர்வி (scope resolver) ஆகியவற்றை
 * நேரடியாக AST முனைப்புக்களை உருவாக்கி சோதிக்கிறது.
 */

#include <gtest/gtest.h>
#include "../../include/agam/ast/ast.h"
#include "../../include/agam/semantic/symbol_table.h"
#include "../../include/agam/semantic/type_checker.h"
#include "../../include/agam/semantic/scope_resolver.h"
#include "../../include/agam/utils/diagnostic.h"
#include <memory>
#include <vector>

using namespace agam;

// ═══════════════════════════════════════════════════════════════════════════════
//  Helper: build AST nodes programmatically
// ═══════════════════════════════════════════════════════════════════════════════

/// கொடுக்கப்பட்ட கூற்றுகளுடன் ஒரு எளிய செயலை உருவாக்குகிறது.
static std::unique_ptr<Program> makeProgram(
    std::vector<std::unique_ptr<FunctionDecl>> funcs) {
    auto prog = std::make_unique<Program>();
    prog->functions = std::move(funcs);
    return prog;
}

static std::unique_ptr<FunctionDecl> makeFunc(
    const std::string &name, TypeKind retType,
    std::vector<Param> params,
    std::vector<std::unique_ptr<Stmt>> body) {
    auto block = std::make_unique<BlockStmt>(std::move(body));
    return std::make_unique<FunctionDecl>(name, std::vector<std::string>{}, std::move(params), TypeInfo::scalar(retType), std::move(block));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Symbol Table Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(SymbolTableTest, DeclareAndLookup) {
    SymbolTable table;
    table.enterScope();

    SymbolInfo info;
    info.name = "க";
    info.typeInfo = TypeInfo::scalar(TypeKind::Int);
    EXPECT_TRUE(table.declare("க", info));

    auto result = table.lookup("க");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "க");
    EXPECT_EQ(result->typeInfo.kind, TypeKind::Int);
}

TEST(SymbolTableTest, DuplicateDeclarationFails) {
    SymbolTable table;
    table.enterScope();

    SymbolInfo info;
    info.name = "க";
    info.typeInfo = TypeInfo::scalar(TypeKind::Int);
    EXPECT_TRUE(table.declare("க", info));
    EXPECT_FALSE(table.declare("க", info)); // தோல்வியடைய வேண்டும்
}

TEST(SymbolTableTest, NestedScopeLookup) {
    SymbolTable table;
    table.enterScope(); // உலகளாவிய எல்லை (Global)

    SymbolInfo outerInfo;
    outerInfo.name = "வெளி";
    outerInfo.typeInfo = TypeInfo::scalar(TypeKind::Int);
    table.declare("வெளி", outerInfo);

    table.enterScope(); // உள்ளூர் எல்லை (Inner)
    SymbolInfo innerInfo;
    innerInfo.name = "உள்";
    innerInfo.typeInfo = TypeInfo::scalar(TypeKind::Float);
    table.declare("உள்", innerInfo);

    // உள்ளூர் எல்லையிலிருந்து வெளி மற்றும் உள் இரண்டையும் பார்க்க முடியும்
    EXPECT_TRUE(table.lookup("வெளி").has_value());
    EXPECT_TRUE(table.lookup("உள்").has_value());

    table.exitScope(); // உள்ளூர் எல்லையை விட்டு வெளியேறு

    // வெளி எல்லையிலிருந்து 'வெளி' ஐ பார்க்க முடியும், ஆனால் 'உள்' ஐ பார்க்க முடியாது
    EXPECT_TRUE(table.lookup("வெளி").has_value());
    EXPECT_FALSE(table.lookup("உள்").has_value());
}

TEST(SymbolTableTest, InnerScopeShadowsOuter) {
    SymbolTable table;
    table.enterScope();
    SymbolInfo infoXInt;
    infoXInt.name = "க";
    infoXInt.typeInfo = TypeInfo::scalar(TypeKind::Int);
    table.declare("க", infoXInt);

    table.enterScope();
    SymbolInfo infoXFloat;
    infoXFloat.name = "க";
    infoXFloat.typeInfo = TypeInfo::scalar(TypeKind::Float);
    table.declare("க", infoXFloat);

    auto result = table.lookup("க");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->typeInfo.kind, TypeKind::Float); // உள்ளூர் எல்லை வெளி எல்லையை மறைக்கிறது

    table.exitScope();
    result = table.lookup("க");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->typeInfo.kind, TypeKind::Int); // மீண்டும் வெளி எல்லைக்கு
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Type Checker Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(TypeCheckerTest, ValidIntArithmetic) {
    // செயல் மைய(): எண் { மாறி க: எண் = 5; விடை க + 3; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(5)));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<VariableExpr>("க"),
            std::make_unique<IntLiteralExpr>(3))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_TRUE(checker.check(*prog, diag));
    EXPECT_FALSE(diag.hasErrors());
}

TEST(TypeCheckerTest, TypeMismatchInVarDecl) {
    // செயல் மைய(): வெற்று { மாறி க: எண் = "வணக்கம்"; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Int), std::make_unique<StringLiteralExpr>("வணக்கம்")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Void, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_FALSE(checker.check(*prog, diag));
    ASSERT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("வகை பொருந்தவில்லை"), std::string::npos);
}

TEST(TypeCheckerTest, ReturnTypeMismatch) {
    // செயல் மைய(): எண் { விடை "வணக்கம்"; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<StringLiteralExpr>("வணக்கம்")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_FALSE(checker.check(*prog, diag));
    ASSERT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("திரும்பும் வகை பொருந்தவில்லை"), std::string::npos);
}

TEST(TypeCheckerTest, UndeclaredVariable) {
    // செயல் மைய(): எண் { விடை க; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("க")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_FALSE(checker.check(*prog, diag));
    ASSERT_TRUE(diag.hasErrors());
    // Note: TypeChecker doesn't check for undeclared variables anymore (ScopeResolver does), 
    // but in this test it might if it looks up the symbol table during check.
}

TEST(TypeCheckerTest, IntToFloatPromotion) {
    // செயல் மைய(): தசமம் { மாறி க: தசமம் = 5; விடை க; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Float), std::make_unique<IntLiteralExpr>(5)));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("க")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Float, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_TRUE(checker.check(*prog, diag));
    EXPECT_FALSE(diag.hasErrors());
}

TEST(TypeCheckerTest, ArithmeticOnStringsError) {
    // செயல் மைய(): சரம் { மாறி அ: சரம் = "அ"; மாறி ஆ: சரம் = "ஆ"; விடை அ - ஆ; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "அ", TypeInfo::scalar(TypeKind::String), std::make_unique<StringLiteralExpr>("அ")));
    body.push_back(std::make_unique<VarDeclStmt>(
        "ஆ", TypeInfo::scalar(TypeKind::String), std::make_unique<StringLiteralExpr>("ஆ")));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Sub,
            std::make_unique<VariableExpr>("அ"),
            std::make_unique<VariableExpr>("ஆ"))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::String, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_FALSE(checker.check(*prog, diag));
}

TEST(TypeCheckerTest, FunctionCallArgCountMismatch) {
    // செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }
    // செயல் மைய(): எண் { விடை கூட்டு(1); }
    std::vector<std::unique_ptr<FunctionDecl>> funcs;

    // கூட்டு (add) செயல்முறை
    std::vector<std::unique_ptr<Stmt>> addBody;
    addBody.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<VariableExpr>("அ"),
            std::make_unique<VariableExpr>("ஆ"))));
    funcs.push_back(makeFunc("கூட்டு", TypeKind::Int,
                              {{"அ", TypeInfo::scalar(TypeKind::Int)}, {"ஆ", TypeInfo::scalar(TypeKind::Int)}},
                              std::move(addBody)));

    // மைய (main) செயல்முறை - கூட்டு செயல்முறையை தவறான அளபுருக்களுடன் அழைக்கிறது
    std::vector<std::unique_ptr<Stmt>> mainBody;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<IntLiteralExpr>(1));
    mainBody.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<CallExpr>("கூட்டு", std::move(args))));
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(mainBody)));

    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    TypeChecker checker;
    EXPECT_FALSE(checker.check(*prog, diag));
    ASSERT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("எதிர்பார்க்கும் அளபுருக்கள்"), std::string::npos);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Scope Resolver Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST(ScopeResolverTest, ValidProgram) {
    // செயல் மைய(): எண் { மாறி க: எண் = 5; விடை க; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(5)));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("க")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    ScopeResolver resolver;
    EXPECT_TRUE(resolver.resolve(*prog, diag));
}

TEST(ScopeResolverTest, UndeclaredVariableDetected) {
    // செயல் மைய(): எண் { விடை ங; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("ங")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    ScopeResolver resolver;
    EXPECT_FALSE(resolver.resolve(*prog, diag));
    ASSERT_TRUE(diag.hasErrors());
    EXPECT_NE(diag.diagnostics()[0].message.find("அறிவிக்கப்படாத மாறி"), std::string::npos);
}

// ── Preservation Tests ───────────────────────────────────────────────────────
// These tests verify baseline behavior on UNFIXED code.
// They MUST PASS before and after any fix is applied.

// Property: for any return-type mismatch, the error message contains
// "திரும்பும் வகை பொருந்தவில்லை" (return type mismatch in Tamil).
// This message must remain unchanged by any fix to the var-decl error message.
// Validates: Requirements 3.6, 3.7
TEST(TypeCheckerTest, PreservationReturnTypeMismatchMessage) {
    // Test multiple return-type mismatch scenarios to confirm the message is stable.
    struct TestCase {
        TypeKind retType;
        std::unique_ptr<Expr> retVal;
    };

    // Case 1: function returns int but body returns string
    {
        std::vector<std::unique_ptr<Stmt>> body;
        body.push_back(std::make_unique<ReturnStmt>(
            std::make_unique<StringLiteralExpr>("வணக்கம்")));
        std::vector<std::unique_ptr<FunctionDecl>> funcs;
        funcs.push_back(makeFunc("மைய", TypeKind::Int, {}, std::move(body)));
        auto prog = makeProgram(std::move(funcs));

        SourceManager sm;
        DiagnosticEngine diag(sm);
        TypeChecker checker;
        EXPECT_FALSE(checker.check(*prog, diag));
        ASSERT_TRUE(diag.hasErrors());
        EXPECT_NE(diag.diagnostics()[0].message.find("திரும்பும் வகை பொருந்தவில்லை"),
                  std::string::npos)
            << "Return-type mismatch message must contain 'திரும்பும் வகை பொருந்தவில்லை'. "
               "Actual: " << diag.diagnostics()[0].message;
    }

    // Case 2: function returns float but body returns int literal (not promotable in return)
    {
        std::vector<std::unique_ptr<Stmt>> body;
        body.push_back(std::make_unique<ReturnStmt>(
            std::make_unique<StringLiteralExpr>("தவறான வகை")));
        std::vector<std::unique_ptr<FunctionDecl>> funcs;
        funcs.push_back(makeFunc("கணக்கு", TypeKind::Float, {}, std::move(body)));
        auto prog = makeProgram(std::move(funcs));

        SourceManager sm;
        DiagnosticEngine diag(sm);
        TypeChecker checker;
        EXPECT_FALSE(checker.check(*prog, diag));
        ASSERT_TRUE(diag.hasErrors());
        EXPECT_NE(diag.diagnostics()[0].message.find("திரும்பும் வகை பொருந்தவில்லை"),
                  std::string::npos)
            << "Return-type mismatch message must contain 'திரும்பும் வகை பொருந்தவில்லை'. "
               "Actual: " << diag.diagnostics()[0].message;
    }
}

TEST(ScopeResolverTest, DuplicateDeclarationDetected) {
    // செயல் மைய(): வெற்று { மாறி க: எண் = 1; மாறி க: எண் = 2; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(1)));
    body.push_back(std::make_unique<VarDeclStmt>(
        "க", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(2)));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("மைய", TypeKind::Void, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    SourceManager sm;
    DiagnosticEngine diag(sm);
    ScopeResolver resolver;
    EXPECT_FALSE(resolver.resolve(*prog, diag));
}
