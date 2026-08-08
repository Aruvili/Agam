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
மாறி grade = 85

பொருத்து grade:
    100 => அச்சிடு("Perfect score! ")
    90 => அச்சிடு("Excellent! A+")
    80 => அச்சிடு("Great! A")
    70 => அச்சிடு("Good! B")
    _ => அச்சிடு("Keep working!")
```

---

## Matching Strings

```
மாறி command = "start"

பொருத்து command:
    "start" => அச்சிடு("Starting system...")
    "stop" => அச்சிடு("Stopping system...")
    "restart" => அச்சிடு("Restarting system...")
    "status" => அச்சிடு("System is running")
    _ => அச்சிடு("Unknown command")
```

---

## Matching Booleans

```
மாறி is_admin = உண்மை

பொருத்து is_admin:
    உண்மை => அச்சிடு("Welcome, Administrator!")
    பொய் => அச்சிடு("Welcome, User!")
```

---

## Matching with Enums

Pattern matching works great with enums:

```
விருப்பம் Color:
    Red
    Green
    Blue

செயல் to_hex(color):
    பொருத்து color:
        Color.Red => திரும்பு "#FF0000"
        Color.Green => திரும்பு "#00FF00"
        Color.Blue => திரும்பு "#0000FF"

மாறி my_color = Color.Blue
அச்சிடு(to_hex(my_color))  # Output: #0000FF
```

---

## The Wildcard Pattern (_)

The underscore `_` matches any value. Always put it last:

```
மாறி day = 4

பொருத்து day:
    1 => அச்சிடு("Monday")
    2 => அச்சிடு("Tuesday")
    3 => அச்சிடு("Wednesday")
    4 => அச்சிடு("Thursday")
    5 => அச்சிடு("Friday")
    6 => அச்சிடு("Saturday")
    7 => அச்சிடு("Sunday")
    _ => அச்சிடு("Invalid day number")
```

---

## Returning Values from Match

Match expressions can return values:

```
செயல் day_type(day):
    திரும்பு பொருத்து day:
        1 => "Weekday"
        2 => "Weekday"
        3 => "Weekday"
        4 => "Weekday"
        5 => "Weekday"
        6 => "Weekend"
        7 => "Weekend"
        _ => "Invalid"

அச்சிடு(day_type(6))  # Output: Weekend
```

---

## Practical Examples

### Example 1: HTTP Status Codes

```
செயல் status_message(code):
    பொருத்து code:
        200 => திரும்பு "OK"
        201 => திரும்பு "Created"
        400 => திரும்பு "Bad Request"
        401 => திரும்பு "Unauthorized"
        403 => திரும்பு "Forbidden"
        404 => திரும்பு "Not Found"
        500 => திரும்பு "Internal Server Error"
        _ => திரும்பு "Unknown Status"

அச்சிடு(status_message(404))  # Output: Not Found
```

### Example 2: Calculator

```
செயல் calculate(a, op, b):
    பொருத்து op:
        "+" => திரும்பு a + b
        "-" => திரும்பு a - b
        "*" => திரும்பு a * b
        "/" => திரும்பு a / b
        "%" => திரும்பு a % b
        _ => திரும்பு "Unknown operator"

அச்சிடு(calculate(10, "+", 5))   # Output: 15
அச்சிடு(calculate(10, "*", 5))   # Output: 50
```

### Example 3: Grade System

```
செயல் get_grade(marks):
    பொருத்து marks:
        100 => திரும்பு "A+ (Perfect!)"
        _ =>
            என்றால் marks >= 90:
                திரும்பு "A+"
            இல்லையென்றால் marks >= 80:
                திரும்பு "A"
            இல்லையென்றால் marks >= 70:
                திரும்பு "B"
            இல்லையென்றால் marks >= 60:
                திரும்பு "C"
            இல்லை:
                திரும்பு "F"

அச்சிடு(get_grade(85))   # Output: A
அச்சிடு(get_grade(100))  # Output: A+ (Perfect!)
```

### Example 4: State Machine

```
விருப்பம் State:
    Idle
    Running
    Paused
    Stopped

செயல் handle_event(state, event):
    பொருத்து state:
        State.Idle =>
            பொருத்து event:
                "start" => திரும்பு State.Running
                _ => திரும்பு State.Idle
        State.Running =>
            பொருத்து event:
                "pause" => திரும்பு State.Paused
                "stop" => திரும்பு State.Stopped
                _ => திரும்பு State.Running
        State.Paused =>
            பொருத்து event:
                "resume" => திரும்பு State.Running
                "stop" => திரும்பு State.Stopped
                _ => திரும்பு State.Paused
        State.Stopped =>
            திரும்பு State.Stopped

மாறி current = State.Idle
current = handle_event(current, "start")
அச்சிடு(current)  # Output: State.Running
```

### Example 5: Menu System

```
செயல் show_menu():
    அச்சிடு("=== Menu ===")
    அச்சிடு("1. New Game")
    அச்சிடு("2. Load Game")
    அச்சிடு("3. Settings")
    அச்சிடு("4. Exit")

செயல் handle_choice(choice):
    பொருத்து choice:
        "1" => அச்சிடு("Starting new game...")
        "2" => அச்சிடு("Loading saved game...")
        "3" => அச்சிடு("Opening settings...")
        "4" => 
            அச்சிடு("Goodbye!")
            வெளியேறு()
        _ => அச்சிடு("Invalid choice. Try again.")

show_menu()
மாறி choice = உள்ளீடு("Enter choice: ")
handle_choice(choice)
```

---

## Pattern Matching vs If-Else

Pattern matching is often cleaner than long if-else chains:

```
# With if-else (verbose)
செயல் status_if(code):
    என்றால் code == 200:
        திரும்பு "OK"
    இல்லையென்றால் code == 404:
        திரும்பு "Not Found"
    இல்லையென்றால் code == 500:
        திரும்பு "Server Error"
    இல்லை:
        திரும்பு "Unknown"

# With pattern matching (cleaner)
செயல் status_match(code):
    பொருத்து code:
        200 => திரும்பு "OK"
        404 => திரும்பு "Not Found"
        500 => திரும்பு "Server Error"
        _ => திரும்பு "Unknown"
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
