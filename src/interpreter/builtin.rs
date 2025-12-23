//! Built-in functions for Agam
//! 
//! Native functions available in all programs

use crate::types::{Value, NativeFunction};
use std::io::{self, Write};
use std::rc::Rc;
use std::cell::RefCell;

/// Create all built-in functions
pub fn create_builtins() -> Vec<(String, NativeFunction)> {
    vec![
        // === Input/Output ===
        // உள்ளீடு - input
        ("உள்ளீடு".to_string(), NativeFunction::new("உள்ளீடு", Some(1), builtin_input)),
        ("input".to_string(), NativeFunction::new("input", Some(1), builtin_input)),
        
        // === Type Functions ===
        // நீளம் - len
        ("நீளம்".to_string(), NativeFunction::new("நீளம்", Some(1), builtin_len)),
        ("len".to_string(), NativeFunction::new("len", Some(1), builtin_len)),
        
        // வகை - type
        ("வகை".to_string(), NativeFunction::new("வகை", Some(1), builtin_type)),
        ("type".to_string(), NativeFunction::new("type", Some(1), builtin_type)),
        
        // === Type Conversion ===
        // எண்ணாக - int
        ("எண்ணாக".to_string(), NativeFunction::new("எண்ணாக", Some(1), builtin_int)),
        ("int".to_string(), NativeFunction::new("int", Some(1), builtin_int)),
        
        // தசமாக - float
        ("தசமாக".to_string(), NativeFunction::new("தசமாக", Some(1), builtin_float)),
        ("float".to_string(), NativeFunction::new("float", Some(1), builtin_float)),
        
        // சரமாக - str
        ("சரமாக".to_string(), NativeFunction::new("சரமாக", Some(1), builtin_str)),
        ("str".to_string(), NativeFunction::new("str", Some(1), builtin_str)),
        
        // === Collection Functions ===
        // வரம்பு - range
        ("வரம்பு".to_string(), NativeFunction::new("வரம்பு", None, builtin_range)),
        ("range".to_string(), NativeFunction::new("range", None, builtin_range)),
        
        // சேர் - append
        ("சேர்".to_string(), NativeFunction::new("சேர்", Some(2), builtin_append)),
        ("append".to_string(), NativeFunction::new("append", Some(2), builtin_append)),
        
        // நீக்கு - pop
        ("நீக்கு".to_string(), NativeFunction::new("நீக்கு", Some(1), builtin_pop)),
        ("pop".to_string(), NativeFunction::new("pop", Some(1), builtin_pop)),
        
        // === Math Functions ===
        // வர்க்கம் - sqrt
        ("வர்க்கம்".to_string(), NativeFunction::new("வர்க்கம்", Some(1), builtin_sqrt)),
        ("sqrt".to_string(), NativeFunction::new("sqrt", Some(1), builtin_sqrt)),
        
        // அடி - pow
        ("அடி".to_string(), NativeFunction::new("அடி", Some(2), builtin_pow)),
        ("pow".to_string(), NativeFunction::new("pow", Some(2), builtin_pow)),
        
        // தளம் - floor
        ("தளம்".to_string(), NativeFunction::new("தளம்", Some(1), builtin_floor)),
        ("floor".to_string(), NativeFunction::new("floor", Some(1), builtin_floor)),
        
        // கூரை - ceil
        ("கூரை".to_string(), NativeFunction::new("கூரை", Some(1), builtin_ceil)),
        ("ceil".to_string(), NativeFunction::new("ceil", Some(1), builtin_ceil)),
        
        // முழுமை - abs
        ("முழுமை".to_string(), NativeFunction::new("முழுமை", Some(1), builtin_abs)),
        ("abs".to_string(), NativeFunction::new("abs", Some(1), builtin_abs)),
        
        // குறைந்தபட்சம் - min
        ("குறைந்தபட்சம்".to_string(), NativeFunction::new("குறைந்தபட்சம்", None, builtin_min)),
        ("min".to_string(), NativeFunction::new("min", None, builtin_min)),
        
        // அதிகபட்சம் - max
        ("அதிகபட்சம்".to_string(), NativeFunction::new("அதிகபட்சம்", None, builtin_max)),
        ("max".to_string(), NativeFunction::new("max", None, builtin_max)),
        
        // தற்செயல் - random
        ("தற்செயல்".to_string(), NativeFunction::new("தற்செயல்", None, builtin_random)),
        ("random".to_string(), NativeFunction::new("random", None, builtin_random)),
        
        // கூட்டு - sum
        ("கூட்டு".to_string(), NativeFunction::new("கூட்டு", Some(1), builtin_sum)),
        ("sum".to_string(), NativeFunction::new("sum", Some(1), builtin_sum)),
        
        // === String Functions ===
        // பிரி - split
        ("பிரி".to_string(), NativeFunction::new("பிரி", Some(2), builtin_split)),
        ("split".to_string(), NativeFunction::new("split", Some(2), builtin_split)),
        
        // இணை - join
        ("இணை".to_string(), NativeFunction::new("இணை", Some(2), builtin_join)),
        ("join".to_string(), NativeFunction::new("join", Some(2), builtin_join)),
        
        // மேல் - upper
        ("மேல்".to_string(), NativeFunction::new("மேல்", Some(1), builtin_upper)),
        ("upper".to_string(), NativeFunction::new("upper", Some(1), builtin_upper)),
        
        // கீழ் - lower
        ("கீழ்".to_string(), NativeFunction::new("கீழ்", Some(1), builtin_lower)),
        ("lower".to_string(), NativeFunction::new("lower", Some(1), builtin_lower)),
        
        // ஒழுங்கு - trim
        ("ஒழுங்கு".to_string(), NativeFunction::new("ஒழுங்கு", Some(1), builtin_trim)),
        ("trim".to_string(), NativeFunction::new("trim", Some(1), builtin_trim)),
        
        // மாற்று - replace
        ("மாற்று".to_string(), NativeFunction::new("மாற்று", Some(3), builtin_replace)),
        ("replace".to_string(), NativeFunction::new("replace", Some(3), builtin_replace)),
        
        // தொடங்கு - startswith
        ("தொடங்கு".to_string(), NativeFunction::new("தொடங்கு", Some(2), builtin_startswith)),
        ("startswith".to_string(), NativeFunction::new("startswith", Some(2), builtin_startswith)),
        
        // முடிவு - endswith
        ("முடிவு".to_string(), NativeFunction::new("முடிவு", Some(2), builtin_endswith)),
        ("endswith".to_string(), NativeFunction::new("endswith", Some(2), builtin_endswith)),
        
        // உள்ளதா - contains
        ("உள்ளதா".to_string(), NativeFunction::new("உள்ளதா", Some(2), builtin_contains)),
        ("contains".to_string(), NativeFunction::new("contains", Some(2), builtin_contains)),
        
        // === List Functions ===
        // வரிசை - sort
        ("வரிசை".to_string(), NativeFunction::new("வரிசை", Some(1), builtin_sort)),
        ("sort".to_string(), NativeFunction::new("sort", Some(1), builtin_sort)),
        
        // தலைகீழ் - reverse
        ("தலைகீழ்".to_string(), NativeFunction::new("தலைகீழ்", Some(1), builtin_reverse)),
        ("reverse".to_string(), NativeFunction::new("reverse", Some(1), builtin_reverse)),
        
        // === File I/O ===
        // படி - read_file
        ("படி".to_string(), NativeFunction::new("படி", Some(1), builtin_read_file)),
        ("read_file".to_string(), NativeFunction::new("read_file", Some(1), builtin_read_file)),
        
        // எழுது - write_file
        ("எழுது".to_string(), NativeFunction::new("எழுது", Some(2), builtin_write_file)),
        ("write_file".to_string(), NativeFunction::new("write_file", Some(2), builtin_write_file)),
        
        // உள்ளது - file_exists
        ("உள்ளது".to_string(), NativeFunction::new("உள்ளது", Some(1), builtin_file_exists)),
        ("file_exists".to_string(), NativeFunction::new("file_exists", Some(1), builtin_file_exists)),
        
        // வெளியேறு - exit
        ("வெளியேறு".to_string(), NativeFunction::new("வெளியேறு", None, builtin_exit)),
        ("exit".to_string(), NativeFunction::new("exit", None, builtin_exit)),
    ]
}

