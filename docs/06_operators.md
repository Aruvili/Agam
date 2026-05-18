# Chapter 6: Operators (செயற்குறிகள்)

## What are Operators?

Operators are symbols that perform operations on values:

```
10 + 5    # + is the operator
#  ↑
# operator
```

---

## Arithmetic Operators (கணித செயற்குறிகள்)

| Operator | Name | Example | Result |
|----------|------|---------|--------|
| `+` | Addition | `10 + 5` | `15` |
| `-` | Subtraction | `10 - 5` | `5` |
| `*` | Multiplication | `10 * 5` | `50` |
| `/` | Division | `10 / 4` | `2.5` |
| `%` | Modulo (remainder) | `10 % 3` | `1` |

### Examples

```
மாறி a = 20;
மாறி b = 7;

பதிப்பி(a + b)   # Output: 27;
பதிப்பி(a - b)   # Output: 13;
பதிப்பி(a * b)   # Output: 140;
பதிப்பி(a / b)   # Output: 2.857...;
பதிப்பி(a % b)   # Output: 6 (remainder);
```

### Negative Numbers

```
பதிப்பி(-10)      # Output: -10;
பதிப்பி(5 - 10)   # Output: -5;

மாறி x = 10;
பதிப்பி(-x)       # Output: -10;
```

---

## Comparison Operators (ஒப்பீட்டு செயற்குறிகள்)

These return `உண்மை` (true) or `பொய்` (false):

| Operator | Name | Example | Result |
|----------|------|---------|--------|
| `==` | Equal | `5 == 5` | `உண்மை` |
| `!=` | Not equal | `5 != 3` | `உண்மை` |
| `<` | Less than | `3 < 5` | `உண்மை` |
| `>` | Greater than | `5 > 3` | `உண்மை` |
| `<=` | Less or equal | `3 <= 3` | `உண்மை` |
| `>=` | Greater or equal | `5 >= 5` | `உண்மை` |

### Examples

```
மாறி age = 18;

பதிப்பி(age == 18)   # Output: உண்மை;
பதிப்பி(age != 20)   # Output: உண்மை;
பதிப்பி(age > 15)    # Output: உண்மை;
பதிப்பி(age < 21)    # Output: உண்மை;
பதிப்பி(age >= 18)   # Output: உண்மை;
பதிப்பி(age <= 17)   # Output: பொய்;
```

### String Comparison

```
பதிப்பி("apple" == "apple")   # Output: உண்மை
பதிப்பி("apple" != "banana")  # Output: உண்மை
பதிப்பி("a" < "b")            # Output: உண்மை (alphabetical)
```

---

## Logical Operators (தருக்க செயற்குறிகள்)

| Tamil | English | Meaning |
|-------|---------|---------|
| `மற்றும்` | `and` | Both must be true |
| `அல்லது` | `or` | At least one must be true |
| `இல்ல` | `not` | Reverses the boolean |

### மற்றும் (AND)

Both conditions must be true:

```
மாறி age = 25;
மாறி has_license = உண்மை;

எனில் age >= 18 மற்றும் has_license:;
    பதிப்பி("You can drive!");
```

Truth table:
| A | B | A மற்றும் B |
|---|---|-------------|
| உண்மை | உண்மை | உண்மை |
| உண்மை | பொய் | பொய் |
| பொய் | உண்மை | பொய் |
| பொய் | பொய் | பொய் |

### அல்லது (OR)

At least one condition must be true:

```
மாறி is_weekend = உண்மை;
மாறி is_holiday = பொய்;

எனில் is_weekend அல்லது is_holiday:
    பதிப்பி("No work today!");
```

Truth table:
| A | B | A அல்லது B |
|---|---|-------------|
| உண்மை | உண்மை | உண்மை |
| உண்மை | பொய் | உண்மை |
| பொய் | உண்மை | உண்மை |
| பொய் | பொய் | பொய் |

### இல்ல (NOT)

Reverses the boolean value:

```
மாறி is_raining = பொய்;

எனில் இல்ல is_raining:
    பதிப்பி("Let's go outside!");
```

| A | இல்ல A |
|---|--------|
| உண்மை | பொய் |
| பொய் | உண்மை |

---

## Operator Precedence

Operations are performed in this order (highest to lowest):

1. `()` - Parentheses
2. `-` - Negation (unary minus)
3. `*`, `/`, `%` - Multiplication, Division, Modulo
4. `+`, `-` - Addition, Subtraction
5. `<`, `>`, `<=`, `>=` - Comparisons
6. `==`, `!=` - Equality
7. `இல்ல` / `not` - Logical NOT
8. `மற்றும்` / `and` - Logical AND
9. `அல்லது` / `or` - Logical OR

### Examples

```
# Without parentheses
பதிப்பி(2 + 3 * 4)     # Output: 14 (not 20!)

# With parentheses
பதிப்பி((2 + 3) * 4)   # Output: 20

# Complex example
பதிப்பி(10 - 2 * 3)    # Output: 4
பதிப்பி((10 - 2) * 3)  # Output: 24
```

---

## String Operators

### Concatenation (+)

```
மாறி greeting = "வணக்கம்" + " " + "உலகம்";
பதிப்பி(greeting)   # Output: வணக்கம் உலகம்;
```

### Repetition (*)

```
மாறி line = "-" * 20;
பதிப்பி(line)   # Output: --------------------;

மாறி stars = "★" * 5;
பதிப்பி(stars)  # Output: ★★★★★;
```

---

## Practical Examples

### Example 1: Check if number is even or odd

```
மாறி number = 7;

எனில் number % 2 == 0:;
    பதிப்பி("Even");
} இல்லையெனில் {
    பதிப்பி("Odd");
}
# Output: Odd
```

### Example 2: Check age category

```
மாறி age = 25;

எனில் age < 13:
    பதிப்பி("Child");
இல்லையெனில் age < 20:
    பதிப்பி("Teenager");
இல்லையெனில் age < 60:
    பதிப்பி("Adult");
} இல்லையெனில் {
    பதிப்பி("Senior");
}
# Output: Adult
```

### Example 3: Compound conditions

```
மாறி score = 85;
மாறி attendance = 90;

எனில் score >= 80 மற்றும் attendance >= 75:;
    பதிப்பி("Eligible for certificate!");
} இல்லையெனில் {
    பதிப்பி("Not eligible");
}
# Output: Eligible for certificate!
```

---

## Summary

| Category | Operators |
|----------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `>` `<=` `>=` |
| Logical | `மற்றும்` `அல்லது` `இல்ல` |
| String | `+` (join) `*` (repeat) |

---

**Next: [Chapter 7: Conditionals →](07_conditionals.md)**
