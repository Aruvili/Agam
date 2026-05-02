#include "../include/agam/lexer/lexer.h"

#include <unordered_map>

namespace agam {

// ── Keyword map ──────────────────────────────────────────────────────────────
static const std::unordered_map<std::string, TokenType> keywords = {
    {"செயல்", TokenType::KW_FUNC},
    {"மாறி", TokenType::KW_LET},
    {"எனில்", TokenType::KW_IF},
    {"இல்லையெனில்", TokenType::KW_ELSE},
    {"வரை", TokenType::KW_WHILE},
    {"விடை", TokenType::KW_RETURN},
    {"எண்", TokenType::KW_INT},
    {"தசமம்", TokenType::KW_FLOAT},
    {"சரம்", TokenType::KW_STRING},
    {"வெற்று", TokenType::KW_VOID},
    {"உண்மை", TokenType::KW_TRUE},
    {"பொய்", TokenType::KW_FALSE},
    {"மெய்மை", TokenType::KW_BOOL},
    // Sized integer types
    {"எண்8", TokenType::KW_INT8},
    {"எண்16", TokenType::KW_INT16},
    {"எண்32", TokenType::KW_INT32},
    {"எண்64", TokenType::KW_INT64},
    {"எண்128", TokenType::KW_INT128},
    {"மிஎண்8", TokenType::KW_UINT8},
    {"மிஎண்16", TokenType::KW_UINT16},
    {"மிஎண்32", TokenType::KW_UINT32},
    {"மிஎண்64", TokenType::KW_UINT64},
    {"மிஎண்128", TokenType::KW_UINT128},
    // Sized float types
    {"தசமம்32", TokenType::KW_FLOAT32},
    {"தசமம்64", TokenType::KW_FLOAT64},
    {"அமைப்பு", TokenType::KW_STRUCT},
    {"பட்டியல்", TokenType::KW_ENUM},
    {"பொருத்து", TokenType::KW_MATCH},
    {"புதிய", TokenType::KW_NEW},
    {"நீக்கு", TokenType::KW_DELETE},
    {"சுற்று", TokenType::KW_FOR},
    {"உள்", TokenType::KW_IN},
    {"இறக்குமதி", TokenType::KW_IMPORT},
    {"வெளி", TokenType::KW_EXTERN},
    {"பண்பு", TokenType::KW_TRAIT},
    {"செயல்படுத்து", TokenType::KW_IMPL},
    {"நிலை", TokenType::KW_MUT},
    {"மண்டலம்", TokenType::KW_ZONE},
    {"ஒதுக்கீடு", TokenType::KW_ALLOC},
    {"கடன்", TokenType::KW_BORROW},
    {"பகிர்வு", TokenType::KW_SHARED},
    {"தப்பித்தல்", TokenType::KW_ESCAPE},
    {"இல்லை", TokenType::KW_NIL},
    {"ஆக", TokenType::KW_AS},
    {"நிலைமாறிலி", TokenType::KW_CONST},
};

// ── Constructor & Reset ──────────────────────────────────────────────────────

Lexer::Lexer(const std::string &source, const std::string &filename, DiagnosticEngine &diag)
    : source_(source), filename_(filename), diag_(diag) {}

void Lexer::reset() {
    pos_ = 0;
    line_ = 1;
    column_ = 1;
}

// ── Character helpers ────────────────────────────────────────────────────────

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

char Lexer::current() const {
    if (isAtEnd())
        return '\0';
    return source_[pos_];
}

char Lexer::peekChar() const {
    if (pos_ + 1 >= source_.size())
        return '\0';
    return source_[pos_ + 1];
}

char Lexer::advance() {
    unsigned char c = static_cast<unsigned char>(source_[pos_++]);
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        // UTF-8 aware column counting:
        // Only increment the column for the start of a code point (1-byte ASCII or lead byte)
        // High bits: 0xxxxxxx (ASCII) or 11xxxxxx (Lead byte)
        // Trailing bytes start with 10xxxxxx
        if ((c & 0xC0) != 0x80) {
            column_++;
        }
    }
    return static_cast<char>(c);
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[pos_] != expected)
        return false;
    advance();
    return true;
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    // ASCII alpha or underscore
    if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') || uc == '_')
        return true;
    // Multi-byte UTF-8 character (lead byte)
    if (uc >= 0xC0)
        return true;
    return false;
}