// ============= Input/Output =============

fn builtin_input(args: &[Value]) -> Result<Value, String> {
    if let Some(Value::String(prompt)) = args.first() {
        print!("{}", prompt);
        io::stdout().flush().ok();
    }
    
    let mut input = String::new();
    io::stdin()
        .read_line(&mut input)
        .map_err(|e| format!("உள்ளீடு பிழை: {}", e))?;
    
    Ok(Value::String(input.trim().to_string()))
}

// ============= Type Functions =============

fn builtin_len(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(s)) => Ok(Value::Number(s.chars().count() as f64)),
        Some(Value::List(list)) => Ok(Value::Number(list.borrow().len() as f64)),
        Some(Value::Dict(dict)) => Ok(Value::Number(dict.borrow().len() as f64)),
        Some(v) => Err(format!("'{}' வகைக்கு நீளம் கணக்கிட இயலாது", v.type_name())),
        None => Err("நீளம்() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_type(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(v) => Ok(Value::String(v.type_name().to_string())),
        None => Err("வகை() ஒரு அளவுரு தேவை".to_string()),
    }
}

// ============= Type Conversion =============

fn builtin_int(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => Ok(Value::Number(n.trunc())),
        Some(Value::String(s)) => {
            s.trim()
                .parse::<f64>()
                .map(|n| Value::Number(n.trunc()))
                .map_err(|_| format!("'{}' எண்ணாக மாற்ற இயலவில்லை", s))
        }
        Some(Value::Boolean(b)) => Ok(Value::Number(if *b { 1.0 } else { 0.0 })),
        Some(v) => Err(format!("'{}' வகையை எண்ணாக மாற்ற இயலாது", v.type_name())),
        None => Err("எண்ணாக() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_float(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => Ok(Value::Number(*n)),
        Some(Value::String(s)) => {
            s.trim()
                .parse::<f64>()
                .map(Value::Number)
                .map_err(|_| format!("'{}' தசமாக மாற்ற இயலவில்லை", s))
        }
        Some(Value::Boolean(b)) => Ok(Value::Number(if *b { 1.0 } else { 0.0 })),
        Some(v) => Err(format!("'{}' வகையை தசமாக மாற்ற இயலாது", v.type_name())),
        None => Err("தசமாக() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_str(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(v) => Ok(Value::String(v.to_string())),
        None => Err("சரமாக() ஒரு அளவுரு தேவை".to_string()),
    }
}

// ============= Collection Functions =============

fn builtin_range(args: &[Value]) -> Result<Value, String> {
    let (start, end, step) = match args.len() {
        1 => {
            let end = match &args[0] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            (0, end, 1)
        }
        2 => {
            let start = match &args[0] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            let end = match &args[1] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            (start, end, 1)
        }
        3 => {
            let start = match &args[0] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            let end = match &args[1] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            let step = match &args[2] {
                Value::Number(n) => *n as i64,
                _ => return Err("வரம்பு() எண் அளவுருக்கள் தேவை".to_string()),
            };
            if step == 0 {
                return Err("படி அளவு பூஜ்ஜியமாக இருக்க முடியாது".to_string());
            }
            (start, end, step)
        }
        _ => return Err("வரம்பு() 1-3 அளவுருக்கள் எடுக்கும்".to_string()),
    };

    let mut result = Vec::new();
    let mut i = start;
    
    // Security: Limit range size to prevent memory exhaustion
    const MAX_RANGE_SIZE: usize = 1_000_000;
    
    if step > 0 {
        while i < end && result.len() < MAX_RANGE_SIZE {
            result.push(Value::Number(i as f64));
            i += step;
        }
    } else {
        while i > end && result.len() < MAX_RANGE_SIZE {
            result.push(Value::Number(i as f64));
            i += step;
        }
    }

    Ok(Value::List(Rc::new(RefCell::new(result))))
}

fn builtin_append(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("சேர்() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match &args[0] {
        Value::List(list) => {
            list.borrow_mut().push(args[1].clone());
            Ok(Value::Null)
        }
        _ => Err("முதல் அளவுரு பட்டியலாக இருக்க வேண்டும்".to_string()),
    }
}

fn builtin_pop(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::List(list)) => {
            list.borrow_mut()
                .pop()
                .ok_or_else(|| "வெற்று பட்டியலில் இருந்து நீக்க முடியாது".to_string())
        }
        Some(_) => Err("அளவுரு பட்டியலாக இருக்க வேண்டும்".to_string()),
        None => Err("நீக்கு() ஒரு அளவுரு தேவை".to_string()),
    }
}

