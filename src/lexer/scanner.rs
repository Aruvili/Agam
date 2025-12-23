//! Lexical scanner for Agam
//! 
//! Converts source code into a stream of tokens, handling Tamil Unicode characters

use crate::lexer::token::{Token, TokenType};
use crate::error::AgamError;
use std::collections::HashMap;
use std::iter::Peekable;
use std::str::Chars;

/// The lexical scanner
pub struct Scanner<'a> {
    #[allow(dead_code)]
    source: &'a str,
    chars: Peekable<Chars<'a>>,
    tokens: Vec<Token>,
    line: usize,
    column: usize,
    start_column: usize,
    indent_stack: Vec<usize>,
    at_line_start: bool,
    keywords: HashMap<&'static str, TokenType>,
}

impl<'a> Scanner<'a> {
    pub fn new(source: &'a str) -> Self {
        let mut keywords = HashMap::new();
        
        // Tamil keywords
        keywords.insert("செயல்", TokenType::Seyal);
        keywords.insert("மாறி", TokenType::Maari);
        keywords.insert("மாறாத", TokenType::Maaraadha);
        keywords.insert("என்றால்", TokenType::Endraal);
        keywords.insert("இல்லையென்றால்", TokenType::Illayendraal);
        keywords.insert("இல்லை", TokenType::Illai);
        keywords.insert("வரை", TokenType::Varai);
        keywords.insert("ஒவ்வொரு", TokenType::Ovvoru);
        keywords.insert("உள்ள", TokenType::Ulla);
        keywords.insert("திரும்பு", TokenType::Thirumbu);
        keywords.insert("உண்மை", TokenType::Unmai);
        keywords.insert("பொய்", TokenType::Poi);
        keywords.insert("இல்லா", TokenType::Illa);
        keywords.insert("நிறுத்து", TokenType::Niruthu);
        keywords.insert("தொடர்", TokenType::Thodar);
        keywords.insert("மற்றும்", TokenType::Matrum);
        keywords.insert("அல்லது", TokenType::Alladhu);
        keywords.insert("இல்ல", TokenType::Illamal);
        
        // New keywords for modules and error handling
        keywords.insert("இறக்குமதி", TokenType::Irakkumadhi);
        keywords.insert("இருந்து", TokenType::Irundhu);
        keywords.insert("முயற்சி", TokenType::Muyarchi);
        keywords.insert("பிடி", TokenType::Pidi);
        keywords.insert("வீசு", TokenType::Veesu);
        
        // Built-in functions - only print and input need special keyword handling
        // நீளம்/len, வகை/type are handled as regular built-in functions
        keywords.insert("அச்சிடு", TokenType::Achidu);
        keywords.insert("உள்ளீடு", TokenType::Ulleedu);
        
        // English equivalents for bilingual support
        keywords.insert("fn", TokenType::Seyal);
        keywords.insert("let", TokenType::Maari);
        keywords.insert("const", TokenType::Maaraadha);
        keywords.insert("if", TokenType::Endraal);
        keywords.insert("elif", TokenType::Illayendraal);
        keywords.insert("else", TokenType::Illai);
        keywords.insert("while", TokenType::Varai);
        keywords.insert("for", TokenType::Ovvoru);
        keywords.insert("in", TokenType::Ulla);
        keywords.insert("return", TokenType::Thirumbu);
        keywords.insert("true", TokenType::Unmai);
        keywords.insert("false", TokenType::Poi);
        keywords.insert("null", TokenType::Illa);
        keywords.insert("break", TokenType::Niruthu);
        keywords.insert("continue", TokenType::Thodar);
        keywords.insert("and", TokenType::Matrum);
        keywords.insert("or", TokenType::Alladhu);
        keywords.insert("not", TokenType::Illamal);
        keywords.insert("print", TokenType::Achidu);
        keywords.insert("input", TokenType::Ulleedu);
        
        // English equivalents for new keywords
        keywords.insert("import", TokenType::Irakkumadhi);
        keywords.insert("from", TokenType::Irundhu);
        keywords.insert("try", TokenType::Muyarchi);
        keywords.insert("catch", TokenType::Pidi);
        keywords.insert("throw", TokenType::Veesu);
        
        // New keywords for structs, enums, pattern matching
        keywords.insert("கட்டமைப்பு", TokenType::Kattamaippu);
        keywords.insert("struct", TokenType::Kattamaippu);
        keywords.insert("விருப்பம்", TokenType::Viruppam);
        keywords.insert("enum", TokenType::Viruppam);
        keywords.insert("பொருத்து", TokenType::Poruthu);
        keywords.insert("match", TokenType::Poruthu);
        keywords.insert("_", TokenType::Underscore);

        Scanner {
            source,
            chars: source.chars().peekable(),
            tokens: Vec::new(),
            line: 1,
            column: 1,
            start_column: 1,
            indent_stack: vec![0],
            at_line_start: true,
            keywords,
        }
    }

