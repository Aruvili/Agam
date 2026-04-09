#include "agam/parser/parser.h"
#include <stdexcept>
#include <sstream>

namespace agam {

Parser::Parser(const std::vector<Token> &tokens, const std::string &source, const std::string &filename, DiagnosticEngine &diag) 
    : tokens_(tokens), source_(source), filename_(filename), diag_(diag) {}

// ═══════════════════════════════════════════════════════════════════════════════
//  Token Navigation
// ═══════════════════════════════════════════════════════════════════════════════

const Token &Parser::peek() const {
    return tokens_[current_];
}

const Token &Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token &Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &errMsg) {
    if (check(type)) return advance();
    error(peek(), errMsg);
    return peek(); // Return current token even on error
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Error Handling
// ═══════════════════════════════════════════════════════════════════════════════

void Parser::error(const std::string &msg) {
    diag_.error({filename_, 0, 0}, msg);
}

void Parser::error(const Token &tok, const std::string &msg) {
    diag_.error({tok.filename, tok.line, tok.column}, msg);
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
        case TokenType::KW_FUNC:
        case TokenType::KW_LET:
        case TokenType::KW_IF:
        case TokenType::KW_WHILE:
        case TokenType::KW_RETURN:
        case TokenType::KW_STRUCT:
        case TokenType::KW_ENUM:
        case TokenType::KW_MATCH:
        case TokenType::KW_DELETE:
            return;
        default:
            advance();
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Entry Point
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<Program> Parser::parse() {
    current_ = 0;
    diag_.clear();
    return parseProgram();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Top-Level Rules
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        try {
            if (match(TokenType::KW_IMPORT)) {
                Token pathTok = consume(TokenType::STRING_LITERAL, "இறக்குமதிக்கு (import) பின் சரம் (string literal) எதிர்பார்க்கிறோம்");
                // Remove quotes from string literal
                std::string path = pathTok.value;
                if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
                    path = path.substr(1, path.size() - 2);
                }
                program->imports.push_back(path);
                consume(TokenType::SEMICOLON, "இறக்குமதிப் பாதைக்குப்பின் ';' எதிர்பார்க்கிறோம்");
            } else if (check(TokenType::KW_FUNC) || check(TokenType::KW_EXTERN)) {
                auto func = parseFunctionDecl();
                if (func) {
                    program->functions.push_back(std::move(func));
                }
            } else if (check(TokenType::KW_CONST)) {
                auto constant = parseConstDecl();
                if (constant) {
                    program->constants.push_back(std::move(constant));
                }
            } else if (check(TokenType::KW_STRUCT)) {
                auto sd = parseStructDecl();
                if (sd) {
                    program->structs.push_back(std::move(sd));
                }
            } else if (check(TokenType::KW_ENUM)) {
                auto ed = parseEnumDecl();
                if (ed) {
                    program->enums.push_back(std::move(ed));
                }
            } else if (match(TokenType::KW_TRAIT)) {
                program->traits.push_back(parseTraitDecl());
            } else if (match(TokenType::KW_IMPL)) {
                program->impls.push_back(parseImplDecl());
            } else {
                error(peek(), "இறக்குமதி (import), செயல் (func), அமைப்பு (struct) அல்லது பட்டியல் (enum) அறிவிப்பை எதிர்பார்க்கிறோம்");
                synchronize();
            }
        } catch (const std::runtime_error &) {
            synchronize();
        }
    }

    return program;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl() {
    bool isExtern = match(TokenType::KW_EXTERN);
    Token funcTok = consume(TokenType::KW_FUNC, "செயல் (func) அறிவிப்பை எதிர்பார்க்கிறோம்");
    Token nameTok = consume(TokenType::IDENTIFIER, "செயல்பெயரை (function name) எதிர்பார்க்கிறோம்");
    std::string name = nameTok.value;

    std::vector<std::string> typeParams;
    if (match(TokenType::LT)) {
        typeParams = parseTypeParams();
    }

    // Save enclosing type params (e.g., from impl<T>) and merge
    auto savedTypeParams = currentTypeParams_;
    for (const auto &tp : typeParams) {
        currentTypeParams_.push_back(tp);
    }

    consume(TokenType::LPAREN, "செயல்பெயருக்குப்பின் '(' எதிர்பார்க்கிறோம்");
    auto params = parseParamList();
    consume(TokenType::RPAREN, "அளபுருக்களுக்குப்பிறகு (parameters) ')' எதிர்பார்க்கிறோம்");

    consume(TokenType::COLON, "திரும்பும் வகைக்கு (return type) முன் ':' எதிர்பார்க்கிறோம்");
    TypeInfo retType = parseTypeSpec();

    std::unique_ptr<BlockStmt> body = nullptr;
    if (isExtern) {
        consume(TokenType::SEMICOLON, "வெளிப்புறச் செயல் (extern func) அறிவிப்புக்குப்பின் ';' எதிர்பார்க்கிறோம்");
    } else if (check(TokenType::SEMICOLON)) {
        advance();
    } else {
        body = parseBlock();
        if (!body) {
            currentTypeParams_ = savedTypeParams;
            return nullptr;
        }
    }

    currentTypeParams_ = savedTypeParams;

    bool isMutating = false;
    for (const auto &p : params) {
        if (p.name == "தன்னிலை" && p.isMutable) {
            isMutating = true;
            break;
        }
    }

    auto func = std::make_unique<FunctionDecl>(
        name, std::move(typeParams), std::move(params), retType, std::move(body), isExtern, isMutating);
    func->loc = {funcTok.filename, funcTok.line, funcTok.column};
    return func;
}

std::unique_ptr<ConstDecl> Parser::parseConstDecl() {
    Token constTok = consume(TokenType::KW_CONST, "நிலைமாறிலி (const) அறிவிப்பை எதிர்பார்க்கிறோம்");
    Token nameTok = consume(TokenType::IDENTIFIER, "நிலைமாறிலி பெயரை எதிர்பார்க்கிறோம்");
    consume(TokenType::COLON, "நிலைமாறிலி பெயருக்குப்பின் ':' எதிர்பார்க்கிறோம்");
    TypeInfo ti = parseTypeSpec();
    consume(TokenType::ASSIGN, "நிலைமாறிலி வகைக்குப்பின் '=' எதிர்பார்க்கிறோம்");
    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "நிலைமாறிலி அறிவிப்புக்குப்பின் ';' எதிர்பார்க்கிறோம்");

    auto cd = std::make_unique<ConstDecl>(nameTok.value, ti, std::move(value));
    cd->loc = {constTok.filename, constTok.line, constTok.column};
    return cd;
}

