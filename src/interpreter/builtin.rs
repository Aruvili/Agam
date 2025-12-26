//! Built-in functions for Agam
//! 
//! Native functions available in all programs

use crate::types::{Value, NativeFunction};
use std::io::{self, Write};
use std::rc::Rc;
use std::cell::RefCell;
use std::sync::Mutex;
use std::collections::HashMap;
use std::net::TcpStream;
use tungstenite::{WebSocket, Message, connect};

// Global WebSocket connections storage
lazy_static::lazy_static! {
    static ref WS_CONNECTIONS: Mutex<HashMap<u64, WebSocket<tungstenite::stream::MaybeTlsStream<TcpStream>>>> = 
        Mutex::new(HashMap::new());
    static ref WS_COUNTER: Mutex<u64> = Mutex::new(0);
}

/// Create all built-in functions
/// Returns Rc-wrapped functions for efficient sharing without cloning
pub fn create_builtins() -> Vec<(String, Rc<NativeFunction>)> {
    vec![
        // === Input/Output ===
        // உள்ளீடு - input
        ("உள்ளீடு".to_string(), Rc::new(NativeFunction::new("உள்ளீடு", Some(1), builtin_input))),
        ("input".to_string(), Rc::new(NativeFunction::new("input", Some(1), builtin_input))),
        
        // === Type Functions ===
        // நீளம் - len
        ("நீளம்".to_string(), Rc::new(NativeFunction::new("நீளம்", Some(1), builtin_len))),
        ("len".to_string(), Rc::new(NativeFunction::new("len", Some(1), builtin_len))),
        
        // வகை - type
        ("வகை".to_string(), Rc::new(NativeFunction::new("வகை", Some(1), builtin_type))),
        ("type".to_string(), Rc::new(NativeFunction::new("type", Some(1), builtin_type))),
        
        // === Type Conversion ===
        // எண்ணாக - int
        ("எண்ணாக".to_string(), Rc::new(NativeFunction::new("எண்ணாக", Some(1), builtin_int))),
        ("int".to_string(), Rc::new(NativeFunction::new("int", Some(1), builtin_int))),
        
        // தசமாக - float
        ("தசமாக".to_string(), Rc::new(NativeFunction::new("தசமாக", Some(1), builtin_float))),
        ("float".to_string(), Rc::new(NativeFunction::new("float", Some(1), builtin_float))),
        
        // சரமாக - str
        ("சரமாக".to_string(), Rc::new(NativeFunction::new("சரமாக", Some(1), builtin_str))),
        ("str".to_string(), Rc::new(NativeFunction::new("str", Some(1), builtin_str))),
        
        // === Collection Functions ===
        // வரம்பு - range
        ("வரம்பு".to_string(), Rc::new(NativeFunction::new("வரம்பு", None, builtin_range))),
        ("range".to_string(), Rc::new(NativeFunction::new("range", None, builtin_range))),
        
        // சேர் - append
        ("சேர்".to_string(), Rc::new(NativeFunction::new("சேர்", Some(2), builtin_append))),
        ("append".to_string(), Rc::new(NativeFunction::new("append", Some(2), builtin_append))),
        
        // நீக்கு - pop
        ("நீக்கு".to_string(), Rc::new(NativeFunction::new("நீக்கு", Some(1), builtin_pop))),
        ("pop".to_string(), Rc::new(NativeFunction::new("pop", Some(1), builtin_pop))),
        
        // === Math Functions ===
        // வர்க்கம் - sqrt
        ("வர்க்கம்".to_string(), Rc::new(NativeFunction::new("வர்க்கம்", Some(1), builtin_sqrt))),
        ("sqrt".to_string(), Rc::new(NativeFunction::new("sqrt", Some(1), builtin_sqrt))),
        
        // அடி - pow
        ("அடி".to_string(), Rc::new(NativeFunction::new("அடி", Some(2), builtin_pow))),
        ("pow".to_string(), Rc::new(NativeFunction::new("pow", Some(2), builtin_pow))),
        
        // தளம் - floor
        ("தளம்".to_string(), Rc::new(NativeFunction::new("தளம்", Some(1), builtin_floor))),
        ("floor".to_string(), Rc::new(NativeFunction::new("floor", Some(1), builtin_floor))),
        
        // கூரை - ceil
        ("கூரை".to_string(), Rc::new(NativeFunction::new("கூரை", Some(1), builtin_ceil))),
        ("ceil".to_string(), Rc::new(NativeFunction::new("ceil", Some(1), builtin_ceil))),
        
        // முழுமை - abs
        ("முழுமை".to_string(), Rc::new(NativeFunction::new("முழுமை", Some(1), builtin_abs))),
        ("abs".to_string(), Rc::new(NativeFunction::new("abs", Some(1), builtin_abs))),
        
        // குறைந்தபட்சம் - min
        ("குறைந்தபட்சம்".to_string(), Rc::new(NativeFunction::new("குறைந்தபட்சம்", None, builtin_min))),
        ("min".to_string(), Rc::new(NativeFunction::new("min", None, builtin_min))),
        
        // அதிகபட்சம் - max
        ("அதிகபட்சம்".to_string(), Rc::new(NativeFunction::new("அதிகபட்சம்", None, builtin_max))),
        ("max".to_string(), Rc::new(NativeFunction::new("max", None, builtin_max))),
        
        // தற்செயல் - random
        ("தற்செயல்".to_string(), Rc::new(NativeFunction::new("தற்செயல்", None, builtin_random))),
        ("random".to_string(), Rc::new(NativeFunction::new("random", None, builtin_random))),
        
        // கூட்டு - sum
        ("கூட்டு".to_string(), Rc::new(NativeFunction::new("கூட்டு", Some(1), builtin_sum))),
        ("sum".to_string(), Rc::new(NativeFunction::new("sum", Some(1), builtin_sum))),
        
        // === String Functions ===
        // பிரி - split
        ("பிரி".to_string(), Rc::new(NativeFunction::new("பிரி", Some(2), builtin_split))),
        ("split".to_string(), Rc::new(NativeFunction::new("split", Some(2), builtin_split))),
        
        // இணை - join
        ("இணை".to_string(), Rc::new(NativeFunction::new("இணை", Some(2), builtin_join))),
        ("join".to_string(), Rc::new(NativeFunction::new("join", Some(2), builtin_join))),
        
        // மேல் - upper
        ("மேல்".to_string(), Rc::new(NativeFunction::new("மேல்", Some(1), builtin_upper))),
        ("upper".to_string(), Rc::new(NativeFunction::new("upper", Some(1), builtin_upper))),
        
        // கீழ் - lower
        ("கீழ்".to_string(), Rc::new(NativeFunction::new("கீழ்", Some(1), builtin_lower))),
        ("lower".to_string(), Rc::new(NativeFunction::new("lower", Some(1), builtin_lower))),
        
        // ஒழுங்கு - trim
        ("ஒழுங்கு".to_string(), Rc::new(NativeFunction::new("ஒழுங்கு", Some(1), builtin_trim))),
        ("trim".to_string(), Rc::new(NativeFunction::new("trim", Some(1), builtin_trim))),
        
        // மாற்று - replace
        ("மாற்று".to_string(), Rc::new(NativeFunction::new("மாற்று", Some(3), builtin_replace))),
        ("replace".to_string(), Rc::new(NativeFunction::new("replace", Some(3), builtin_replace))),
        
        // தொடங்கு - startswith
        ("தொடங்கு".to_string(), Rc::new(NativeFunction::new("தொடங்கு", Some(2), builtin_startswith))),
        ("startswith".to_string(), Rc::new(NativeFunction::new("startswith", Some(2), builtin_startswith))),
        
        // முடிவு - endswith
        ("முடிவு".to_string(), Rc::new(NativeFunction::new("முடிவு", Some(2), builtin_endswith))),
        ("endswith".to_string(), Rc::new(NativeFunction::new("endswith", Some(2), builtin_endswith))),
        
        // உள்ளதா - contains
        ("உள்ளதா".to_string(), Rc::new(NativeFunction::new("உள்ளதா", Some(2), builtin_contains))),
        ("contains".to_string(), Rc::new(NativeFunction::new("contains", Some(2), builtin_contains))),
        
        // === List Functions ===
        // வரிசை - sort
        ("வரிசை".to_string(), Rc::new(NativeFunction::new("வரிசை", Some(1), builtin_sort))),
        ("sort".to_string(), Rc::new(NativeFunction::new("sort", Some(1), builtin_sort))),
        
        // தலைகீழ் - reverse
        ("தலைகீழ்".to_string(), Rc::new(NativeFunction::new("தலைகீழ்", Some(1), builtin_reverse))),
        ("reverse".to_string(), Rc::new(NativeFunction::new("reverse", Some(1), builtin_reverse))),
        
        // === File I/O ===
        // படி - read_file
        ("படி".to_string(), Rc::new(NativeFunction::new("படி", Some(1), builtin_read_file))),
        ("read_file".to_string(), Rc::new(NativeFunction::new("read_file", Some(1), builtin_read_file))),
        
        // எழுது - write_file
        ("எழுது".to_string(), Rc::new(NativeFunction::new("எழுது", Some(2), builtin_write_file))),
        ("write_file".to_string(), Rc::new(NativeFunction::new("write_file", Some(2), builtin_write_file))),
        
        // உள்ளது - file_exists
        ("உள்ளது".to_string(), Rc::new(NativeFunction::new("உள்ளது", Some(1), builtin_file_exists))),
        ("file_exists".to_string(), Rc::new(NativeFunction::new("file_exists", Some(1), builtin_file_exists))),
        
        // வெளியேறு - exit
        ("வெளியேறு".to_string(), Rc::new(NativeFunction::new("வெளியேறு", None, builtin_exit))),
        ("exit".to_string(), Rc::new(NativeFunction::new("exit", None, builtin_exit))),
        
        // === Time Module ===
        // நேரம் - time (unix timestamp)
        ("நேரம்".to_string(), Rc::new(NativeFunction::new("நேரம்", Some(0), builtin_time))),
        ("time".to_string(), Rc::new(NativeFunction::new("time", Some(0), builtin_time))),
        
        // தூக்கம் - sleep (pause execution)
        ("தூக்கம்".to_string(), Rc::new(NativeFunction::new("தூக்கம்", Some(1), builtin_sleep))),
        ("sleep".to_string(), Rc::new(NativeFunction::new("sleep", Some(1), builtin_sleep))),
        
        // தேதி - date (formatted date string)
        ("தேதி".to_string(), Rc::new(NativeFunction::new("தேதி", None, builtin_date))),
        ("date".to_string(), Rc::new(NativeFunction::new("date", None, builtin_date))),
        
        // நாள் - now (current time components)
        ("நாள்".to_string(), Rc::new(NativeFunction::new("நாள்", Some(0), builtin_now))),
        ("now".to_string(), Rc::new(NativeFunction::new("now", Some(0), builtin_now))),
        
        // === HTTP Module ===
        // வலை_படி - http_get (GET request)
        ("வலை_படி".to_string(), Rc::new(NativeFunction::new("வலை_படி", Some(1), builtin_http_get))),
        ("http_get".to_string(), Rc::new(NativeFunction::new("http_get", Some(1), builtin_http_get))),
        
        // வலை_அனுப்பு - http_post (POST request)
        ("வலை_அனுப்பு".to_string(), Rc::new(NativeFunction::new("வலை_அனுப்பு", Some(2), builtin_http_post))),
        ("http_post".to_string(), Rc::new(NativeFunction::new("http_post", Some(2), builtin_http_post))),
        
        // வலை_புதுப்பி - http_put (PUT request)
        ("வலை_புதுப்பி".to_string(), Rc::new(NativeFunction::new("வலை_புதுப்பி", Some(2), builtin_http_put))),
        ("http_put".to_string(), Rc::new(NativeFunction::new("http_put", Some(2), builtin_http_put))),
        
        // வலை_நீக்கு - http_delete (DELETE request)
        ("வலை_நீக்கு".to_string(), Rc::new(NativeFunction::new("வலை_நீக்கு", Some(1), builtin_http_delete))),
        ("http_delete".to_string(), Rc::new(NativeFunction::new("http_delete", Some(1), builtin_http_delete))),
        
        // கோப்பு_பதிவேற்று - file_upload (upload file via HTTP)
        ("கோப்பு_பதிவேற்று".to_string(), Rc::new(NativeFunction::new("கோப்பு_பதிவேற்று", Some(2), builtin_file_upload))),
        ("file_upload".to_string(), Rc::new(NativeFunction::new("file_upload", Some(2), builtin_file_upload))),
        
        // வலை_கோரிக்கை - http_request (custom HTTP request with headers)
        ("வலை_கோரிக்கை".to_string(), Rc::new(NativeFunction::new("வலை_கோரிக்கை", None, builtin_http_request))),
        ("http_request".to_string(), Rc::new(NativeFunction::new("http_request", None, builtin_http_request))),
        
        // === WebSocket Module ===
        // சாக்கெட்_இணை - ws_connect (connect to WebSocket)
        ("சாக்கெட்_இணை".to_string(), Rc::new(NativeFunction::new("சாக்கெட்_இணை", Some(1), builtin_ws_connect))),
        ("ws_connect".to_string(), Rc::new(NativeFunction::new("ws_connect", Some(1), builtin_ws_connect))),
        
        // சாக்கெட்_அனுப்பு - ws_send (send message)
        ("சாக்கெட்_அனுப்பு".to_string(), Rc::new(NativeFunction::new("சாக்கெட்_அனுப்பு", Some(2), builtin_ws_send))),
        ("ws_send".to_string(), Rc::new(NativeFunction::new("ws_send", Some(2), builtin_ws_send))),
        
        // சாக்கெட்_படி - ws_receive (receive message)
        ("சாக்கெட்_படி".to_string(), Rc::new(NativeFunction::new("சாக்கெட்_படி", Some(1), builtin_ws_receive))),
        ("ws_receive".to_string(), Rc::new(NativeFunction::new("ws_receive", Some(1), builtin_ws_receive))),
        
        // சாக்கெட்_மூடு - ws_close (close connection)
        ("சாக்கெட்_மூடு".to_string(), Rc::new(NativeFunction::new("சாக்கெட்_மூடு", Some(1), builtin_ws_close))),
        ("ws_close".to_string(), Rc::new(NativeFunction::new("ws_close", Some(1), builtin_ws_close))),
        
        // JSON பகுப்பு - json_parse
        ("json_படி".to_string(), Rc::new(NativeFunction::new("json_படி", Some(1), builtin_json_parse))),
        ("json_parse".to_string(), Rc::new(NativeFunction::new("json_parse", Some(1), builtin_json_parse))),
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

// ============= Time Module =============

/// Get current Unix timestamp in seconds
fn builtin_time(_args: &[Value]) -> Result<Value, String> {
    use std::time::{SystemTime, UNIX_EPOCH};
    
    let duration = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| format!("நேர பிழை: {}", e))?;
    
    Ok(Value::Number(duration.as_secs_f64()))
}

/// Sleep for specified seconds
fn builtin_sleep(args: &[Value]) -> Result<Value, String> {
    use std::thread;
    use std::time::Duration;
    
    match args.first() {
        Some(Value::Number(secs)) => {
            if *secs < 0.0 {
                return Err("தூக்கம்() நேர்மறை எண் தேவை".to_string());
            }
            let millis = (*secs * 1000.0) as u64;
            thread::sleep(Duration::from_millis(millis));
            Ok(Value::Null)
        }
        Some(v) => Err(format!("'{}' வகை தூக்கம்() க்கு பயன்படுத்த இயலாது", v.type_name())),
        None => Err("தூக்கம்() ஒரு அளவுரு தேவை".to_string()),
    }
}

/// Get formatted date string
fn builtin_date(args: &[Value]) -> Result<Value, String> {
    use std::time::{SystemTime, UNIX_EPOCH};
    
    let timestamp = match args.first() {
        Some(Value::Number(ts)) => *ts as u64,
        None => {
            SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map_err(|e| format!("நேர பிழை: {}", e))?
                .as_secs()
        }
        Some(v) => return Err(format!("'{}' வகை தேதி() க்கு பயன்படுத்த இயலாது", v.type_name())),
    };
    
    // Convert to date components manually (simple implementation)
    let secs_per_day = 86400u64;
    let secs_per_hour = 3600u64;
    let secs_per_min = 60u64;
    
    let days_since_epoch = timestamp / secs_per_day;
    let time_of_day = timestamp % secs_per_day;
    
    let hours = time_of_day / secs_per_hour;
    let minutes = (time_of_day % secs_per_hour) / secs_per_min;
    let seconds = time_of_day % secs_per_min;
    
    // Calculate year/month/day (simplified - doesn't account for all leap years perfectly)
    let mut year = 1970i64;
    let mut remaining_days = days_since_epoch as i64;
    
    loop {
        let days_in_year = if year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) { 366 } else { 365 };
        if remaining_days < days_in_year {
            break;
        }
        remaining_days -= days_in_year;
        year += 1;
    }
    
    let is_leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    let days_in_months = [31, if is_leap { 29 } else { 28 }, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    
    let mut month = 1;
    for days in days_in_months {
        if remaining_days < days {
            break;
        }
        remaining_days -= days;
        month += 1;
    }
    let day = remaining_days + 1;
    
    Ok(Value::String(format!(
        "{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
        year, month, day, hours, minutes, seconds
    )))
}

/// Get current time as a dictionary with components
fn builtin_now(_args: &[Value]) -> Result<Value, String> {
    use std::time::{SystemTime, UNIX_EPOCH};
    use std::collections::HashMap;
    
    let timestamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| format!("நேர பிழை: {}", e))?
        .as_secs();
    
    let secs_per_day = 86400u64;
    let secs_per_hour = 3600u64;
    let secs_per_min = 60u64;
    
    let days_since_epoch = timestamp / secs_per_day;
    let time_of_day = timestamp % secs_per_day;
    
    let hours = time_of_day / secs_per_hour;
    let minutes = (time_of_day % secs_per_hour) / secs_per_min;
    let seconds = time_of_day % secs_per_min;
    
    // Calculate year/month/day
    let mut year = 1970i64;
    let mut remaining_days = days_since_epoch as i64;
    
    loop {
        let days_in_year = if year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) { 366 } else { 365 };
        if remaining_days < days_in_year {
            break;
        }
        remaining_days -= days_in_year;
        year += 1;
    }
    
    let is_leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    let days_in_months = [31, if is_leap { 29 } else { 28 }, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    
    let mut month = 1;
    for days in days_in_months {
        if remaining_days < days {
            break;
        }
        remaining_days -= days;
        month += 1;
    }
    let day = remaining_days + 1;
    
    // Create dictionary with time components
    let mut map = HashMap::new();
    map.insert("ஆண்டு".to_string(), Value::Number(year as f64));
    map.insert("year".to_string(), Value::Number(year as f64));
    map.insert("மாதம்".to_string(), Value::Number(month as f64));
    map.insert("month".to_string(), Value::Number(month as f64));
    map.insert("நாள்".to_string(), Value::Number(day as f64));
    map.insert("day".to_string(), Value::Number(day as f64));
    map.insert("மணி".to_string(), Value::Number(hours as f64));
    map.insert("hour".to_string(), Value::Number(hours as f64));
    map.insert("நிமிடம்".to_string(), Value::Number(minutes as f64));
    map.insert("minute".to_string(), Value::Number(minutes as f64));
    map.insert("வினாடி".to_string(), Value::Number(seconds as f64));
    map.insert("second".to_string(), Value::Number(seconds as f64));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

// ============= HTTP Module =============

/// HTTP GET request
fn builtin_http_get(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(url)) => {
            let response = ureq::get(url)
                .call()
                .map_err(|e| format!("HTTP பிழை: {}", e))?;
            
            let status = response.status();
            let body = response.into_string()
                .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
            
            // Return a dictionary with status and body
            let mut map = std::collections::HashMap::new();
            map.insert("நிலை".to_string(), Value::Number(status as f64));
            map.insert("status".to_string(), Value::Number(status as f64));
            map.insert("உடல்".to_string(), Value::String(body.clone()));
            map.insert("body".to_string(), Value::String(body));
            
            Ok(Value::Dict(Rc::new(RefCell::new(map))))
        }
        Some(v) => Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => Err("வலை_படி() URL தேவை".to_string()),
    }
}