// ============= Math Functions =============

fn builtin_sqrt(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => {
            if *n < 0.0 {
                Err("எதிர்மறை எண்ணுக்கு வர்க்கமூலம் இல்லை".to_string())
            } else {
                Ok(Value::Number(n.sqrt()))
            }
        }
        Some(v) => Err(format!("'{}' வகைக்கு வர்க்கமூலம் கணக்கிட இயலாது", v.type_name())),
        None => Err("வர்க்கம்() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_pow(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("அடி() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::Number(base), Value::Number(exp)) => Ok(Value::Number(base.powf(*exp))),
        _ => Err("அடி() எண் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_floor(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => Ok(Value::Number(n.floor())),
        Some(v) => Err(format!("'{}' வகைக்கு தளம் கணக்கிட இயலாது", v.type_name())),
        None => Err("தளம்() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_ceil(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => Ok(Value::Number(n.ceil())),
        Some(v) => Err(format!("'{}' வகைக்கு கூரை கணக்கிட இயலாது", v.type_name())),
        None => Err("கூரை() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_abs(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::Number(n)) => Ok(Value::Number(n.abs())),
        Some(v) => Err(format!("'{}' வகைக்கு முழுமை கணக்கிட இயலாது", v.type_name())),
        None => Err("முழுமை() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_min(args: &[Value]) -> Result<Value, String> {
    if args.is_empty() {
        return Err("குறைந்தபட்சம்() குறைந்தது ஒரு அளவுரு தேவை".to_string());
    }
    
    // If single list argument, find min in list
    if args.len() == 1 {
        if let Value::List(list) = &args[0] {
            let list_ref = list.borrow();
            if list_ref.is_empty() {
                return Err("வெற்று பட்டியலுக்கு குறைந்தபட்சம் இல்லை".to_string());
            }
            let mut min_val = &list_ref[0];
            for item in list_ref.iter() {
                if let (Value::Number(a), Value::Number(b)) = (item, min_val) {
                    if a < b {
                        min_val = item;
                    }
                }
            }
            return Ok(min_val.clone());
        }
    }
    
    // Multiple arguments
    let mut min_val = &args[0];
    for arg in args.iter() {
        if let (Value::Number(a), Value::Number(b)) = (arg, min_val) {
            if a < b {
                min_val = arg;
            }
        }
    }
    Ok(min_val.clone())
}

fn builtin_max(args: &[Value]) -> Result<Value, String> {
    if args.is_empty() {
        return Err("அதிகபட்சம்() குறைந்தது ஒரு அளவுரு தேவை".to_string());
    }
    
    // If single list argument, find max in list
    if args.len() == 1 {
        if let Value::List(list) = &args[0] {
            let list_ref = list.borrow();
            if list_ref.is_empty() {
                return Err("வெற்று பட்டியலுக்கு அதிகபட்சம் இல்லை".to_string());
            }
            let mut max_val = &list_ref[0];
            for item in list_ref.iter() {
                if let (Value::Number(a), Value::Number(b)) = (item, max_val) {
                    if a > b {
                        max_val = item;
                    }
                }
            }
            return Ok(max_val.clone());
        }
    }
    
    // Multiple arguments
    let mut max_val = &args[0];
    for arg in args.iter() {
        if let (Value::Number(a), Value::Number(b)) = (arg, max_val) {
            if a > b {
                max_val = arg;
            }
        }
    }
    Ok(max_val.clone())
}

fn builtin_random(args: &[Value]) -> Result<Value, String> {
    use std::time::{SystemTime, UNIX_EPOCH};
    
    let seed = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .subsec_nanos();
    
    let random = seed as f64 / u32::MAX as f64;
    
    match args.len() {
        0 => Ok(Value::Number(random)),
        1 => {
            if let Value::Number(max) = &args[0] {
                Ok(Value::Number((random * max).floor()))
            } else {
                Err("தற்செயல்() எண் அளவுரு தேவை".to_string())
            }
        }
        2 => {
            if let (Value::Number(min), Value::Number(max)) = (&args[0], &args[1]) {
                Ok(Value::Number(min + (random * (max - min)).floor()))
            } else {
                Err("தற்செயல்() எண் அளவுருக்கள் தேவை".to_string())
            }
        }
        _ => Err("தற்செயல்() 0-2 அளவுருக்கள் எடுக்கும்".to_string()),
    }
}

fn builtin_sum(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::List(list)) => {
            let mut total = 0.0;
            for item in list.borrow().iter() {
                if let Value::Number(n) = item {
                    total += n;
                } else {
                    return Err("கூட்டு() எண் பட்டியல் தேவை".to_string());
                }
            }
            Ok(Value::Number(total))
        }
        Some(v) => Err(format!("'{}' வகைக்கு கூட்டு கணக்கிட இயலாது", v.type_name())),
        None => Err("கூட்டு() ஒரு அளவுரு தேவை".to_string()),
    }
}

// ============= String Functions =============

fn builtin_split(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("பிரி() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(s), Value::String(delim)) => {
            let parts: Vec<Value> = s
                .split(delim.as_str())
                .map(|p| Value::String(p.to_string()))
                .collect();
            Ok(Value::List(Rc::new(RefCell::new(parts))))
        }
        _ => Err("பிரி() சரம் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_join(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("இணை() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(delim), Value::List(list)) => {
            let parts: Vec<String> = list
                .borrow()
                .iter()
                .map(|v| v.to_string())
                .collect();
            Ok(Value::String(parts.join(delim)))
        }
        _ => Err("இணை() சரம் மற்றும் பட்டியல் தேவை".to_string()),
    }
}

fn builtin_upper(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(s)) => Ok(Value::String(s.to_uppercase())),
        Some(v) => Err(format!("'{}' வகைக்கு மேல்() பயன்படுத்த இயலாது", v.type_name())),
        None => Err("மேல்() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_lower(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(s)) => Ok(Value::String(s.to_lowercase())),
        Some(v) => Err(format!("'{}' வகைக்கு கீழ்() பயன்படுத்த இயலாது", v.type_name())),
        None => Err("கீழ்() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_trim(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(s)) => Ok(Value::String(s.trim().to_string())),
        Some(v) => Err(format!("'{}' வகைக்கு ஒழுங்கு() பயன்படுத்த இயலாது", v.type_name())),
        None => Err("ஒழுங்கு() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_replace(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("மாற்று() மூன்று அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1], &args[2]) {
        (Value::String(s), Value::String(old), Value::String(new)) => {
            Ok(Value::String(s.replace(old.as_str(), new.as_str())))
        }
        _ => Err("மாற்று() சரம் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_startswith(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("தொடங்கு() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(s), Value::String(prefix)) => {
            Ok(Value::Boolean(s.starts_with(prefix.as_str())))
        }
        _ => Err("தொடங்கு() சரம் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_endswith(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("முடிவு() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(s), Value::String(suffix)) => {
            Ok(Value::Boolean(s.ends_with(suffix.as_str())))
        }
        _ => Err("முடிவு() சரம் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_contains(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("உள்ளதா() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(s), Value::String(sub)) => {
            Ok(Value::Boolean(s.contains(sub.as_str())))
        }
        (Value::List(list), val) => {
            Ok(Value::Boolean(list.borrow().contains(val)))
        }
        _ => Err("உள்ளதா() சரம் அல்லது பட்டியல் தேவை".to_string()),
    }
}

// ============= List Functions =============

fn builtin_sort(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::List(list)) => {
            let mut sorted = list.borrow().clone();
            sorted.sort_by(|a, b| {
                match (a, b) {
                    (Value::Number(a), Value::Number(b)) => a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal),
                    (Value::String(a), Value::String(b)) => a.cmp(b),
                    _ => std::cmp::Ordering::Equal,
                }
            });
            Ok(Value::List(Rc::new(RefCell::new(sorted))))
        }
        Some(v) => Err(format!("'{}' வகைக்கு வரிசை() பயன்படுத்த இயலாது", v.type_name())),
        None => Err("வரிசை() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_reverse(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::List(list)) => {
            let mut reversed = list.borrow().clone();
            reversed.reverse();
            Ok(Value::List(Rc::new(RefCell::new(reversed))))
        }
        Some(Value::String(s)) => {
            Ok(Value::String(s.chars().rev().collect()))
        }
        Some(v) => Err(format!("'{}' வகைக்கு தலைகீழ்() பயன்படுத்த இயலாது", v.type_name())),
        None => Err("தலைகீழ்() ஒரு அளவுரு தேவை".to_string()),
    }
}

