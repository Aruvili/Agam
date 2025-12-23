//! Interpreter module for Agam
//! 
//! Executes parsed programs

pub mod evaluator;
pub mod builtin;

pub use evaluator::Evaluator;

use crate::parser::Program;
use crate::error::AgamError;
use crate::types::Value;

/// Convenience struct for the interpreter
pub struct Interpreter {
    evaluator: Evaluator,
}

impl Interpreter {
    pub fn new() -> Self {
        Interpreter {
            evaluator: Evaluator::new(),
        }
    }

    /// Execute a parsed program
    pub fn execute(&mut self, program: &Program) -> Result<Value, AgamError> {
        self.evaluator.execute(program)
    }
}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}
