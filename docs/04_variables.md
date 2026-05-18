# Chapter 4: Variables (மாறிகள்)

## What is a Variable?

A **variable** is like a labeled box that stores data. You give it a name, and it remembers a value for you.

```
மாறி பெயர் = "agam";
#     ↑      ↑
#   name   value
```

---

## Creating Variables

Use `மாறி` (meaning "variable" or "let") to create a variable:

```
மாறி பெயர் = "Tamil";
மாறி வயது = 25;
மாறி விலை = 99.50;

பதிப்பி(பெயர்)   # Output: Tamil;
பதிப்பி(வயது)   # Output: 25;
பதிப்பி(விலை)   # Output: 99.5;
```

---

## Variable Names

### Valid Names ✅

```
மாறி பெயர் = "name"        # Tamil letters;
மாறி age = 25             # English letters;
மாறி பெயர்1 = "first"     # With numbers (not at start);
மாறி my_variable = 10     # With underscores;
மாறி எண்_1 = 100          # Mixed;
```

### Invalid Names ❌

```
மாறி 1பெயர் = "bad"       # Can't start with number;
மாறி my-var = 10          # Can't use hyphens;
மாறி my var = 10          # Can't have spaces;
```

---

## Changing Variable Values

You can change a variable's value after creating it:

```
மாறி எண் = 10;
பதிப்பி(எண்)   # Output: 10;

எண் = 20        # Change the value;
பதிப்பி(எண்)   # Output: 20;

எண் = எண் + 5   # Use old value to calculate new;
பதிப்பி(எண்)   # Output: 25;
```

---

## Constants (நிலைமாறிலி)

Use `நிலைமாறிலி` (meaning "constant" or "unchanging") for values that should never change:

```
நிலைமாறிலி PI = 3.14159
நிலைமாறிலி MAX_SCORE = 100

பதிப்பி(PI)         # Output: 3.14159
பதிப்பி(MAX_SCORE)  # Output: 100

# This will cause an error:
# PI = 3.14  # Error! Cannot change a constant
```

### When to Use Constants

- Mathematical values (π, e, etc.)
- Configuration values
- Maximum/minimum limits
- Any value that shouldn't change

---

## Using Variables in Expressions

Variables can be used in calculations:

```
மாறி a = 10;
மாறி b = 5;

பதிப்பி(a + b)   # Output: 15;
பதிப்பி(a - b)   # Output: 5;
பதிப்பி(a * b)   # Output: 50;
பதிப்பி(a / b)   # Output: 2;
```

---

## String Concatenation

Use `+` to join strings:

```
மாறி first = "வணக்கம்";
மாறி second = "உலகம்";

மாறி full = first + " " + second;
பதிப்பி(full)   # Output: வணக்கம் உலகம்;
```

### Combining Strings and Numbers

Use `சரமாக()` to convert numbers to strings:

```
மாறி பெயர் = "Tamil";
மாறி year = 2024;

பதிப்பி(பெயர் + " - " + சரமாக(year));
# Output: Tamil - 2024
```

---

## Variable Scope

Variables exist only in the block where they are created:

```
மாறி x = 10   # This x is available everywhere below;

எனில் x > 5:
    மாறி y = 20   # This y only exists inside the if block;
    பதிப்பி(x)    # OK - x is available here;
    பதிப்பி(y)    # OK - y exists here;

# பதிப்பி(y)     # Error! y doesn't exist outside the if block
```

---

## Practical Examples

### Example 1: Calculate Area

```
மாறி length = 10;
மாறி width = 5;

மாறி area = length * width;
பதிப்பி("Area:", area)   # Output: Area: 50;
```

### Example 2: Temperature Conversion

```
மாறி celsius = 25;
மாறி fahrenheit = (celsius * 9 / 5) + 32;

பதிப்பி(சரமாக(celsius) + "°C = " + சரமாக(fahrenheit) + "°F");
# Output: 25°C = 77°F
```

### Example 3: Simple Counter

```
மாறி count = 0;

count = count + 1;
பதிப்பி(count)   # Output: 1;

count = count + 1;
பதிப்பி(count)   # Output: 2;
```

---

## Try It Yourself!

### Exercise 1

Create two variables with your first and last name, then print your full name.

<details>
<summary>Solution</summary>

```
மாறி first_name = "முகில்";
மாறி last_name = "செல்வம்";

பதிப்பி(first_name + " " + last_name);
```
</details>

### Exercise 2

Create a program that calculates the area of a circle with radius 7.

<details>
<summary>Solution</summary>

```
நிலைமாறிலி PI = 3.14159;
மாறி radius = 7;

மாறி area = PI * radius * radius;
பதிப்பி("Area:", area)   # Output: Area: 153.93791;
```
</details>

---

## Summary

| Concept | Syntax | Example |
|---------|--------|---------|
| Variable | `மாறி name = value` | `மாறி x = 10` |
| Constant | `நிலைமாறிலி name = value` | `நிலைமாறிலி PI = 3.14` |
| Update | `name = new_value` | `x = 20` |
| Use in expressions | anywhere | `x + 5` |

---

**Next: [Chapter 5: Data Types →](05_data_types.md)**
