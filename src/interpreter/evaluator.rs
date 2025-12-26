//! Tree-walk interpreter for Agam
//! 
//! Executes the Abstract Syntax Tree

use std::rc::Rc;
use std::cell::RefCell;
use std::collections::HashMap;

use crate::parser::ast::*;
use crate::types::{Value, Environment, AgamFunction};
use crate::error::AgamError;
use crate::interpreter::builtin::create_builtins;

/// Maximum recursion depth to prevent stack overflow
const MAX_RECURSION_DEPTH: usize = 1000;

/// Maximum loop iterations to prevent infinite loops
const MAX_LOOP_ITERATIONS: usize = 10_000_000;

/// Control flow signals
pub enum ControlFlow {
    None,
    Return(Value),
    Break,
    Continue,
}

/// The interpreter
pub struct Evaluator {
    environment: Rc<RefCell<Environment>>,
    #[allow(dead_code)]
    globals: Rc<RefCell<Environment>>,
    /// Current call stack depth for recursion protection
    call_depth: usize,
}

impl Evaluator {
    pub fn new() -> Self {
        let globals = Rc::new(RefCell::new(Environment::new()));
        
        // Register built-in functions
        for (name, func) in create_builtins() {
            globals.borrow_mut().define(
                name,
                Value::NativeFunction(func),
                true,
            );
        }

        Evaluator {
            environment: Rc::clone(&globals),
            globals,
            call_depth: 0,
        }
    }

    /// Execute a program
    pub fn execute(&mut self, program: &Program) -> Result<Value, AgamError> {
        for statement in &program.statements {
            match self.execute_statement(statement)? {
                ControlFlow::Return(v) => return Ok(v),
                ControlFlow::Break => {
                    return Err(AgamError::runtime_error(
                        0, 0,
                        "'நிறுத்து' வளைய வெளியே பயன்படுத்த முடியாது".to_string(),
                    ))
                }
                ControlFlow::Continue => {
                    return Err(AgamError::runtime_error(
                        0, 0,
                        "'தொடர்' வளைய வெளியே பயன்படுத்த முடியாது".to_string(),
                    ))
                }
                ControlFlow::None => {}
            }
            
            // In REPL mode, keep track of last expression value
            if let Statement::Expression(_) = statement {
                // For now, just continue
            }
        }

        Ok(Value::Null)
    }

