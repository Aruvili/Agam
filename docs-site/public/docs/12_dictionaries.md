# Chapter 12: Dictionaries (அகராதிகள்)

## What is a Dictionary?

A dictionary stores data as **key-value pairs**:

```
மாறி person = {
    "name": "Tamil",
    "age": 25,
    "city": "Chennai"
}
```

---

## Creating Dictionaries

```
# Empty dictionary
மாறி empty = {}

# With values
மாறி student = {
    "name": "Arjun",
    "grade": 10,
    "subjects": ["Math", "Science", "Tamil"]
}
```

---

## Accessing Values

Use the key to get its value:

```
மாறி person = {
    "name": "Tamil",
    "age": 25
}

அச்சிடு(person["name"])   # Output: Tamil
அச்சிடு(person["age"])    # Output: 25
```

---

## Practical Examples

### Example 1: Student Record

```
மாறி student = {
    "name": "Priya",
    "roll": 101,
    "marks": {
        "math": 95,
        "science": 88,
        "tamil": 92
    }
}

அச்சிடு("Name:", student["name"])
அச்சிடு("Roll:", student["roll"])
அச்சிடு("Math Marks:", student["marks"]["math"])
```

### Example 2: Word Counter

```
செயல் count_chars(text):
    மாறி counts = {}
    
    ஒவ்வொரு char உள்ள text:
        # This is simplified - in real usage you'd update counts
        அச்சிடு(char)
    
    திரும்பு counts

count_chars("hello")
```

---

## Summary

| Operation | Code |
|-----------|------|
| Create empty | `{}` |
| Create with items | `{"key": "value"}` |
| Access value | `dict["key"]` |
| Nested access | `dict["key1"]["key2"]` |

---

**Next: [Chapter 13: Keywords Reference →](13_keywords.md)**
