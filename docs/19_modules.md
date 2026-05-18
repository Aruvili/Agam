# Chapter 19: Modules (தொகுப்புகள்)

## Overview

Modules allow you to organize your code into separate files and reuse code across projects. In agam, you use `இறக்குமதி` (import) and `இருந்து` (from) to work with modules.

---

## Basic Import

Import an entire module:

```
இறக்குமதி math_utils
```

---

## Selective Import

Import specific items from a module:

```
இருந்து math_utils இறக்குமதி add, subtract
```

---

## Creating a Module

Create a file with functions and variables you want to share.

**math_utils.agam:**
```
# Math utility functions

செயல் add(a, b) {
    விடை a + b;

}
செயல் subtract(a, b) {
    விடை a - b;

}
செயல் multiply(a, b) {
    விடை a * b;

}
செயல் divide(a, b) {
    எனில் b == 0:;
        வீசு("Cannot divide by zero!")
    விடை a / b;

}
நிலைமாறிலி PI = 3.14159;
```

---

## Using a Module

**main.agam:**
```
இறக்குமதி math_utils;

மாறி result = math_utils.add(5, 3);
பதிப்பி("5 + 3 =", result);

மாறி area = math_utils.PI * 5 * 5;
பதிப்பி("Circle area:", area);
```

---

## Selective Import Example

```
இருந்து math_utils இறக்குமதி add, multiply, PI

மாறி sum = add(10, 20);
மாறி product = multiply(5, 6);

பதிப்பி("Sum:", sum);
பதிப்பி("Product:", product);
பதிப்பி("PI:", PI);
```

---

## Practical Examples

### Example 1: String Utilities Module

**string_utils.agam:**
```
செயல் capitalize(text) {
    எனில் நீளம்(text) == 0:;
        விடை text;
    விடை மேல்(text[0]) + கீழ்(text[1:]);

}
செயல் reverse(text) {
    விடை தலைகீழ்(text);

}
செயல் word_count(text) {
    மாறி words = பிரி(text, " ");
    விடை நீளம்(words);

}
செயல் is_palindrome(text) {
    மாறி clean = கீழ்(ஒழுங்கு(text));
    விடை clean == தலைகீழ்(clean);
}
```

**main.agam:**
```
இருந்து string_utils இறக்குமதி capitalize, is_palindrome

பதிப்பி(capitalize("hello"))        # Output: Hello
பதிப்பி(is_palindrome("radar"))     # Output: உண்மை
பதிப்பி(is_palindrome("hello"))     # Output: பொய்
```

### Example 2: Data Validation Module

**validate.agam:**
```
செயல் is_email(text) {
    விடை உள்ளதா(text, "@") மற்றும் உள்ளதா(text, ".");

}
செயல் is_phone(text) {
    எனில் நீளம்(text) != 10:;
        விடை பொய்;
    சுற்று (char உள் text) {
        எனில் இல்ல is_digit(char):
            விடை பொய்;
    }
    விடை உண்மை;

}
செயல் is_digit(char) {
    விடை char >= "0" மற்றும் char <= "9";

}
செயல் is_empty(text) {
    விடை நீளம்(ஒழுங்கு(text)) == 0;
}
```

**main.agam:**
```
இருந்து validate இறக்குமதி is_email, is_empty

மாறி email = உள்ளீடு("Enter email: ");

எனில் is_empty(email):
    பதிப்பி("Email cannot be empty!");
இல்லையெனில் இல்ல is_email(email):
    பதிப்பி("Invalid email format!");
} இல்லையெனில் {
    பதிப்பி("Email is valid!");
}
```

### Example 3: Game Utilities Module

**game_utils.agam:**
```
பட்டியல் Direction {
    Up,
    Down,
    Left,
    Right,

}
அமைப்பு Player {
    x,
    y,
    health,
    score,

}
செயல் create_player(x, y) {
    விடை Player(x, y, 100, 0),

}
செயல் move(player, direction) {
    பொருத்து (direction) {
        Direction.Up => player.y = player.y - 1;
        Direction.Down => player.y = player.y + 1;
        Direction.Left => player.x = player.x - 1;
        Direction.Right => player.x = player.x + 1;

    }
செயல் take_damage(player, amount) {
    player.health = player.health - amount;
    எனில் player.health < 0:,
        player.health = 0;

}
செயல் add_score(player, points) {
    player.score = player.score + points;
}
```

**game.agam:**
```
இறக்குமதி game_utils;

மாறி player = game_utils.create_player(5, 5);
game_utils.move(player, game_utils.Direction.Up)
game_utils.add_score(player, 100)

பதிப்பி("Position:", player.x, player.y);
பதிப்பி("Score:", player.score);
```

---

## Module Organization

### Project Structure

```
my_project/
├── main.agam           # Entry point
├── utils/
│   ├── math.agam       # Math utilities
│   ├── string.agam     # String utilities
│   └── file.agam       # File utilities
├── models/
│   ├── user.agam       # User model
│   └── product.agam    # Product model
└── config.agam         # Configuration
```

### Importing from Subdirectories

```
இறக்குமதி utils.math;
இறக்குமதி models.user;

# Use with prefix
மாறி result = utils.math.add(1, 2);
```

---

## Best Practices

### 1. Keep Modules Focused

Each module should have a single responsibility:

```
# Good: Focused modules
math_utils.agam     # Math operations only
string_utils.agam   # String operations only
file_utils.agam     # File operations only

# Bad: Mixed module
utils.agam          # Everything mixed together
```

### 2. Use Descriptive Names

```
# Good
import game_physics
import user_authentication

# Bad
import gp
import ua
```

### 3. Document Your Modules

```
# math_utils.agam
# 
# Math utility functions for common calculations
#
# Functions:
#   - add(a, b): Add two numbers
#   - subtract(a, b): Subtract b from a
#   - multiply(a, b): Multiply two numbers
#   - divide(a, b): Divide a by b

செயல் add(a, b) {
    விடை a + b;
}
```

### 4. Avoid Circular Imports

```
# Bad: Circular dependency
# a.agam imports b.agam
# b.agam imports a.agam

# Good: Restructure to avoid cycles
# Create a shared module that both can import
```

---

## Summary

- Use `இறக்குமதி` to import entire modules
- Use `இருந்து ... இறக்குமதி` for selective imports
- Create modules by putting code in `.agam` files
- Organize code into focused, single-purpose modules
- Use descriptive names and document your modules

---

**Next: [Chapter 20: File I/O →](20_file_io.md)**
