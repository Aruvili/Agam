#pragma once

#include <ostream>
#include <string>

namespace agam {

/// All token types recognized by the Agam lexer.
enum class TokenType {
    // ── Literals ─────────────────────────────────────────────────────────
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,

    // ── Identifier ───────────────────────────────────────────────────────
    IDENTIFIER,

    // ── Keywords ─────────────────────────────────────────────────────────
    KW_FUNC,
    KW_LET,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_INT,
    KW_FLOAT,
    KW_STRING,
    KW_VOID,
    KW_TRUE,
    KW_FALSE,
    KW_BOOL,
    KW_STRUCT,
    KW_ENUM,
    KW_MATCH,
    KW_NEW,
    KW_DELETE,
    KW_FOR,
    KW_IN,
    KW_IMPORT,
    KW_EXTERN,
    KW_TRAIT,
    KW_IMPL,
    KW_MUT,
    KW_ZONE,
    KW_ALLOC,
    KW_BORROW,
    KW_SHARED,
    KW_ESCAPE,
    KW_NIL,
    KW_AS,
    KW_CONST,

    // Sized integer types
    KW_INT8,
    KW_INT16,
    KW_INT32,
    KW_INT64,
    KW_INT128,
    KW_UINT8,
    KW_UINT16,
    KW_UINT32,
    KW_UINT64,
    KW_UINT128,

    // Sized float types
    KW_FLOAT32,
    KW_FLOAT64,

    // ── Operators ────────────────────────────────────────────────────────
    PLUS,      // +
    MINUS,     // -
    STAR,      // *
    SLASH,     // /
    ASSIGN,    // =
    EQ,        // ==
    NEQ,       // !=
    LT,        // <
    GT,        // >
    LTE,       // <=
    GTE,       // >=
    AND,       // &&
    AMPERSAND, // &
    OR,        // ||
    NOT,       // !
    ARROW,     // ->
    FAT_ARROW, // =>
    TILDE,     // ~
    PERCENT,   // %

    // ── Delimiters ───────────────────────────────────────────────────────
    LPAREN,       // (
    RPAREN,       // )
    LBRACE,       // {
    RBRACE,       // }
    LBRACKET,     // [
    RBRACKET,     // ]
    SEMICOLON,    // ;
    COMMA,        // ,
    COLON,        // :
    DOUBLE_COLON, // ::
    DOT,          // .
    DOTDOT,       // ..

    // ── Special ──────────────────────────────────────────────────────────
    TOKEN_EOF,
    TOKEN_ERROR,
};

/// A single token produced by the lexer.
struct Token {
    TokenType type;
    std::string value;
    std::string filename;
    int line;
    int column;
};

/// Convert a TokenType to its string representation (for debugging/testing).
inline const char *tokenTypeToString(TokenType t) {
    switch (t) {
    case TokenType::INT_LITERAL:
        return "INT_LITERAL";
    case TokenType::FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::KW_FUNC:
        return "KW_FUNC";
    case TokenType::KW_CONST:
        return "KW_CONST";
    case TokenType::KW_LET:
        return "KW_LET";
    case TokenType::KW_IF:
        return "KW_IF";
    case TokenType::KW_ELSE:
        return "KW_ELSE";
    case TokenType::KW_WHILE:
        return "KW_WHILE";
    case TokenType::KW_RETURN:
        return "KW_RETURN";
    case TokenType::KW_INT:
        return "KW_INT";
    case TokenType::KW_FLOAT:
        return "KW_FLOAT";
    case TokenType::KW_STRING:
        return "KW_STRING";
    case TokenType::KW_VOID:
        return "KW_VOID";
    case TokenType::KW_TRUE:
        return "KW_TRUE";
    case TokenType::KW_FALSE:
        return "KW_FALSE";
    case TokenType::KW_BOOL:
        return "KW_BOOL";
    case TokenType::KW_STRUCT:
        return "KW_STRUCT";
    case TokenType::KW_ENUM:
        return "KW_ENUM";
    case TokenType::KW_MATCH:
        return "KW_MATCH";
    case TokenType::KW_NEW:
        return "KW_NEW";
    case TokenType::KW_DELETE:
        return "KW_DELETE";
    case TokenType::KW_FOR:
        return "KW_FOR";
    case TokenType::KW_IN:
        return "KW_IN";
    case TokenType::KW_IMPORT:
        return "KW_IMPORT";
    case TokenType::KW_EXTERN:
        return "KW_EXTERN";
    case TokenType::KW_TRAIT:
        return "KW_TRAIT";
    case TokenType::KW_IMPL:
        return "KW_IMPL";
    case TokenType::KW_MUT:
        return "KW_MUT";
    case TokenType::KW_ZONE:
        return "KW_ZONE";
    case TokenType::KW_ALLOC: return "alloc";
    case TokenType::KW_BORROW: return "borrow";
    case TokenType::KW_SHARED: return "shared";
    case TokenType::KW_ESCAPE: return "escape";
    case TokenType::KW_NIL:
        return "KW_NIL";
    case TokenType::KW_AS:
        return "KW_AS";
    case TokenType::KW_INT8:
        return "KW_INT8";
    case TokenType::KW_INT16:
        return "KW_INT16";
    case TokenType::KW_INT32:
        return "KW_INT32";
    case TokenType::KW_INT64:
        return "KW_INT64";
    case TokenType::KW_INT128:
        return "KW_INT128";
    case TokenType::KW_UINT8:
        return "KW_UINT8";
    case TokenType::KW_UINT16:
        return "KW_UINT16";
    case TokenType::KW_UINT32:
        return "KW_UINT32";
    case TokenType::KW_UINT64:
        return "KW_UINT64";
    case TokenType::KW_UINT128:
        return "KW_UINT128";
    case TokenType::KW_FLOAT32:
        return "KW_FLOAT32";
    case TokenType::KW_FLOAT64:
        return "KW_FLOAT64";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::ASSIGN:
        return "ASSIGN";
    case TokenType::EQ:
        return "EQ";
    case TokenType::NEQ:
        return "NEQ";
    case TokenType::LT:
        return "LT";
    case TokenType::GT:
        return "GT";
    case TokenType::LTE:
        return "LTE";
    case TokenType::GTE:
        return "GTE";
    case TokenType::AND:
        return "AND";
    case TokenType::AMPERSAND:
        return "AMPERSAND";
    case TokenType::OR:
        return "OR";
    case TokenType::NOT:
        return "NOT";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::LBRACE:
        return "LBRACE";
    case TokenType::RBRACE:
        return "RBRACE";
    case TokenType::LBRACKET:
        return "LBRACKET";
    case TokenType::RBRACKET:
        return "RBRACKET";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::COLON:
        return "COLON";
    case TokenType::DOUBLE_COLON:
        return "DOUBLE_COLON";
    case TokenType::ARROW:
        return "ARROW";
    case TokenType::FAT_ARROW:
        return "FAT_ARROW";
    case TokenType::TILDE:
        return "TILDE";
    case TokenType::PERCENT:
        return "PERCENT";
    case TokenType::DOT:
        return "DOT";
    case TokenType::DOTDOT:
        return "DOTDOT";
    case TokenType::TOKEN_EOF:
        return "EOF";
    case TokenType::TOKEN_ERROR:
        return "ERROR";
    }
    return "UNKNOWN";
}

inline std::ostream &operator<<(std::ostream &os, TokenType t) {
    return os << tokenTypeToString(t);
}

} // namespace agam