bool Lexer::isAlphaNumeric(char c) {
    if (isAlpha(c) || isDigit(c))
        return true;
    unsigned char uc = static_cast<unsigned char>(c);
    // UTF-8 trailing byte
    if ((uc & 0xC0) == 0x80)
        return true;
    return false;
}

// ── Whitespace & Comments ────────────────────────────────────────────────────

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peekChar() == '/') {
            skipSingleLineComment();
        } else if (c == '/' && peekChar() == '*') {
            if (!skipMultiLineComment())
                return;
        } else {
            break;
        }
    }
}

void Lexer::skipSingleLineComment() {
    while (!isAtEnd() && current() != '\n') {
        advance();
    }
}

bool Lexer::skipMultiLineComment() {
    advance(); // skip '/'
    advance(); // skip '*'
    while (!isAtEnd()) {
        if (current() == '*' && peekChar() == '/') {
            advance(); // skip '*'
            advance(); // skip '/'
            return true;
        }
        advance();
    }
    diag_.error({filename_, line_, column_}, "முடிவுறாத பலவரி கருத்துரை (Multi-line comment)");
    return false;
}

// ── Token factories ──────────────────────────────────────────────────────────

Token Lexer::makeToken(TokenType type, const std::string &value) const {
    return {type, value, filename_, line_, column_};
}

Token Lexer::errorToken(const std::string &msg) {
    diag_.error({filename_, line_, column_}, msg);
    return {TokenType::TOKEN_ERROR, msg, filename_, line_, column_};
}

// ── Read complex tokens ──────────────────────────────────────────────────────

Token Lexer::readNumber() {
    int startLine = line_;
    int startCol = column_;
    size_t start = pos_;
    bool isFloat = false;

    while (!isAtEnd() && (isDigit(current()) || current() == '_')) {
        advance();
    }

    // Check for decimal point
    if (!isAtEnd() && current() == '.' && (isDigit(peekChar()) || peekChar() == '_')) {
        isFloat = true;
        advance(); // consume '.'
        while (!isAtEnd() && (isDigit(current()) || current() == '_')) {
            advance();
        }
    }

    std::string value = source_.substr(start, pos_ - start);
    // Remove underscores
    std::string cleanValue;
    for (char c : value) {
        if (c != '_')
            cleanValue += c;
    }

    TokenType type = isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;
    return {type, cleanValue, filename_, startLine, startCol};
}

Token Lexer::readString() {
    int startLine = line_;
    int startCol = column_;
    advance(); // consume opening '"'

    std::string value;
    while (!isAtEnd() && current() != '"') {
        if (current() == '\\') {
            advance(); // consume backslash
            if (isAtEnd())
                break;
            char escaped = current();
            switch (escaped) {
            case 'n':
                value += '\n';
                break;
            case 't':
                value += '\t';
                break;
            case '\\':
                value += '\\';
                break;
            case '"':
                value += '"';
                break;
            default:
                value += escaped;
                break;
            }
        } else {
            value += current();
        }
        advance();
    }

    if (isAtEnd()) {
        return errorToken("முடிவுறாத சரம் (Unterminated string) - வரி: " +
                          std::to_string(startLine));
    }

    advance(); // consume closing '"'
    return {TokenType::STRING_LITERAL, value, filename_, startLine, startCol};
}

Token Lexer::readIdentifierOrKeyword() {
    int startLine = line_;
    int startCol = column_;
    size_t start = pos_;

    while (!isAtEnd() && isAlphaNumeric(current())) {
        advance();
    }

    std::string text = source_.substr(start, pos_ - start);

    // Check if it's a keyword
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return {it->second, text, filename_, startLine, startCol};
    }

    return {TokenType::IDENTIFIER, text, filename_, startLine, startCol};
}