/// HTTP POST request
fn builtin_http_post(args: &[Value]) -> Result<Value, String> {
    let url = match args.first() {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("வலை_அனுப்பு() URL தேவை".to_string()),
    };
    
    let body = match args.get(1) {
        Some(Value::String(s)) => s.clone(),
        Some(Value::Dict(d)) => {
            // Convert dict to JSON-like string
            let d = d.borrow();
            let pairs: Vec<String> = d.iter()
                .map(|(k, v)| format!("\"{}\": {}", k, value_to_json(v)))
                .collect();
            format!("{{{}}}", pairs.join(", "))
        }
        Some(v) => return Err(format!("'{}' வகை POST body ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("வலை_அனுப்பு() body தேவை".to_string()),
    };
    
    let response = ureq::post(&url)
        .set("Content-Type", "application/json")
        .send_string(&body)
        .map_err(|e| format!("HTTP பிழை: {}", e))?;
    
    let status = response.status();
    let response_body = response.into_string()
        .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
    
    let mut map = std::collections::HashMap::new();
    map.insert("நிலை".to_string(), Value::Number(status as f64));
    map.insert("status".to_string(), Value::Number(status as f64));
    map.insert("உடல்".to_string(), Value::String(response_body.clone()));
    map.insert("body".to_string(), Value::String(response_body));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

/// HTTP PUT request
fn builtin_http_put(args: &[Value]) -> Result<Value, String> {
    let url = match args.first() {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("வலை_புதுப்பி() URL தேவை".to_string()),
    };
    
    let body = match args.get(1) {
        Some(Value::String(s)) => s.clone(),
        Some(Value::Dict(d)) => {
            let d = d.borrow();
            let pairs: Vec<String> = d.iter()
                .map(|(k, v)| format!("\"{}\": {}", k, value_to_json(v)))
                .collect();
            format!("{{{}}}", pairs.join(", "))
        }
        Some(v) => return Err(format!("'{}' வகை PUT body ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("வலை_புதுப்பி() body தேவை".to_string()),
    };
    
    let response = ureq::put(&url)
        .set("Content-Type", "application/json")
        .send_string(&body)
        .map_err(|e| format!("HTTP பிழை: {}", e))?;
    
    let status = response.status();
    let response_body = response.into_string()
        .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
    
    let mut map = std::collections::HashMap::new();
    map.insert("நிலை".to_string(), Value::Number(status as f64));
    map.insert("status".to_string(), Value::Number(status as f64));
    map.insert("உடல்".to_string(), Value::String(response_body.clone()));
    map.insert("body".to_string(), Value::String(response_body));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

/// HTTP DELETE request
fn builtin_http_delete(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(url)) => {
            let response = ureq::delete(url)
                .call()
                .map_err(|e| format!("HTTP பிழை: {}", e))?;
            
            let status = response.status();
            let body = response.into_string()
                .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
            
            let mut map = std::collections::HashMap::new();
            map.insert("நிலை".to_string(), Value::Number(status as f64));
            map.insert("status".to_string(), Value::Number(status as f64));
            map.insert("உடல்".to_string(), Value::String(body.clone()));
            map.insert("body".to_string(), Value::String(body));
            
            Ok(Value::Dict(Rc::new(RefCell::new(map))))
        }
        Some(v) => Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => Err("வலை_நீக்கு() URL தேவை".to_string()),
    }
}

/// Upload a file via HTTP POST multipart/form-data
fn builtin_file_upload(args: &[Value]) -> Result<Value, String> {
    let url = match args.first() {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("கோப்பு_பதிவேற்று() URL தேவை".to_string()),
    };
    
    let file_path = match args.get(1) {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை கோப்பு பாதையாக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("கோப்பு_பதிவேற்று() கோப்பு பாதை தேவை".to_string()),
    };
    
    // Read file content
    let file_content = std::fs::read(&file_path)
        .map_err(|e| format!("கோப்பு படிக்க இயலவில்லை: {}", e))?;
    
    // Get filename from path
    let filename = std::path::Path::new(&file_path)
        .file_name()
        .and_then(|n| n.to_str())
        .unwrap_or("file")
        .to_string();
    
    // Detect content type based on extension
    let content_type = match std::path::Path::new(&file_path)
        .extension()
        .and_then(|e| e.to_str())
        .unwrap_or("")
    {
        "txt" => "text/plain",
        "html" | "htm" => "text/html",
        "css" => "text/css",
        "js" => "application/javascript",
        "json" => "application/json",
        "xml" => "application/xml",
        "jpg" | "jpeg" => "image/jpeg",
        "png" => "image/png",
        "gif" => "image/gif",
        "pdf" => "application/pdf",
        "zip" => "application/zip",
        _ => "application/octet-stream",
    };
    
    // Create multipart boundary
    let boundary = format!("----AgamUpload{}", std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_millis())
        .unwrap_or(0));
    
    // Build multipart body
    let mut body = Vec::new();
    body.extend_from_slice(format!("--{}\r\n", boundary).as_bytes());
    body.extend_from_slice(format!(
        "Content-Disposition: form-data; name=\"file\"; filename=\"{}\"\r\n",
        filename
    ).as_bytes());
    body.extend_from_slice(format!("Content-Type: {}\r\n\r\n", content_type).as_bytes());
    body.extend_from_slice(&file_content);
    body.extend_from_slice(format!("\r\n--{}--\r\n", boundary).as_bytes());
    
    // Send request
    let response = ureq::post(&url)
        .set("Content-Type", &format!("multipart/form-data; boundary={}", boundary))
        .send_bytes(&body)
        .map_err(|e| format!("HTTP பிழை: {}", e))?;
    
    let status = response.status();
    let response_body = response.into_string()
        .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
    
    let mut map = std::collections::HashMap::new();
    map.insert("நிலை".to_string(), Value::Number(status as f64));
    map.insert("status".to_string(), Value::Number(status as f64));
    map.insert("உடல்".to_string(), Value::String(response_body.clone()));
    map.insert("body".to_string(), Value::String(response_body));
    map.insert("கோப்பு".to_string(), Value::String(filename.clone()));
    map.insert("filename".to_string(), Value::String(filename));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

/// Flexible HTTP request with custom headers
/// Usage: http_request({"url": "...", "method": "GET|POST|PUT|DELETE", "headers": {...}, "body": "..."})
fn builtin_http_request(args: &[Value]) -> Result<Value, String> {
    let config = match args.first() {
        Some(Value::Dict(d)) => d.borrow(),
        Some(v) => return Err(format!("'{}' வகை config ஆக பயன்படுத்த இயலாது - dictionary தேவை", v.type_name())),
        None => return Err("வலை_கோரிக்கை() config dictionary தேவை".to_string()),
    };
    
    // Get URL (required)
    let url = match config.get("url") {
        Some(Value::String(s)) => s.clone(),
        _ => return Err("வலை_கோரிக்கை() 'url' தேவை".to_string()),
    };
    
    // Get method (default: GET)
    let method = match config.get("method") {
        Some(Value::String(s)) => s.to_uppercase(),
        _ => "GET".to_string(),
    };
    
    // Get body (optional)
    let body = match config.get("body") {
        Some(Value::String(s)) => Some(s.clone()),
        Some(Value::Dict(d)) => {
            let d = d.borrow();
            let pairs: Vec<String> = d.iter()
                .map(|(k, v)| format!("\"{}\": {}", k, value_to_json(v)))
                .collect();
            Some(format!("{{{}}}", pairs.join(", ")))
        }
        _ => None,
    };
    
    // Get headers (optional)
    let headers: Vec<(String, String)> = match config.get("headers") {
        Some(Value::Dict(d)) => {
            d.borrow().iter()
                .filter_map(|(k, v)| {
                    if let Value::String(val) = v {
                        Some((k.clone(), val.clone()))
                    } else {
                        None
                    }
                })
                .collect()
        }
        _ => Vec::new(),
    };
    
    // Build request
    let mut request = match method.as_str() {
        "GET" => ureq::get(&url),
        "POST" => ureq::post(&url),
        "PUT" => ureq::put(&url),
        "DELETE" => ureq::delete(&url),
        "PATCH" => ureq::patch(&url),
        "HEAD" => ureq::head(&url),
        _ => return Err(format!("தெரியாத HTTP method: {}", method)),
    };
    
    // Add headers
    for (key, value) in &headers {
        request = request.set(key, value);
    }
    
    // Send request
    let response = if let Some(body_content) = body {
        request.send_string(&body_content)
    } else {
        request.call()
    }.map_err(|e| format!("HTTP பிழை: {}", e))?;
    
    let status = response.status();
    let response_body = response.into_string()
        .map_err(|e| format!("பதில் படிக்க இயலவில்லை: {}", e))?;
    let mut map = std::collections::HashMap::new();
    map.insert("நிலை".to_string(), Value::Number(status as f64));
    map.insert("status".to_string(), Value::Number(status as f64));
    map.insert("உடல்".to_string(), Value::String(response_body.clone()));
    map.insert("body".to_string(), Value::String(response_body));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

// ============= WebSocket Module =============

/// Connect to a WebSocket server
fn builtin_ws_connect(args: &[Value]) -> Result<Value, String> {
    let url = match args.first() {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை URL ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("சாக்கெட்_இணை() URL தேவை".to_string()),
    };
    
    // Connect to WebSocket
    let (socket, response) = connect(&url)
        .map_err(|e| format!("WebSocket இணைப்பு பிழை: {}", e))?;
    
    // Generate connection ID
    let conn_id = {
        let mut counter = WS_COUNTER.lock().map_err(|_| "Lock error")?;
        *counter += 1;
        *counter
    };
    
    // Store connection
    {
        let mut connections = WS_CONNECTIONS.lock().map_err(|_| "Lock error")?;
        connections.insert(conn_id, socket);
    }
    
    // Return connection info
    let mut map = std::collections::HashMap::new();
    map.insert("id".to_string(), Value::Number(conn_id as f64));
    map.insert("அடையாளம்".to_string(), Value::Number(conn_id as f64));
    map.insert("status".to_string(), Value::Number(response.status().as_u16() as f64));
    map.insert("நிலை".to_string(), Value::Number(response.status().as_u16() as f64));
    map.insert("connected".to_string(), Value::Boolean(true));
    map.insert("இணைந்தது".to_string(), Value::Boolean(true));
    
    Ok(Value::Dict(Rc::new(RefCell::new(map))))
}

/// Send a message through WebSocket
fn builtin_ws_send(args: &[Value]) -> Result<Value, String> {
    let conn_id = match args.first() {
        Some(Value::Number(n)) => *n as u64,
        Some(Value::Dict(d)) => {
            match d.borrow().get("id") {
                Some(Value::Number(n)) => *n as u64,
                _ => return Err("connection dictionary 'id' தேவை".to_string()),
            }
        }
        Some(v) => return Err(format!("'{}' வகை connection ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("சாக்கெட்_அனுப்பு() connection தேவை".to_string()),
    };
    
    let message = match args.get(1) {
        Some(Value::String(s)) => s.clone(),
        Some(v) => return Err(format!("'{}' வகை செய்தியாக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("சாக்கெட்_அனுப்பு() செய்தி தேவை".to_string()),
    };
    
    // Send message
    {
        let mut connections = WS_CONNECTIONS.lock().map_err(|_| "Lock error")?;
        if let Some(socket) = connections.get_mut(&conn_id) {
            socket.send(Message::Text(message))
                .map_err(|e| format!("WebSocket அனுப்பு பிழை: {}", e))?;
        } else {
            return Err(format!("WebSocket connection {} கிடைக்கவில்லை", conn_id));
        }
    }
    
    Ok(Value::Boolean(true))
}

/// Receive a message from WebSocket
fn builtin_ws_receive(args: &[Value]) -> Result<Value, String> {
    let conn_id = match args.first() {
        Some(Value::Number(n)) => *n as u64,
        Some(Value::Dict(d)) => {
            match d.borrow().get("id") {
                Some(Value::Number(n)) => *n as u64,
                _ => return Err("connection dictionary 'id' தேவை".to_string()),
            }
        }
        Some(v) => return Err(format!("'{}' வகை connection ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("சாக்கெட்_படி() connection தேவை".to_string()),
    };
    
    // Receive message
    let message = {
        let mut connections = WS_CONNECTIONS.lock().map_err(|_| "Lock error")?;
        if let Some(socket) = connections.get_mut(&conn_id) {
            match socket.read() {
                Ok(msg) => match msg {
                    Message::Text(text) => Some(Value::String(text)),
                    Message::Binary(data) => Some(Value::String(format!("[binary: {} bytes]", data.len()))),
                    Message::Ping(_) | Message::Pong(_) => Some(Value::Null),
                    Message::Close(_) => {
                        return Ok(Value::Dict(Rc::new(RefCell::new({
                            let mut map = std::collections::HashMap::new();
                            map.insert("closed".to_string(), Value::Boolean(true));
                            map.insert("மூடப்பட்டது".to_string(), Value::Boolean(true));
                            map
                        }))));
                    }
                    _ => Some(Value::Null),
                },
                Err(e) => return Err(format!("WebSocket படிக்க பிழை: {}", e)),
            }
        } else {
            return Err(format!("WebSocket connection {} கிடைக்கவில்லை", conn_id));
        }
    };
    
    Ok(message.unwrap_or(Value::Null))
}

/// Close a WebSocket connection
fn builtin_ws_close(args: &[Value]) -> Result<Value, String> {
    let conn_id = match args.first() {
        Some(Value::Number(n)) => *n as u64,
        Some(Value::Dict(d)) => {
            match d.borrow().get("id") {
                Some(Value::Number(n)) => *n as u64,
                _ => return Err("connection dictionary 'id' தேவை".to_string()),
            }
        }
        Some(v) => return Err(format!("'{}' வகை connection ஆக பயன்படுத்த இயலாது", v.type_name())),
        None => return Err("சாக்கெட்_மூடு() connection தேவை".to_string()),
    };
    
    // Close and remove connection
    {
        let mut connections = WS_CONNECTIONS.lock().map_err(|_| "Lock error")?;
        if let Some(mut socket) = connections.remove(&conn_id) {
            let _ = socket.close(None);
        } else {
            return Err(format!("WebSocket connection {} கிடைக்கவில்லை", conn_id));
        }
    }
    
    Ok(Value::Boolean(true))
}

/// Convert Value to JSON string representation
fn value_to_json(value: &Value) -> String {
    match value {
        Value::Number(n) => n.to_string(),
        Value::String(s) => format!("\"{}\"", s.replace('\"', "\\\"")),
        Value::Boolean(b) => b.to_string(),
        Value::Null => "null".to_string(),
        Value::List(l) => {
            let items: Vec<String> = l.borrow().iter().map(value_to_json).collect();
            format!("[{}]", items.join(", "))
        }
        Value::Dict(d) => {
            let pairs: Vec<String> = d.borrow().iter()
                .map(|(k, v)| format!("\"{}\": {}", k, value_to_json(v)))
                .collect();
            format!("{{{}}}", pairs.join(", "))
        }
        _ => "null".to_string(),
    }
}

/// Parse JSON string into Value
fn builtin_json_parse(args: &[Value]) -> Result<Value, String> {
    match args.first() {
        Some(Value::String(json_str)) => {
            parse_json_value(json_str.trim())
        }
        Some(v) => Err(format!("'{}' வகை JSON ஆக பகுக்க இயலாது", v.type_name())),
        None => Err("json_படி() JSON சரம் தேவை".to_string()),
    }
}

/// Simple JSON parser
fn parse_json_value(s: &str) -> Result<Value, String> {
    let s = s.trim();
    
    if s.is_empty() {
        return Err("வெற்று JSON".to_string());
    }
    
    // Null
    if s == "null" {
        return Ok(Value::Null);
    }
    
    // Boolean
    if s == "true" {
        return Ok(Value::Boolean(true));
    }
    if s == "false" {
        return Ok(Value::Boolean(false));
    }
    
    // Number
    if let Ok(n) = s.parse::<f64>() {
        return Ok(Value::Number(n));
    }
    
    // String
    if s.starts_with('"') && s.ends_with('"') && s.len() >= 2 {
        let inner = &s[1..s.len()-1];
        return Ok(Value::String(inner.replace("\\\"", "\"").replace("\\n", "\n")));
    }
    
    // Array
    if s.starts_with('[') && s.ends_with(']') {
        let inner = &s[1..s.len()-1].trim();
        if inner.is_empty() {
            return Ok(Value::List(Rc::new(RefCell::new(Vec::new()))));
        }
        
        let elements = split_json_array(inner)?;
        let mut list = Vec::new();
        for elem in elements {
            list.push(parse_json_value(&elem)?);
        }
        return Ok(Value::List(Rc::new(RefCell::new(list))));
    }
    
    // Object
    if s.starts_with('{') && s.ends_with('}') {
        let inner = &s[1..s.len()-1].trim();
        if inner.is_empty() {
            return Ok(Value::Dict(Rc::new(RefCell::new(std::collections::HashMap::new()))));
        }
        
        let pairs = split_json_object(inner)?;
        let mut map = std::collections::HashMap::new();
        for (key, value) in pairs {
            map.insert(key, parse_json_value(&value)?);
        }
        return Ok(Value::Dict(Rc::new(RefCell::new(map))));
    }
    
    Err(format!("தவறான JSON: {}", s))
}

/// Split JSON array elements
fn split_json_array(s: &str) -> Result<Vec<String>, String> {
    let mut elements = Vec::new();
    let mut current = String::new();
    let mut depth = 0;
    let mut in_string = false;
    let mut escape = false;
    
    for c in s.chars() {
        if escape {
            current.push(c);
            escape = false;
            continue;
        }
        
        if c == '\\' && in_string {
            current.push(c);
            escape = true;
            continue;
        }
        
        if c == '"' {
            in_string = !in_string;
            current.push(c);
            continue;
        }
        
        if !in_string {
            match c {
                '[' | '{' => {
                    depth += 1;
                    current.push(c);
                }
                ']' | '}' => {
                    depth -= 1;
                    current.push(c);
                }
                ',' if depth == 0 => {
                    elements.push(current.trim().to_string());
                    current.clear();
                }
                _ => current.push(c),
            }
        } else {
            current.push(c);
        }
    }
    
    if !current.trim().is_empty() {
        elements.push(current.trim().to_string());
    }
    
    Ok(elements)
}

/// Split JSON object into key-value pairs
fn split_json_object(s: &str) -> Result<Vec<(String, String)>, String> {
    let elements = split_json_array(s)?;
    let mut pairs = Vec::new();
    
    for elem in elements {
        if let Some(colon_pos) = find_colon_outside_string(&elem) {
            let key = elem[..colon_pos].trim();
            let value = elem[colon_pos + 1..].trim();
            
            // Remove quotes from key
            let key = if key.starts_with('"') && key.ends_with('"') && key.len() >= 2 {
                key[1..key.len()-1].to_string()
            } else {
                key.to_string()
            };
            
            pairs.push((key, value.to_string()));
        }
    }
    
    Ok(pairs)
}

/// Find colon position outside of strings
fn find_colon_outside_string(s: &str) -> Option<usize> {
    let mut in_string = false;
    let mut escape = false;
    
    for (i, c) in s.chars().enumerate() {
        if escape {
            escape = false;
            continue;
        }
        if c == '\\' && in_string {
            escape = true;
            continue;
        }
        if c == '"' {
            in_string = !in_string;
            continue;
        }
        if c == ':' && !in_string {
            return Some(i);
        }
    }
    None
}
