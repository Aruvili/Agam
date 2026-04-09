/*
 * Agam Lexer Unit Tests
 *
 * Tests the hand-written lexer by feeding input strings and verifying
 * the resulting token stream.
 */

#include <gtest/gtest.h>
#include "agam/lexer/lexer.h"
#include "agam/utils/diagnostic.h"
#include <string>
#include <vector>

using namespace agam;

// Generator for a quick local diagnostic context
#define LEX_SETUP(source) \
    SourceManager sm; \
    DiagnosticEngine diag(sm); \
    Lexer lexer(source, "test.agam", diag)

// ── Keyword Tests ────────────────────────────────────────────────────────────

TEST(LexerTest, RecognizesKeywords) {
    LEX_SETUP("செயல் மாறி எனில் இல்லையெனில் வரை விடை எண் தசமம் சரம் வெற்று மெய்மை உண்மை பொய் அமைப்பு பட்டியல் பொருத்து");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 17u); // 16 keywords + EOF
    EXPECT_EQ(tokens[0].type, TokenType::KW_FUNC);
    EXPECT_EQ(tokens[1].type, TokenType::KW_LET);
    EXPECT_EQ(tokens[2].type, TokenType::KW_IF);
    EXPECT_EQ(tokens[3].type, TokenType::KW_ELSE);
    EXPECT_EQ(tokens[4].type, TokenType::KW_WHILE);
    EXPECT_EQ(tokens[5].type, TokenType::KW_RETURN);
    EXPECT_EQ(tokens[6].type, TokenType::KW_INT);
    EXPECT_EQ(tokens[7].type, TokenType::KW_FLOAT);
    EXPECT_EQ(tokens[8].type, TokenType::KW_STRING);
    EXPECT_EQ(tokens[9].type, TokenType::KW_VOID);
    EXPECT_EQ(tokens[10].type, TokenType::KW_BOOL);
    EXPECT_EQ(tokens[11].type, TokenType::KW_TRUE);
    EXPECT_EQ(tokens[12].type, TokenType::KW_FALSE);
    EXPECT_EQ(tokens[13].type, TokenType::KW_STRUCT);
    EXPECT_EQ(tokens[14].type, TokenType::KW_ENUM);
    EXPECT_EQ(tokens[15].type, TokenType::KW_MATCH);
    EXPECT_EQ(tokens[16].type, TokenType::TOKEN_EOF);
}

// ── Identifier Tests ────────────────────────────────────────────────────────

TEST(LexerTest, RecognizesIdentifiers) {
    LEX_SETUP("x myVar _private camelCase");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].value, "x");
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].value, "myVar");
    EXPECT_EQ(tokens[2].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].value, "_private");
    EXPECT_EQ(tokens[3].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[3].value, "camelCase");
}

// ── Literal Tests ───────────────────────────────────────────────────────────

TEST(LexerTest, RecognizesIntLiterals) {
    LEX_SETUP("0 42 12345");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::INT_LITERAL);
    EXPECT_EQ(tokens[0].value, "0");
    EXPECT_EQ(tokens[1].type, TokenType::INT_LITERAL);
    EXPECT_EQ(tokens[1].value, "42");
    EXPECT_EQ(tokens[2].type, TokenType::INT_LITERAL);
    EXPECT_EQ(tokens[2].value, "12345");
}

TEST(LexerTest, RecognizesFloatLiterals) {
    LEX_SETUP("3.14 0.5 100.0");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::FLOAT_LITERAL);
    EXPECT_EQ(tokens[0].value, "3.14");
    EXPECT_EQ(tokens[1].type, TokenType::FLOAT_LITERAL);
    EXPECT_EQ(tokens[1].value, "0.5");
    EXPECT_EQ(tokens[2].type, TokenType::FLOAT_LITERAL);
    EXPECT_EQ(tokens[2].value, "100.0");
}