    fn execute_statement(&mut self, stmt: &Statement) -> Result<ControlFlow, AgamError> {
        match stmt {
            Statement::Let { name, value, is_const } => {
                let val = self.evaluate(value)?;
                self.environment.borrow_mut().define(name.clone(), val, *is_const);
                Ok(ControlFlow::None)
            }

            Statement::Expression(expr) => {
                self.evaluate(expr)?;
                Ok(ControlFlow::None)
            }

            Statement::Print(args) => {
                let values: Vec<String> = args
                    .iter()
                    .map(|arg| self.evaluate(arg).map(|v| v.to_string()))
                    .collect::<Result<Vec<_>, _>>()?;
                println!("{}", values.join(" "));
                Ok(ControlFlow::None)
            }

            Statement::If {
                condition,
                then_branch,
                elif_branches,
                else_branch,
            } => {
                let cond_val = self.evaluate(condition)?;
                
                if cond_val.is_truthy() {
                    return self.execute_block(then_branch);
                }

                for (elif_cond, elif_body) in elif_branches {
                    let elif_val = self.evaluate(elif_cond)?;
                    if elif_val.is_truthy() {
                        return self.execute_block(elif_body);
                    }
                }

                if let Some(else_body) = else_branch {
                    return self.execute_block(else_body);
                }

                Ok(ControlFlow::None)
            }

            Statement::While { condition, body } => {
                let mut iterations = 0usize;
                loop {
                    // Check iteration limit
                    iterations += 1;
                    if iterations > MAX_LOOP_ITERATIONS {
                        return Err(AgamError::runtime_error(
                            0, 0,
                            format!(
                                "அதிகபட்ச வளைய மீளமுறை ({}) அடைந்தது - முடிவிலா வளையம் இருக்கலாம்",
                                MAX_LOOP_ITERATIONS
                            ),
                        ));
                    }

                    let cond_val = self.evaluate(condition)?;
                    if !cond_val.is_truthy() {
                        break;
                    }

                    match self.execute_block(body)? {
                        ControlFlow::Break => break,
                        ControlFlow::Continue => continue,
                        ControlFlow::Return(v) => return Ok(ControlFlow::Return(v)),
                        ControlFlow::None => {}
                    }
                }
                Ok(ControlFlow::None)
            }

            Statement::For { variable, iterable, body } => {
                let iter_val = self.evaluate(iterable)?;
                
                let items = match iter_val {
                    Value::List(list) => list.borrow().clone(),
                    Value::String(s) => s.chars().map(|c| Value::String(c.to_string())).collect(),
                    _ => {
                        return Err(AgamError::runtime_error(
                            0, 0,
                            format!("'{}' வகையை மீளமுடியாது", iter_val.type_name()),
                        ))
                    }
                };

                for item in items {
                    self.environment.borrow_mut().define(variable.clone(), item, false);
                    
                    match self.execute_block(body)? {
                        ControlFlow::Break => break,
                        ControlFlow::Continue => continue,
                        ControlFlow::Return(v) => return Ok(ControlFlow::Return(v)),
                        ControlFlow::None => {}
                    }
                }

                Ok(ControlFlow::None)
            }

            Statement::Function { name, params, body } => {
                let func = Rc::new(AgamFunction::new(
                    name.clone(),
                    params.clone(),
                    body.clone(),
                    Rc::clone(&self.environment),
                ));
                self.environment.borrow_mut().define(
                    name.clone(),
                    Value::Function(func),
                    true,
                );
                Ok(ControlFlow::None)
            }

            Statement::Return(value) => {
                let val = match value {
                    Some(expr) => self.evaluate(expr)?,
                    None => Value::Null,
                };
                Ok(ControlFlow::Return(val))
            }

            Statement::Break => Ok(ControlFlow::Break),
            Statement::Continue => Ok(ControlFlow::Continue),
            
            // Import statement - load and execute external module
            Statement::Import { module, items } => {
                // Try to find and load the module file
                let module_path = format!("{}.agam", module);
                
                if let Ok(source) = std::fs::read_to_string(&module_path) {
                    // Parse and execute the module
                    let tokens = crate::lexer::Lexer::tokenize(&source).map_err(|e| {
                        AgamError::runtime_error(0, 0, format!("Module load error: {}", e))
                    })?;
                    
                    let mut parser = crate::parser::Parser::new(tokens);
                    let program = parser.parse().map_err(|e| {
                        AgamError::runtime_error(0, 0, format!("Module parse error: {}", e))
                    })?;
                    
                    // Create a temporary environment for module execution
                    let module_env = Rc::new(RefCell::new(Environment::new()));
                    
                    // Register built-ins in module environment
                    for (name, func) in crate::interpreter::builtin::create_builtins() {
                        module_env.borrow_mut().define(name, Value::NativeFunction(func), true);
                    }
                    
                    // Execute module in temporary environment
                    let previous_env = Rc::clone(&self.environment);
                    self.environment = Rc::clone(&module_env);
                    
                    for stmt in &program.statements {
                        self.execute_statement(stmt)?;
                    }
                    
                    self.environment = previous_env;
                    
                    // Import items from module environment to current environment
                    match items {
                        Some(item_names) => {
                            // Selective import - only import specified items
                            for item_name in item_names {
                                if let Some(value) = module_env.borrow().get(item_name) {
                                    self.environment.borrow_mut().define(item_name.clone(), value, true);
                                } else {
                                    return Err(AgamError::runtime_error(
                                        0, 0,
                                        format!("'{}' கூறில் '{}' கிடைக்கவில்லை", module, item_name),
                                    ));
                                }
                            }
                        }
                        None => {
                            // Namespace import - create a Module value with all exports
                            let mut exports = std::collections::HashMap::new();
                            let module_values: Vec<_> = module_env.borrow().get_all_names();
                            for name in module_values {
                                // Skip builtins (they're already in globals)
                                if let Some(value) = module_env.borrow().get(&name) {
                                    if !matches!(value, Value::NativeFunction(_)) {
                                        exports.insert(name, value);
                                    }
                                }
                            }
                            
                            // Store the module as a namespace
                            let module_value = Value::Module {
                                name: module.clone(),
                                exports: Rc::new(RefCell::new(exports)),
                            };
                            self.environment.borrow_mut().define(module.clone(), module_value, true);
                        }
                    }
                    
                    Ok(ControlFlow::None)
                } else {
                    Err(AgamError::runtime_error(
                        0, 0,
                        format!("கூறு காணவில்லை: '{}'", module),
                    ))
                }
            }
            
            // Try-catch statement for error handling
            Statement::TryCatch { try_block, error_var, catch_block } => {
                match self.execute_block(try_block) {
                    Ok(flow) => Ok(flow),
                    Err(e) => {
                        // Bind error message to error_var and execute catch block
                        let error_msg = format!("{}", e);
                        self.environment.borrow_mut().define(
                            error_var.clone(),
                            Value::String(error_msg),
                            false,
                        );
                        self.execute_block(catch_block)
                    }
                }
            }
            
            // Throw statement - raise an error
            Statement::Throw(expr) => {
                let error_val = self.evaluate(expr)?;
                Err(AgamError::runtime_error(
                    0, 0,
                    format!("வீசப்பட்ட பிழை: {}", error_val),
                ))
            }

            // Struct definition - store the struct type
            Statement::Struct { name, fields } => {
                let field_names: Vec<String> = fields.iter().map(|(n, _)| n.clone()).collect();
                let struct_def = Value::StructDef {
                    name: name.clone(),
                    field_names,
                };
                self.environment.borrow_mut().define(name.clone(), struct_def, true);
                Ok(ControlFlow::None)
            }

            // Enum definition - store the enum type
            Statement::Enum { name, variants } => {
                let enum_def = Value::EnumDef {
                    name: name.clone(),
                    variants: variants.clone(),
                };
                self.environment.borrow_mut().define(name.clone(), enum_def, true);
                Ok(ControlFlow::None)
            }

            // Match statement - pattern matching
            Statement::Match { value, arms } => {
                let val = self.evaluate(value)?;
                
                for arm in arms {
                    if self.pattern_matches(&arm.pattern, &val)? {
                        return self.execute_block(&arm.body);
                    }
                }
                
                // No pattern matched - this is an error
                Err(AgamError::runtime_error(
                    0, 0,
                    format!("எந்த வடிவமும் பொருந்தவில்லை: {}", val),
                ))
            }
        }
    }