// ── Main tokenization ────────────────────────────────────────────────────────

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return makeToken(TokenType::TOKEN_EOF);
    }

    int startLine = line_;
    int startCol = column_;
    char c = current();

    // Numbers
    if (isDigit(c))
        return readNumber();

    // Strings
    if (c == '"')
        return readString();

    // Identifiers and keywords
    if (isAlpha(c))
        return readIdentifierOrKeyword();

    // Operators and delimiters
    advance(); // consume the character
    switch (c) {
    case '+':
        return {TokenType::PLUS, "+", filename_, startLine, startCol};
    case '-':
        if (match('>'))
            return {TokenType::ARROW, "->", filename_, startLine, startCol};
        return {TokenType::MINUS, "-", filename_, startLine, startCol};
    case '*':
        return {TokenType::STAR, "*", filename_, startLine, startCol};
    case '/':
        return {TokenType::SLASH, "/", filename_, startLine, startCol};
    case '=':
        if (match('='))
            return {TokenType::EQ, "==", filename_, startLine, startCol};
        if (match('>'))
            return {TokenType::FAT_ARROW, "=>", filename_, startLine, startCol};
        return {TokenType::ASSIGN, "=", filename_, startLine, startCol};
    case '!':
        if (match('='))
            return {TokenType::NEQ, "!=", filename_, startLine, startCol};
        return {TokenType::NOT, "!", filename_, startLine, startCol};
    case '<':
        if (match('='))
            return {TokenType::LTE, "<=", filename_, startLine, startCol};
        return {TokenType::LT, "<", filename_, startLine, startCol};
    case '>':
        if (match('='))
            return {TokenType::GTE, ">=", filename_, startLine, startCol};
        return {TokenType::GT, ">", filename_, startLine, startCol};
    case '&':
        if (match('&'))
            return {TokenType::AND, "&&", filename_, startLine, startCol};
        return {TokenType::AMPERSAND, "&", filename_, startLine, startCol};
    case '|':
        if (match('|'))
            return {TokenType::OR, "||", filename_, startLine, startCol};
        return errorToken("எதிர்பாராத எழுத்து '|' (Unexpected character) - வரி: " +
                          std::to_string(startLine) + ", நிரல்: " + std::to_string(startCol));

    case '(':
        return {TokenType::LPAREN, "(", filename_, startLine, startCol};
    case ')':
        return {TokenType::RPAREN, ")", filename_, startLine, startCol};
    case '{':
        return {TokenType::LBRACE, "{", filename_, startLine, startCol};
    case '}':
        return {TokenType::RBRACE, "}", filename_, startLine, startCol};
    case '[':
        return {TokenType::LBRACKET, "[", filename_, startLine, startCol};
    case ']':
        return {TokenType::RBRACKET, "]", filename_, startLine, startCol};
    case ';':
        return {TokenType::SEMICOLON, ";", filename_, startLine, startCol};
    case ',':
        return {TokenType::COMMA, ",", filename_, startLine, startCol};
    case ':':
        if (match(':'))
            return {TokenType::DOUBLE_COLON, "::", filename_, startLine, startCol};
        return {TokenType::COLON, ":", filename_, startLine, startCol};
    case '.':
        if (match('.'))
            return {TokenType::DOTDOT, "..", filename_, startLine, startCol};
        return {TokenType::DOT, ".", filename_, startLine, startCol};
    case '~':
        return {TokenType::TILDE, "~", filename_, startLine, startCol};
    case '%':
        return {TokenType::PERCENT, "%", filename_, startLine, startCol};

    default:
        return errorToken("எதிர்பாராத எழுத்து '" + std::string(1, c) +
                          "' (Unexpected character) - வரி: " + std::to_string(startLine) +
                          ", நிரல்: " + std::to_string(startCol));
    }
}

std::vector<Token> Lexer::tokenize() {
    reset();
    std::vector<Token> tokens;
    while (true) {
        Token tok = nextToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::TOKEN_EOF || tok.type == TokenType::TOKEN_ERROR) {
            break;
        }
    }
    return tokens;
}

} // namespace agam
