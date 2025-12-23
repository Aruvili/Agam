//! Token definitions for the Agam lexer
//! 
//! Contains all Tamil keywords, operators, and token types

use std::fmt;

/// Token types for the Agam language
#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    // Keywords (முக்கிய சொற்கள்)
    Seyal,           // செயல் - function
    Maari,           // மாறி - let (variable)
    Maaraadha,       // மாறாத - const
    Endraal,         // என்றால் - if
    Illayendraal,    // இல்லையென்றால் - else if
    Illai,           // இல்லை - else
    Varai,           // வரை - while
    Ovvoru,          // ஒவ்வொரு - for
    Ulla,            // உள்ள - in
    Thirumbu,        // திரும்பு - return
    Unmai,           // உண்மை - true
    Poi,             // பொய் - false
    Illa,            // இல்லா - null/none
    Niruthu,         // நிறுத்து - break
    Thodar,          // தொடர் - continue
    Matrum,          // மற்றும் - and
    Alladhu,         // அல்லது - or
    Illamal,         // இல்ல - not
    
    // New keywords for modules and error handling
    Irakkumadhi,     // இறக்குமதி - import
    Irundhu,         // இருந்து - from
    Muyarchi,        // முயற்சி - try
    Pidi,            // பிடி - catch
    Veesu,           // வீசு - throw
    
    // New keywords for structs, enums, pattern matching
    Kattamaippu,     // கட்டமைப்பு - struct
    Viruppam,        // விருப்பம் - enum
    Poruthu,         // பொருத்து - match
    Underscore,      // _ - wildcard
    Arrow,           // => - match arm arrow

    // Built-in functions
    Achidu,          // அச்சிடு - print
    Ulleedu,         // உள்ளீடு - input
    Neelam,          // நீளம் - len
    Vagai,           // வகை - type

    // Literals
    Number(f64),     // Numbers (எண்)
    String(String),  // Strings (சரம்)
    Identifier(String), // Identifiers

    // Operators
    Plus,            // +
    Minus,           // -
    Star,            // *
    Slash,           // /
    Percent,         // %
    Equal,           // =
    EqualEqual,      // ==
    NotEqual,        // !=
    Less,            // <
    Greater,         // >
    LessEqual,       // <=
    GreaterEqual,    // >=

    // Delimiters
    LeftParen,       // (
    RightParen,      // )
    LeftBracket,     // [
    RightBracket,    // ]
    LeftBrace,       // {
    RightBrace,      // }
    Comma,           // ,
    Colon,           // :
    Dot,             // .

    // Special tokens
    Newline,         // Line ending
    Indent,          // Indentation increase
    Dedent,          // Indentation decrease
    Eof,             // End of file
}

impl fmt::Display for TokenType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            TokenType::Seyal => write!(f, "செயல்"),
            TokenType::Maari => write!(f, "மாறி"),
            TokenType::Maaraadha => write!(f, "மாறாத"),
            TokenType::Endraal => write!(f, "என்றால்"),
            TokenType::Illayendraal => write!(f, "இல்லையென்றால்"),
            TokenType::Illai => write!(f, "இல்லை"),
            TokenType::Varai => write!(f, "வரை"),
            TokenType::Ovvoru => write!(f, "ஒவ்வொரு"),
            TokenType::Ulla => write!(f, "உள்ள"),
            TokenType::Thirumbu => write!(f, "திரும்பு"),
            TokenType::Unmai => write!(f, "உண்மை"),
            TokenType::Poi => write!(f, "பொய்"),
            TokenType::Illa => write!(f, "இல்லா"),
            TokenType::Niruthu => write!(f, "நிறுத்து"),
            TokenType::Thodar => write!(f, "தொடர்"),
            TokenType::Matrum => write!(f, "மற்றும்"),
            TokenType::Alladhu => write!(f, "அல்லது"),
            TokenType::Illamal => write!(f, "இல்ல"),
            TokenType::Irakkumadhi => write!(f, "இறக்குமதி"),
            TokenType::Irundhu => write!(f, "இருந்து"),
            TokenType::Muyarchi => write!(f, "முயற்சி"),
            TokenType::Pidi => write!(f, "பிடி"),
            TokenType::Veesu => write!(f, "வீசு"),
            TokenType::Achidu => write!(f, "அச்சிடு"),
            TokenType::Ulleedu => write!(f, "உள்ளீடு"),
            TokenType::Neelam => write!(f, "நீளம்"),
            TokenType::Vagai => write!(f, "வகை"),
            TokenType::Number(n) => write!(f, "{}", n),
            TokenType::String(s) => write!(f, "\"{}\"", s),
            TokenType::Identifier(s) => write!(f, "{}", s),
            TokenType::Plus => write!(f, "+"),
            TokenType::Minus => write!(f, "-"),
            TokenType::Star => write!(f, "*"),
            TokenType::Slash => write!(f, "/"),
            TokenType::Percent => write!(f, "%"),
            TokenType::Equal => write!(f, "="),
            TokenType::EqualEqual => write!(f, "=="),
            TokenType::NotEqual => write!(f, "!="),
            TokenType::Less => write!(f, "<"),
            TokenType::Greater => write!(f, ">"),
            TokenType::LessEqual => write!(f, "<="),
            TokenType::GreaterEqual => write!(f, ">="),
            TokenType::LeftParen => write!(f, "("),
            TokenType::RightParen => write!(f, ")"),
            TokenType::LeftBracket => write!(f, "["),
            TokenType::RightBracket => write!(f, "]"),
            TokenType::LeftBrace => write!(f, "{{"),
            TokenType::RightBrace => write!(f, "}}"),
            TokenType::Comma => write!(f, ","),
            TokenType::Colon => write!(f, ":"),
            TokenType::Dot => write!(f, "."),
            TokenType::Newline => write!(f, "NEWLINE"),
            TokenType::Indent => write!(f, "INDENT"),
            TokenType::Dedent => write!(f, "DEDENT"),
            TokenType::Kattamaippu => write!(f, "கட்டமைப்பு"),
            TokenType::Viruppam => write!(f, "விருப்பம்"),
            TokenType::Poruthu => write!(f, "பொருத்து"),
            TokenType::Underscore => write!(f, "_"),
            TokenType::Arrow => write!(f, "=>"),
            TokenType::Eof => write!(f, "EOF"),
        }
    }
}

/// A token with position information
#[derive(Debug, Clone, PartialEq)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    pub line: usize,
    pub column: usize,
}

impl Token {
    pub fn new(token_type: TokenType, lexeme: String, line: usize, column: usize) -> Self {
        Token {
            token_type,
            lexeme,
            line,
            column,
        }
    }
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} '{}' at {}:{}", self.token_type, self.lexeme, self.line, self.column)
    }
}