std::vector<Param> Parser::parseParamList() {
    std::vector<Param> params;

    if (check(TokenType::RPAREN)) return params; // Empty param list

    do {
        bool isMut = match(TokenType::KW_MUT);
        Token nameTok = consume(TokenType::IDENTIFIER, "அளபுரு (parameter) பெயரை எதிர்பார்க்கிறோம்");
        consume(TokenType::COLON, "அளபுரு பெயருக்குப்பின் ':' எதிர்பார்க்கிறோம்");
        TypeInfo ti = parseTypeSpec();
        params.push_back({nameTok.value, ti, isMut});
    } while (match(TokenType::COMMA));

    return params;
}

TypeInfo Parser::parseTypeSpec() {
    int pointerDepth = 0;
    while (match(TokenType::STAR)) {
        pointerDepth++;
    }

    if (match(TokenType::AMPERSAND)) {
        if (match(TokenType::LBRACKET)) {
            TypeInfo elemTI = parseTypeSpec();
            consume(TokenType::RBRACKET, "சிறுதுண்டு வகைக்குப்பின் (slice type) ']' எதிர்பார்க்கிறோம்");
            TypeInfo ti = TypeInfo::slice(elemTI);
            ti.pointerDepth = pointerDepth;
            return ti;
        } else {
            // If we ever support &T references, they'd go here.
            // For now, & before [ is a slice.
            error(peek(), "சிறுதுண்டு வகையில் (slice type) '&' பின் '[' எதிர்பார்க்கிறோம்");
        }
    }

    TypeInfo ti;
    // Array type: [elementType; size]
    if (match(TokenType::LBRACKET)) {
        TypeInfo elemTI = parseTypeSpec();
        if (elemTI.isArray) {
            error(peek(), "உட்பொதிந்த அணிகள் (Nested arrays) இன்னும் ஆதரிக்கப்படவில்லை");
        }
        consume(TokenType::SEMICOLON, "அணி வகை [T; N] இல் ';' எதிர்பார்க்கிறோம்");
        Token sizeTok = consume(TokenType::INT_LITERAL, "அணியின் அளவை (array size) எதிர்பார்க்கிறோம்");
        int size = std::stoi(sizeTok.value);
        consume(TokenType::RBRACKET, "அணி வகைக்குப்பின் ']' எதிர்பார்க்கிறோம்");
        ti = TypeInfo::array(elemTI, size);
    } else if (match(TokenType::KW_INT))     ti = TypeInfo::scalar(TypeKind::Int);
    else if (match(TokenType::KW_FLOAT))   ti = TypeInfo::scalar(TypeKind::Float);
    else if (match(TokenType::KW_STRING))  ti = TypeInfo::scalar(TypeKind::String);
    else if (match(TokenType::KW_BOOL))    ti = TypeInfo::scalar(TypeKind::Bool);
    else if (match(TokenType::KW_VOID))    ti = TypeInfo::scalar(TypeKind::Void);
    // Sized integers
    else if (match(TokenType::KW_INT8))    ti = TypeInfo::scalar(TypeKind::Int8);
    else if (match(TokenType::KW_INT16))   ti = TypeInfo::scalar(TypeKind::Int16);
    else if (match(TokenType::KW_INT32))   ti = TypeInfo::scalar(TypeKind::Int32);
    else if (match(TokenType::KW_INT64))   ti = TypeInfo::scalar(TypeKind::Int64);
    else if (match(TokenType::KW_INT128))  ti = TypeInfo::scalar(TypeKind::Int128);
    // Unsigned
    else if (match(TokenType::KW_UINT8))   ti = TypeInfo::scalar(TypeKind::UInt8);
    else if (match(TokenType::KW_UINT16))  ti = TypeInfo::scalar(TypeKind::UInt16);
    else if (match(TokenType::KW_UINT32))  ti = TypeInfo::scalar(TypeKind::UInt32);
    else if (match(TokenType::KW_UINT64))  ti = TypeInfo::scalar(TypeKind::UInt64);
    else if (match(TokenType::KW_UINT128)) ti = TypeInfo::scalar(TypeKind::UInt128);
    // Sized floats
    else if (match(TokenType::KW_FLOAT32)) ti = TypeInfo::scalar(TypeKind::Float32);
    else if (match(TokenType::KW_FLOAT64)) ti = TypeInfo::scalar(TypeKind::Float64);
    else if (match(TokenType::IDENTIFIER)) {
        Token typeNameTok = previous();
        if (typeNameTok.value == "தன்னிலை") {
            ti = TypeInfo::scalar(TypeKind::Self);
        } else {
            // Check if it's a generic parameter
            bool isGen = false;
            for (const auto& tp : currentTypeParams_) {
                if (tp == typeNameTok.value) {
                    ti = TypeInfo::generic(tp);
                    isGen = true;
                    break;
                }
            }
            if (!isGen) {
                ti = TypeInfo::namedStruct(typeNameTok.value);
            }
        }
    } else {
        error(peek(), "வகை பெயரை (type name) எதிர்பார்க்கிறோம்");
        ti = TypeInfo::scalar(TypeKind::Unknown);
    }

    ti.pointerDepth = pointerDepth;

    if (match(TokenType::LT)) {
        ti.genericArgs = parseTypeArgs();
    }
    
    // ZPM Pulse Tag: ~A
    if (match(TokenType::TILDE)) {
        Token tagTok = consume(TokenType::IDENTIFIER, "மண்டல அடையாளங்காட்டியை (region identifier) '~' பின் எதிர்பார்க்கிறோம்");
        ti.pulseTag = tagTok.value;
    }

    return ti;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Statements
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    Token lbrace = consume(TokenType::LBRACE, "'{' எதிர்பார்க்கிறோம்");

    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }

    consume(TokenType::RBRACE, "'}' எதிர்பார்க்கிறோம்");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::KW_LET))    return parseVarDecl();
    if (check(TokenType::KW_IF))     return parseIfStmt();
    if (check(TokenType::KW_WHILE))  return parseWhileStmt();
    if (check(TokenType::KW_FOR))    return parseForStmt();
    if (check(TokenType::KW_RETURN)) return parseReturnStmt();
    if (check(TokenType::KW_DELETE)) return parseDeleteStmt();

    if (check(TokenType::LBRACE)) {
        auto block = parseBlock();
        // Allow optional semicolon after block statement
        match(TokenType::SEMICOLON);
        return block;
    }
    if (match(TokenType::SEMICOLON)) {
        // Empty statement
        return std::make_unique<BlockStmt>(std::vector<std::unique_ptr<Stmt>>());
    }

    // Expression statement
    Token startTok = peek();
    auto expr = parseExpression();
    if (!expr) {
        synchronize();
        return nullptr;
    }
    consume(TokenType::SEMICOLON, "கோவையின் (expression) பின் ';' எதிர்பார்க்கிறோம்");
    auto stmt = std::make_unique<ExprStmt>(std::move(expr));
    stmt->loc = {startTok.filename, startTok.line, startTok.column};
    return stmt;
}

