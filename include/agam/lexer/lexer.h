#pragma once

#include "tokens.h"
#include "agam/utils/diagnostic.h"
#include <string>
#include <vector>

namespace agam {

/// Hand-written lexer for the Agam language.
/// Converts a source string into a stream of tokens.
class Lexer {
public:
    explicit Lexer(const std::string &source, const std::string &filename, DiagnosticEngine &diag);

    /// Tokenize the entire source and return all tokens.
    std::vector<Token> tokenize();

    /// Get the next token from the source.
    Token nextToken();

    /// Peek at the current token without consuming it.
    /// Only valid after calling nextToken() at least once or tokenize().
    Token peek() const;

    /// Check if the lexer has reached the end of input.
    bool isAtEnd() const;

    /// Reset the lexer to the beginning of the source.
    void reset();

    /// Get all errors encountered during lexing.
    bool hasErrors() const { return diag_.hasErrors(); }

private:
    std::string source_;
    std::string filename_;
    DiagnosticEngine &diag_;
    size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;

    char current() const;
    char peekChar() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    void skipSingleLineComment();
    bool skipMultiLineComment();

    Token makeToken(TokenType type, const std::string &value = "") const;
    Token errorToken(const std::string &msg);

    Token readNumber();
    Token readString();
    Token readIdentifierOrKeyword();

    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
};

} // namespace agam