    fn execute_block(&mut self, statements: &[Statement]) -> Result<ControlFlow, AgamError> {
        let previous = Rc::clone(&self.environment);
        self.environment = Rc::new(RefCell::new(Environment::with_parent(previous.clone())));

        let result = (|| {
            for stmt in statements {
                let flow = self.execute_statement(stmt)?;
                match flow {
                    ControlFlow::None => {}
                    _ => return Ok(flow),
                }
            }
            Ok(ControlFlow::None)
        })();

        self.environment = previous;
        result
    }

    fn evaluate(&mut self, expr: &Expression) -> Result<Value, AgamError> {
        match expr {
            Expression::Number(n) => Ok(Value::Number(*n)),
            Expression::String(s) => Ok(Value::String(s.clone())),
            
            // Interpolated f-string: f"Hello {name}!"
            Expression::FString { parts } => {
                let mut result = String::new();
                for part in parts {
                    match part {
                        FStringPart::Literal(s) => result.push_str(s),
                        FStringPart::Expression(expr) => {
                            let value = self.evaluate(expr)?;
                            result.push_str(&value.to_string());
                        }
                    }
                }
                Ok(Value::String(result))
            }
            
            Expression::Boolean(b) => Ok(Value::Boolean(*b)),
            Expression::Null => Ok(Value::Null),

            Expression::Identifier(name) => {
                self.environment
                    .borrow()
                    .get(name)
                    .ok_or_else(|| {
                        AgamError::runtime_error(
                            0, 0,
                            format!("வரையறுக்கப்படாத மாறி '{}'", name),
                        )
                    })
            }

            Expression::Binary { left, operator, right } => {
                let left_val = self.evaluate(left)?;
                
                // Short-circuit for logical operators
                match operator {
                    BinaryOp::And => {
                        if !left_val.is_truthy() {
                            return Ok(left_val);
                        }
                        return self.evaluate(right);
                    }
                    BinaryOp::Or => {
                        if left_val.is_truthy() {
                            return Ok(left_val);
                        }
                        return self.evaluate(right);
                    }
                    _ => {}
                }

                let right_val = self.evaluate(right)?;
                self.apply_binary_op(operator, &left_val, &right_val)
            }

            Expression::Unary { operator, operand } => {
                let val = self.evaluate(operand)?;
                match operator {
                    UnaryOp::Negate => match val {
                        Value::Number(n) => Ok(Value::Number(-n)),
                        _ => Err(AgamError::runtime_error(
                            0, 0,
                            format!("'{}' எதிர்மறை செய்ய இயலாது", val.type_name()),
                        )),
                    },
                    UnaryOp::Not => Ok(Value::Boolean(!val.is_truthy())),
                }
            }

            Expression::Call { callee, arguments } => {
                let callee_val = self.evaluate(callee)?;
                
                let args: Vec<Value> = arguments
                    .iter()
                    .map(|arg| self.evaluate(arg))
                    .collect::<Result<Vec<_>, _>>()?;

                self.call_function(callee_val, args)
            }

            Expression::List(elements) => {
                let values: Vec<Value> = elements
                    .iter()
                    .map(|e| self.evaluate(e))
                    .collect::<Result<Vec<_>, _>>()?;
                Ok(Value::List(Rc::new(RefCell::new(values))))
            }

            Expression::Dict(pairs) => {
                let mut map = HashMap::new();
                for (key, value) in pairs {
                    let k = match self.evaluate(key)? {
                        Value::String(s) => s,
                        v => v.to_string(),
                    };
                    let v = self.evaluate(value)?;
                    map.insert(k, v);
                }
                Ok(Value::Dict(Rc::new(RefCell::new(map))))
            }

            Expression::Index { object, index } => {
                let obj = self.evaluate(object)?;
                let idx = self.evaluate(index)?;

                match (&obj, &idx) {
                    (Value::List(list), Value::Number(n)) => {
                        let i = *n as i64;
                        let list = list.borrow();
                        let len = list.len() as i64;
                        
                        let actual_idx = if i < 0 { len + i } else { i };
                        
                        if actual_idx < 0 || actual_idx >= len {
                            return Err(AgamError::runtime_error(
                                0, 0,
                                format!("குறியீட்டு {} வரம்பிற்கு வெளியே", n),
                            ));
                        }
                        Ok(list[actual_idx as usize].clone())
                    }
                    (Value::String(s), Value::Number(n)) => {
                        let i = *n as i64;
                        let chars: Vec<char> = s.chars().collect();
                        let len = chars.len() as i64;
                        
                        let actual_idx = if i < 0 { len + i } else { i };
                        
                        if actual_idx < 0 || actual_idx >= len {
                            return Err(AgamError::runtime_error(
                                0, 0,
                                format!("குறியீட்டு {} வரம்பிற்கு வெளியே", n),
                            ));
                        }
                        Ok(Value::String(chars[actual_idx as usize].to_string()))
                    }
                    (Value::Dict(dict), _) => {
                        let key = match &idx {
                            Value::String(s) => s.clone(),
                            v => v.to_string(),
                        };
                        dict.borrow()
                            .get(&key)
                            .cloned()
                            .ok_or_else(|| {
                                AgamError::runtime_error(
                                    0, 0,
                                    format!("சாவி '{}' காணப்படவில்லை", key),
                                )
                            })
                    }
                    _ => Err(AgamError::runtime_error(
                        0, 0,
                        format!("'{}' வகையை குறியீட்டு செய்ய இயலாது", obj.type_name()),
                    )),
                }
            }

            Expression::Grouping(inner) => self.evaluate(inner),

            Expression::Assignment { name, value } => {
                let val = self.evaluate(value)?;
                self.environment
                    .borrow_mut()
                    .assign(name, val.clone())
                    .map_err(|msg| AgamError::runtime_error(0, 0, msg))?;
                Ok(val)
            }

            Expression::MemberAccess { object, member } => {
                let obj = self.evaluate(object)?;
                match obj {
                    Value::Struct { name: _, fields } => {
                        fields.borrow().get(member).cloned().ok_or_else(|| {
                            AgamError::runtime_error(
                                0, 0,
                                format!("புலம் '{}' கிடைக்கவில்லை", member),
                            )
                        })
                    }
                    Value::EnumDef { name: enum_name, variants } => {
                        if variants.contains(member) {
                            Ok(Value::EnumVariant {
                                enum_name,
                                variant: member.clone(),
                            })
                        } else {
                            Err(AgamError::runtime_error(
                                0, 0,
                                format!("மாறுபாடு '{}' கிடைக்கவில்லை", member),
                            ))
                        }
                    }
                    // Module namespace access: module.function
                    Value::Module { name: _, exports } => {
                        exports.borrow().get(member).cloned().ok_or_else(|| {
                            AgamError::runtime_error(
                                0, 0,
                                format!("'{}' கூறில் கிடைக்கவில்லை", member),
                            )
                        })
                    }
                    _ => Err(AgamError::runtime_error(
                        0, 0,
                        format!("'{}' வகையில் புலம் அணுக இயலாது", obj.type_name()),
                    )),
                }
            }

            Expression::StructInit { name, arguments } => {
                // Look up the struct definition
                let struct_def = self.environment
                    .borrow()
                    .get(name)
                    .ok_or_else(|| {
                        AgamError::runtime_error(
                            0, 0,
                            format!("கட்டமைப்பு '{}' வரையறுக்கப்படவில்லை", name),
                        )
                    })?;

                match struct_def {
                    Value::StructDef { name: struct_name, field_names } => {
                        if arguments.len() != field_names.len() {
                            return Err(AgamError::runtime_error(
                                0, 0,
                                format!(
                                    "கட்டமைப்பு '{}' {} புலங்களை எதிர்பார்க்கிறது, {} கொடுக்கப்பட்டது",
                                    struct_name, field_names.len(), arguments.len()
                                ),
                            ));
                        }

                        let mut fields = std::collections::HashMap::new();
                        for (field_name, arg) in field_names.iter().zip(arguments.iter()) {
                            let val = self.evaluate(arg)?;
                            fields.insert(field_name.clone(), val);
                        }

                        Ok(Value::Struct {
                            name: struct_name,
                            fields: Rc::new(RefCell::new(fields)),
                        })
                    }
                    _ => Err(AgamError::runtime_error(
                        0, 0,
                        format!("'{}' ஒரு கட்டமைப்பு அல்ல", name),
                    )),
                }
            }

            // Index assignment: list[0] = value, dict["key"] = value
            Expression::IndexAssignment { object, index, value } => {
                let obj = self.evaluate(object)?;
                let idx = self.evaluate(index)?;
                let val = self.evaluate(value)?;

                match (&obj, &idx) {
                    (Value::List(list), Value::Number(n)) => {
                        let i = *n as i64;
                        let mut list_ref = list.borrow_mut();
                        let len = list_ref.len() as i64;
                        
                        let actual_idx = if i < 0 { len + i } else { i };
                        
                        if actual_idx < 0 || actual_idx >= len {
                            return Err(AgamError::runtime_error(
                                0, 0,
                                format!("குறியீட்டு {} வரம்பிற்கு வெளியே", n),
                            ));
                        }
                        list_ref[actual_idx as usize] = val.clone();
                        Ok(val)
                    }
                    (Value::Dict(dict), _) => {
                        let key = match &idx {
                            Value::String(s) => s.clone(),
                            v => v.to_string(),
                        };
                        dict.borrow_mut().insert(key, val.clone());
                        Ok(val)
                    }
                    _ => Err(AgamError::runtime_error(
                        0, 0,
                        format!("'{}' வகையை குறியீட்டு ஒதுக்க இயலாது", obj.type_name()),
                    )),
                }
            }

            // Member assignment: struct.field = value
            Expression::MemberAssignment { object, member, value } => {
                // Evaluate the struct object - this returns a Value::Struct with Rc<RefCell> fields
                // Since fields use Rc<RefCell>, modifications will persist in the original struct
                let obj = self.evaluate(object)?;
                let val = self.evaluate(value)?;
                
                match obj {
                    Value::Struct { name: _, fields } => {
                        if !fields.borrow().contains_key(member) {
                            return Err(AgamError::runtime_error(
                                0, 0,
                                format!("புலம் '{}' கிடைக்கவில்லை", member),
                            ));
                        }
                        fields.borrow_mut().insert(member.clone(), val.clone());
                        Ok(val)
                    }
                    _ => Err(AgamError::runtime_error(
                        0, 0,
                        format!("'{}' வகையில் புலம் ஒதுக்க இயலாது", obj.type_name()),
                    )),
                }
            }

            // Lambda/anonymous function: செயலி(x): x * 2 or (x) => x * 2
            Expression::Lambda { params, body } => {
                // Convert lambda to a function value
                // We wrap the body expression in a return statement
                let body_stmt = vec![Statement::Return(Some((**body).clone()))];
                
                let func = Rc::new(AgamFunction::new(
                    "<lambda>".to_string(),
                    params.clone(),
                    body_stmt,
                    Rc::clone(&self.environment),
                ));
                
                Ok(Value::Function(func))
            }
        }
    }