std::unique_ptr<VarDeclStmt> Parser::parseVarDecl() {
    Token letTok = consume(TokenType::KW_LET, "'மாறி' (let) அறிவிப்பை எதிர்பார்க்கிறோம்");
    bool isMut = match(TokenType::KW_MUT);
    Token nameTok = consume(TokenType::IDENTIFIER, "மாறி (variable) பெயரை எதிர்பார்க்கிறோம்");
    consume(TokenType::COLON, "மாறி பெயருக்குப்பின் ':' எதிர்பார்க்கிறோம்");
    TypeInfo ti = parseTypeSpec();

    std::unique_ptr<Expr> init = nullptr;
    if (match(TokenType::ASSIGN)) {
        init = parseExpression();
    }

    consume(TokenType::SEMICOLON, "மாறி அறிவிப்புக்குப்பின் ';' எதிர்பார்க்கிறோம்");
    auto stmt = std::make_unique<VarDeclStmt>(nameTok.value, ti, std::move(init), isMut);
    stmt->loc = {letTok.filename, letTok.line, letTok.column};
    return stmt;
}

std::unique_ptr<IfStmt> Parser::parseIfStmt() {
    Token ifTok = consume(TokenType::KW_IF, "'எனில்' (if) அறிவிப்பை எதிர்பார்க்கிறோம்");
    consume(TokenType::LPAREN, "'எனில்' (if) பின் '(' எதிர்பார்க்கிறோம்");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "'எனில்' (if) நிபந்தனைக்குப்பின் ')' எதிர்பார்க்கிறோம்");

    auto thenBranch = parseBlock();

    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match(TokenType::KW_ELSE)) {
        if (check(TokenType::KW_IF)) {
            elseBranch = parseIfStmt(); // else if
        } else {
            elseBranch = parseBlock(); // else { ... }
        }
    }

    auto stmt = std::make_unique<IfStmt>(
        std::move(condition), std::move(thenBranch), std::move(elseBranch));
    stmt->loc = {ifTok.filename, ifTok.line, ifTok.column};
    return stmt;
}

