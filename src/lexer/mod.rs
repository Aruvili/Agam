//! Lexer module for Agam
//! 
//! Tokenizes Tamil source code into a stream of tokens

pub mod token;
pub mod scanner;

pub use token::{Token, TokenType};
pub use scanner::Scanner;

/// Convenience struct for the lexer
pub struct Lexer;

impl Lexer {
    /// Tokenize source code
    pub fn tokenize(source: &str) -> Result<Vec<Token>, crate::error::AgamError> {
        let mut scanner = Scanner::new(source);
        scanner.scan_tokens()
    }
}