    fn apply_binary_op(
        &self,
        op: &BinaryOp,
        left: &Value,
        right: &Value,
    ) -> Result<Value, AgamError> {
        match (op, left, right) {
            // Arithmetic
            (BinaryOp::Add, Value::Number(a), Value::Number(b)) => Ok(Value::Number(a + b)),
            (BinaryOp::Subtract, Value::Number(a), Value::Number(b)) => Ok(Value::Number(a - b)),
            (BinaryOp::Multiply, Value::Number(a), Value::Number(b)) => Ok(Value::Number(a * b)),
            (BinaryOp::Divide, Value::Number(a), Value::Number(b)) => {
                if *b == 0.0 {
                    return Err(AgamError::runtime_error(0, 0, "பூஜ்ஜியத்தால் வகுக்க இயலாது".to_string()));
                }
                Ok(Value::Number(a / b))
            }
            (BinaryOp::Modulo, Value::Number(a), Value::Number(b)) => {
                if *b == 0.0 {
                    return Err(AgamError::runtime_error(0, 0, "பூஜ்ஜியத்தால் வகுக்க இயலாது".to_string()));
                }
                Ok(Value::Number(a % b))
            }

            // String concatenation
            (BinaryOp::Add, Value::String(a), Value::String(b)) => {
                Ok(Value::String(format!("{}{}", a, b)))
            }
            (BinaryOp::Add, Value::String(a), b) => {
                Ok(Value::String(format!("{}{}", a, b)))
            }
            (BinaryOp::Add, a, Value::String(b)) => {
                Ok(Value::String(format!("{}{}", a, b)))
            }

            // String repetition
            (BinaryOp::Multiply, Value::String(s), Value::Number(n)) => {
                let count = *n as usize;
                Ok(Value::String(s.repeat(count)))
            }

            // Comparison
            (BinaryOp::Equal, a, b) => Ok(Value::Boolean(a == b)),
            (BinaryOp::NotEqual, a, b) => Ok(Value::Boolean(a != b)),
            (BinaryOp::Less, Value::Number(a), Value::Number(b)) => Ok(Value::Boolean(a < b)),
            (BinaryOp::Greater, Value::Number(a), Value::Number(b)) => Ok(Value::Boolean(a > b)),
            (BinaryOp::LessEqual, Value::Number(a), Value::Number(b)) => Ok(Value::Boolean(a <= b)),
            (BinaryOp::GreaterEqual, Value::Number(a), Value::Number(b)) => Ok(Value::Boolean(a >= b)),

            // String comparison
            (BinaryOp::Less, Value::String(a), Value::String(b)) => Ok(Value::Boolean(a < b)),
            (BinaryOp::Greater, Value::String(a), Value::String(b)) => Ok(Value::Boolean(a > b)),
            (BinaryOp::LessEqual, Value::String(a), Value::String(b)) => Ok(Value::Boolean(a <= b)),
            (BinaryOp::GreaterEqual, Value::String(a), Value::String(b)) => Ok(Value::Boolean(a >= b)),

            _ => Err(AgamError::runtime_error(
                0, 0,
                format!(
                    "'{}' மற்றும் '{}' வகைகளுக்கு '{}' செயல்படாது",
                    left.type_name(),
                    right.type_name(),
                    op
                ),
            )),
        }
    }

