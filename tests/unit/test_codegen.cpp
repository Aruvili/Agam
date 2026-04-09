/*
 * Agam Code Generation Unit Tests
 *
 * Tests the LLVM IR code generator by constructing ASTs and verifying
 * the generated IR is valid.
 */

#include <gtest/gtest.h>
#include "agam/ast/ast.h"
#include "agam/codegen/codegen.h"
#include <memory>
#include <string>
#include <vector>

using namespace agam;

// Helper to build ASTs
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
//  Basic Code Generation
// ═══════════════════════════════════════════════════════════════════════════════

TEST(CodegenTest, GeneratesSimpleReturnInt) {
    // func main(): int { return 42; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<IntLiteralExpr>(42)));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("main", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("define i32 @main()"), std::string::npos);
    EXPECT_NE(ir.find("ret i32 42"), std::string::npos);
}

TEST(CodegenTest, GeneratesVoidFunction) {
    // func doNothing(): void {}
    std::vector<std::unique_ptr<Stmt>> body;

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("doNothing", TypeKind::Void, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("define void @doNothing()"), std::string::npos);
    EXPECT_NE(ir.find("ret void"), std::string::npos);
}

// ── Arithmetic ──────────────────────────────────────────────────────────────

TEST(CodegenTest, GeneratesIntArithmetic) {
    // func add(a: int, b: int): int { return a + b; }
    // Using variables prevents LLVM from constant-folding
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<VariableExpr>("a"),
            std::make_unique<VariableExpr>("b"))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("add", TypeKind::Int,
                              {{"a", TypeInfo::scalar(TypeKind::Int)}, {"b", TypeInfo::scalar(TypeKind::Int)}},
                              std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("add"), std::string::npos);
}

TEST(CodegenTest, GeneratesFloatArithmetic) {
    // func add(a: float, b: float): float { return a + b; }
    // Using variables prevents LLVM from constant-folding
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<VariableExpr>("a"),
            std::make_unique<VariableExpr>("b"))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("fadd", TypeKind::Float,
                              {{"a", TypeInfo::scalar(TypeKind::Float)}, {"b", TypeInfo::scalar(TypeKind::Float)}},
                              std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("fadd"), std::string::npos);
}

// ── Variables ───────────────────────────────────────────────────────────────

TEST(CodegenTest, GeneratesVariableAllocaAndStore) {
    // func main(): int { let x: int = 10; return x; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "x", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(10)));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("x")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("main", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("alloca i32"), std::string::npos);
    EXPECT_NE(ir.find("store i32 10"), std::string::npos);
    EXPECT_NE(ir.find("load i32"), std::string::npos);
}

// ── Functions with Parameters ───────────────────────────────────────────────

TEST(CodegenTest, GeneratesFunctionWithParams) {
    // func add(a: int, b: int): int { return a + b; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<VariableExpr>("a"),
            std::make_unique<VariableExpr>("b"))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("add", TypeKind::Int,
                              {{"a", TypeInfo::scalar(TypeKind::Int)}, {"b", TypeInfo::scalar(TypeKind::Int)}},
                              std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("define i32 @add(i32 %a, i32 %b)"), std::string::npos);
}

// ── Control Flow ────────────────────────────────────────────────────────────

TEST(CodegenTest, GeneratesIfElseBranching) {
    // func test(x: int): int { if (x > 0) { return 1; } else { return 0; } }
    std::vector<std::unique_ptr<Stmt>> thenStmts;
    thenStmts.push_back(std::make_unique<ReturnStmt>(std::make_unique<IntLiteralExpr>(1)));

    std::vector<std::unique_ptr<Stmt>> elseStmts;
    elseStmts.push_back(std::make_unique<ReturnStmt>(std::make_unique<IntLiteralExpr>(0)));

    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<IfStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Gt,
            std::make_unique<VariableExpr>("x"),
            std::make_unique<IntLiteralExpr>(0)),
        std::make_unique<BlockStmt>(std::move(thenStmts)),
        std::make_unique<BlockStmt>(std::move(elseStmts))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("test", TypeKind::Int,
                              {{"x", TypeInfo::scalar(TypeKind::Int)}},
                              std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    // Should contain conditional branch
    EXPECT_NE(ir.find("br i1"), std::string::npos);
    EXPECT_NE(ir.find("then"), std::string::npos);
    EXPECT_NE(ir.find("else"), std::string::npos);
}

