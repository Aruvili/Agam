//! Agam - Tamil Programming Language
//! 
//! அகம் - தமிழ் நிரல் மொழி
//! 
//! A Tamil-first programming language with Python-like syntax

use std::env;
use std::fs;
use std::io::{self, Write};
use std::process;

use agam::{Lexer, Parser, Interpreter};
use agam::error::{AgamError, format_error};

const VERSION: &str = "0.1.0";
const WELCOME_MESSAGE: &str = r#"
╔══════════════════════════════════════════════════════════════╗
║     அகம் - Agam Programming Language v0.1.0                 ║
║     தமிழில் நிரலாக்கம் செய்யுங்கள்!                           ║
║                                                              ║
║     உதவி: help() அல்லது உதவி()                              ║
║     வெளியேற: exit() அல்லது வெளியேறு()                        ║
╚══════════════════════════════════════════════════════════════╝
"#;

fn main() {
    let args: Vec<String> = env::args().collect();

    match args.len() {
        1 => run_repl(),
        2 => {
            let arg = &args[1];
            match arg.as_str() {
                "-h" | "--help" | "--உதவி" => print_help(),
                "-v" | "--version" | "--பதிப்பு" => print_version(),
                _ => run_file(arg),
            }
        }
        _ => {
            eprintln!("பயன்பாடு: agam [கோப்பு.agam]");
            eprintln!("         agam --help");
            process::exit(1);
        }
    }
}

fn print_help() {
    println!(r#"
அகம் (Agam) - Tamil Programming Language
=========================================

பயன்பாடு:
    agam                        REPL முறையில் தொடங்கு
    agam <கோப்பு>               கோப்பை இயக்கு
    agam --help                 உதவி காட்டு
    agam --version              பதிப்பு காட்டு

எடுத்துக்காட்டுகள்:
    agam hello.agam             hello.agam கோப்பை இயக்கு
    agam                        ஊடாடும் முறையில் நிரலாக்கம்

முக்கிய சொற்கள்:
    செயல்     - செயல் வரையறை (function)
    மாறி      - மாறி அறிவிப்பு (let)
    மாறாத     - மாறாத மாறி (const)
    என்றால்   - நிபந்தனை (if)
    இல்லை     - இல்லையெனில் (else)
    வரை       - வரை வளையம் (while)
    ஒவ்வொரு   - ஒவ்வொரு வளையம் (for)
    திரும்பு  - திரும்பு (return)
    அச்சிடு   - அச்சிடு (print)

மேலும் தகவல்: https://github.com/agam-lang/agam
"#);
}

fn print_version() {
    println!("அகம் (Agam) v{}", VERSION);
    println!("Tamil Programming Language");
}

fn run_file(path: &str) {
    let source = match fs::read_to_string(path) {
        Ok(content) => content,
        Err(e) => {
            eprintln!("கோப்பை படிக்க இயலவில்லை '{}': {}", path, e);
            process::exit(1);
        }
    };

    if let Err(error) = run(&source) {
        eprintln!("{}", format_error(&error, &source));
        process::exit(1);
    }
}

fn run_repl() {
    println!("{}", WELCOME_MESSAGE);

    let mut interpreter = Interpreter::new();
    let mut line_buffer = String::new();
    let mut continuation = false;

    loop {
        // Print prompt
        if continuation {
            print!("... ");
        } else {
            print!(">>> ");
        }
        io::stdout().flush().unwrap();

        // Read input
        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(0) => break, // EOF
            Ok(_) => {}
            Err(e) => {
                eprintln!("உள்ளீடு பிழை: {}", e);
                continue;
            }
        }

        let input = input.trim_end();

        // Handle special commands
        if !continuation {
            match input.to_lowercase().as_str() {
                "exit()" | "வெளியேறு()" | "quit()" => {
                    println!("நன்றி! மீண்டும் வருக! 🙏");
                    break;
                }
                "help()" | "உதவி()" => {
                    print_repl_help();
                    continue;
                }
                "" => continue,
                _ => {}
            }
        }

        // Handle multi-line input
        line_buffer.push_str(input);
        line_buffer.push('\n');

        // Check if we need more input (ends with :)
        if input.ends_with(':') || (continuation && !input.is_empty() && input.starts_with(' ')) {
            continuation = true;
            continue;
        }

        if continuation && input.is_empty() {
            continuation = false;
        } else if !continuation {
            // Single line, execute immediately
        } else {
            continue;
        }

        // Execute the accumulated code
        let source = line_buffer.clone();
        line_buffer.clear();

        match run_with_interpreter(&source, &mut interpreter) {
            Ok(result) => {
                // Don't print null results
                let result_str = format!("{}", result);
                if result_str != "இல்லா" {
                    // println!("=> {}", result);
                }
            }
            Err(error) => {
                eprintln!("{}", format_error(&error, &source));
            }
        }
    }
}

fn print_repl_help() {
    println!(r#"
REPL உதவி:
    exit() / வெளியேறு()    - நிரலை முடி
    help() / உதவி()       - இந்த உதவியை காட்டு

எளிய எடுத்துக்காட்டுகள்:

>>> மாறி பெயர் = "தமிழ்"
>>> அச்சிடு(பெயர்)
தமிழ்

>>> மாறி எண் = 10
>>> என்றால் எண் > 5:
...     அச்சிடு("பெரியது!")
...
பெரியது!

>>> செயல் வணக்கம்(பெயர்):
...     திரும்பு "வணக்கம், " + பெயர்
...
>>> அச்சிடு(வணக்கம்("நண்பா"))
வணக்கம், நண்பா
"#);
}

fn run(source: &str) -> Result<(), AgamError> {
    let mut interpreter = Interpreter::new();
    run_with_interpreter(source, &mut interpreter)?;
    Ok(())
}

fn run_with_interpreter(source: &str, interpreter: &mut Interpreter) -> Result<agam::types::Value, AgamError> {
    // Tokenize
    let tokens = Lexer::tokenize(source)?;

    // Parse
    let mut parser = Parser::new(tokens);
    let program = parser.parse()?;

    // Execute
    interpreter.execute(&program)
}