// ============= File I/O =============

fn builtin_read_file(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(path)) => {
            // Security: Limit file size to prevent memory exhaustion
            const MAX_FILE_SIZE: u64 = 10 * 1024 * 1024; // 10 MB
            
            let metadata = std::fs::metadata(path)
                .map_err(|e| format!("கோப்பு பிழை: {}", e))?;
            
            if metadata.len() > MAX_FILE_SIZE {
                return Err("கோப்பு மிகப் பெரியது".to_string());
            }
            
            std::fs::read_to_string(path)
                .map(Value::String)
                .map_err(|e| format!("கோப்பு படிக்க இயலவில்லை: {}", e))
        }
        Some(v) => Err(format!("'{}' வகை கோப்பு பாதையாக பயன்படுத்த இயலாது", v.type_name())),
        None => Err("படி() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_write_file(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("எழுது() இரண்டு அளவுருக்கள் தேவை".to_string());
    }
    
    match (&args[0], &args[1]) {
        (Value::String(path), Value::String(content)) => {
            std::fs::write(path, content)
                .map(|_| Value::Boolean(true))
                .map_err(|e| format!("கோப்பு எழுத இயலவில்லை: {}", e))
        }
        _ => Err("எழுது() சரம் அளவுருக்கள் தேவை".to_string()),
    }
}

fn builtin_file_exists(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(path)) => {
            Ok(Value::Boolean(std::path::Path::new(path).exists()))
        }
        Some(v) => Err(format!("'{}' வகை கோப்பு பாதையாக பயன்படுத்த இயலாது", v.type_name())),
        None => Err("உள்ளது() ஒரு அளவுரு தேவை".to_string()),
    }
}

fn builtin_exit(args: &[Value]) -> Result<Value, String> {
    let code = match args.first() {
        Some(Value::Number(n)) => *n as i32,
        None => 0,
        _ => return Err("வெளியேறு() எண் அளவுரு தேவை".to_string()),
    };
    std::process::exit(code);
}
