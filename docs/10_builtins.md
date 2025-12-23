# Chapter 10: Built-in Functions (உள்ளமைந்த செயல்கள்)

## Overview

agam comes with many built-in functions ready to use. Each function is available in both Tamil and English.

---

## Input/Output Functions

### அச்சிடு / print

Display output to the screen:

```
அச்சிடு("வணக்கம்!")           # Print text
அச்சிடு(42)                   # Print number
அச்சிடு("Value:", 100)        # Print multiple items
```

### உள்ளீடு / input

Read input from the user:

```
மாறி name = உள்ளீடு("Enter your name: ")
அச்சிடு("Hello, " + name)

மாறி age_str = உள்ளீடு("Enter your age: ")
மாறி age = எண்ணாக(age_str)    # Convert to number
```

---

## Type Conversion Functions

### எண்ணாக / int

Convert to integer:

```
அச்சிடு(எண்ணாக("42"))       # Output: 42
அச்சிடு(எண்ணாக(3.7))        # Output: 3 (truncates)
அச்சிடு(எண்ணாக(உண்மை))     # Output: 1
```

### தசமாக / float

Convert to decimal number:

```
அச்சிடு(தசமாக("3.14"))     # Output: 3.14
அச்சிடு(தசமாக(42))          # Output: 42.0
```

### சரமாக / str

Convert to string:

```
அச்சிடு(சரமாக(42))          # Output: "42"
அச்சிடு(சரமாக(3.14))        # Output: "3.14"
அச்சிடு(சரமாக(உண்மை))      # Output: "உண்மை"

# Useful for concatenation
மாறி age = 25
அச்சிடு("Age: " + சரமாக(age))
```

---

## Collection Functions

### நீளம் / len

Get the length of a string, list, or dictionary:

```
# String length
அச்சிடு(நீளம்("Hello"))           # Output: 5
அச்சிடு(நீளம்("தமிழ்"))           # Output: 5

# List length
அச்சிடு(நீளம்([1, 2, 3, 4, 5]))   # Output: 5

# Dictionary length
மாறி data = {"a": 1, "b": 2}
அச்சிடு(நீளம்(data))              # Output: 2
```

### வரம்பு / range

Create a sequence of numbers:

```
# range(end) - 0 to end-1
ஒவ்வொரு i உள்ள வரம்பு(5):
    அச்சிடு(i)   # 0, 1, 2, 3, 4

# range(start, end) - start to end-1
ஒவ்வொரு i உள்ள வரம்பு(1, 6):
    அச்சிடு(i)   # 1, 2, 3, 4, 5

# range(start, end, step)
ஒவ்வொரு i உள்ள வரம்பு(0, 10, 2):
    அச்சிடு(i)   # 0, 2, 4, 6, 8

# Negative step (countdown)
ஒவ்வொரு i உள்ள வரம்பு(5, 0, -1):
    அச்சிடு(i)   # 5, 4, 3, 2, 1
```

### சேர் / append

Add an item to a list:

```
மாறி fruits = ["apple", "banana"]
சேர்(fruits, "cherry")
அச்சிடு(fruits)   # Output: [apple, banana, cherry]
```

### நீக்கு / pop

Remove and return the last item from a list:

```
மாறி numbers = [1, 2, 3, 4, 5]
மாறி last = நீக்கு(numbers)
அச்சிடு(last)      # Output: 5
அச்சிடு(numbers)   # Output: [1, 2, 3, 4]
```

---

## Information Functions

### வகை / type

Get the type of a value:

```
அச்சிடு(வகை(42))           # Output: எண்
அச்சிடு(வகை("Hello"))      # Output: சரம்
அச்சிடு(வகை(உண்மை))        # Output: உண்மைபொய்
அச்சிடு(வகை([1, 2, 3]))    # Output: பட்டியல்
அச்சிடு(வகை({"a": 1}))     # Output: அகராதி
அச்சிடு(வகை(இல்லா))        # Output: இல்லா
```

---

## Quick Reference Table

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `அச்சிடு` | `print` | Display output | `அச்சிடு("Hi")` |
| `உள்ளீடு` | `input` | Read input | `உள்ளீடு("Name: ")` |
| `நீளம்` | `len` | Get length | `நீளம்("Hello")` → 5 |
| `வகை` | `type` | Get type | `வகை(42)` → எண் |
| `எண்ணாக` | `int` | To integer | `எண்ணாக("42")` → 42 |
| `தசமாக` | `float` | To decimal | `தசமாக("3.14")` → 3.14 |
| `சரமாக` | `str` | To string | `சரமாக(42)` → "42" |
| `வரம்பு` | `range` | Number sequence | `வரம்பு(1, 5)` |
| `சேர்` | `append` | Add to list | `சேர்(list, item)` |
| `நீக்கு` | `pop` | Remove from list | `நீக்கு(list)` |

---

## Practical Examples

### Example 1: Simple Calculator

```
செயல் calculator():
    அச்சிடு("=== கணிப்பான் ===")
    
    மாறி a = எண்ணாக(உள்ளீடு("First number: "))
    மாறி b = எண்ணாக(உள்ளீடு("Second number: "))
    
    அச்சிடு("Sum:", a + b)
    அச்சிடு("Difference:", a - b)
    அச்சிடு("Product:", a * b)
    அச்சிடு("Quotient:", a / b)

calculator()
```

### Example 2: Word Counter

```
செயல் count_words(text):
    மாறி words = 0
    மாறி in_word = பொய்
    
    ஒவ்வொரு char உள்ள text:
        என்றால் char == " ":
            in_word = பொய்
        இல்லையென்றால் இல்ல in_word:
            words = words + 1
            in_word = உண்மை
    
    திரும்பு words

மாறி sentence = "Hello World from agam"
அச்சிடு("Words:", count_words(sentence))
# Output: Words: 4
```

### Example 3: List Statistics

```
செயல் statistics(numbers):
    மாறி total = 0
    மாறி min_val = numbers[0]
    மாறி max_val = numbers[0]
    
    ஒவ்வொரு num உள்ள numbers:
        total = total + num
        என்றால் num < min_val:
            min_val = num
        என்றால் num > max_val:
            max_val = num
    
    மாறி avg = total / நீளம்(numbers)
    
    அச்சிடு("Count:", நீளம்(numbers))
    அச்சிடு("Sum:", total)
    அச்சிடு("Average:", avg)
    அச்சிடு("Min:", min_val)
    அச்சிடு("Max:", max_val)

statistics([10, 25, 5, 30, 15, 20])
```

---

## Summary

Built-in functions save you time by providing common operations:

- **I/O**: `அச்சிடு`, `உள்ளீடு`
- **Types**: `எண்ணாக`, `தசமாக`, `சரமாக`, `வகை`
- **Collections**: `நீளம்`, `வரம்பு`, `சேர்`, `நீக்கு`

---

**Next: [Chapter 11: Lists →](11_lists.md)**