TEST(LexerTest, RecognizesStringLiterals) {
    LEX_SETUP("\"hello\" \"world\"");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::STRING_LITERAL);
    EXPECT_EQ(tokens[0].value, "hello");
    EXPECT_EQ(tokens[1].type, TokenType::STRING_LITERAL);
    EXPECT_EQ(tokens[1].value, "world");
}

TEST(LexerTest, HandlesStringEscapeSequences) {
    LEX_SETUP("\"hello\\nworld\" \"tab\\there\"");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].value, "hello\nworld");
    EXPECT_EQ(tokens[1].value, "tab\there");
}

// ── Operator Tests ──────────────────────────────────────────────────────────

TEST(LexerTest, RecognizesOperators) {
    LEX_SETUP("+ - * / = == != < > <= >= && || ! -> =>");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 17u);
    EXPECT_EQ(tokens[0].type, TokenType::PLUS);
    EXPECT_EQ(tokens[1].type, TokenType::MINUS);
    EXPECT_EQ(tokens[2].type, TokenType::STAR);
    EXPECT_EQ(tokens[3].type, TokenType::SLASH);
    EXPECT_EQ(tokens[4].type, TokenType::ASSIGN);
    EXPECT_EQ(tokens[5].type, TokenType::EQ);
    EXPECT_EQ(tokens[6].type, TokenType::NEQ);
    EXPECT_EQ(tokens[7].type, TokenType::LT);
    EXPECT_EQ(tokens[8].type, TokenType::GT);
    EXPECT_EQ(tokens[9].type, TokenType::LTE);
    EXPECT_EQ(tokens[10].type, TokenType::GTE);
    EXPECT_EQ(tokens[11].type, TokenType::AND);
    EXPECT_EQ(tokens[12].type, TokenType::OR);
    EXPECT_EQ(tokens[13].type, TokenType::NOT);
    EXPECT_EQ(tokens[14].type, TokenType::ARROW);
    EXPECT_EQ(tokens[15].type, TokenType::FAT_ARROW);
}

// ── Delimiter Tests ─────────────────────────────────────────────────────────

TEST(LexerTest, RecognizesDelimiters) {
    LEX_SETUP("( ) { } ; , : :: .");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 10u);
    EXPECT_EQ(tokens[0].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[1].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[2].type, TokenType::LBRACE);
    EXPECT_EQ(tokens[3].type, TokenType::RBRACE);
    EXPECT_EQ(tokens[4].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokens[5].type, TokenType::COMMA);
    EXPECT_EQ(tokens[6].type, TokenType::COLON);
    EXPECT_EQ(tokens[7].type, TokenType::DOUBLE_COLON);
    EXPECT_EQ(tokens[8].type, TokenType::DOT);
}

// ── Comment Tests ───────────────────────────────────────────────────────────

TEST(LexerTest, SkipsSingleLineComments) {
    LEX_SETUP("x // this is a comment\ny");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].value, "x");
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].value, "y");
}

TEST(LexerTest, SkipsMultiLineComments) {
    LEX_SETUP("x /* multi\nline\ncomment */ y");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].value, "x");
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].value, "y");
}

// ── Complex Token Sequence ──────────────────────────────────────────────────

TEST(LexerTest, TokenizesCompleteFunctionDecl) {
    LEX_SETUP("செயல் கூட்டு(அ: எண், ஆ: எண்): எண் { விடை அ + ஆ; }");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 20u);
    EXPECT_EQ(tokens[0].type, TokenType::KW_FUNC);
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].value, "கூட்டு");
    EXPECT_EQ(tokens[2].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[3].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[3].value, "அ");
    EXPECT_EQ(tokens[4].type, TokenType::COLON);
    EXPECT_EQ(tokens[5].type, TokenType::KW_INT);
    EXPECT_EQ(tokens[6].type, TokenType::COMMA);
    EXPECT_EQ(tokens[7].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[7].value, "ஆ");
    EXPECT_EQ(tokens[8].type, TokenType::COLON);
    EXPECT_EQ(tokens[9].type, TokenType::KW_INT);
    EXPECT_EQ(tokens[10].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[11].type, TokenType::COLON);
    EXPECT_EQ(tokens[12].type, TokenType::KW_INT);
    EXPECT_EQ(tokens[13].type, TokenType::LBRACE);
    EXPECT_EQ(tokens[14].type, TokenType::KW_RETURN);
}