TEST(CodegenTest, GeneratesWhileLoop) {
    // func count(n: int): int {
    //     let i: int = 0;
    //     while (i < n) { i = i + 1; }
    //     return i;
    // }
    std::vector<std::unique_ptr<Stmt>> loopBody;
    loopBody.push_back(std::make_unique<ExprStmt>(
        std::make_unique<AssignExpr>("i",
            std::make_unique<BinaryExpr>(
                BinaryOp::Add,
                std::make_unique<VariableExpr>("i"),
                std::make_unique<IntLiteralExpr>(1)))));

    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<VarDeclStmt>(
        "i", TypeInfo::scalar(TypeKind::Int), std::make_unique<IntLiteralExpr>(0)));
    body.push_back(std::make_unique<WhileStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Lt,
            std::make_unique<VariableExpr>("i"),
            std::make_unique<VariableExpr>("n")),
        std::make_unique<BlockStmt>(std::move(loopBody))));
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<VariableExpr>("i")));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("count", TypeKind::Int,
                              {{"n", TypeInfo::scalar(TypeKind::Int)}},
                              std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));

    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("whilecond"), std::string::npos);
    EXPECT_NE(ir.find("whilebody"), std::string::npos);
    EXPECT_NE(ir.find("whileend"), std::string::npos);
}

// ── Module Verification ─────────────────────────────────────────────────────

TEST(CodegenTest, ModuleVerificationPasses) {
    // func main(): int { return 0; }
    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<IntLiteralExpr>(0)));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("main", TypeKind::Int, {}, std::move(body)));
    auto prog = makeProgram(std::move(funcs));

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));
    EXPECT_TRUE(codegen.verify());
}

TEST(CodegenTest, GeneratesStructAccess) {
    auto structDecl = std::make_unique<StructDecl>("Point", std::vector<std::string>{}, std::vector<StructField>{
        {"x", TypeInfo::scalar(TypeKind::Int)},
        {"y", TypeInfo::scalar(TypeKind::Int)}
    });

    std::vector<std::unique_ptr<Stmt>> body;
    
    std::vector<StructFieldInit> fields;
    fields.push_back({"x", std::make_unique<IntLiteralExpr>(10)});
    fields.push_back({"y", std::make_unique<IntLiteralExpr>(20)});
    auto structLit = std::make_unique<StructLiteralExpr>(TypeInfo::namedStruct("Point"), std::move(fields));
    
    body.push_back(std::make_unique<VarDeclStmt>(
        "p", TypeInfo::namedStruct("Point"), std::move(structLit)));
        
    body.push_back(std::make_unique<ExprStmt>(
        std::make_unique<FieldAssignExpr>(
            std::make_unique<VariableExpr>("p"),
            "x",
            std::make_unique<IntLiteralExpr>(30))));
            
    body.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<BinaryExpr>(
            BinaryOp::Add,
            std::make_unique<FieldAccessExpr>(std::make_unique<VariableExpr>("p"), "x"),
            std::make_unique<FieldAccessExpr>(std::make_unique<VariableExpr>("p"), "y"))));

    std::vector<std::unique_ptr<FunctionDecl>> funcs;
    funcs.push_back(makeFunc("main", TypeKind::Int, {}, std::move(body)));
    
    auto prog = std::make_unique<Program>();
    prog->structs.push_back(std::move(structDecl));
    prog->functions = std::move(funcs);

    CodeGenerator codegen;
    EXPECT_TRUE(codegen.generate(*prog));
    
    std::string ir = codegen.getIRString();
    EXPECT_NE(ir.find("%Point = type { i32, i32 }"), std::string::npos);
    EXPECT_NE(ir.find("%Point { i32 10, i32 20 }"), std::string::npos);
    EXPECT_NE(ir.find("getelementptr %Point"), std::string::npos);
    EXPECT_NE(ir.find("extractvalue %Point"), std::string::npos);
}
