//! Runtime values for Agam
//! 
//! Represents values during program execution

use std::collections::HashMap;
use std::fmt;
use std::rc::Rc;
use std::cell::RefCell;

use rustc_hash::FxHashMap;

use crate::parser::Statement;

/// Runtime values
#[derive(Clone)]
pub enum Value {
    /// Number (எண்)
    Number(f64),
    /// String (சரம்)
    String(String),
    /// Boolean (உண்மைபொய்)
    Boolean(bool),
    /// Null (இல்லா)
    Null,
    /// List (பட்டியல்)
    List(Rc<RefCell<Vec<Value>>>),
    /// Dictionary (அகராதி)
    Dict(Rc<RefCell<HashMap<String, Value>>>),
    /// Function (செயல்) - wrapped in Rc to avoid cloning body
    Function(Rc<AgamFunction>),
    /// Native/built-in function - wrapped in Rc for efficient sharing
    NativeFunction(Rc<NativeFunction>),
    /// User-defined struct instance
    Struct {
        name: String,
        fields: Rc<RefCell<HashMap<String, Value>>>,
    },
    /// Enum variant
    EnumVariant {
        enum_name: String,
        variant: String,
    },
    /// Struct definition (type)
    StructDef {
        name: String,
        field_names: Vec<String>,
    },
    /// Enum definition (type)
    EnumDef {
        name: String,
        variants: Vec<String>,
    },
    /// Module namespace (கூறு)
    Module {
        name: String,
        exports: Rc<RefCell<HashMap<String, Value>>>,
    },
}

impl fmt::Debug for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Number(n) => write!(f, "{}", n),
            Value::String(s) => write!(f, "\"{}\"", s),
            Value::Boolean(b) => write!(f, "{}", if *b { "உண்மை" } else { "பொய்" }),
            Value::Null => write!(f, "இல்லா"),
            Value::List(list) => {
                let list = list.borrow();
                write!(f, "[")?;
                for (i, v) in list.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{:?}", v)?;
                }
                write!(f, "]")
            }
            Value::Dict(dict) => {
                let dict = dict.borrow();
                write!(f, "{{")?;
                for (i, (k, v)) in dict.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "\"{}\": {:?}", k, v)?;
                }
                write!(f, "}}")
            }
            Value::Function(func) => write!(f, "<செயல் {}>", func.name),
            Value::NativeFunction(func) => write!(f, "<உள்ளமைப்பு செயல் {}>", func.name),
            Value::Struct { name, fields } => {
                let fields = fields.borrow();
                write!(f, "{} {{", name)?;
                for (i, (k, v)) in fields.iter().enumerate() {
                    if i > 0 { write!(f, ", ")?; }
                    write!(f, "{}: {:?}", k, v)?;
                }
                write!(f, "}}")
            }
            Value::EnumVariant { enum_name, variant } => write!(f, "{}.{}", enum_name, variant),
            Value::StructDef { name, .. } => write!(f, "<கட்டமைப்பு {}>", name),
            Value::EnumDef { name, .. } => write!(f, "<விருப்பம் {}>", name),
            Value::Module { name, .. } => write!(f, "<கூறு {}>", name),
        }
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Number(n) => {
                if n.fract() == 0.0 {
                    write!(f, "{}", *n as i64)
                } else {
                    write!(f, "{}", n)
                }
            }
            Value::String(s) => write!(f, "{}", s),
            Value::Boolean(b) => write!(f, "{}", if *b { "உண்மை" } else { "பொய்" }),
            Value::Null => write!(f, "இல்லா"),
            Value::List(list) => {
                let list = list.borrow();
                write!(f, "[")?;
                for (i, v) in list.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", v)?;
                }
                write!(f, "]")
            }
            Value::Dict(dict) => {
                let dict = dict.borrow();
                write!(f, "{{")?;
                for (i, (k, v)) in dict.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "\"{}\": {}", k, v)?;
                }
                write!(f, "}}")
            }
            Value::Function(func) => write!(f, "<செயல் {}>", func.name),
            Value::NativeFunction(func) => write!(f, "<உள்ளமைப்பு செயல் {}>", func.name),
            Value::Struct { name, fields } => {
                let fields = fields.borrow();
                write!(f, "{} {{", name)?;
                for (i, (k, v)) in fields.iter().enumerate() {
                    if i > 0 { write!(f, ", ")?; }
                    write!(f, "{}: {}", k, v)?;
                }
                write!(f, "}}")
            }
            Value::EnumVariant { enum_name, variant } => write!(f, "{}.{}", enum_name, variant),
            Value::StructDef { name, .. } => write!(f, "<கட்டமைப்பு {}>", name),
            Value::EnumDef { name, .. } => write!(f, "<விருப்பம் {}>", name),
            Value::Module { name, .. } => write!(f, "<கூறு {}>", name),
        }
    }
}

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Value::Number(a), Value::Number(b)) => a == b,
            (Value::String(a), Value::String(b)) => a == b,
            (Value::Boolean(a), Value::Boolean(b)) => a == b,
            (Value::Null, Value::Null) => true,
            (Value::List(a), Value::List(b)) => Rc::ptr_eq(a, b),
            (Value::Dict(a), Value::Dict(b)) => Rc::ptr_eq(a, b),
            _ => false,
        }
    }
}