    /// Scan all tokens from source
    pub fn scan_tokens(&mut self) -> Result<Vec<Token>, AgamError> {
        while !self.is_at_end() {
            self.start_column = self.column;
            self.scan_token()?;
        }

        // Close any remaining indents
        while self.indent_stack.len() > 1 {
            self.indent_stack.pop();
            self.add_token(TokenType::Dedent, "".to_string());
        }

        self.tokens.push(Token::new(
            TokenType::Eof,
            "".to_string(),
            self.line,
            self.column,
        ));

        Ok(self.tokens.clone())
    }

    fn scan_token(&mut self) -> Result<(), AgamError> {
        // Handle indentation at line start
        if self.at_line_start {
            self.handle_indentation()?;
            self.at_line_start = false;
        }

        let c = match self.advance() {
            Some(c) => c,
            None => return Ok(()),
        };

        match c {
            // Single character tokens
            '(' => self.add_token(TokenType::LeftParen, c.to_string()),
            ')' => self.add_token(TokenType::RightParen, c.to_string()),
            '[' => self.add_token(TokenType::LeftBracket, c.to_string()),
            ']' => self.add_token(TokenType::RightBracket, c.to_string()),
            '{' => self.add_token(TokenType::LeftBrace, c.to_string()),
            '}' => self.add_token(TokenType::RightBrace, c.to_string()),
            ',' => self.add_token(TokenType::Comma, c.to_string()),
            ':' => self.add_token(TokenType::Colon, c.to_string()),
            '.' => self.add_token(TokenType::Dot, c.to_string()),
            '+' => self.add_token(TokenType::Plus, c.to_string()),
            '-' => self.add_token(TokenType::Minus, c.to_string()),
            '*' => self.add_token(TokenType::Star, c.to_string()),
            '/' => self.add_token(TokenType::Slash, c.to_string()),
            '%' => self.add_token(TokenType::Percent, c.to_string()),

            // Two-character tokens
            '=' => {
                if self.match_char('=') {
                    self.add_token(TokenType::EqualEqual, "==".to_string());
                } else if self.match_char('>') {
                    self.add_token(TokenType::Arrow, "=>".to_string());
                } else {
                    self.add_token(TokenType::Equal, "=".to_string());
                }
            }
            '!' => {
                if self.match_char('=') {
                    self.add_token(TokenType::NotEqual, "!=".to_string());
                } else {
                    return Err(AgamError::lexer_error(
                        self.line,
                        self.column,
                        "எதிர்பாராத எழுத்து '!'".to_string(),
                    ));
                }
            }
            '<' => {
                if self.match_char('=') {
                    self.add_token(TokenType::LessEqual, "<=".to_string());
                } else {
                    self.add_token(TokenType::Less, "<".to_string());
                }
            }
            '>' => {
                if self.match_char('=') {
                    self.add_token(TokenType::GreaterEqual, ">=".to_string());
                } else {
                    self.add_token(TokenType::Greater, ">".to_string());
                }
            }

            // Comments
            '#' => {
                while self.peek() != Some('\n') && !self.is_at_end() {
                    self.advance();
                }
            }

            // Whitespace
            ' ' | '\t' | '\r' => {}

            // Newline
            '\n' => {
                self.add_token(TokenType::Newline, "\\n".to_string());
                self.line += 1;
                self.column = 1;
                self.at_line_start = true;
            }

            // String literals
            '"' => self.string()?,

            // Numbers
            c if c.is_ascii_digit() => self.number(c)?,

            // Tamil numerals
            c if is_tamil_numeral(c) => self.tamil_number(c)?,

            // Identifiers and keywords
            c if is_identifier_start(c) => self.identifier(c)?,

            _ => {
                return Err(AgamError::lexer_error(
                    self.line,
                    self.column,
                    format!("எதிர்பாராத எழுத்து '{}'", c),
                ));
            }
        }

        Ok(())
    }

    fn handle_indentation(&mut self) -> Result<(), AgamError> {
        let mut indent = 0;
        
        while let Some(&c) = self.chars.peek() {
            match c {
                ' ' => {
                    indent += 1;
                    self.advance();
                }
                '\t' => {
                    indent += 4; // Treat tab as 4 spaces
                    self.advance();
                }
                '\n' => {
                    // Empty line, skip
                    self.advance();
                    self.line += 1;
                    self.column = 1;
                    indent = 0;
                }
                '#' => {
                    // Comment line, skip to end
                    while self.peek() != Some('\n') && !self.is_at_end() {
                        self.advance();
                    }
                    return Ok(());
                }
                _ => break,
            }
        }

        if self.is_at_end() {
            return Ok(());
        }

        let current_indent = *self.indent_stack.last().unwrap_or(&0);

        if indent > current_indent {
            self.indent_stack.push(indent);
            self.add_token(TokenType::Indent, "".to_string());
        } else if indent < current_indent {
            while self.indent_stack.len() > 1 && *self.indent_stack.last().unwrap() > indent {
                self.indent_stack.pop();
                self.add_token(TokenType::Dedent, "".to_string());
            }
        }

        Ok(())
    }

