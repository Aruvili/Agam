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

const VERSION: &str = "0.1.3";
const WELCOME_MESSAGE: &str = r#"
╔══════════════════════════════════════════════════════════════╗
║     அகம் - Agam Programming Language v0.1.3                 ║
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
    let mut history: Vec<String> = Vec::new();
    let mut line_count = 0usize;

    loop {
        // Print prompt with line number
        if continuation {
            print!("... ");
        } else {
            line_count += 1;
            print!("[{}] >>> ", line_count);
        }
        io::stdout().flush().unwrap();

        // Read input
        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(0) => {
                println!("\nவிடை! 👋");
                break;
            }
            Ok(_) => {}
            Err(e) => {
                eprintln!("உள்ளீடு பிழை: {}", e);
                continue;
            }
        }

        let input = input.trim_end();

        // Handle special commands (only on non-continuation lines)
        if !continuation {
            match input.to_lowercase().as_str() {
                "exit()" | "வெளியேறு()" | "quit()" | "exit" | "quit" => {
                    println!("நன்றி! மீண்டும் வருக! 🙏");
                    break;
                }
                "help()" | "உதவி()" | "help" | "?" => {
                    print_repl_help();
                    continue;
                }
                "clear()" | "அழி()" | "clear" | "cls" => {
                    // Clear screen (ANSI escape codes)
                    print!("\x1B[2J\x1B[1;1H");
                    io::stdout().flush().unwrap();
                    println!("{}", WELCOME_MESSAGE);
                    continue;
                }
                "history()" | "வரலாறு()" | "history" => {
                    println!("\n📜 கட்டளை வரலாறு:");
                    for (i, cmd) in history.iter().enumerate() {
                        println!("  [{}] {}", i + 1, cmd.lines().next().unwrap_or(""));
                    }
                    println!();
                    continue;
                }
                "" => continue,
                _ => {}
            }
        }

        // Handle multi-line input
        line_buffer.push_str(input);
        line_buffer.push('\n');

        // Check if we need more input
        // Lines ending with : indicate a block start
        // Continuation lines that are indented continue the block
        // An empty line in continuation mode ends the block
        let trimmed = input.trim();
        if trimmed.ends_with(':') {
            continuation = true;
            continue;
        }
        
        if continuation {
            // If the line is indented, continue the block
            if !input.is_empty() && (input.starts_with(' ') || input.starts_with('\t')) {
                continue;
            }
            // Empty line or unindented line ends the block
            continuation = false;
        }

        // Execute the accumulated code
        let source = line_buffer.clone();
        line_buffer.clear();

        // Save to history (non-empty commands only)
        if !source.trim().is_empty() {
            history.push(source.clone());
            // Keep only last 100 commands
            if history.len() > 100 {
                history.remove(0);
            }
        }

        match run_with_interpreter(&source, &mut interpreter) {
            Ok(result) => {
                // Show result for expressions (not null and not from statements)
                let result_str = format!("{}", result);
                if result_str != "இல்லா" {
                    println!("=> {}", result);
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
╔══════════════════════════════════════════════════════════════╗
║                    REPL கட்டளைகள் / Commands                   ║
╠══════════════════════════════════════════════════════════════╣
║  exit() / வெளியேறு()     நிரலை முடி                          ║
║  help() / உதவி()        இந்த உதவியை காட்டு                   ║
║  clear() / அழி()        திரையை துடை                         ║
║  history() / வரலாறு()   கட்டளை வரலாறு                        ║
╚══════════════════════════════════════════════════════════════╝

📝 எளிய எடுத்துக்காட்டுகள்:

[1] >>> மாறி x = 10
[2] >>> x + 5
=> 15

[3] >>> மாறி பெயர் = "தமிழ்"
[4] >>> நீளம்(பெயர்)
=> 5

[5] >>> என்றால் x > 5:
...     அச்சிடு("பெரியது!")
...
பெரியது!

[6] >>> செயல் கூட்டு(a, b):
...     திரும்பு a + b
...
[7] >>> கூட்டு(3, 4)
=> 7

[8] >>> ஒவ்வொரு i உள்ள வரம்பு(3):
...     அச்சிடு(i)
...
0
1
2

💡 குறிப்புகள்:
  • ':' உடன் முடியும் வரிகள் பல-வரி உள்ளீடு தொடங்கும்
  • வெற்று வரி பல-வரி உள்ளீட்டை முடிக்கும்
  • தமிழ் அல்லது ஆங்கிலத்தில் நிரலாக்கம் செய்யலாம்
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
