#pragma once

#include "agam/ast/ast.h"
#include "agam/lexer/lexer.h"
#include "agam/utils/diagnostic.h"

#include <memory>
#include <string>
#include <vector>

namespace agam {

/// Hand-written recursive descent parser for the Agam language.
/// Converts a token stream into an AST.
class Parser {
  public:
    explicit Parser(const std::vector<Token> &tokens, const std::string &source,
                    const std::string &filename, DiagnosticEngine &diag);

    /// Parse the token stream into a Program AST.
    /// Returns nullptr on failure.
    std::unique_ptr<Program> parse();

    /// Check if parsing succeeded (no errors).
    bool hasErrors() const { return diag_.hasErrors(); }

  private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::string source_;
    std::string filename_;
    DiagnosticEngine &diag_;
    std::vector<std::string> currentTypeParams_;

    // ── Token navigation ─────────────────────────────────────────────────────
    const Token &peek() const;
    const Token &previous() const;
    const Token &advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string &errMsg);

    // ── Error handling ───────────────────────────────────────────────────────
    void error(const std::string &msg);
    void error(const Token &tok, const std::string &msg);
    void synchronize();

    // ── Grammar rules ────────────────────────────────────────────────────────

    // Top-level
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<ConstDecl> parseConstDecl();
    std::unique_ptr<StructDecl> parseStructDecl();
    std::unique_ptr<EnumDecl> parseEnumDecl();
    std::unique_ptr<TraitDecl> parseTraitDecl();
    std::unique_ptr<ImplDecl> parseImplDecl();
    std::vector<Param> parseParamList();
    TypeInfo parseTypeSpec();
    std::vector<std::string> parseTypeParams();
    std::vector<TypeInfo> parseTypeArgs();

    // Statements
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<VarDeclStmt> parseVarDecl();
    std::unique_ptr<IfStmt> parseIfStmt();
    std::unique_ptr<WhileStmt> parseWhileStmt();
    std::unique_ptr<ForStmt> parseForStmt();
    std::unique_ptr<ReturnStmt> parseReturnStmt();
    std::unique_ptr<DeleteStmt> parseDeleteStmt();
    std::unique_ptr<BlockStmt> parseBlock();

    // Expressions (precedence climbing)
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAddition();
    std::unique_ptr<Expr> parseMultiplication();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseMatchExpr();
    std::unique_ptr<Expr> parseZoneExpr();
    std::unique_ptr<Expr> parseAllocExpr();
    std::unique_ptr<Expr> parseBorrowExpr();
    std::unique_ptr<Expr> parseEscapeExpr();
};

} // namespace agam
