# Chapter 11: Lists (பட்டியல்கள்)

## What is a List?

A list is an ordered collection of items. Items can be of any type.

```
மாறி numbers = [1, 2, 3, 4, 5]
மாறி names = ["Alice", "Bob", "Charlie"]
மாறி mixed = [1, "hello", உண்மை, 3.14]
```

---

## Creating Lists

```
# Empty list
மாறி empty = []

# List with values
மாறி fruits = ["apple", "banana", "cherry"]

# List with numbers
மாறி scores = [85, 90, 78, 92, 88]
```

---

## Accessing Elements

Use index numbers (starting from 0):

```
மாறி fruits = ["apple", "banana", "cherry", "date"]

அச்சிடு(fruits[0])    # Output: apple (first)
அச்சிடு(fruits[1])    # Output: banana (second)
அச்சிடு(fruits[2])    # Output: cherry (third)
அச்சிடு(fruits[-1])   # Output: date (last)
அச்சிடு(fruits[-2])   # Output: cherry (second to last)
```

### Index Reference

| Index | Negative Index | Element |
|-------|----------------|---------|
| 0 | -4 | apple |
| 1 | -3 | banana |
| 2 | -2 | cherry |
| 3 | -1 | date |

---

## Modifying Lists

### Add Items

```
மாறி fruits = ["apple", "banana"]

சேர்(fruits, "cherry")
அச்சிடு(fruits)   # Output: [apple, banana, cherry]

சேர்(fruits, "date")
அச்சிடு(fruits)   # Output: [apple, banana, cherry, date]
```

### Remove Items

```
மாறி fruits = ["apple", "banana", "cherry"]

மாறி removed = நீக்கு(fruits)
அச்சிடு(removed)   # Output: cherry
அச்சிடு(fruits)    # Output: [apple, banana]
```

---

## List Length

```
மாறி numbers = [10, 20, 30, 40, 50]
அச்சிடு(நீளம்(numbers))   # Output: 5
```

---

## Iterating Over Lists

```
மாறி colors = ["red", "green", "blue"]

ஒவ்வொரு color உள்ள colors:
    அச்சிடு("Color:", color)

# Output:
# Color: red
# Color: green
# Color: blue
```

### With Index

```
மாறி fruits = ["apple", "banana", "cherry"]
மாறி index = 0

ஒவ்வொரு fruit உள்ள fruits:
    அச்சிடு(சரமாக(index) + ": " + fruit)
    index = index + 1

# Output:
# 0: apple
# 1: banana
# 2: cherry
```

---

## Practical Examples

### Example 1: Sum of Numbers

```
மாறி numbers = [10, 20, 30, 40, 50]
மாறி total = 0

ஒவ்வொரு num உள்ள numbers:
    total = total + num

அச்சிடு("Sum:", total)   # Output: Sum: 150
```

### Example 2: Find Maximum

```
செயல் find_max(numbers):
    மாறி max_val = numbers[0]
    
    ஒவ்வொரு num உள்ள numbers:
        என்றால் num > max_val:
            max_val = num
    
    திரும்பு max_val

மாறி scores = [78, 92, 85, 96, 88]
அச்சிடு("Highest:", find_max(scores))
# Output: Highest: 96
```

### Example 3: Filter Even Numbers

```
செயல் get_evens(numbers):
    மாறி evens = []
    
    ஒவ்வொரு num உள்ள numbers:
        என்றால் num % 2 == 0:
            சேர்(evens, num)
    
    திரும்பு evens

மாறி all = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
அச்சிடு(get_evens(all))   # Output: [2, 4, 6, 8, 10]
```

### Example 4: Reverse a List

```
செயல் reverse_list(items):
    மாறி result = []
    மாறி i = நீளம்(items) - 1
    
    வரை i >= 0:
        சேர்(result, items[i])
        i = i - 1
    
    திரும்பு result

அச்சிடு(reverse_list([1, 2, 3, 4, 5]))
# Output: [5, 4, 3, 2, 1]
```

---

## Summary

| Operation | Code |
|-----------|------|
| Create empty | `[]` |
| Create with items | `[1, 2, 3]` |
| Access by index | `list[0]` |
| Last item | `list[-1]` |
| Add item | `சேர்(list, item)` |
| Remove last | `நீக்கு(list)` |
| Length | `நீளம்(list)` |
| Iterate | `ஒவ்வொரு x உள்ள list:` |

---

**Next: [Chapter 12: Dictionaries →](12_dictionaries.md)**