impl Value {
    /// Check if value is truthy
    pub fn is_truthy(&self) -> bool {
        match self {
            Value::Null => false,
            Value::Boolean(b) => *b,
            Value::Number(n) => *n != 0.0,
            Value::String(s) => !s.is_empty(),
            Value::List(list) => !list.borrow().is_empty(),
            Value::Dict(dict) => !dict.borrow().is_empty(),
            Value::Function(_) => true,
            Value::NativeFunction(_) => true,
            Value::Struct { .. } => true,
            Value::EnumVariant { .. } => true,
            Value::StructDef { .. } => true,
            Value::EnumDef { .. } => true,
            Value::Module { .. } => true,
        }
    }

    /// Get the type name in Tamil
    pub fn type_name(&self) -> &'static str {
        match self {
            Value::Number(_) => "எண்",
            Value::String(_) => "சரம்",
            Value::Boolean(_) => "உண்மைபொய்",
            Value::Null => "இல்லா",
            Value::List(_) => "பட்டியல்",
            Value::Dict(_) => "அகராதி",
            Value::Function(_) => "செயல்",
            Value::NativeFunction(_) => "உள்ளமைப்பு_செயல்",
            Value::Struct { name: _, .. } => "கட்டமைப்பு",
            Value::EnumVariant { .. } => "விருப்பம்_மதிப்பு",
            Value::StructDef { .. } => "கட்டமைப்பு_வரையறை",
            Value::EnumDef { .. } => "விருப்பம்_வரையறை",
            Value::Module { .. } => "கூறு",
        }
    }
}

/// User-defined function
#[derive(Clone)]
pub struct AgamFunction {
    pub name: String,
    pub params: Vec<String>,
    pub body: Vec<Statement>,
    pub closure: Rc<RefCell<Environment>>,
}

impl AgamFunction {
    pub fn new(
        name: String,
        params: Vec<String>,
        body: Vec<Statement>,
        closure: Rc<RefCell<Environment>>,
    ) -> Self {
        AgamFunction {
            name,
            params,
            body,
            closure,
        }
    }
}

/// Native function type
pub type NativeFn = fn(&[Value]) -> Result<Value, String>;

/// Native function wrapper
#[derive(Clone)]
pub struct NativeFunction {
    pub name: String,
    pub arity: Option<usize>, // None means variadic
    pub function: NativeFn,
}

impl NativeFunction {
    pub fn new(name: &str, arity: Option<usize>, function: NativeFn) -> Self {
        NativeFunction {
            name: name.to_string(),
            arity,
            function,
        }
    }
}

/// Variable environment with scope chain
/// Uses FxHashMap for faster string key lookups
#[derive(Clone)]
pub struct Environment {
    values: FxHashMap<String, (Value, bool)>, // (value, is_const)
    parent: Option<Rc<RefCell<Environment>>>,
}

impl Environment {
    pub fn new() -> Self {
        Environment {
            values: FxHashMap::default(),
            parent: None,
        }
    }

    pub fn with_parent(parent: Rc<RefCell<Environment>>) -> Self {
        Environment {
            values: FxHashMap::default(),
            parent: Some(parent),
        }
    }

    /// Define a new variable
    pub fn define(&mut self, name: String, value: Value, is_const: bool) {
        self.values.insert(name, (value, is_const));
    }

    /// Get a variable's value
    pub fn get(&self, name: &str) -> Option<Value> {
        if let Some((value, _)) = self.values.get(name) {
            return Some(value.clone());
        }
        if let Some(parent) = &self.parent {
            return parent.borrow().get(name);
        }
        None
    }

    /// Check if a variable is const
    pub fn is_const(&self, name: &str) -> Option<bool> {
        if let Some((_, is_const)) = self.values.get(name) {
            return Some(*is_const);
        }
        if let Some(parent) = &self.parent {
            return parent.borrow().is_const(name);
        }
        None
    }

    /// Assign to an existing variable
    pub fn assign(&mut self, name: &str, value: Value) -> Result<(), String> {
        if let Some((_, is_const)) = self.values.get(name) {
            if *is_const {
                return Err(format!("'{}' மாறாத மாறி, மாற்ற இயலாது", name));
            }
            self.values.insert(name.to_string(), (value, false));
            return Ok(());
        }
        if let Some(parent) = &self.parent {
            return parent.borrow_mut().assign(name, value);
        }
        Err(format!("வரையறுக்கப்படாத மாறி '{}'", name))
    }

    /// Get all variable names in this environment (not including parent)
    pub fn get_all_names(&self) -> Vec<String> {
        self.values.keys().cloned().collect()
    }
}

impl Default for Environment {
    fn default() -> Self {
        Self::new()
    }
}
