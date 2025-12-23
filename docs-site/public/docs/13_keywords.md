# Chapter 13: Keywords Reference (முக்கிய சொற்கள்)

## Complete Keyword List

agam supports both Tamil and English keywords. You can use either!

---

## Declaration Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `செயல்` | `fn` | Define a function | `செயல் greet():` |
| `மாறி` | `let` | Declare a variable | `மாறி x = 10` |
| `மாறாத` | `const` | Declare a constant | `மாறாத PI = 3.14` |

---

## Control Flow Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `என்றால்` | `if` | If condition | `என்றால் x > 5:` |
| `இல்லையென்றால்` | `elif` | Else if | `இல்லையென்றால் x > 3:` |
| `இல்லை` | `else` | Else | `இல்லை:` |
| `வரை` | `while` | While loop | `வரை x < 10:` |
| `ஒவ்வொரு` | `for` | For loop | `ஒவ்வொரு i உள்ள items:` |
| `உள்ள` | `in` | In operator | `x உள்ள list` |
| `திரும்பு` | `return` | Return value | `திரும்பு result` |
| `நிறுத்து` | `break` | Break loop | `நிறுத்து` |
| `தொடர்` | `continue` | Continue loop | `தொடர்` |

---

## Boolean Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `உண்மை` | `true` | Boolean true | `மாறி flag = உண்மை` |
| `பொய்` | `false` | Boolean false | `மாறி flag = பொய்` |
| `இல்லா` | `null` | Null value | `மாறி data = இல்லா` |

---

## Logical Operators

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `மற்றும்` | `and` | Logical AND | `x > 0 மற்றும் x < 10` |
| `அல்லது` | `or` | Logical OR | `x < 0 அல்லது x > 10` |
| `இல்ல` | `not` | Logical NOT | `இல்ல is_empty` |

---

## Built-in Functions

| Tamil | English | Description |
|-------|---------|-------------|
| `அச்சிடு` | `print` | Print output |
| `உள்ளீடு` | `input` | Read input |
| `நீளம்` | `len` | Get length |
| `வகை` | `type` | Get type |
| `எண்ணாக` | `int` | Convert to integer |
| `தசமாக` | `float` | Convert to float |
| `சரமாக` | `str` | Convert to string |
| `வரம்பு` | `range` | Create range |
| `சேர்` | `append` | Add to list |
| `நீக்கு` | `pop` | Remove from list |

---

## Operators

### Arithmetic

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` → `8` |
| `-` | Subtraction | `5 - 3` → `2` |
| `*` | Multiplication | `5 * 3` → `15` |
| `/` | Division | `5 / 2` → `2.5` |
| `%` | Modulo | `5 % 2` → `1` |

### Comparison

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `5 == 5` → `உண்மை` |
| `!=` | Not equal | `5 != 3` → `உண்மை` |
| `<` | Less than | `3 < 5` → `உண்மை` |
| `>` | Greater than | `5 > 3` → `உண்மை` |
| `<=` | Less or equal | `3 <= 3` → `உண்மை` |
| `>=` | Greater or equal | `5 >= 5` → `உண்மை` |

---

## Quick Reference Card

```
# Variable
மாறி name = value

# Constant
மாறாத NAME = value

# Function
செயல் name(params):
    code
    திரும்பு value

# If/Else
என்றால் condition:
    code
இல்லையென்றால் condition:
    code
இல்லை:
    code

# While Loop
வரை condition:
    code

# For Loop
ஒவ்வொரு item உள்ள collection:
    code

# Print
அச்சிடு(value)

# Input
மாறி x = உள்ளீடு("prompt")
```

---

**Next: [Chapter 14: Error Messages →](14_errors.md)**