    fn call_function(&mut self, callee: Value, args: Vec<Value>) -> Result<Value, AgamError> {
        // Check recursion depth limit
        if self.call_depth >= MAX_RECURSION_DEPTH {
            return Err(AgamError::runtime_error(
                0, 0,
                format!(
                    "அதிகபட்ச மறுநிகழ்வு ஆழம் ({}) அடைந்தது - முடிவிலா மறுநிகழ்வு இருக்கலாம்",
                    MAX_RECURSION_DEPTH
                ),
            ));
        }

        match callee {
            Value::Function(func) => {
                if args.len() != func.params.len() {
                    return Err(AgamError::runtime_error(
                        0, 0,
                        format!(
                            "'{}' செயலுக்கு {} அளவுருக்கள் தேவை, {} கொடுக்கப்பட்டது",
                            func.name,
                            func.params.len(),
                            args.len()
                        ),
                    ));
                }

                // Create new environment with closure as parent
                let previous = Rc::clone(&self.environment);
                let func_env = Rc::new(RefCell::new(Environment::with_parent(Rc::clone(&func.closure))));

                // Bind parameters
                for (param, arg) in func.params.iter().zip(args) {
                    func_env.borrow_mut().define(param.clone(), arg, false);
                }

                self.environment = func_env;
                self.call_depth += 1;

                let result = self.execute_block(&func.body);

                self.call_depth -= 1;
                self.environment = previous;

                match result? {
                    ControlFlow::Return(v) => Ok(v),
                    _ => Ok(Value::Null),
                }
            }

            Value::NativeFunction(func) => {
                if let Some(arity) = func.arity {
                    if args.len() != arity {
                        return Err(AgamError::runtime_error(
                            0, 0,
                            format!(
                                "'{}' செயலுக்கு {} அளவுருக்கள் தேவை, {} கொடுக்கப்பட்டது",
                                func.name,
                                arity,
                                args.len()
                            ),
                        ));
                    }
                }

                (func.function)(&args)
                    .map_err(|msg| AgamError::runtime_error(0, 0, msg))
            }

            // Struct instantiation via constructor call: StructName(arg1, arg2, ...)
            Value::StructDef { name: struct_name, field_names } => {
                if args.len() != field_names.len() {
                    return Err(AgamError::runtime_error(
                        0, 0,
                        format!(
                            "கட்டமைப்பு '{}' {} புலங்களை எதிர்பார்க்கிறது, {} கொடுக்கப்பட்டது",
                            struct_name, field_names.len(), args.len()
                        ),
                    ));
                }

                let mut fields = std::collections::HashMap::new();
                for (field_name, arg) in field_names.iter().zip(args.into_iter()) {
                    fields.insert(field_name.clone(), arg);
                }

                Ok(Value::Struct {
                    name: struct_name,
                    fields: Rc::new(RefCell::new(fields)),
                })
            }

            _ => Err(AgamError::runtime_error(
                0, 0,
                format!("'{}' வகையை செயல்படுத்த இயலாது", callee.type_name()),
            )),
        }
    }

