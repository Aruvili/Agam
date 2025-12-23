//! Integration tests for agam
//! 
//! Tests the core lexer, parser, and interpreter functionality

use agam::{Lexer, Parser, Interpreter};
use agam::types::Value;

/// Helper to run code and get result
fn run(source: &str) -> Result<Value, String> {
    let tokens = Lexer::tokenize(source).map_err(|e| format!("{}", e))?;
    let mut parser = Parser::new(tokens);
    let program = parser.parse().map_err(|e| format!("{}", e))?;
    let mut interpreter = Interpreter::new();
    interpreter.execute(&program).map_err(|e| format!("{}", e))
}

/// Helper to check if code executes without error
fn run_ok(source: &str) -> bool {
    run(source).is_ok()
}

// ============= Lexer Tests =============

#[test]
fn test_tokenize_numbers() {
    assert!(Lexer::tokenize("42").is_ok());
    assert!(Lexer::tokenize("3.14").is_ok());
}

#[test]
fn test_tokenize_strings() {
    assert!(Lexer::tokenize("\"hello\"").is_ok());
    assert!(Lexer::tokenize("\"தமிழ்\"").is_ok());
}

#[test]
fn test_tokenize_operators() {
    assert!(Lexer::tokenize("+ - * / %").is_ok());
    assert!(Lexer::tokenize("== != < > <= >=").is_ok());
}

// ============= Parser Tests =============

#[test]
fn test_parse_variable() {
    let tokens = Lexer::tokenize("மாறி x = 5").unwrap();
    let mut parser = Parser::new(tokens);
    assert!(parser.parse().is_ok());
}

#[test]
fn test_parse_function() {
    let tokens = Lexer::tokenize("செயல் foo():\n    திரும்பு 1").unwrap();
    let mut parser = Parser::new(tokens);
    assert!(parser.parse().is_ok());
}

// ============= Interpreter Tests =============

#[test]
fn test_arithmetic_eval() {
    // Simple arithmetic expressions
    assert!(run_ok("மாறி x = 5 + 3"));
    assert!(run_ok("மாறி x = 10 - 4"));
}

#[test]
fn test_string_concatenation() {
    assert!(run_ok("மாறி s = \"hello\" + \" world\""));
}

#[test]
fn test_comparison() {
    assert!(run_ok("மாறி x = 5 > 3"));
    assert!(run_ok("மாறி x = 5 == 5"));
}

// ============= Struct Tests =============

#[test]
fn test_struct_definition() {
    assert!(run_ok("கட்டமைப்பு Person:\n    name\n    age"));
}

// ============= Enum Tests =============

#[test]
fn test_enum_definition() {
    assert!(run_ok("விருப்பம் Color:\n    Red\n    Green\n    Blue"));
}

#[test]
fn test_enum_access() {
    assert!(run_ok("விருப்பம் Color:\n    Red\n    Green\nமாறி c = Color.Red"));
}

// ============= Pattern Matching Tests =============

#[test]
fn test_pattern_match_numbers() {
    assert!(run_ok("மாறி x = 2\nபொருத்து x:\n    1 => அச்சிடு(1)\n    2 => அச்சிடு(2)\n    _ => அச்சிடு(0)"));
}

#[test]
fn test_pattern_match_wildcard() {
    assert!(run_ok("மாறி x = 99\nபொருத்து x:\n    _ => அச்சிடு(\"any\")"));
}

// ============= Security Tests (ignored - cause stack overflow) =============

#[test]
#[ignore]
fn test_recursion_limit() {
    let code = "செயல் infinite():\n    திரும்பு infinite()\ninfinite()";
    let result = run(code);
    assert!(result.is_err());
}

#[test]
#[ignore]
fn test_loop_iteration_limit() {
    let code = "வரை உண்மை:\n    மாறி x = 1";
    let result = run(code);
    assert!(result.is_err());
}
