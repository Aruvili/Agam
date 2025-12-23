# Chapter 9: Functions (செயல்கள்)

## What is a Function?

A function is a reusable block of code that performs a specific task.

```
செயல் greet():
    அச்சிடு("Hello!")

greet()   # Call the function
# Output: Hello!
```

---

## Defining Functions

Use `செயல்` (meaning "function" or "action"):

```
செயல் say_hello():
    அச்சிடு("வணக்கம்!")
    அச்சிடு("Welcome to agam")

# Call the function
say_hello()
```

### Syntax

```
செயல் function_name(parameters):
    # code block
    # (indented with 4 spaces)
```

---

## Parameters (அளவுருக்கள்)

Functions can accept input values:

```
செயல் greet(name):
    அச்சிடு("வணக்கம், " + name + "!")

greet("Tamil")      # Output: வணக்கம், Tamil!
greet("agam")    # Output: வணக்கம், agam!
```

### Multiple Parameters

```
செயல் add(a, b):
    மாறி result = a + b
    அச்சிடு(result)

add(5, 3)    # Output: 8
add(10, 20)  # Output: 30
```

---

## Return Values (திரும்பு)

Use `திரும்பு` to return a value:

```
செயல் add(a, b):
    திரும்பு a + b

மாறி result = add(5, 3)
அச்சிடு(result)   # Output: 8

# Use directly in expressions
அச்சிடு(add(10, 20) * 2)   # Output: 60
```

### Multiple Returns

```
செயல் get_min_max(numbers):
    மாறி min = numbers[0]
    மாறி max = numbers[0]
    
    ஒவ்வொரு num உள்ள numbers:
        என்றால் num < min:
            min = num
        என்றால் num > max:
            max = num
    
    திரும்பு [min, max]

மாறி result = get_min_max([5, 2, 8, 1, 9])
அச்சிடு("Min:", result[0])   # Output: Min: 1
அச்சிடு("Max:", result[1])   # Output: Max: 9
```

---

## Early Return

Return can exit a function early:

```
செயல் find_first_negative(numbers):
    ஒவ்வொரு num உள்ள numbers:
        என்றால் num < 0:
            திரும்பு num    # Exit immediately
    திரும்பு இல்லா         # No negative found

அச்சிடு(find_first_negative([1, 2, -3, 4]))   # Output: -3
அச்சிடு(find_first_negative([1, 2, 3, 4]))    # Output: இல்லா
```

---

## Recursion (மீளுறு)

A function can call itself:

```
செயல் factorial(n):
    என்றால் n <= 1:
        திரும்பு 1
    இல்லை:
        திரும்பு n * factorial(n - 1)

அச்சிடு(factorial(5))   # Output: 120
# 5! = 5 × 4 × 3 × 2 × 1 = 120
```

### Fibonacci Example

```
செயல் fibonacci(n):
    என்றால் n <= 0:
        திரும்பு 0
    இல்லையென்றால் n == 1:
        திரும்பு 1
    இல்லை:
        திரும்பு fibonacci(n - 1) + fibonacci(n - 2)

ஒவ்வொரு i உள்ள வரம்பு(10):
    அச்சிடு(fibonacci(i))
# Output: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
```

---

## Practical Examples

### Example 1: Check if Prime

```
செயல் is_prime(n):
    என்றால் n < 2:
        திரும்பு பொய்
    
    ஒவ்வொரு i உள்ள வரம்பு(2, n):
        என்றால் n % i == 0:
            திரும்பு பொய்
    
    திரும்பு உண்மை

# Test
ஒவ்வொரு num உள்ள வரம்பு(1, 20):
    என்றால் is_prime(num):
        அச்சிடு(சரமாக(num) + " is prime")
```

### Example 2: Calculate Average

```
செயல் average(numbers):
    மாறி total = 0
    
    ஒவ்வொரு num உள்ள numbers:
        total = total + num
    
    திரும்பு total / நீளம்(numbers)

மாறி scores = [85, 90, 78, 92, 88]
அச்சிடு("Average:", average(scores))
# Output: Average: 86.6
```

### Example 3: String Reversal

```
செயல் reverse(text):
    மாறி result = ""
    
    ஒவ்வொரு char உள்ள text:
        result = char + result
    
    திரும்பு result

அச்சிடு(reverse("Tamil"))     # Output: limaT
அச்சிடு(reverse("வணக்கம்"))    # Output: ம்கக்ணவ
```

### Example 4: Temperature Converter

```
செயல் celsius_to_fahrenheit(c):
    திரும்பு (c * 9 / 5) + 32

செயல் fahrenheit_to_celsius(f):
    திரும்பு (f - 32) * 5 / 9

அச்சிடு("25°C =", celsius_to_fahrenheit(25), "°F")
அச்சிடு("77°F =", fahrenheit_to_celsius(77), "°C")
```

---

## Function Best Practices

### 1. Use Descriptive Names

```
# Good ✅
செயல் calculate_area(length, width):
    திரும்பு length * width

# Bad ❌
செயல் calc(l, w):
    திரும்பு l * w
```

### 2. One Function, One Task

```
# Good ✅ - Separate functions
செயல் validate_input(data):
    # validation logic

செயல் process_data(data):
    # processing logic

# Bad ❌ - Too much in one function
செயல் do_everything(data):
    # validation + processing + output
```

### 3. Keep Functions Short

- Ideally under 20 lines
- If longer, consider splitting into smaller functions

---

## Summary

| Concept | Syntax |
|---------|--------|
| Define | `செயல் name():` |
| Parameters | `செயல் name(param):` |
| Return | `திரும்பு value` |
| Call | `name()` or `name(arg)` |

---

**Next: [Chapter 10: Built-in Functions →](10_builtins.md)**