    fn pattern_matches(&mut self, pattern: &crate::parser::ast::Pattern, value: &Value) -> Result<bool, AgamError> {
        use crate::parser::ast::Pattern;
        
        match pattern {
            Pattern::Wildcard => Ok(true),
            
            Pattern::Literal(expr) => {
                let lit_val = self.evaluate(expr)?;
                Ok(self.values_equal(&lit_val, value))
            }
            
            Pattern::Variable(name) => {
                // Bind the value to the variable
                self.environment.borrow_mut().define(name.clone(), value.clone(), false);
                Ok(true)
            }
            
            Pattern::EnumVariant(enum_name, variant_name) => {
                match value {
                    Value::EnumVariant { enum_name: val_enum, variant: val_variant } => {
                        Ok(enum_name == val_enum && variant_name == val_variant)
                    }
                    _ => Ok(false),
                }
            }
        }
    }

    fn values_equal(&self, left: &Value, right: &Value) -> bool {
        match (left, right) {
            (Value::Number(a), Value::Number(b)) => (a - b).abs() < f64::EPSILON,
            (Value::String(a), Value::String(b)) => a == b,
            (Value::Boolean(a), Value::Boolean(b)) => a == b,
            (Value::Null, Value::Null) => true,
            (Value::EnumVariant { enum_name: e1, variant: v1 }, 
             Value::EnumVariant { enum_name: e2, variant: v2 }) => e1 == e2 && v1 == v2,
            _ => false,
        }
    }
}

impl Default for Evaluator {
    fn default() -> Self {
        Self::new()
    }
}
