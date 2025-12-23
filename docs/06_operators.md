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
மாறி a = 20
மாறி b = 7

அச்சிடு(a + b)   # Output: 27
அச்சிடு(a - b)   # Output: 13
அச்சிடு(a * b)   # Output: 140
அச்சிடு(a / b)   # Output: 2.857...
அச்சிடு(a % b)   # Output: 6 (remainder)
```

### Negative Numbers

```
அச்சிடு(-10)      # Output: -10
அச்சிடு(5 - 10)   # Output: -5

மாறி x = 10
அச்சிடு(-x)       # Output: -10
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
மாறி age = 18

அச்சிடு(age == 18)   # Output: உண்மை
அச்சிடு(age != 20)   # Output: உண்மை
அச்சிடு(age > 15)    # Output: உண்மை
அச்சிடு(age < 21)    # Output: உண்மை
அச்சிடு(age >= 18)   # Output: உண்மை
அச்சிடு(age <= 17)   # Output: பொய்
```

### String Comparison

```
அச்சிடு("apple" == "apple")   # Output: உண்மை
அச்சிடு("apple" != "banana")  # Output: உண்மை
அச்சிடு("a" < "b")            # Output: உண்மை (alphabetical)
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
மாறி age = 25
மாறி has_license = உண்மை

என்றால் age >= 18 மற்றும் has_license:
    அச்சிடு("You can drive!")
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
மாறி is_weekend = உண்மை
மாறி is_holiday = பொய்

என்றால் is_weekend அல்லது is_holiday:
    அச்சிடு("No work today!")
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
மாறி is_raining = பொய்

என்றால் இல்ல is_raining:
    அச்சிடு("Let's go outside!")
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
அச்சிடு(2 + 3 * 4)     # Output: 14 (not 20!)

# With parentheses
அச்சிடு((2 + 3) * 4)   # Output: 20

# Complex example
அச்சிடு(10 - 2 * 3)    # Output: 4
அச்சிடு((10 - 2) * 3)  # Output: 24
```

---

## String Operators

### Concatenation (+)

```
மாறி greeting = "வணக்கம்" + " " + "உலகம்"
அச்சிடு(greeting)   # Output: வணக்கம் உலகம்
```

### Repetition (*)

```
மாறி line = "-" * 20
அச்சிடு(line)   # Output: --------------------

மாறி stars = "★" * 5
அச்சிடு(stars)  # Output: ★★★★★
```

---

## Practical Examples

### Example 1: Check if number is even or odd

```
மாறி number = 7

என்றால் number % 2 == 0:
    அச்சிடு("Even")
இல்லை:
    அச்சிடு("Odd")
# Output: Odd
```

### Example 2: Check age category

```
மாறி age = 25

என்றால் age < 13:
    அச்சிடு("Child")
இல்லையென்றால் age < 20:
    அச்சிடு("Teenager")
இல்லையென்றால் age < 60:
    அச்சிடு("Adult")
இல்லை:
    அச்சிடு("Senior")
# Output: Adult
```

### Example 3: Compound conditions

```
மாறி score = 85
மாறி attendance = 90

என்றால் score >= 80 மற்றும் attendance >= 75:
    அச்சிடு("Eligible for certificate!")
இல்லை:
    அச்சிடு("Not eligible")
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
