# Chapter 5: Data Types (தரவு வகைகள்)

## What are Data Types?

Data types tell the computer what kind of information you're working with. agam has several built-in types:

| Type | Tamil | Example |
|------|-------|---------|
| Number | எண் | `42`, `3.14` |
| String | சரம் | `"Hello"`, `"வணக்கம்"` |
| Boolean | உண்மைபொய் | `உண்மை`, `பொய்` |
| List | பட்டியல் | `[1, 2, 3]` |
| Dictionary | அகராதி | `{"key": "value"}` |
| Null | இல்லா | `இல்லா` |

---

## Numbers (எண்)

Numbers can be integers or decimals:

```
# Integers (முழு எண்கள்)
மாறி age = 25
மாறி year = 2024
மாறி negative = -10

# Decimals (தசம எண்கள்)
மாறி price = 99.50
மாறி pi = 3.14159
மாறி tiny = 0.001
```

### Tamil Numerals

agam also supports Tamil numerals!

```
மாறி எண் = ௧௨௩    # 123 in Tamil numerals
அச்சிடு(எண்)      # Output: 123
```

Tamil numerals:
| Tamil | Value |
|-------|-------|
| ௦ | 0 |
| ௧ | 1 |
| ௨ | 2 |
| ௩ | 3 |
| ௪ | 4 |
| ௫ | 5 |
| ௬ | 6 |
| ௭ | 7 |
| ௮ | 8 |
| ௯ | 9 |

---

## Strings (சரம்)

Strings are text enclosed in double quotes:

```
மாறி greeting = "வணக்கம்!"
மாறி name = "agam"
மாறி empty = ""
மாறி sentence = "I love programming in Tamil"
```

### String Operations

```
# Concatenation (joining)
மாறி full = "Hello" + " " + "World"
அச்சிடு(full)   # Output: Hello World

# Repetition
மாறி stars = "*" * 5
அச்சிடு(stars)   # Output: *****

# Length
அச்சிடு(நீளம்("Hello"))   # Output: 5
அச்சிடு(நீளம்("தமிழ்"))   # Output: 5
```

### Accessing Characters

```
மாறி word = "Hello"
அச்சிடு(word[0])   # Output: H (first character)
அச்சிடு(word[1])   # Output: e (second character)
அச்சிடு(word[-1])  # Output: o (last character)
```

### Escape Characters

```
மாறி text = "Line 1\nLine 2"   # \n = new line
அச்சிடு(text)
# Output:
# Line 1
# Line 2

மாறி quote = "He said \"Hello\""   # \" = quote inside string
அச்சிடு(quote)   # Output: He said "Hello"
```

---

## Booleans (உண்மைபொய்)

Booleans represent true or false:

```
மாறி is_student = உண்மை    # true
மாறி is_old = பொய்        # false

# In comparisons
மாறி result = 10 > 5      # உண்மை
மாறி equal = 5 == 6       # பொய்
```

### Boolean Keywords

| Tamil | English | Meaning |
|-------|---------|---------|
| உண்மை | true | True |
| பொய் | false | False |

---

## Null (இல்லா)

Null represents "no value" or "nothing":

```
மாறி data = இல்லா   # null/nothing

என்றால் data == இல்லா:
    அச்சிடு("No data available")
```

---

## Lists (பட்டியல்)

Lists store multiple values in order:

```
# Creating lists
மாறி numbers = [1, 2, 3, 4, 5]
மாறி names = ["Alice", "Bob", "Charlie"]
மாறி mixed = [1, "hello", உண்மை, 3.14]

# Empty list
மாறி empty = []
```

### List Operations

```
மாறி fruits = ["apple", "banana", "cherry"]

# Access by index (starts at 0)
அச்சிடு(fruits[0])   # Output: apple
அச்சிடு(fruits[2])   # Output: cherry
அச்சிடு(fruits[-1])  # Output: cherry (last item)

# Length
அச்சிடு(நீளம்(fruits))   # Output: 3

# Add item
சேர்(fruits, "orange")
அச்சிடு(fruits)   # Output: [apple, banana, cherry, orange]

# Remove last item
மாறி removed = நீக்கு(fruits)
அச்சிடு(removed)   # Output: orange
```

---

## Dictionaries (அகராதி)

Dictionaries store key-value pairs:

```
மாறி person = {
    "name": "Tamil",
    "age": 25,
    "city": "Chennai"
}

# Access by key
அச்சிடு(person["name"])   # Output: Tamil
அச்சிடு(person["age"])    # Output: 25
```

---

## Type Checking

Use `வகை()` to check a value's type:

```
அச்சிடு(வகை(42))           # Output: எண்
அச்சிடு(வகை("Hello"))      # Output: சரம்
அச்சிடு(வகை(உண்மை))        # Output: உண்மைபொய்
அச்சிடு(வகை([1, 2, 3]))    # Output: பட்டியல்
அச்சிடு(வகை(இல்லா))        # Output: இல்லா
```

---

## Type Conversion

Convert between types:

```
# To integer
மாறி num = எண்ணாக("42")
அச்சிடு(num + 8)   # Output: 50

# To float
மாறி decimal = தசமாக("3.14")
அச்சிடு(decimal)   # Output: 3.14

# To string
மாறி text = சரமாக(100)
அச்சிடு("Value: " + text)   # Output: Value: 100
```

### Conversion Functions

| Function | Purpose | Example |
|----------|---------|---------|
| `எண்ணாக()` / `int()` | To integer | `எண்ணாக("42")` → `42` |
| `தசமாக()` / `float()` | To decimal | `தசமாக("3.14")` → `3.14` |
| `சரமாக()` / `str()` | To string | `சரமாக(42)` → `"42"` |

---

## Summary

| Type | Create | Example |
|------|--------|---------|
| Number | Just write it | `42`, `3.14` |
| String | Use quotes | `"Hello"` |
| Boolean | Use keywords | `உண்மை`, `பொய்` |
| List | Use brackets | `[1, 2, 3]` |
| Dict | Use braces | `{"key": "value"}` |
| Null | Use keyword | `இல்லா` |

---

**Next: [Chapter 6: Operators →](06_operators.md)**