    fn string(&mut self) -> Result<(), AgamError> {
        let mut value = String::new();
        
        while let Some(&c) = self.chars.peek() {
            if c == '"' {
                break;
            }
            if c == '\n' {
                self.line += 1;
                self.column = 0;
            }
            
            // Handle escape sequences
            if c == '\\' {
                self.advance();
                if let Some(escaped) = self.advance() {
                    match escaped {
                        'n' => value.push('\n'),
                        't' => value.push('\t'),
                        'r' => value.push('\r'),
                        '"' => value.push('"'),
                        '\\' => value.push('\\'),
                        _ => value.push(escaped),
                    }
                }
            } else {
                value.push(c);
                self.advance();
            }
        }

        if self.is_at_end() {
            return Err(AgamError::lexer_error(
                self.line,
                self.start_column,
                "முடிவுறாத சரம் (Unterminated string)".to_string(),
            ));
        }

        // Consume closing quote
        self.advance();
        
        self.add_token(TokenType::String(value.clone()), format!("\"{}\"", value));
        Ok(())
    }

    fn number(&mut self, first: char) -> Result<(), AgamError> {
        let mut value = String::from(first);

        while let Some(&c) = self.chars.peek() {
            if c.is_ascii_digit() {
                value.push(c);
                self.advance();
            } else if c == '.' {
                // Check for decimal
                value.push(c);
                self.advance();
                while let Some(&c) = self.chars.peek() {
                    if c.is_ascii_digit() {
                        value.push(c);
                        self.advance();
                    } else {
                        break;
                    }
                }
                break;
            } else {
                break;
            }
        }

        let num: f64 = value.parse().map_err(|_| {
            AgamError::lexer_error(
                self.line,
                self.start_column,
                format!("தவறான எண் '{}'", value),
            )
        })?;

        self.add_token(TokenType::Number(num), value);
        Ok(())
    }

    fn tamil_number(&mut self, first: char) -> Result<(), AgamError> {
        let mut value = String::from(first);

        while let Some(&c) = self.chars.peek() {
            if is_tamil_numeral(c) {
                value.push(c);
                self.advance();
            } else {
                break;
            }
        }

        let num = tamil_to_number(&value)?;
        self.add_token(TokenType::Number(num), value);
        Ok(())
    }

    fn identifier(&mut self, first: char) -> Result<(), AgamError> {
        let mut value = String::from(first);

        while let Some(&c) = self.chars.peek() {
            if is_identifier_continue(c) {
                value.push(c);
                self.advance();
            } else {
                break;
            }
        }

        // Check if it's a keyword
        let token_type = self
            .keywords
            .get(value.as_str())
            .cloned()
            .unwrap_or_else(|| TokenType::Identifier(value.clone()));

        self.add_token(token_type, value);
        Ok(())
    }

    fn advance(&mut self) -> Option<char> {
        let c = self.chars.next();
        if c.is_some() {
            self.column += 1;
        }
        c
    }

    fn peek(&mut self) -> Option<char> {
        self.chars.peek().copied()
    }

    fn match_char(&mut self, expected: char) -> bool {
        if self.chars.peek() == Some(&expected) {
            self.advance();
            true
        } else {
            false
        }
    }

    fn is_at_end(&mut self) -> bool {
        self.chars.peek().is_none()
    }

    fn add_token(&mut self, token_type: TokenType, lexeme: String) {
        self.tokens.push(Token::new(
            token_type,
            lexeme,
            self.line,
            self.start_column,
        ));
    }
}

/// Check if character is a valid identifier start (Tamil letter or ASCII letter or underscore)
fn is_identifier_start(c: char) -> bool {
    c.is_alphabetic() || c == '_' || is_tamil_letter(c)
}

/// Check if character can continue an identifier
fn is_identifier_continue(c: char) -> bool {
    is_identifier_start(c) || c.is_ascii_digit() || is_tamil_numeral(c)
}

/// Check if character is a Tamil letter (Unicode range)
fn is_tamil_letter(c: char) -> bool {
    let code = c as u32;
    // Tamil Unicode block: U+0B80 to U+0BFF
    (0x0B80..=0x0BFF).contains(&code) && !is_tamil_numeral(c)
}

/// Check if character is a Tamil numeral (௦-௯)
fn is_tamil_numeral(c: char) -> bool {
    let code = c as u32;
    (0x0BE6..=0x0BEF).contains(&code) // Tamil digits ௦-௯
}

/// Convert Tamil numeral string to f64
fn tamil_to_number(s: &str) -> Result<f64, AgamError> {
    let mut result = 0.0;
    for c in s.chars() {
        let digit = match c {
            '௦' => 0.0,
            '௧' => 1.0,
            '௨' => 2.0,
            '௩' => 3.0,
            '௪' => 4.0,
            '௫' => 5.0,
            '௬' => 6.0,
            '௭' => 7.0,
            '௮' => 8.0,
            '௯' => 9.0,
            _ => {
                return Err(AgamError::lexer_error(
                    0,
                    0,
                    format!("தவறான தமிழ் எண் '{}'", c),
                ))
            }
        };
        result = result * 10.0 + digit;
    }
    Ok(result)
}
