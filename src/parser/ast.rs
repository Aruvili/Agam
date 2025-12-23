//! Abstract Syntax Tree definitions for Agam
//! 
//! Represents the structure of parsed Tamil programs

use std::fmt;

/// A complete program
#[derive(Debug, Clone)]
pub struct Program {
    pub statements: Vec<Statement>,
}

/// Statement types
#[derive(Debug, Clone)]
pub enum Statement {
    /// Variable declaration: மாறி x = 5
    Let {
        name: String,
        value: Expression,
        is_const: bool,
    },
    /// Expression statement
    Expression(Expression),
    /// If statement: என்றால் condition:
    If {
        condition: Expression,
        then_branch: Vec<Statement>,
        elif_branches: Vec<(Expression, Vec<Statement>)>,
        else_branch: Option<Vec<Statement>>,
    },
    /// While loop: வரை condition:
    While {
        condition: Expression,
        body: Vec<Statement>,
    },
    /// For loop: ஒவ்வொரு item உள்ள list:
    For {
        variable: String,
        iterable: Expression,
        body: Vec<Statement>,
    },
    /// Function definition: செயல் name(params):
    Function {
        name: String,
        params: Vec<String>,
        body: Vec<Statement>,
    },
    /// Return statement: திரும்பு value
    Return(Option<Expression>),
    /// Break statement: நிறுத்து
    Break,
    /// Continue statement: தொடர்
    Continue,
    /// Print statement: அச்சிடு(value)
    Print(Vec<Expression>),
    /// Import statement: இறக்குமதி module
    Import {
        module: String,
        items: Option<Vec<String>>,
    },
    /// Try-catch statement: முயற்சி...பிடி
    TryCatch {
        try_block: Vec<Statement>,
        error_var: String,
        catch_block: Vec<Statement>,
    },
    /// Throw statement: வீசு error
    Throw(Expression),
    /// Struct definition: கட்டமைப்பு Name:
    Struct {
        name: String,
        fields: Vec<(String, Option<String>)>, // (field_name, optional_type)
    },
    /// Enum definition: விருப்பம் Name:
    Enum {
        name: String,
        variants: Vec<String>,
    },
    /// Match statement: பொருத்து value:
    Match {
        value: Expression,
        arms: Vec<MatchArm>,
    },
}

/// Match arm for pattern matching
#[derive(Debug, Clone)]
pub struct MatchArm {
    pub pattern: Pattern,
    pub body: Vec<Statement>,
}

/// Pattern for matching
#[derive(Debug, Clone)]
pub enum Pattern {
    /// Literal value
    Literal(Expression),
    /// Variable binding
    Variable(String),
    /// Wildcard (_)
    Wildcard,
    /// Enum variant
    EnumVariant(String, String), // (enum_name, variant_name)
}

/// Expression types
#[derive(Debug, Clone)]
pub enum Expression {
    /// Number literal
    Number(f64),
    /// String literal
    String(String),
    /// Boolean literal
    Boolean(bool),
    /// Null literal
    Null,
    /// Variable reference
    Identifier(String),
    /// Binary operation: a + b
    Binary {
        left: Box<Expression>,
        operator: BinaryOp,
        right: Box<Expression>,
    },
    /// Unary operation: -a, !a
    Unary {
        operator: UnaryOp,
        operand: Box<Expression>,
    },
    /// Function call: name(args)
    Call {
        callee: Box<Expression>,
        arguments: Vec<Expression>,
    },
    /// List literal: [1, 2, 3]
    List(Vec<Expression>),
    /// Dictionary literal: {"key": value}
    Dict(Vec<(Expression, Expression)>),
    /// Index access: list[0]
    Index {
        object: Box<Expression>,
        index: Box<Expression>,
    },
    /// Grouping: (expr)
    Grouping(Box<Expression>),
    /// Assignment: name = value
    Assignment {
        name: String,
        value: Box<Expression>,
    },
    /// Member access: obj.field
    MemberAccess {
        object: Box<Expression>,
        member: String,
    },
    /// Struct instantiation: Person("name", 25)
    StructInit {
        name: String,
        arguments: Vec<Expression>,
    },
}

/// Binary operators
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BinaryOp {
    // Arithmetic
    Add,      // +
    Subtract, // -
    Multiply, // *
    Divide,   // /
    Modulo,   // %
    
    // Comparison
    Equal,        // ==
    NotEqual,     // !=
    Less,         // <
    Greater,      // >
    LessEqual,    // <=
    GreaterEqual, // >=
    
    // Logical
    And, // மற்றும் / and
    Or,  // அல்லது / or
}

impl fmt::Display for BinaryOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BinaryOp::Add => write!(f, "+"),
            BinaryOp::Subtract => write!(f, "-"),
            BinaryOp::Multiply => write!(f, "*"),
            BinaryOp::Divide => write!(f, "/"),
            BinaryOp::Modulo => write!(f, "%"),
            BinaryOp::Equal => write!(f, "=="),
            BinaryOp::NotEqual => write!(f, "!="),
            BinaryOp::Less => write!(f, "<"),
            BinaryOp::Greater => write!(f, ">"),
            BinaryOp::LessEqual => write!(f, "<="),
            BinaryOp::GreaterEqual => write!(f, ">="),
            BinaryOp::And => write!(f, "மற்றும்"),
            BinaryOp::Or => write!(f, "அல்லது"),
        }
    }
}

/// Unary operators
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum UnaryOp {
    Negate, // -
    Not,    // இல்ல / not
}

impl fmt::Display for UnaryOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            UnaryOp::Negate => write!(f, "-"),
            UnaryOp::Not => write!(f, "இல்ல"),
        }
    }
}