std::unique_ptr<WhileStmt> Parser::parseWhileStmt() {
    Token whileTok = consume(TokenType::KW_WHILE, "'வரை' (while) அறிவிப்பை எதிர்பார்க்கிறோம்");
    consume(TokenType::LPAREN, "'வரை' (while) பின் '(' எதிர்பார்க்கிறோம்");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "'வரை' (while) நிபந்தனைக்குப்பின் ')' எதிர்பார்க்கிறோம்");

    auto body = parseBlock();

    auto stmt = std::make_unique<WhileStmt>(std::move(condition), std::move(body));
    stmt->loc = {whileTok.filename, whileTok.line, whileTok.column};
    return stmt;
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt() {
    Token retTok = consume(TokenType::KW_RETURN, "'விடை' (return) அறிவிப்பை எதிர்பார்க்கிறோம்");

    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }

    consume(TokenType::SEMICOLON, "'விடை' (return) அறிவிப்பிற்குப்பின் ';' எதிர்பார்க்கிறோம்");
    auto stmt = std::make_unique<ReturnStmt>(std::move(value));
    stmt->loc = {retTok.filename, retTok.line, retTok.column};
    return stmt;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Expressions (precedence climbing, lowest to highest)
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto expr = parseOr();

    if (check(TokenType::ASSIGN)) {
        if (auto *varExpr = dynamic_cast<VariableExpr *>(expr.get())) {
            std::string name = varExpr->name;
            Token eqTok = advance(); // consume '='
            auto value = parseAssignment(); // Right-associative
            auto assign = std::make_unique<AssignExpr>(name, std::move(value));
            assign->loc = {eqTok.filename, eqTok.line, eqTok.column};
            return assign;
        } else if (auto *unary = dynamic_cast<UnaryExpr *>(expr.get())) {
            if (unary->op == UnaryOp::Dereference) {
                Token eqTok = advance(); // consume '='
                auto value = parseAssignment();
                auto derefAssign = std::make_unique<DerefAssignExpr>(std::move(unary->operand), std::move(value));
                derefAssign->loc = {eqTok.filename, eqTok.line, eqTok.column};
                return derefAssign;
            }
        }
        
        error(peek(), "தவறான ஒதுக்கீடு இலக்கு (Invalid assignment target)");
        return expr;
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseOr() {
    auto left = parseAnd();

    while (match(TokenType::OR)) {
        Token opTok = previous();
        auto right = parseAnd();
        auto bin = std::make_unique<BinaryExpr>(
            BinaryOp::Or, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    auto left = parseEquality();

    while (match(TokenType::AND)) {
        Token opTok = previous();
        auto right = parseEquality();
        auto bin = std::make_unique<BinaryExpr>(
            BinaryOp::And, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseComparison();

    while (match({TokenType::EQ, TokenType::NEQ})) {
        Token opTok = previous();
        BinaryOp op = (opTok.type == TokenType::EQ) ? BinaryOp::Eq : BinaryOp::Neq;
        auto right = parseComparison();
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddition();

    while (match({TokenType::LT, TokenType::GT, TokenType::LTE, TokenType::GTE})) {
        Token opTok = previous();
        BinaryOp op;
        switch (opTok.type) {
        case TokenType::LT:  op = BinaryOp::Lt; break;
        case TokenType::GT:  op = BinaryOp::Gt; break;
        case TokenType::LTE: op = BinaryOp::Lte; break;
        case TokenType::GTE: op = BinaryOp::Gte; break;
        default: op = BinaryOp::Lt; break;
        }
        auto right = parseAddition();
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseAddition() {
    auto left = parseMultiplication();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token opTok = previous();
        BinaryOp op = (opTok.type == TokenType::PLUS) ? BinaryOp::Add : BinaryOp::Sub;
        auto right = parseMultiplication();
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplication() {
    auto left = parseUnary();

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token opTok = previous();
        BinaryOp op;
        switch (opTok.type) {
            case TokenType::STAR: op = BinaryOp::Mul; break;
            case TokenType::SLASH: op = BinaryOp::Div; break;
            case TokenType::PERCENT: op = BinaryOp::Mod; break;
            default: op = BinaryOp::Mul; break;
        }
        auto right = parseUnary();
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->loc = {opTok.filename, opTok.line, opTok.column};
        left = std::move(bin);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::MINUS)) {
        Token opTok = previous();
        auto operand = parseUnary();
        auto unary = std::make_unique<UnaryExpr>(UnaryOp::Negate, std::move(operand));
        unary->loc = {opTok.filename, opTok.line, opTok.column};
        return unary;
    }

    if (match(TokenType::NOT)) {
        Token opTok = previous();
        auto operand = parseUnary();
        auto unary = std::make_unique<UnaryExpr>(UnaryOp::Not, std::move(operand));
        unary->loc = {opTok.filename, opTok.line, opTok.column};
        return unary;
    }

    if (match(TokenType::AMPERSAND)) {
        Token opTok = previous();
        auto operand = parseUnary();
        auto unary = std::make_unique<UnaryExpr>(UnaryOp::AddressOf, std::move(operand));
        unary->loc = {opTok.filename, opTok.line, opTok.column};
        return unary;
    }

    if (match(TokenType::STAR)) {
        Token opTok = previous();
        auto operand = parseUnary();
        auto unary = std::make_unique<UnaryExpr>(UnaryOp::Dereference, std::move(operand));
        unary->loc = {opTok.filename, opTok.line, opTok.column};
        return unary;
    }

    return parseCall();
}

std::unique_ptr<Expr> Parser::parseCall() {
    auto expr = parsePrimary();

    // Handle postfix operators: function calls and indexing
    while (true) {
        if (auto *varExpr = dynamic_cast<VariableExpr *>(expr.get())) {
            if (match(TokenType::LPAREN)) {
                std::string callee = varExpr->name;
                SourceLocation loc = varExpr->loc;

                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RPAREN)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RPAREN, "அளபுருக்களுக்குப்பிறகு (arguments) ')' எதிர்பார்க்கிறோம்");

                auto call = std::make_unique<CallExpr>(callee, std::move(args), varExpr->genericArgs);
                call->loc = loc;
                expr = std::move(call);
                continue;
            }
        }

        if (match(TokenType::LBRACKET)) {
            auto index = parseExpression();
            bool isRange = false;
            std::unique_ptr<Expr> endIndex = nullptr;

            if (match(TokenType::DOTDOT)) {
                isRange = true;
                endIndex = parseExpression();
            }

            consume(TokenType::RBRACKET, "அட்டவணைக்குப்பின் (index) ']' எதிர்பார்க்கிறோம்");

            // Check for index assignment: arr[i] = value (only for simple index)
            if (!isRange && match(TokenType::ASSIGN)) {
                auto value = parseExpression();
                auto ia = std::make_unique<IndexAssignExpr>(
                    std::move(expr), std::move(index), std::move(value));
                expr = std::move(ia);
            } else {
                auto ie = std::make_unique<IndexExpr>(std::move(expr), std::move(index), std::move(endIndex), isRange);
                expr = std::move(ie);
            }
            continue;
        }

        // Postfix field access or method call
        if (match(TokenType::DOT)) {
            Token memberTok = consume(TokenType::IDENTIFIER, "'.'-க்குப்பின் உறுப்பு பெயரை (member name) எதிர்பார்க்கிறோம்");
            
            std::vector<TypeInfo> genArgs;
            if (match(TokenType::LT)) {
                do {
                    genArgs.push_back(parseTypeSpec());
                } while (match(TokenType::COMMA));
                consume(TokenType::GT, "பொதுவான தருமங்களுக்குப்பின் (generic arguments) '>' எதிர்பார்க்கிறோம்");
            }

            if (match(TokenType::LPAREN)) {
                // Method call: obj.method<T>(args)
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RPAREN)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RPAREN, "செயல்முறை அளபுருக்களுக்குப்பின் (method arguments) ')' எதிர்பார்க்கிறோம்");
                auto mc = std::make_unique<MethodCallExpr>(std::move(expr), memberTok.value, std::move(args), std::move(genArgs));
                mc->loc = {memberTok.filename, memberTok.line, memberTok.column};
                expr = std::move(mc);
            } else if (match(TokenType::ASSIGN)) {
                // Field assignment: expr.field = value
                auto value = parseExpression();
                auto fa = std::make_unique<FieldAssignExpr>(
                    std::move(expr), memberTok.value, std::move(value));
                fa->loc = {memberTok.filename, memberTok.line, memberTok.column};
                expr = std::move(fa);
            } else {
                // Regular field access: expr.field
                auto fa = std::make_unique<FieldAccessExpr>(std::move(expr), memberTok.value);
                fa->loc = {memberTok.filename, memberTok.line, memberTok.column};
                expr = std::move(fa);
            }
            continue;
        }

        // Cast expression: expr as Type
        if (match(TokenType::KW_AS)) {
            TypeInfo targetType = parseTypeSpec();
            auto cast = std::make_unique<CastExpr>(std::move(expr), std::move(targetType));
            cast->loc = {previous().filename, previous().line, previous().column};
            expr = std::move(cast);
            continue;
        }

        break;
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    // Integer literal
    if (match(TokenType::INT_LITERAL)) {
        Token tok = previous();
        auto expr = std::make_unique<IntLiteralExpr>(std::stoll(tok.value));
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    // Float literal
    if (match(TokenType::FLOAT_LITERAL)) {
        Token tok = previous();
        auto expr = std::make_unique<FloatLiteralExpr>(std::stod(tok.value));
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    // String literal
    if (match(TokenType::STRING_LITERAL)) {
        Token tok = previous();
        auto expr = std::make_unique<StringLiteralExpr>(tok.value);
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    // Boolean literals
    if (match(TokenType::KW_TRUE)) {
        Token tok = previous();
        auto expr = std::make_unique<BoolLiteralExpr>(true);
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }
    if (match(TokenType::KW_FALSE)) {
        Token tok = previous();
        auto expr = std::make_unique<BoolLiteralExpr>(false);
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    if (match(TokenType::KW_NIL)) {
        Token tok = previous();
        auto expr = std::make_unique<NullLiteralExpr>();
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    if (match(TokenType::KW_MATCH)) {
        return parseMatchExpr();
    }

    // Heap allocation: new T or new [T; n]
    if (match(TokenType::KW_NEW)) {
        Token newTok = previous();
        
        // Special case: new [T; n]
        if (check(TokenType::LBRACKET)) {
            consume(TokenType::LBRACKET, "'[' எதிர்பார்க்கிறோம்");
            TypeInfo elemTI = parseTypeSpec();
            consume(TokenType::SEMICOLON, "இயக்கநிலை அணி ஒதுக்கீட்டில் (dynamic array allocation) ';' எதிர்பார்க்கிறோம்");
            auto sizeExpr = parseExpression();
            consume(TokenType::RBRACKET, "']' எதிர்பார்க்கிறோம்");
            
            TypeInfo arrayTI = TypeInfo::array(elemTI, 0); // size 0 means dynamic
            auto expr = std::make_unique<NewExpr>(arrayTI, std::move(sizeExpr));
            expr->loc = {newTok.filename, newTok.line, newTok.column};
            return expr;
        }

        TypeInfo ti = parseTypeSpec();
        auto expr = std::make_unique<NewExpr>(ti);
        expr->loc = {newTok.filename, newTok.line, newTok.column};
        return expr;
    }

    if (match(TokenType::KW_ZONE)) return parseZoneExpr();
    if (match(TokenType::KW_ALLOC)) return parseAllocExpr();
    if (match(TokenType::KW_BORROW)) return parseBorrowExpr();
    if (match(TokenType::KW_ESCAPE)) return parseEscapeExpr();

    // Identifier: either a variable or the start of a struct literal: SomeName { ... } or SomeName<T> { ... }
    if (match(TokenType::IDENTIFIER)) {
        Token tok = previous();
        TypeInfo ti = TypeInfo::namedStruct(tok.value);

        // Optional generic arguments: Name<T, U>
        // Disambiguate: only treat as generics if followed by '{' or '('
        bool isGeneric = false;
        if (check(TokenType::LT)) {
            int depth = 0;
            int lookahead = 0;
            while (true) {
                if (current_ + lookahead >= tokens_.size()) break;
                TokenType type = tokens_[current_ + lookahead].type;
                if (type == TokenType::LT) depth++;
                else if (type == TokenType::GT) {
                    depth--;
                    if (depth == 0) {
                        // Found matching '>', now check if next is '{' or '('
                        if (current_ + lookahead + 1 < tokens_.size()) {
                            TokenType nextType = tokens_[current_ + lookahead + 1].type;
                            if (nextType == TokenType::LBRACE || nextType == TokenType::LPAREN || nextType == TokenType::DOUBLE_COLON) {
                                isGeneric = true;
                            }
                        }
                        break;
                    }
                }
                else if (type == TokenType::TOKEN_EOF || type == TokenType::LBRACE || type == TokenType::RBRACE || type == TokenType::SEMICOLON) {
                    break;
                }
                lookahead++;
            }
        }

        if (isGeneric && match(TokenType::LT)) {
            do {
                ti.genericArgs.push_back(parseTypeSpec());
            } while (match(TokenType::COMMA));
            consume(TokenType::GT, "பொதுவான தருமங்களுக்குப்பின் (generic arguments) '>' எதிர்பார்க்கிறோம்");
        }
        
        // Enum variant instantiation: EnumName::Variant or EnumName::Variant(payload)
        if (match(TokenType::DOUBLE_COLON)) {
            Token variantTok = consume(TokenType::IDENTIFIER, "'::' பின் எண்ணும்தொகுதி (enum variant) பெயரை எதிர்பார்க்கிறோம்");
            std::unique_ptr<Expr> payload = nullptr;
            if (match(TokenType::LPAREN)) {
                payload = parseExpression();
                consume(TokenType::RPAREN, "எண்ணும்தொகுதி சுமைக்குப்பிறகு (enum variant payload) ')' எதிர்பார்க்கிறோம்");
            }
            auto expr = std::make_unique<EnumVariantExpr>(tok.value, variantTok.value, ti.genericArgs, std::move(payload));
            expr->loc = {tok.filename, tok.line, tok.column};
            return expr;
        }

        // Struct literal: Name { field: expr, ... }
        if (check(TokenType::LBRACE)) {
            advance(); // consume '{'
            std::vector<StructFieldInit> fields;
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                Token fnameTok = consume(TokenType::IDENTIFIER, "புலப் பெயரை (field name) எதிர்பார்க்கிறோம்");
                consume(TokenType::COLON, "புலப் பெயருக்குப்பின் ':' எதிர்பார்க்கிறோம்");
                auto fval = parseExpression();
                StructFieldInit sfi;
                sfi.name = fnameTok.value;
                sfi.value = std::move(fval);
                fields.push_back(std::move(sfi));
                if (!match(TokenType::COMMA)) break;
            }
            consume(TokenType::RBRACE, "அமைப்பு மாறிலிக்குப்பின் (struct literal) '}' எதிர்பார்க்கிறோம்");
            auto slit = std::make_unique<StructLiteralExpr>(ti, std::move(fields));
            slit->loc = {tok.filename, tok.line, tok.column};
            return slit;
        }
        auto expr = std::make_unique<VariableExpr>(tok.value, ti.genericArgs);
        expr->loc = {tok.filename, tok.line, tok.column};
        return expr;
    }

    // Grouped expression: ( expr )
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "கோவைக்குப்பிறகு (expression) ')' எதிர்பார்க்கிறோம்");
        return expr;
    }

    // Array literal: [expr, expr, ...]
    if (match(TokenType::LBRACKET)) {
        Token bracketTok = previous();
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(parseExpression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACKET, "அணி மாறிலிக்குப்பிறகு (array literal) ']' எதிர்பார்க்கிறோம்");
        auto expr = std::make_unique<ArrayLiteralExpr>(std::move(elements));
        expr->loc = {bracketTok.filename, bracketTok.line, bracketTok.column};
        return expr;
    }

    error(peek(), "கோவையை (expression) எதிர்பார்க்கிறோம்");
    advance(); // Skip the problematic token
    return std::make_unique<IntLiteralExpr>(0); // Error recovery: return dummy
}

std::unique_ptr<StructDecl> Parser::parseStructDecl() {
    Token sTok = consume(TokenType::KW_STRUCT, "'அமைப்பு' (struct) அறிவிப்பை எதிர்பார்க்கிறோம்");
    Token nameTok = consume(TokenType::IDENTIFIER, "அமைப்பு (struct) பெயரை எதிர்பார்க்கிறோம்");

    std::vector<std::string> typeParams;
    if (match(TokenType::LT)) {
        typeParams = parseTypeParams();
    }

    currentTypeParams_ = typeParams;
    consume(TokenType::LBRACE, "அமைப்பு பெயருக்குப்பின் '{' எதிர்பார்க்கிறோம்");

    std::vector<StructField> fields;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        bool isMut = match(TokenType::KW_MUT);
        Token fnameTok = consume(TokenType::IDENTIFIER, "புலப் பெயரை (field name) எதிர்பார்க்கிறோம்");
        consume(TokenType::COLON, "புலப் பெயருக்குப்பின் ':' எதிர்பார்க்கிறோம்");
        TypeInfo ftype = parseTypeSpec();
        fields.push_back({fnameTok.value, ftype, isMut});
        if (!match(TokenType::COMMA)) break;
    }
    consume(TokenType::RBRACE, "புலங்களுக்குப்பின் (fields) '}' எதிர்பார்க்கிறோம்");
    currentTypeParams_.clear();

    auto sd = std::make_unique<StructDecl>(nameTok.value, std::move(typeParams), std::move(fields));
    sd->loc = {sTok.filename, sTok.line, sTok.column};
    return sd;
}

std::unique_ptr<EnumDecl> Parser::parseEnumDecl() {
    Token eTok = consume(TokenType::KW_ENUM, "'பட்டியல்' (enum) அறிவிப்பை எதிர்பார்க்கிறோம்");
    Token nameTok = consume(TokenType::IDENTIFIER, "பட்டியல் (enum) பெயரை எதிர்பார்க்கிறோம்");

    std::vector<std::string> typeParams;
    if (match(TokenType::LT)) {
        typeParams = parseTypeParams();
    }

    currentTypeParams_ = typeParams;
    consume(TokenType::LBRACE, "பட்டியல் பெயருக்குப்பின் '{' எதிர்பார்க்கிறோம்");

    std::vector<EnumVariant> variants;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        Token vnameTok;
        if (check(TokenType::IDENTIFIER) || check(TokenType::KW_NIL)) {
            vnameTok = advance();
        } else {
            vnameTok = consume(TokenType::IDENTIFIER, "மாற்றினை (variant) எதிர்பார்க்கிறோம்");
        }
        
        EnumVariant variant;
        variant.name = vnameTok.value;
        if (match(TokenType::LPAREN)) {
            variant.hasPayload = true;
            variant.payloadType = parseTypeSpec();
            consume(TokenType::RPAREN, "சுமை வகைக்குப்பின் (payload type) ')' எதிர்பார்க்கிறோம்");
        }
        variants.push_back(std::move(variant));
        if (!match(TokenType::COMMA)) break;
    }
    consume(TokenType::RBRACE, "பட்டியல் மாற்றின்களுக்குப்பின் (variants) '}' எதிர்பார்க்கிறோம்");
    currentTypeParams_.clear();

    auto ed = std::make_unique<EnumDecl>(nameTok.value, std::move(typeParams), std::move(variants));
    ed->loc = {eTok.filename, eTok.line, eTok.column};
    return ed;
}

std::unique_ptr<Expr> Parser::parseMatchExpr() {
    Token matchTok = previous(); // Called after match(KW_MATCH)
    consume(TokenType::LPAREN, "'பொருத்து' (match) பின் '(' எதிர்பார்க்கிறோம்");
    auto value = parseExpression();
    consume(TokenType::RPAREN, "'பொருத்து' (match) மதிப்புக்குப்பின் ')' எதிர்பார்க்கிறோம்");
    consume(TokenType::LBRACE, "'பொருத்து' (match) மதிப்புக்குப்பிறகு '{' எதிர்பார்க்கிறோம்");

    std::vector<MatchArm> arms;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        MatchArm arm;
        
        Token variantTok;
        if (check(TokenType::IDENTIFIER) || check(TokenType::KW_NIL)) {
            variantTok = advance();
        } else {
            variantTok = consume(TokenType::IDENTIFIER, "'பொருத்து' (match) பிரிவின் மாற்றின் (variant) பெயரை எதிர்பார்க்கிறோம்");
        }
        arm.variantName = variantTok.value;

        if (match(TokenType::DOUBLE_COLON)) {
            Token actualVariantTok;
            if (check(TokenType::IDENTIFIER) || check(TokenType::KW_NIL)) {
                actualVariantTok = advance();
            } else {
                actualVariantTok = consume(TokenType::IDENTIFIER, "'::' பின் மாற்றின் (variant) பெயரை எதிர்பார்க்கிறோம்");
            }
            arm.variantName = actualVariantTok.value;
        }

        if (match(TokenType::LPAREN)) {
            arm.hasBinding = true;
            Token bindTok = consume(TokenType::IDENTIFIER, "சுமை பிணைப்பு (payload binding) பெயரை எதிர்பார்க்கிறோம்");
            arm.bindingName = bindTok.value;
            consume(TokenType::RPAREN, "சுமை பிணைப்பிற்குப்பின் ')' எதிர்பார்க்கிறோம்");
        }

        consume(TokenType::FAT_ARROW, "'பொருத்து' (match) பிரிவு அமைப்பிற்குப்பின் '=>' எதிர்பார்க்கிறோம்");

        if (check(TokenType::LBRACE)) {
            arm.body = parseBlock();
        } else {
            arm.body = parseExpression();
            if (!check(TokenType::RBRACE) && !check(TokenType::COMMA)) {
                // error?
            }
            match(TokenType::COMMA);
            arms.push_back(std::move(arm));
            continue;
        }

        arms.push_back(std::move(arm));
        match(TokenType::COMMA); // optional after block
    }
    consume(TokenType::RBRACE, "'பொருத்து' (match) பிரிவுகளுக்குப்பின் (arms) '}' எதிர்பார்க்கிறோம்");
    auto expr = std::make_unique<MatchExpr>(std::move(value), std::move(arms));
    expr->loc = {matchTok.filename, matchTok.line, matchTok.column};
    return expr;
}

std::unique_ptr<Expr> Parser::parseZoneExpr() {
    Token zoneTok = previous();
    Token nameTok = consume(TokenType::IDENTIFIER, "மண்டல (zone) பெயரை எதிர்பார்க்கிறோம்");
    auto body = parseBlock();
    auto expr = std::make_unique<ZoneExpr>(nameTok.value, 
        std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(body.release())));
    expr->loc = {zoneTok.filename, zoneTok.line, zoneTok.column};
    return expr;
}

std::unique_ptr<Expr> Parser::parseAllocExpr() {
    Token allocTok = previous();
    consume(TokenType::LT, "'ஒதுக்கீடு' (alloc) பின் '<' எதிர்பார்க்கிறோம்");
    TypeInfo ti = parseTypeSpec();
    consume(TokenType::GT, "வகையின் பின் '>' எதிர்பார்க்கிறோம்");
    
    std::unique_ptr<Expr> count = nullptr;
    if (match(TokenType::LPAREN)) {
        count = parseExpression();
        consume(TokenType::RPAREN, "எதிர்பார்க்கிறோம் ')' after count");
    }
    
    auto expr = std::make_unique<AllocExpr>(ti, std::move(count));
    expr->loc = {allocTok.filename, allocTok.line, allocTok.column};
    return expr;
}

std::unique_ptr<Expr> Parser::parseBorrowExpr() {
    Token borrowTok = previous();
    bool isMut = false;
    
    if (match(TokenType::KW_MUT)) isMut = true;
    else if (match(TokenType::KW_SHARED)) isMut = false;
    else error(peek(), "'கடன்' (borrow) பின் 'பகிர்வு' (shared) அல்லது 'நிலை' (mut) எதிர்பார்க்கிறோம்");
    
    consume(TokenType::LPAREN, "கடன் முறைக்குப் (borrow mode) பின் '(' எதிர்பார்க்கிறோம்");
    auto target = parseExpression();
    consume(TokenType::RPAREN, "இலக்கிற்குப்பின் (target) ')' எதிர்பார்க்கிறோம்");
    
    // Optional: as <identifier>
    if (match(TokenType::KW_AS)) {
        consume(TokenType::IDENTIFIER, "'ஆக' (as) பின் அடையாளங்காட்டியை (view name) எதிர்பார்க்கிறோம்");
        // TODO: Store view name in BorrowExpr if needed
    }
    
    auto expr = std::make_unique<BorrowExpr>(isMut, std::move(target));
    expr->loc = {borrowTok.filename, borrowTok.line, borrowTok.column};
    return expr;
}

std::unique_ptr<Expr> Parser::parseEscapeExpr() {
    Token escapeTok = previous();
    consume(TokenType::LPAREN, "'தப்பித்தல்' (escape) பின் '(' எதிர்பார்க்கிறோம்");
    auto target = parseExpression();
    consume(TokenType::RPAREN, "இலக்கிற்குப்பின் (target) ')' எதிர்பார்க்கிறோம்");
    
    consume(TokenType::ARROW, "தப்பிக்கும் இலக்கிற்குப்பின் '->' எதிர்பார்க்கிறோம்");
    Token destTok = consume(TokenType::IDENTIFIER, "இலக்கு மண்டலத்தின் (destination zone) பெயரை எதிர்பார்க்கிறோம்");
    
    auto expr = std::make_unique<EscapeExpr>(std::move(target), destTok.value);
    expr->loc = {escapeTok.filename, escapeTok.line, escapeTok.column};
    return expr;
}

std::unique_ptr<DeleteStmt> Parser::parseDeleteStmt() {
    Token delTok = consume(TokenType::KW_DELETE, "எதிர்பார்க்கிறோம் 'delete'");
    auto pointer = parseExpression();
    consume(TokenType::SEMICOLON, "எதிர்பார்க்கிறோம் ';' after delete statement");
    auto stmt = std::make_unique<DeleteStmt>(std::move(pointer));
    stmt->loc = {delTok.filename, delTok.line, delTok.column};
    return stmt;
}

std::unique_ptr<ForStmt> Parser::parseForStmt() {
    Token forTok = consume(TokenType::KW_FOR, "எதிர்பார்க்கிறோம் 'for'");
    consume(TokenType::LPAREN, "எதிர்பார்க்கிறோம் '(' after 'for'");
    Token varTok = consume(TokenType::IDENTIFIER, "எதிர்பார்க்கிறோம் loop variable name");
    consume(TokenType::KW_IN, "எதிர்பார்க்கிறோம் 'in' after loop variable");
    auto iterable = parseExpression();
    consume(TokenType::RPAREN, "எதிர்பார்க்கிறோம் ')' after for header");
    auto body = parseBlock();

    auto stmt = std::make_unique<ForStmt>(varTok.value, std::move(iterable), std::move(body));
    stmt->loc = {forTok.filename, forTok.line, forTok.column};
    return stmt;
}


std::unique_ptr<TraitDecl> Parser::parseTraitDecl() {
    Token nameTok = consume(TokenType::IDENTIFIER, "பண்பு (trait) பெயரை எதிர்பார்க்கிறோம்");
    consume(TokenType::LBRACE, "பண்பு உடற்பகுதிக்குமுன் '{' எதிர்பார்க்கிறோம்");
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        methods.push_back(parseFunctionDecl());
    }
    consume(TokenType::RBRACE, "பண்பு உடற்பகுதிக்குப்பின் '}' எதிர்பார்க்கிறோம்");
    auto td = std::make_unique<TraitDecl>(nameTok.value, std::move(methods));
    td->loc = {nameTok.filename, nameTok.line, nameTok.column};
    return td;
}

std::unique_ptr<ImplDecl> Parser::parseImplDecl() {
    SourceLocation loc = {tokens_[current_ - 1].filename, tokens_[current_ - 1].line, tokens_[current_ - 1].column};
    
    std::vector<std::string> typeParams;
    if (match(TokenType::LT)) {
        do {
            typeParams.push_back(consume(TokenType::IDENTIFIER, "வகை அளபுரு (type parameter) பெயரை எதிர்பார்க்கிறோம்").value);
        } while (match(TokenType::COMMA));
        consume(TokenType::GT, "வகை அளபுருக்களுக்குப்பின் '>' எதிர்பார்க்கிறோம்");
    }

    std::string traitName = "";

    // Check if it's `impl Trait for Type` or `impl Type`
    if (check(TokenType::IDENTIFIER) && (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].type == TokenType::KW_FOR) {
        traitName = consume(TokenType::IDENTIFIER, "பண்பு (trait) பெயரை எதிர்பார்க்கிறோம்").value;
        consume(TokenType::KW_FOR, "பண்பு பெயருக்குப்பின் 'சுற்று' (for) அல்லது 'உள்' (in) எதிர்பார்க்கிறோம்");
    }

    // Set currentTypeParams_ so that parseTypeSpec inside methods
    // recognizes the impl's generic params (e.g., வ) as generics, not structs.
    auto prevTypeParams = currentTypeParams_;
    currentTypeParams_ = typeParams;

    TypeInfo targetType = parseTypeSpec();
    consume(TokenType::LBRACE, "செயல்படுத்து (impl) உடற்பகுதிக்குமுன் '{' எதிர்பார்க்கிறோம்");
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        methods.push_back(parseFunctionDecl());
    }
    consume(TokenType::RBRACE, "செயல்படுத்து (impl) உடற்பகுதிக்குப்பின் '}' எதிர்பார்க்கிறோம்");
    currentTypeParams_ = prevTypeParams;
    auto id = std::make_unique<ImplDecl>(traitName, std::move(typeParams), targetType, std::move(methods));
    id->loc = loc;
    return id;
}

std::vector<std::string> Parser::parseTypeParams() {
    std::vector<std::string> params;
    do {
        params.push_back(consume(TokenType::IDENTIFIER, "வகை அளபுரு (type parameter) பெயரை எதிர்பார்க்கிறோம்").value);
    } while (match(TokenType::COMMA));
    consume(TokenType::GT, "வகை அளபுருக்களுக்குப்பின் '>' எதிர்பார்க்கிறோம்");
    return params;
}

std::vector<TypeInfo> Parser::parseTypeArgs() {
    std::vector<TypeInfo> args;
    do {
        args.push_back(parseTypeSpec());
    } while (match(TokenType::COMMA));
    consume(TokenType::GT, "வகை தருமங்களுக்குப்பின் (type arguments) '>' எதிர்பார்க்கிறோம்");
    return args;
}

} // namespace agam
