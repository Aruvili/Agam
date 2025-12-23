//! Error types for Agam
//! 
//! Provides error handling with Tamil messages

#![allow(clippy::module_inception)]

use thiserror::Error;

/// Main error type for Agam
#[derive(Error, Debug)]
pub enum AgamError {
    #[error("சொற்பிழை (Lexer Error) [{line}:{column}]: {message}")]
    LexerError {
        line: usize,
        column: usize,
        message: String,
    },

    #[error("தொடரியல் பிழை (Syntax Error) [{line}:{column}]: {message}")]
    ParserError {
        line: usize,
        column: usize,
        message: String,
    },

    #[error("இயக்க பிழை (Runtime Error) [{line}:{column}]: {message}")]
    RuntimeError {
        line: usize,
        column: usize,
        message: String,
    },

    #[error("கோப்பு பிழை (File Error): {0}")]
    FileError(String),
}

impl AgamError {
    pub fn lexer_error(line: usize, column: usize, message: String) -> Self {
        AgamError::LexerError { line, column, message }
    }

    pub fn parser_error(line: usize, column: usize, message: String) -> Self {
        AgamError::ParserError { line, column, message }
    }

    pub fn runtime_error(line: usize, column: usize, message: String) -> Self {
        AgamError::RuntimeError { line, column, message }
    }

    pub fn file_error(message: String) -> Self {
        AgamError::FileError(message)
    }

    /// Get a helpful suggestion for common errors
    pub fn suggestion(&self) -> Option<String> {
        match self {
            AgamError::LexerError { message, .. } => {
                if message.contains("முடிவுறாத சரம்") {
                    Some("சரம் \" குறியீட்டுடன் முடிக்கவும்".to_string())
                } else {
                    None
                }
            }
            AgamError::ParserError { message, .. } => {
                if message.contains("':' எதிர்பார்க்கப்படுகிறது") {
                    Some("என்றால், வரை, செயல் போன்றவற்றின் பின் ':' தேவை".to_string())
                } else if message.contains("உள்தள்ளுதல்") {
                    Some("தொகுதிக்குள் குறியீட்டை உள்தள்ளவும்".to_string())
                } else {
                    None
                }
            }
            AgamError::RuntimeError { message, .. } => {
                if message.contains("வரையறுக்கப்படாத மாறி") {
                    Some("மாறி 'மாறி' சொல்லுடன் முதலில் வரையறுக்கவும்".to_string())
                } else if message.contains("பூஜ்ஜியத்தால் வகுக்க") {
                    Some("வகுக்கும் முன் எண் பூஜ்ஜியமா என சரிபார்க்கவும்".to_string())
                } else {
                    None
                }
            }
            _ => None,
        }
    }
}

/// Pretty print an error with context
pub fn format_error(error: &AgamError, source: &str) -> String {
    let mut output = String::new();

    match error {
        AgamError::LexerError { line, column, .. }
        | AgamError::ParserError { line, column, .. }
        | AgamError::RuntimeError { line, column, .. } => {
            output.push_str(&format!("\n{}\n", error));

            // Show the line with the error
            if *line > 0 {
                if let Some(source_line) = source.lines().nth(*line - 1) {
                    output.push_str(&format!("  {} | {}\n", line, source_line));
                    
                    // Show pointer to error location
                    let padding = format!("{}", line).len() + 3 + column.saturating_sub(1);
                    output.push_str(&format!("{}^\n", " ".repeat(padding)));
                }
            }

            // Add suggestion if available
            if let Some(suggestion) = error.suggestion() {
                output.push_str(&format!("\n💡 குறிப்பு: {}\n", suggestion));
            }
        }
        AgamError::FileError(_) => {
            output.push_str(&format!("\n{}\n", error));
        }
    }

    output
}
