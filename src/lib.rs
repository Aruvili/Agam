//! அகம் (Agam) - Tamil Programming Language
//! 
//! A Tamil-first programming language with Python-like syntax,
//! built using Rust for performance, memory safety, and type security.

#![allow(clippy::module_inception)]

pub mod lexer;
pub mod parser;
pub mod interpreter;
pub mod types;
pub mod error;

pub use lexer::Lexer;
pub use parser::Parser;
pub use interpreter::Interpreter;
pub use error::AgamError;
