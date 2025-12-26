//! Recursive descent parser for Agam
//! 
//! Parses token stream into an Abstract Syntax Tree

use crate::lexer::{Token, TokenType};
use crate::parser::ast::*;
use crate::error::AgamError;

/// The parser
pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Parser { tokens, current: 0 }
    }

    /// Parse the token stream into a program
    pub fn parse(&mut self) -> Result<Program, AgamError> {
        let mut statements = Vec::new();

        while !self.is_at_end() {
            // Skip newlines between statements
            self.skip_newlines();
            
            if self.is_at_end() {
                break;
            }

            statements.push(self.declaration()?);
        }

        Ok(Program { statements })
    }

    fn declaration(&mut self) -> Result<Statement, AgamError> {
        if self.check(&TokenType::Seyal) {
            self.function_declaration()
        } else if self.check(&TokenType::Maari) || self.check(&TokenType::Maaraadha) {
            self.var_declaration()
        } else if self.check(&TokenType::Kattamaippu) {
            self.struct_declaration()
        } else if self.check(&TokenType::Viruppam) {
            self.enum_declaration()
        } else if self.check(&TokenType::Irakkumadhi) {
            self.import_statement()
        } else if self.check(&TokenType::Irundhu) {
            self.from_import_statement()
        } else {
            self.statement()
        }
    }

    fn function_declaration(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume செயல்

        let name = self.consume_identifier("செயல் பெயர் எதிர்பார்க்கப்படுகிறது")?;
        
        self.consume(&TokenType::LeftParen, "'(' எதிர்பார்க்கப்படுகிறது")?;
        
        let mut params = Vec::new();
        if !self.check(&TokenType::RightParen) {
            loop {
                params.push(self.consume_identifier("அளவுரு பெயர் எதிர்பார்க்கப்படுகிறது")?);
                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }
        
        self.consume(&TokenType::RightParen, "')' எதிர்பார்க்கப்படுகிறது")?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
        
        let body = self.block()?;

        Ok(Statement::Function { name, params, body })
    }

    fn var_declaration(&mut self) -> Result<Statement, AgamError> {
        let is_const = self.check(&TokenType::Maaraadha);
        self.advance(); // consume மாறி or மாறாத

        let name = self.consume_identifier("மாறி பெயர் எதிர்பார்க்கப்படுகிறது")?;
        
        self.consume(&TokenType::Equal, "'=' எதிர்பார்க்கப்படுகிறது")?;
        
        let value = self.expression()?;
        
        self.consume_newline_or_eof()?;

        Ok(Statement::Let { name, value, is_const })
    }

    fn statement(&mut self) -> Result<Statement, AgamError> {
        if self.check(&TokenType::Endraal) {
            self.if_statement()
        } else if self.check(&TokenType::Varai) {
            self.while_statement()
        } else if self.check(&TokenType::Ovvoru) {
            self.for_statement()
        } else if self.check(&TokenType::Thirumbu) {
            self.return_statement()
        } else if self.check(&TokenType::Niruthu) {
            self.break_statement()
        } else if self.check(&TokenType::Thodar) {
            self.continue_statement()
        } else if self.check(&TokenType::Achidu) {
            self.print_statement()
        } else if self.check(&TokenType::Poruthu) {
            self.match_statement()
        } else if self.check(&TokenType::Muyarchi) {
            self.try_catch_statement()
        } else if self.check(&TokenType::Veesu) {
            self.throw_statement()
        } else {
            self.expression_statement()
        }
    }

    fn if_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume என்றால்

        let condition = self.expression()?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
        
        let then_branch = self.block()?;

        let mut elif_branches = Vec::new();
        while self.check(&TokenType::Illayendraal) {
            self.advance();
            let elif_condition = self.expression()?;
            self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
            let elif_body = self.block()?;
            elif_branches.push((elif_condition, elif_body));
        }

        let else_branch = if self.check(&TokenType::Illai) {
            self.advance();
            self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
            Some(self.block()?)
        } else {
            None
        };

        Ok(Statement::If {
            condition,
            then_branch,
            elif_branches,
            else_branch,
        })
    }

    fn while_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume வரை

        let condition = self.expression()?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
        
        let body = self.block()?;

        Ok(Statement::While { condition, body })
    }

    fn for_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume ஒவ்வொரு

        let variable = self.consume_identifier("மாறி பெயர் எதிர்பார்க்கப்படுகிறது")?;
        
        self.consume(&TokenType::Ulla, "'உள்ள' எதிர்பார்க்கப்படுகிறது")?;
        
        let iterable = self.expression()?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
        
        let body = self.block()?;

        Ok(Statement::For {
            variable,
            iterable,
            body,
        })
    }

    fn return_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume திரும்பு

        let value = if self.check(&TokenType::Newline) || self.is_at_end() {
            None
        } else {
            Some(self.expression()?)
        };

        self.consume_newline_or_eof()?;

        Ok(Statement::Return(value))
    }

    fn break_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume நிறுத்து
        self.consume_newline_or_eof()?;
        Ok(Statement::Break)
    }

    fn continue_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume தொடர்
        self.consume_newline_or_eof()?;
        Ok(Statement::Continue)
    }

    fn print_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume அச்சிடு
        
        self.consume(&TokenType::LeftParen, "'(' எதிர்பார்க்கப்படுகிறது")?;
        
        let mut args = Vec::new();
        if !self.check(&TokenType::RightParen) {
            loop {
                args.push(self.expression()?);
                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }
        
        self.consume(&TokenType::RightParen, "')' எதிர்பார்க்கப்படுகிறது")?;
        self.consume_newline_or_eof()?;

        Ok(Statement::Print(args))
    }

    fn try_catch_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume முயற்சி
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;

        let try_block = self.block()?;

        // Expect catch block
        self.skip_newlines();
        self.consume(&TokenType::Pidi, "'பிடி' எதிர்பார்க்கப்படுகிறது")?;
        let error_var = self.consume_identifier("பிழை மாறி பெயர் எதிர்பார்க்கப்படுகிறது")?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;

        let catch_block = self.block()?;

        Ok(Statement::TryCatch {
            try_block,
            error_var,
            catch_block,
        })
    }

    fn throw_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume வீசு
        let expr = self.expression()?;
        self.consume_newline_or_eof()?;
        Ok(Statement::Throw(expr))
    }

    fn import_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume இறக்குமதி
        
        let module = self.consume_identifier("கூறு பெயர் எதிர்பார்க்கப்படுகிறது")?;
        self.consume_newline_or_eof()?;
        
        Ok(Statement::Import {
            module,
            items: None, // Import all items for now
        })
    }

    // Selective import: இருந்து module இறக்குமதி func1, func2
    #[allow(clippy::wrong_self_convention)]
    fn from_import_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume இருந்து
        
        let module = self.consume_identifier("கூறு பெயர் எதிர்பார்க்கப்படுகிறது")?;
        
        self.consume(&TokenType::Irakkumadhi, "'இறக்குமதி' எதிர்பார்க்கப்படுகிறது")?;
        
        // Parse list of items to import
        let mut items = vec![];
        items.push(self.consume_identifier("இறக்குமதி பொருள் பெயர் எதிர்பார்க்கப்படுகிறது")?);
        
        while self.match_token(&[TokenType::Comma]) {
            items.push(self.consume_identifier("இறக்குமதி பொருள் பெயர் எதிர்பார்க்கப்படுகிறது")?);
        }
        
        self.consume_newline_or_eof()?;
        
        Ok(Statement::Import {
            module,
            items: Some(items),
        })
    }

    fn struct_declaration(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume கட்டமைப்பு

        let name = self.consume_identifier("கட்டமைப்பு பெயர் எதிர்பார்க்கப்படுகிறது")?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;

        // Parse fields in block
        self.skip_newlines();
        self.consume(&TokenType::Indent, "உள்தள்ளுதல் எதிர்பார்க்கப்படுகிறது")?;

        let mut fields = Vec::new();
        while !self.check(&TokenType::Dedent) && !self.is_at_end() {
            self.skip_newlines();
            if self.check(&TokenType::Dedent) || self.is_at_end() {
                break;
            }

            let field_name = self.consume_identifier("புலம் பெயர் எதிர்பார்க்கப்படுகிறது")?;
            let field_type = if self.match_token(&[TokenType::Colon]) {
                Some(self.consume_identifier("வகை எதிர்பார்க்கப்படுகிறது")?)
            } else {
                None
            };
            fields.push((field_name, field_type));
            self.consume_newline_or_eof()?;
        }

        if self.check(&TokenType::Dedent) {
            self.advance();
        }

        Ok(Statement::Struct { name, fields })
    }

    fn enum_declaration(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume விருப்பம்

        let name = self.consume_identifier("விருப்பம் பெயர் எதிர்பார்க்கப்படுகிறது")?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;

        // Parse variants in block
        self.skip_newlines();
        self.consume(&TokenType::Indent, "உள்தள்ளுதல் எதிர்பார்க்கப்படுகிறது")?;

        let mut variants = Vec::new();
        while !self.check(&TokenType::Dedent) && !self.is_at_end() {
            self.skip_newlines();
            if self.check(&TokenType::Dedent) || self.is_at_end() {
                break;
            }

            let variant = self.consume_identifier("மாறுபாடு பெயர் எதிர்பார்க்கப்படுகிறது")?;
            variants.push(variant);
            self.consume_newline_or_eof()?;
        }

        if self.check(&TokenType::Dedent) {
            self.advance();
        }

        Ok(Statement::Enum { name, variants })
    }

    fn match_statement(&mut self) -> Result<Statement, AgamError> {
        self.advance(); // consume பொருத்து

        let value = self.expression()?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;

        // Parse arms in block
        self.skip_newlines();
        self.consume(&TokenType::Indent, "உள்தள்ளுதல் எதிர்பார்க்கப்படுகிறது")?;

        let mut arms = Vec::new();
        while !self.check(&TokenType::Dedent) && !self.is_at_end() {
            self.skip_newlines();
            if self.check(&TokenType::Dedent) || self.is_at_end() {
                break;
            }

            // Parse pattern
            let pattern = self.parse_pattern()?;
            self.consume(&TokenType::Arrow, "'=>' எதிர்பார்க்கப்படுகிறது")?;

            // Parse body (single statement or block)
            let body = if self.check(&TokenType::Newline) {
                self.advance();
                self.block()?
            } else {
                vec![self.statement()?]
            };

            arms.push(MatchArm { pattern, body });
        }

        if self.check(&TokenType::Dedent) {
            self.advance();
        }

        Ok(Statement::Match { value, arms })
    }

    fn parse_pattern(&mut self) -> Result<Pattern, AgamError> {
        // Wildcard pattern
        if self.match_token(&[TokenType::Underscore]) {
            return Ok(Pattern::Wildcard);
        }

        // Number literal pattern
        if let Some(TokenType::Number(n)) = self.peek_token_type() {
            let n = *n;
            self.advance();
            return Ok(Pattern::Literal(Expression::Number(n)));
        }

        // String literal pattern
        if let Some(TokenType::String(s)) = self.peek_token_type() {
            let s = s.clone();
            self.advance();
            return Ok(Pattern::Literal(Expression::String(s)));
        }

        // Boolean patterns
        if self.match_token(&[TokenType::Unmai]) {
            return Ok(Pattern::Literal(Expression::Boolean(true)));
        }
        if self.match_token(&[TokenType::Poi]) {
            return Ok(Pattern::Literal(Expression::Boolean(false)));
        }

        // Identifier (could be variable binding or enum variant)
        if let Some(TokenType::Identifier(name)) = self.peek_token_type() {
            let name = name.clone();
            self.advance();

            // Check for enum variant pattern (EnumName.Variant)
            if self.match_token(&[TokenType::Dot]) {
                let variant = self.consume_identifier("மாறுபாடு பெயர் எதிர்பார்க்கப்படுகிறது")?;
                return Ok(Pattern::EnumVariant(name, variant));
            }

            // Variable binding pattern
            return Ok(Pattern::Variable(name));
        }

        Err(self.error("வடிவம் எதிர்பார்க்கப்படுகிறது"))
    }

    fn expression_statement(&mut self) -> Result<Statement, AgamError> {
        let expr = self.expression()?;
        self.consume_newline_or_eof()?;
        Ok(Statement::Expression(expr))
    }

    fn block(&mut self) -> Result<Vec<Statement>, AgamError> {
        self.skip_newlines();
        self.consume(&TokenType::Indent, "உள்தள்ளுதல் எதிர்பார்க்கப்படுகிறது")?;
        
        let mut statements = Vec::new();
        
        while !self.check(&TokenType::Dedent) && !self.is_at_end() {
            self.skip_newlines();
            if self.check(&TokenType::Dedent) || self.is_at_end() {
                break;
            }
            statements.push(self.declaration()?);
        }
        
        if self.check(&TokenType::Dedent) {
            self.advance();
        }

        Ok(statements)
    }

    // Expression parsing with precedence climbing
    fn expression(&mut self) -> Result<Expression, AgamError> {
        self.assignment()
    }

    fn assignment(&mut self) -> Result<Expression, AgamError> {
        let expr = self.or()?;

        if self.match_token(&[TokenType::Equal]) {
            let value = self.assignment()?;
            
            // Simple identifier assignment
            if let Expression::Identifier(name) = expr {
                return Ok(Expression::Assignment {
                    name,
                    value: Box::new(value),
                });
            }
            
            // Index assignment: list[0] = value, dict["key"] = value
            if let Expression::Index { object, index } = expr {
                return Ok(Expression::IndexAssignment {
                    object,
                    index,
                    value: Box::new(value),
                });
            }
            
            // Member assignment: struct.field = value
            if let Expression::MemberAccess { object, member } = expr {
                return Ok(Expression::MemberAssignment {
                    object,
                    member,
                    value: Box::new(value),
                });
            }
            
            return Err(self.error("தவறான ஒதுக்கீடு இலக்கு"));
        }

        Ok(expr)
    }

    fn or(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.and()?;

        while self.match_token(&[TokenType::Alladhu]) {
            let right = self.and()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator: BinaryOp::Or,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn and(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.equality()?;

        while self.match_token(&[TokenType::Matrum]) {
            let right = self.equality()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator: BinaryOp::And,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn equality(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.comparison()?;

        while self.match_token(&[TokenType::EqualEqual, TokenType::NotEqual]) {
            let operator = match self.previous().token_type {
                TokenType::EqualEqual => BinaryOp::Equal,
                TokenType::NotEqual => BinaryOp::NotEqual,
                _ => unreachable!(),
            };
            let right = self.comparison()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn comparison(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.term()?;

        while self.match_token(&[
            TokenType::Less,
            TokenType::Greater,
            TokenType::LessEqual,
            TokenType::GreaterEqual,
        ]) {
            let operator = match self.previous().token_type {
                TokenType::Less => BinaryOp::Less,
                TokenType::Greater => BinaryOp::Greater,
                TokenType::LessEqual => BinaryOp::LessEqual,
                TokenType::GreaterEqual => BinaryOp::GreaterEqual,
                _ => unreachable!(),
            };
            let right = self.term()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn term(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.factor()?;

        while self.match_token(&[TokenType::Plus, TokenType::Minus]) {
            let operator = match self.previous().token_type {
                TokenType::Plus => BinaryOp::Add,
                TokenType::Minus => BinaryOp::Subtract,
                _ => unreachable!(),
            };
            let right = self.factor()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn factor(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.unary()?;

        while self.match_token(&[TokenType::Star, TokenType::Slash, TokenType::Percent]) {
            let operator = match self.previous().token_type {
                TokenType::Star => BinaryOp::Multiply,
                TokenType::Slash => BinaryOp::Divide,
                TokenType::Percent => BinaryOp::Modulo,
                _ => unreachable!(),
            };
            let right = self.unary()?;
            expr = Expression::Binary {
                left: Box::new(expr),
                operator,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn unary(&mut self) -> Result<Expression, AgamError> {
        if self.match_token(&[TokenType::Minus]) {
            let operand = self.unary()?;
            return Ok(Expression::Unary {
                operator: UnaryOp::Negate,
                operand: Box::new(operand),
            });
        }

        if self.match_token(&[TokenType::Illamal]) {
            let operand = self.unary()?;
            return Ok(Expression::Unary {
                operator: UnaryOp::Not,
                operand: Box::new(operand),
            });
        }

        self.call()
    }

    fn call(&mut self) -> Result<Expression, AgamError> {
        let mut expr = self.primary()?;

        loop {
            if self.match_token(&[TokenType::LeftParen]) {
                expr = self.finish_call(expr)?;
            } else if self.match_token(&[TokenType::LeftBracket]) {
                let index = self.expression()?;
                self.consume(&TokenType::RightBracket, "']' எதிர்பார்க்கப்படுகிறது")?;
                expr = Expression::Index {
                    object: Box::new(expr),
                    index: Box::new(index),
                };
            } else if self.match_token(&[TokenType::Dot]) {
                let member = self.consume_identifier("புலம் பெயர் எதிர்பார்க்கப்படுகிறது")?;
                expr = Expression::MemberAccess {
                    object: Box::new(expr),
                    member,
                };
            } else {
                break;
            }
        }

        Ok(expr)
    }

    fn finish_call(&mut self, callee: Expression) -> Result<Expression, AgamError> {
        let mut arguments = Vec::new();

        if !self.check(&TokenType::RightParen) {
            loop {
                arguments.push(self.expression()?);
                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }

        self.consume(&TokenType::RightParen, "')' எதிர்பார்க்கப்படுகிறது")?;

        Ok(Expression::Call {
            callee: Box::new(callee),
            arguments,
        })
    }

    fn primary(&mut self) -> Result<Expression, AgamError> {
        // Boolean true
        if self.match_token(&[TokenType::Unmai]) {
            return Ok(Expression::Boolean(true));
        }

        // Boolean false
        if self.match_token(&[TokenType::Poi]) {
            return Ok(Expression::Boolean(false));
        }

        // Null
        if self.match_token(&[TokenType::Illa]) {
            return Ok(Expression::Null);
        }

        // Number
        if let Some(TokenType::Number(n)) = self.peek_token_type() {
            let n = *n;
            self.advance();
            return Ok(Expression::Number(n));
        }

        // String
        if let Some(TokenType::String(s)) = self.peek_token_type() {
            let s = s.clone();
            self.advance();
            return Ok(Expression::String(s));
        }

        // F-String (interpolated string): f"Hello {name}!"
        if let Some(TokenType::FString(content)) = self.peek_token_type() {
            let content = content.clone();
            self.advance();
            return self.parse_fstring_parts(&content);
        }

        // Identifier
        if let Some(TokenType::Identifier(name)) = self.peek_token_type() {
            let name = name.clone();
            self.advance();
            return Ok(Expression::Identifier(name));
        }

        // Input function (உள்ளீடு / input) - treat keyword as callable identifier
        if self.match_token(&[TokenType::Ulleedu]) {
            return Ok(Expression::Identifier("உள்ளீடு".to_string()));
        }

        // Lambda expression: செயலி(params): expression or lambda(params): expression
        if self.match_token(&[TokenType::Seyali]) {
            return self.lambda_expression();
        }

        // Grouping or arrow lambda: (expr) or (params) => expr
        if self.match_token(&[TokenType::LeftParen]) {
            // Check if this might be an arrow lambda
            // Try to parse as parameters first, then check for =>
            let start = self.current;
            
            // Try parsing as parameter list
            let mut params = Vec::new();
            let mut might_be_lambda = true;
            
            if !self.check(&TokenType::RightParen) {
                loop {
                    if let Some(TokenType::Identifier(name)) = self.peek_token_type() {
                        params.push(name.clone());
                        self.advance();
                    } else {
                        // Not a valid parameter, this is a grouping
                        might_be_lambda = false;
                        break;
                    }
                    
                    if !self.match_token(&[TokenType::Comma]) {
                        break;
                    }
                }
            }
            
            // Check for ) =>
            if might_be_lambda && self.check(&TokenType::RightParen) {
                self.advance(); // consume )
                
                if self.check(&TokenType::Arrow) {
                    self.advance(); // consume =>
                    let body = self.expression()?;
                    return Ok(Expression::Lambda {
                        params,
                        body: Box::new(body),
                    });
                }
            }
            
            // Not a lambda, backtrack and parse as grouping
            self.current = start;
            let expr = self.expression()?;
            self.consume(&TokenType::RightParen, "')' எதிர்பார்க்கப்படுகிறது")?;
            return Ok(Expression::Grouping(Box::new(expr)));
        }

        // List literal
        if self.match_token(&[TokenType::LeftBracket]) {
            let mut elements = Vec::new();
            if !self.check(&TokenType::RightBracket) {
                loop {
                    elements.push(self.expression()?);
                    if !self.match_token(&[TokenType::Comma]) {
                        break;
                    }
                }
            }
            self.consume(&TokenType::RightBracket, "']' எதிர்பார்க்கப்படுகிறது")?;
            return Ok(Expression::List(elements));
        }

        // Dictionary literal
        if self.match_token(&[TokenType::LeftBrace]) {
            let mut pairs = Vec::new();
            if !self.check(&TokenType::RightBrace) {
                loop {
                    let key = self.expression()?;
                    self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
                    let value = self.expression()?;
                    pairs.push((key, value));
                    if !self.match_token(&[TokenType::Comma]) {
                        break;
                    }
                }
            }
            self.consume(&TokenType::RightBrace, "'}' எதிர்பார்க்கப்படுகிறது")?;
            return Ok(Expression::Dict(pairs));
        }

        Err(self.error("வெளிப்பாடு எதிர்பார்க்கப்படுகிறது"))
    }

    /// Parse lambda expression: செயலி(params): expression
    fn lambda_expression(&mut self) -> Result<Expression, AgamError> {
        self.consume(&TokenType::LeftParen, "'(' எதிர்பார்க்கப்படுகிறது")?;
        
        let mut params = Vec::new();
        if !self.check(&TokenType::RightParen) {
            loop {
                params.push(self.consume_identifier("அளவுரு பெயர் எதிர்பார்க்கப்படுகிறது")?);
                if !self.match_token(&[TokenType::Comma]) {
                    break;
                }
            }
        }
        
        self.consume(&TokenType::RightParen, "')' எதிர்பார்க்கப்படுகிறது")?;
        self.consume(&TokenType::Colon, "':' எதிர்பார்க்கப்படுகிறது")?;
        
        let body = self.expression()?;
        
        Ok(Expression::Lambda {
            params,
            body: Box::new(body),
        })
    }

    /// Parse f-string content into parts
    /// Content like "Hello {name}! You are {age} years old" becomes:
    /// [Literal("Hello "), Expression(name), Literal("! You are "), Expression(age), Literal(" years old")]
    #[allow(clippy::while_let_on_iterator)]
    fn parse_fstring_parts(&mut self, content: &str) -> Result<Expression, AgamError> {
        let mut parts = Vec::new();
        let mut chars = content.chars().peekable();
        let mut current_literal = String::new();
        
        while let Some(c) = chars.next() {
            if c == '{' {
                // Check for escaped brace {{
                if chars.peek() == Some(&'{') {
                    chars.next();
                    current_literal.push('{');
                    continue;
                }
                
                // Save any accumulated literal
                if !current_literal.is_empty() {
                    parts.push(FStringPart::Literal(current_literal.clone()));
                    current_literal.clear();
                }
                
                // Extract expression until }
                let mut expr_str = String::new();
                let mut brace_depth = 1;
                
                while let Some(c) = chars.next() {
                    if c == '{' {
                        brace_depth += 1;
                        expr_str.push(c);
                    } else if c == '}' {
                        brace_depth -= 1;
                        if brace_depth == 0 {
                            break;
                        }
                        expr_str.push(c);
                    } else {
                        expr_str.push(c);
                    }
                }
                
                if brace_depth != 0 {
                    return Err(self.error("f-சரத்தில் '}' எதிர்பார்க்கப்படுகிறது"));
                }
                
                // Parse the expression
                let expr = self.parse_embedded_expression(&expr_str)?;
                parts.push(FStringPart::Expression(Box::new(expr)));
            } else if c == '}' {
                // Check for escaped brace }}
                if chars.peek() == Some(&'}') {
                    chars.next();
                    current_literal.push('}');
                    continue;
                }
                // Single } without matching { is an error
                return Err(self.error("f-சரத்தில் எதிர்பாராத '}'"));
            } else {
                current_literal.push(c);
            }
        }
        
        // Add any remaining literal
        if !current_literal.is_empty() {
            parts.push(FStringPart::Literal(current_literal));
        }
        
        Ok(Expression::FString { parts })
    }
    
    /// Parse an embedded expression within an f-string
    fn parse_embedded_expression(&mut self, expr_str: &str) -> Result<Expression, AgamError> {
        // Tokenize the expression
        let tokens = crate::lexer::Lexer::tokenize(expr_str).map_err(|e| {
            self.error(&format!("f-சரத்தில் பிழை: {}", e))
        })?;
        
        // Parse the expression
        let mut sub_parser = Parser::new(tokens);
        sub_parser.expression()
    }

    // Helper methods
    fn peek_token_type(&self) -> Option<&TokenType> {
        self.tokens.get(self.current).map(|t| &t.token_type)
    }

    fn check(&self, token_type: &TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }
        std::mem::discriminant(&self.tokens[self.current].token_type) 
            == std::mem::discriminant(token_type)
    }

    fn match_token(&mut self, types: &[TokenType]) -> bool {
        for t in types {
            if self.check(t) {
                self.advance();
                return true;
            }
        }
        false
    }

    fn advance(&mut self) -> &Token {
        if !self.is_at_end() {
            self.current += 1;
        }
        self.previous()
    }

    fn previous(&self) -> &Token {
        &self.tokens[self.current - 1]
    }

    fn is_at_end(&self) -> bool {
        matches!(self.tokens.get(self.current).map(|t| &t.token_type), Some(TokenType::Eof))
    }

    fn consume(&mut self, token_type: &TokenType, message: &str) -> Result<&Token, AgamError> {
        if self.check(token_type) {
            return Ok(self.advance());
        }
        Err(self.error(message))
    }

    fn consume_identifier(&mut self, message: &str) -> Result<String, AgamError> {
        if let Some(TokenType::Identifier(name)) = self.peek_token_type() {
            let name = name.clone();
            self.advance();
            return Ok(name);
        }
        Err(self.error(message))
    }

    fn consume_newline_or_eof(&mut self) -> Result<(), AgamError> {
        if self.is_at_end() || self.check(&TokenType::Dedent) {
            return Ok(());
        }
        if self.check(&TokenType::Newline) {
            self.advance();
            return Ok(());
        }
        Err(self.error("புதிய வரி எதிர்பார்க்கப்படுகிறது"))
    }

    fn skip_newlines(&mut self) {
        while self.check(&TokenType::Newline) {
            self.advance();
        }
    }

    fn error(&self, message: &str) -> AgamError {
        let token = self.tokens.get(self.current).unwrap_or(&self.tokens[self.tokens.len() - 1]);
        AgamError::parser_error(token.line, token.column, message.to_string())
    }
}