// ── Line Tracking ───────────────────────────────────────────────────────────

TEST(LexerTest, TracksLineNumbers) {
    LEX_SETUP("x\ny\nz");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[1].line, 2);
    EXPECT_EQ(tokens[2].line, 3);
}

// ── Error Token Test ────────────────────────────────────────────────────────

TEST(LexerTest, ReportsErrorOnUnknownCharacter) {
    LEX_SETUP("@");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::TOKEN_ERROR);
    EXPECT_TRUE(lexer.hasErrors());
}

// ── Empty Input ─────────────────────────────────────────────────────────────

TEST(LexerTest, HandlesEmptyInput) {
    LEX_SETUP("");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::TOKEN_EOF);
}

// ── Bug 5 Exploration: English Keywords Are Not Recognized ──────────────────
// These tests encode the EXPECTED (correct) behavior: English keywords must
// NOT be recognized as keyword tokens. They will FAIL on unfixed code if the
// lexer ever adds English keywords, and PASS once English keywords are absent.
// On the current codebase the lexer already lacks English keywords, so these
// tests confirm the bug condition: English-keyword source programs fail to parse.

TEST(LexerTest, EnglishFuncIsIdentifier) {
    // "func" must tokenize as IDENTIFIER, not KW_FUNC
    // Validates: Requirements 2.5
    LEX_SETUP("func");
    auto tokens = lexer.tokenize();
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER)
        << "Bug 5: 'func' should be IDENTIFIER, not a keyword";
    EXPECT_EQ(tokens[0].value, "func");
}

TEST(LexerTest, EnglishLetIsIdentifier) {
    // "let" must tokenize as IDENTIFIER, not KW_LET
    // Validates: Requirements 2.5
    LEX_SETUP("let");
    auto tokens = lexer.tokenize();
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER)
        << "Bug 5: 'let' should be IDENTIFIER, not a keyword";
}

TEST(LexerTest, EnglishReturnIsIdentifier) {
    // "return" must tokenize as IDENTIFIER, not KW_RETURN
    // Validates: Requirements 2.5
    LEX_SETUP("return");
    auto tokens = lexer.tokenize();
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER)
        << "Bug 5: 'return' should be IDENTIFIER, not a keyword";
}

TEST(LexerTest, EnglishIntIsIdentifier) {
    // "int" must tokenize as IDENTIFIER, not KW_INT
    // Validates: Requirements 2.5
    LEX_SETUP("int");
    auto tokens = lexer.tokenize();
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER)
        << "Bug 5: 'int' should be IDENTIFIER, not a keyword";
}

TEST(LexerTest, AllEnglishKeywordsAreIdentifiers) {
    // All English keywords must tokenize as IDENTIFIER
    // Validates: Requirements 2.5
    const std::vector<std::string> englishKeywords = {
        "func", "let", "return", "int", "float", "bool", "void",
        "enum", "for", "in", "mut", "new", "delete", "if", "while",
        "true", "false", "match"
    };

    for (const auto &kw : englishKeywords) {
        SourceManager sm;
        DiagnosticEngine diag(sm);
        Lexer lexer(kw, "test.agam", diag);
        auto tokens = lexer.tokenize();
        ASSERT_GE(tokens.size(), 2u) << "Token stream too short for: " << kw;
        EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER)
            << "Bug 5: English keyword '" << kw
            << "' should tokenize as IDENTIFIER, not a keyword token";
        EXPECT_EQ(tokens[0].value, kw)
            << "Bug 5: Token value should be '" << kw << "'";
    }
}
