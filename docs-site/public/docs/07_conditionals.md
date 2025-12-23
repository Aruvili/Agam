# Chapter 7: Conditionals (நிபந்தனைகள்)

## Making Decisions

Conditionals let your program make decisions based on conditions.

```
என்றால் condition:
    # do something if condition is true
```

---

## The என்றால் Statement (if)

Use `என்றால்` to execute code only when a condition is true:

```
மாறி age = 20

என்றால் age >= 18:
    அச்சிடு("You are an adult!")
```

> **Important:** Don't forget the `:` after the condition and indent the code inside!

---

## என்றால்...இல்லை (if...else)

Use `இல்லை` to execute code when the condition is false:

```
மாறி age = 15

என்றால் age >= 18:
    அச்சிடு("You are an adult")
இல்லை:
    அச்சிடு("You are a minor")

# Output: You are a minor
```

---

## என்றால்...இல்லையென்றால்...இல்லை (if...elif...else)

Use `இல்லையென்றால்` to check multiple conditions:

```
மாறி score = 85

என்றால் score >= 90:
    அச்சிடு("Grade: A")
இல்லையென்றால் score >= 80:
    அச்சிடு("Grade: B")
இல்லையென்றால் score >= 70:
    அச்சிடு("Grade: C")
இல்லையென்றால் score >= 60:
    அச்சிடு("Grade: D")
இல்லை:
    அச்சிடு("Grade: F")

# Output: Grade: B
```

---

## Keyword Reference

| Tamil | English | Usage |
|-------|---------|-------|
| `என்றால்` | `if` | First condition |
| `இல்லையென்றால்` | `elif` | Additional conditions |
| `இல்லை` | `else` | When no condition matches |

---

## Compound Conditions

Combine conditions using logical operators:

### மற்றும் (AND) - Both must be true

```
மாறி age = 25
மாறி has_id = உண்மை

என்றால் age >= 18 மற்றும் has_id:
    அச்சிடு("Entry allowed")
இல்லை:
    அச்சிடு("Entry denied")
```

### அல்லது (OR) - At least one must be true

```
மாறி is_member = பொய்
மாறி has_pass = உண்மை

என்றால் is_member அல்லது has_pass:
    அச்சிடு("Welcome!")
```

### இல்ல (NOT) - Reverse the condition

```
மாறி is_closed = பொய்

என்றால் இல்ல is_closed:
    அச்சிடு("The shop is open")
```

---

## Nested Conditionals

You can put conditions inside conditions:

```
மாறி age = 25
மாறி is_student = உண்மை

என்றால் age >= 18:
    அச்சிடு("Adult")
    
    என்றால் is_student:
        அச்சிடு("Student discount: 20%")
    இல்லை:
        அச்சிடு("Regular price")
இல்லை:
    அச்சிடு("Minor - free entry!")
```

---

## Practical Examples

### Example 1: Login Check

```
மாறி username = "admin"
மாறி password = "secret123"

என்றால் username == "admin" மற்றும் password == "secret123":
    அச்சிடு("✅ Login successful!")
இல்லை:
    அச்சிடு("❌ Invalid credentials")
```

### Example 2: Number Classification

```
மாறி num = -5

என்றால் num > 0:
    அச்சிடு("Positive number")
இல்லையென்றால் num < 0:
    அச்சிடு("Negative number")
இல்லை:
    அச்சிடு("Zero")

# Output: Negative number
```

### Example 3: Ticket Pricing

```
மாறி age = 10
மாறி is_weekend = உண்மை

மாறி price = 100  # Base price

என்றால் age < 12:
    price = 50  # Kids discount
இல்லையென்றால் age >= 60:
    price = 60  # Senior discount

என்றால் is_weekend:
    price = price + 20  # Weekend surcharge

அச்சிடு("Ticket price:", price)
# Output: Ticket price: 70
```

---

## Try It Yourself!

### Exercise 1: Temperature Check

Write a program that prints:
- "Cold" if temperature < 15
- "Comfortable" if temperature is 15-25
- "Hot" if temperature > 25

<details>
<summary>Solution</summary>

```
மாறி temperature = 22

என்றால் temperature < 15:
    அச்சிடு("Cold")
இல்லையென்றால் temperature <= 25:
    அச்சிடு("Comfortable")
இல்லை:
    அச்சிடு("Hot")
```
</details>

### Exercise 2: Leap Year Check

A year is a leap year if:
- Divisible by 4 AND not divisible by 100
- OR divisible by 400

<details>
<summary>Solution</summary>

```
மாறி year = 2024

என்றால் (year % 4 == 0 மற்றும் year % 100 != 0) அல்லது (year % 400 == 0):
    அச்சிடு(சரமாக(year) + " is a leap year")
இல்லை:
    அச்சிடு(சரமாக(year) + " is not a leap year")
```
</details>

---

## Summary

| Pattern | Usage |
|---------|-------|
| `என்றால் condition:` | Single condition |
| `என்றால்...இல்லை:` | Two choices |
| `என்றால்...இல்லையென்றால்...இல்லை:` | Multiple choices |

---

**Next: [Chapter 8: Loops →](08_loops.md)**
