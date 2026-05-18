# Chapter 17: Pattern Matching (பொருத்து)

## Overview

Pattern matching is a powerful feature that lets you match values against patterns and execute code based on which pattern matches. In agam, you use `பொருத்து` (match) for pattern matching.

---

## Basic Syntax

```
பொருத்து value:
    pattern1 => result1
    pattern2 => result2
    _ => default_result
```

- The `=>` arrow separates the pattern from the result
- The `_` (underscore) is a wildcard that matches anything

---

## Matching Numbers

```
மாறி grade = 85;

பொருத்து (grade) {
    100 => பதிப்பி("Perfect score! 🎉");
    90 => பதிப்பி("Excellent! A+");
    80 => பதிப்பி("Great! A");
    70 => பதிப்பி("Good! B");
    _ => பதிப்பி("Keep working!");
}
```

---

## Matching Strings

```
மாறி command = "start";

பொருத்து (command) {
    "start" => பதிப்பி("Starting system...");
    "stop" => பதிப்பி("Stopping system...");
    "restart" => பதிப்பி("Restarting system...");
    "status" => பதிப்பி("System is running");
    _ => பதிப்பி("Unknown command");
}
```

---

## Matching Booleans

```
மாறி is_admin = உண்மை;

பொருத்து (is_admin) {
    உண்மை => பதிப்பி("Welcome, Administrator!");
    பொய் => பதிப்பி("Welcome, User!");
}
```

---

## Matching with Enums

Pattern matching works great with enums:

```
பட்டியல் Color {
    Red,
    Green,
    Blue,

}
செயல் to_hex(color) {
    பொருத்து (color) {
        Color.Red => விடை "#FF0000";
        Color.Green => விடை "#00FF00";
        Color.Blue => விடை "#0000FF";

    }
மாறி my_color = Color.Blue;
பதிப்பி(to_hex(my_color))  # Output: #0000FF,
```

---

## The Wildcard Pattern (_)

The underscore `_` matches any value. Always put it last:

```
மாறி day = 4;

பொருத்து (day) {
    1 => பதிப்பி("Monday");
    2 => பதிப்பி("Tuesday");
    3 => பதிப்பி("Wednesday");
    4 => பதிப்பி("Thursday");
    5 => பதிப்பி("Friday");
    6 => பதிப்பி("Saturday");
    7 => பதிப்பி("Sunday");
    _ => பதிப்பி("Invalid day number");
}
```

---

## Returning Values from Match

Match expressions can return values:

```
செயல் day_type(day) {
    விடை பொருத்து day:;
        1 => "Weekday";
        2 => "Weekday";
        3 => "Weekday";
        4 => "Weekday";
        5 => "Weekday";
        6 => "Weekend";
        7 => "Weekend";
        _ => "Invalid";

}
பதிப்பி(day_type(6))  # Output: Weekend;
```

---

## Practical Examples

### Example 1: HTTP Status Codes

```
செயல் status_message(code) {
    பொருத்து (code) {
        200 => விடை "OK";
        201 => விடை "Created";
        400 => விடை "Bad Request";
        401 => விடை "Unauthorized";
        403 => விடை "Forbidden";
        404 => விடை "Not Found";
        500 => விடை "Internal Server Error";
        _ => விடை "Unknown Status";

    }
பதிப்பி(status_message(404))  # Output: Not Found;
```

### Example 2: Calculator

```
செயல் calculate(a, op, b) {
    பொருத்து (op) {
        "+" => விடை a + b;
        "-" => விடை a - b;
        "*" => விடை a * b;
        "/" => விடை a / b;
        "%" => விடை a % b;
        _ => விடை "Unknown operator";

    }
பதிப்பி(calculate(10, "+", 5))   # Output: 15;
பதிப்பி(calculate(10, "*", 5))   # Output: 50;
```

### Example 3: Grade System

```
செயல் get_grade(marks) {
    பொருத்து (marks) {
        100 => விடை "A+ (Perfect!)";
        _ =>;
            எனில் marks >= 90:;
                விடை "A+";
            இல்லையெனில் marks >= 80:;
                விடை "A";
            இல்லையெனில் marks >= 70:;
                விடை "B";
            இல்லையெனில் marks >= 60:;
                விடை "C";
            } இல்லையெனில் {
                விடை "F";

        }
பதிப்பி(get_grade(85))   # Output: A;
பதிப்பி(get_grade(100))  # Output: A+ (Perfect!);
```

### Example 4: State Machine

```
பட்டியல் State {
    Idle,
    Running,
    Paused,
    Stopped,

}
செயல் handle_event(state, event) {
    பொருத்து (state) {
        State.Idle =>;
            பொருத்து (event) {
                "start" => விடை State.Running;
                _ => விடை State.Idle;
        }
        State.Running =>;
            பொருத்து (event) {
                "pause" => விடை State.Paused;
                "stop" => விடை State.Stopped;
                _ => விடை State.Running;
        }
        State.Paused =>;
            பொருத்து (event) {
                "resume" => விடை State.Running;
                "stop" => விடை State.Stopped;
                _ => விடை State.Paused;
        }
        State.Stopped =>;
            விடை State.Stopped,

    }
மாறி current = State.Idle;
current = handle_event(current, "start");
பதிப்பி(current)  # Output: State.Running,
```

### Example 5: Menu System

```
செயல் show_menu() {
    பதிப்பி("=== Menu ===");
    பதிப்பி("1. New Game");
    பதிப்பி("2. Load Game");
    பதிப்பி("3. Settings");
    பதிப்பி("4. Exit");

}
செயல் handle_choice(choice) {
    பொருத்து (choice) {
        "1" => பதிப்பி("Starting new game...");
        "2" => பதிப்பி("Loading saved game...");
        "3" => பதிப்பி("Opening settings...");
        "4" => ;
            பதிப்பி("Goodbye!");
            வெளியேறு()
        _ => பதிப்பி("Invalid choice. Try again.");

    }
show_menu()
மாறி choice = உள்ளீடு("Enter choice: ");
handle_choice(choice)
```

---

## Pattern Matching vs If-Else

Pattern matching is often cleaner than long if-else chains:

```
# With if-else (verbose)
செயல் status_if(code) {
    எனில் code == 200:;
        விடை "OK";
    இல்லையெனில் code == 404:;
        விடை "Not Found";
    இல்லையெனில் code == 500:;
        விடை "Server Error";
    } இல்லையெனில் {
        விடை "Unknown";

    }
# With pattern matching (cleaner)
செயல் status_match(code) {
    பொருத்து (code) {
        200 => விடை "OK";
        404 => விடை "Not Found";
        500 => விடை "Server Error";
        _ => விடை "Unknown";
    }
```

---

## Summary

- Use `பொருத்து` for pattern matching
- Each arm has `pattern => result` format
- Use `_` as a wildcard for catch-all cases
- Works with numbers, strings, booleans, and enums
- Returns values or executes code
- Cleaner than long if-else chains

---

**Next: [Chapter 18: Error Handling →](18_error_handling.md)**
