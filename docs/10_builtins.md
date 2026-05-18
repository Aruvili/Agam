# Chapter 10: Built-in Functions (உள்ளமைந்த செயல்கள்)

## Overview

agam comes with many built-in functions ready to use. Each function is available in both Tamil and English.

---

## Input/Output Functions

### பதிப்பி / print

Display output to the screen:

```
பதிப்பி("வணக்கம்!")           # Print text
பதிப்பி(42)                   # Print number
பதிப்பி("Value:", 100)        # Print multiple items
```

### உள்ளீடு / input

Read input from the user:

```
மாறி name = உள்ளீடு("Enter your name: ");
பதிப்பி("Hello, " + name);

மாறி age_str = உள்ளீடு("Enter your age: ");
மாறி age = எண்ணாக(age_str)    # Convert to number;
```

---

## Type Conversion Functions

### எண்ணாக / int

Convert to integer:

```
பதிப்பி(எண்ணாக("42"))       # Output: 42
பதிப்பி(எண்ணாக(3.7))        # Output: 3 (truncates)
பதிப்பி(எண்ணாக(உண்மை))     # Output: 1
```

### தசமாக / float

Convert to decimal number:

```
பதிப்பி(தசமாக("3.14"))     # Output: 3.14
பதிப்பி(தசமாக(42))          # Output: 42.0
```

### சரமாக / str

Convert to string:

```
பதிப்பி(சரமாக(42))          # Output: "42";
பதிப்பி(சரமாக(3.14))        # Output: "3.14";
பதிப்பி(சரமாக(உண்மை))      # Output: "உண்மை";

# Useful for concatenation
மாறி age = 25;
பதிப்பி("Age: " + சரமாக(age));
```

---

## Collection Functions

### நீளம் / len

Get the length of a string, list, or dictionary:

```
# String length
பதிப்பி(நீளம்("Hello"))           # Output: 5;
பதிப்பி(நீளம்("தமிழ்"))           # Output: 5;

# List length
பதிப்பி(நீளம்([1, 2, 3, 4, 5]))   # Output: 5;

# Dictionary length
மாறி data = {"a": 1, "b": 2}
பதிப்பி(நீளம்(data))              # Output: 2;
```

### வரம்பு / range

Create a sequence of numbers:

```
# range(end) - 0 to end-1
ஒவ்வொரு i உள்ள வரம்பு(5):
    பதிப்பி(i)   # 0, 1, 2, 3, 4

# range(start, end) - start to end-1
ஒவ்வொரு i உள்ள வரம்பு(1, 6):
    பதிப்பி(i)   # 1, 2, 3, 4, 5

# range(start, end, step)
ஒவ்வொரு i உள்ள வரம்பு(0, 10, 2):
    பதிப்பி(i)   # 0, 2, 4, 6, 8

# Negative step (countdown)
ஒவ்வொரு i உள்ள வரம்பு(5, 0, -1):
    பதிப்பி(i)   # 5, 4, 3, 2, 1
```

### சேர் / append

Add an item to a list:

```
மாறி fruits = ["apple", "banana"];
சேர்(fruits, "cherry")
பதிப்பி(fruits)   # Output: [apple, banana, cherry];
```

### நீக்கு / pop

Remove and return the last item from a list:

```
மாறி numbers = [1, 2, 3, 4, 5];
மாறி last = நீக்கு(numbers);
பதிப்பி(last)      # Output: 5;
பதிப்பி(numbers)   # Output: [1, 2, 3, 4];
```

### வரிசை / sort

Sort a list in ascending order:

```
மாறி numbers = [3, 1, 4, 1, 5, 9, 2, 6];
மாறி sorted = வரிசை(numbers);
பதிப்பி(sorted)   # Output: [1, 1, 2, 3, 4, 5, 6, 9];

# Also works with strings
மாறி names = ["சரவணன்", "அருண்", "குமார்"];
பதிப்பி(வரிசை(names));
```

### தலைகீழ் / reverse

Reverse a list or string:

```
# Reverse a list
மாறி numbers = [1, 2, 3, 4, 5];
பதிப்பி(தலைகீழ்(numbers))   # Output: [5, 4, 3, 2, 1];

# Reverse a string
மாறி text = "தமிழ்";
பதிப்பி(தலைகீழ்(text))     # Output: ழ்மித;
```

---

## Information Functions

### வகை / type

Get the type of a value:

```
பதிப்பி(வகை(42))           # Output: எண்
பதிப்பி(வகை("Hello"))      # Output: சரம்
பதிப்பி(வகை(உண்மை))        # Output: உண்மைபொய்
பதிப்பி(வகை([1, 2, 3]))    # Output: பட்டியல்
பதிப்பி(வகை({"a": 1}))     # Output: அகராதி
பதிப்பி(வகை(இல்லா))        # Output: இல்லா
```

---

## Math Functions

### வர்க்கம் / sqrt

Calculate the square root of a number:

```
பதிப்பி(வர்க்கம்(16))     # Output: 4
பதிப்பி(வர்க்கம்(2))      # Output: 1.4142135623730951
பதிப்பி(வர்க்கம்(100))    # Output: 10
```

### அடி / pow

Raise a number to a power:

```
பதிப்பி(அடி(2, 3))        # Output: 8 (2³)
பதிப்பி(அடி(10, 2))       # Output: 100 (10²)
பதிப்பி(அடி(5, 0))        # Output: 1 (any number to power 0)
```

### தளம் / floor

Round down to the nearest integer:

```
பதிப்பி(தளம்(3.7))        # Output: 3
பதிப்பி(தளம்(3.2))        # Output: 3
பதிப்பி(தளம்(-2.5))       # Output: -3
```

### கூரை / ceil

Round up to the nearest integer:

```
பதிப்பி(கூரை(3.2))        # Output: 4
பதிப்பி(கூரை(3.7))        # Output: 4
பதிப்பி(கூரை(-2.5))       # Output: -2
```

### முழுமை / abs

Get the absolute value of a number:

```
பதிப்பி(முழுமை(-5))       # Output: 5
பதிப்பி(முழுமை(10))       # Output: 10
பதிப்பி(முழுமை(-3.14))    # Output: 3.14
```

### குறைந்தபட்சம் / min

Find the minimum value:

```
# Multiple arguments
பதிப்பி(குறைந்தபட்சம்(5, 3, 8, 1))    # Output: 1;

# From a list
மாறி numbers = [10, 5, 20, 3, 15];
பதிப்பி(குறைந்தபட்சம்(numbers))        # Output: 3;
```

### அதிகபட்சம் / max

Find the maximum value:

```
# Multiple arguments
பதிப்பி(அதிகபட்சம்(5, 3, 8, 1))       # Output: 8;

# From a list
மாறி numbers = [10, 5, 20, 3, 15];
பதிப்பி(அதிகபட்சம்(numbers))           # Output: 20;
```

### கூட்டு / sum

Calculate the sum of a list:

```
மாறி numbers = [1, 2, 3, 4, 5];
பதிப்பி(கூட்டு(numbers))    # Output: 15;

மாறி prices = [99.99, 50.00, 25.50];
பதிப்பி(கூட்டு(prices))     # Output: 175.49;
```

### தற்செயல் / random

Generate random numbers:

```
# Random decimal between 0 and 1
மாறி r = தற்செயல்();
பதிப்பி(r)                 # Output: 0.xxx (random);

# Random integer from 0 to max-1
மாறி dice = தற்செயல்(6);
பதிப்பி(dice)              # Output: 0-5;

# Random integer in range
மாறி num = தற்செயல்(1, 100);
பதிப்பி(num)               # Output: 1-99;
```

---

## String Functions

### பிரி / split

Split a string into a list:

```
மாறி sentence = "Hello World Tamil";
மாறி words = பிரி(sentence, " ");
பதிப்பி(words)             # Output: [Hello, World, Tamil];

மாறி csv = "apple,banana,cherry";
மாறி items = பிரி(csv, ",");
பதிப்பி(items)             # Output: [apple, banana, cherry];
```

### இணை / join

Join a list into a string:

```
மாறி words = ["Hello", "World", "Tamil"];
மாறி sentence = இணை(" ", words);
பதிப்பி(sentence)          # Output: Hello World Tamil;

மாறி items = ["a", "b", "c"];
பதிப்பி(இணை("-", items))   # Output: a-b-c;
```

### மேல் / upper

Convert string to uppercase:

```
மாறி text = "hello world";
பதிப்பி(மேல்(text))        # Output: HELLO WORLD;
```

### கீழ் / lower

Convert string to lowercase:

```
மாறி text = "HELLO WORLD";
பதிப்பி(கீழ்(text))        # Output: hello world;
```

### ஒழுங்கு / trim

Remove whitespace from both ends:

```
மாறி text = "   Hello World   ";
பதிப்பி(ஒழுங்கு(text))     # Output: Hello World;
```

### மாற்று / replace

Replace occurrences in a string:

```
மாறி text = "Hello World";
பதிப்பி(மாற்று(text, "World", "Tamil"));
# Output: Hello Tamil

மாறி phone = "123-456-7890";
பதிப்பி(மாற்று(phone, "-", ""));
# Output: 1234567890
```

### தொடங்கு / startswith

Check if string starts with a prefix:

```
மாறி text = "Hello World";
பதிப்பி(தொடங்கு(text, "Hello"))    # Output: உண்மை;
பதிப்பி(தொடங்கு(text, "World"))    # Output: பொய்;
```

### முடிவு / endswith

Check if string ends with a suffix:

```
மாறி file = "document.pdf";
பதிப்பி(முடிவு(file, ".pdf"))      # Output: உண்மை;
பதிப்பி(முடிவு(file, ".txt"))      # Output: பொய்;
```

### உள்ளதா / contains

Check if a string or list contains a value:

```
# String contains
மாறி text = "Hello World";
பதிப்பி(உள்ளதா(text, "World"))     # Output: உண்மை;
பதிப்பி(உள்ளதா(text, "Tamil"))     # Output: பொய்;

# List contains
மாறி fruits = ["apple", "banana", "cherry"];
பதிப்பி(உள்ளதா(fruits, "banana"))  # Output: உண்மை;
```

---

## File I/O Functions

### படி / read_file

Read contents of a file:

```
மாறி content = படி("data.txt");
பதிப்பி(content);

# Read and process line by line
மாறி lines = பிரி(படி("data.txt"), "\n");
சுற்று (line உள் lines) {
    பதிப்பி(line);
}
```

### எழுது / write_file

Write content to a file:

```
மாறி content = "வணக்கம் உலகம்!";
எழுது("output.txt", content)

# Write multiple lines
மாறி lines = ["Line 1", "Line 2", "Line 3"];
எழுது("output.txt", இணை("\n", lines))
```

### உள்ளது / file_exists

Check if a file exists:

```
எனில் உள்ளது("config.txt"):
    மாறி config = படி("config.txt");
    பதிப்பி("Config loaded!");
} இல்லையெனில் {
    பதிப்பி("Config file not found!");
}
```

---

## System Functions

### வெளியேறு / exit

Exit the program:

```
பதிப்பி("Goodbye!")
வெளியேறு()           # Exit with code 0

# Exit with error code
வெளியேறு(1)          # Exit with code 1
```

---

## Quick Reference Table

### I/O Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `பதிப்பி` | `print` | Display output | `பதிப்பி("Hi")` |
| `உள்ளீடு` | `input` | Read input | `உள்ளீடு("Name: ")` |

### Type Conversion

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `எண்ணாக` | `int` | To integer | `எண்ணாக("42")` → 42 |
| `தசமாக` | `float` | To decimal | `தசமாக("3.14")` → 3.14 |
| `சரமாக` | `str` | To string | `சரமாக(42)` → "42" |
| `வகை` | `type` | Get type | `வகை(42)` → எண் |

### Collection Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `நீளம்` | `len` | Get length | `நீளம்("Hello")` → 5 |
| `வரம்பு` | `range` | Number sequence | `வரம்பு(1, 5)` |
| `சேர்` | `append` | Add to list | `சேர்(list, item)` |
| `நீக்கு` | `pop` | Remove from list | `நீக்கு(list)` |
| `வரிசை` | `sort` | Sort list | `வரிசை([3,1,2])` → [1,2,3] |
| `தலைகீழ்` | `reverse` | Reverse list/string | `தலைகீழ்([1,2,3])` → [3,2,1] |

### Math Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `வர்க்கம்` | `sqrt` | Square root | `வர்க்கம்(16)` → 4 |
| `அடி` | `pow` | Power | `அடி(2, 3)` → 8 |
| `தளம்` | `floor` | Round down | `தளம்(3.7)` → 3 |
| `கூரை` | `ceil` | Round up | `கூரை(3.2)` → 4 |
| `முழுமை` | `abs` | Absolute value | `முழுமை(-5)` → 5 |
| `குறைந்தபட்சம்` | `min` | Minimum | `குறைந்தபட்சம்(3, 1, 4)` → 1 |
| `அதிகபட்சம்` | `max` | Maximum | `அதிகபட்சம்(3, 1, 4)` → 4 |
| `கூட்டு` | `sum` | Sum of list | `கூட்டு([1,2,3])` → 6 |
| `தற்செயல்` | `random` | Random number | `தற்செயல்()` → 0.xxx |

### String Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `பிரி` | `split` | Split string | `பிரி("a,b", ",")` → [a, b] |
| `இணை` | `join` | Join list | `இணை("-", [a,b])` → "a-b" |
| `மேல்` | `upper` | Uppercase | `மேல்("hi")` → "HI" |
| `கீழ்` | `lower` | Lowercase | `கீழ்("HI")` → "hi" |
| `ஒழுங்கு` | `trim` | Trim whitespace | `ஒழுங்கு(" hi ")` → "hi" |
| `மாற்று` | `replace` | Replace text | `மாற்று("ab", "a", "x")` → "xb" |
| `தொடங்கு` | `startswith` | Starts with | `தொடங்கு("hi", "h")` → உண்மை |
| `முடிவு` | `endswith` | Ends with | `முடிவு("hi", "i")` → உண்மை |
| `உள்ளதா` | `contains` | Contains | `உள்ளதா("hi", "i")` → உண்மை |

### File I/O Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `படி` | `read_file` | Read file | `படி("file.txt")` |
| `எழுது` | `write_file` | Write file | `எழுது("file.txt", "data")` |
| `உள்ளது` | `file_exists` | File exists | `உள்ளது("file.txt")` → உண்மை/பொய் |

---

## Standard Library Modules

### Time Module

| Tamil | English | Description |
|-------|---------|-------------|
| `நேரம்()` | `time()` | Current timestamp |
| `தூக்கம்(n)` | `sleep(n)` | Sleep for n seconds |
| `தேதி()` | `date()` | Current date string |
| `நாள்()` | `now()` | Current time components |

**Example:**
```
பதிப்பி(தேதி())   # 2024-10-25 15:30:00
தூக்கம்(2)       # Wait 2 seconds
பதிப்பி(நேரம்())  # 1729864200
```

### HTTP Module

| Tamil | English | Description |
|-------|---------|-------------|
| `வலை_படி(url)` | `http_get(url)` | GET request |
| `வலை_அனுப்பு(url, data)` | `http_post(url, data)` | POST request |
| `வலை_புதுப்பி(url, data)` | `http_put(url, data)` | PUT request |
| `வலை_நீக்கு(url)` | `http_delete(url)` | DELETE request |
| `வலை_கோரிக்கை(config)` | `http_request(config)` | Custom request |
| `கோப்பு_பதிவேற்று(url, path)` | `file_upload(url)` | Upload file |
| `json_படி(str)` | `json_parse(str)` | Parse JSON |

**Example:**
```
# GET request
மாறி data = http_get("https://api.example.com/users");
பதிப்பி(data["status"]);

# Custom request
மாறி resp = http_request({
    "url": "https://api.example.com",
    "method": "POST",
    "headers": {"Auth": "Token"},
    "body": {"name": "Test"}
})
```

### WebSocket Module

| Tamil | English | Description |
|-------|---------|-------------|
| `சாக்கெட்_இணை(url)` | `ws_connect(url)` | Connect to WebSocket |
| `சாக்கெட்_அனுப்பு(conn, msg)` | `ws_send(conn, msg)` | Send message |
| `சாக்கெட்_படி(conn)` | `ws_receive(conn)` | Receive message |
| `சாக்கெட்_மூடு(conn)` | `ws_close(conn)` | Close connection |

---

## System Functions

| Tamil | English | Purpose | Example |
|-------|---------|---------|---------|
| `வெளியேறு` | `exit` | Exit program | `வெளியேறு(0)` |

---

## Practical Examples

### Example 1: Simple Calculator

```
செயல் calculator() {
    பதிப்பி("=== கணிப்பான் ===");
    
    மாறி a = எண்ணாக(உள்ளீடு("First number: "));
    மாறி b = எண்ணாக(உள்ளீடு("Second number: "));
    
    பதிப்பி("Sum:", a + b);
    பதிப்பி("Difference:", a - b);
    பதிப்பி("Product:", a * b);
    பதிப்பி("Quotient:", a / b);

}
calculator()
```

### Example 2: Word Counter

```
செயல் count_words(text) {
    மாறி words = பிரி(text, " ");
    விடை நீளம்(words);

}
மாறி sentence = "Hello World from agam";
பதிப்பி("Words:", count_words(sentence));
# Output: Words: 4
```

### Example 3: List Statistics

```
செயல் statistics(numbers) {
    பதிப்பி("Count:", நீளம்(numbers));
    பதிப்பி("Sum:", கூட்டு(numbers));
    பதிப்பி("Average:", கூட்டு(numbers) / நீளம்(numbers));
    பதிப்பி("Min:", குறைந்தபட்சம்(numbers));
    பதிப்பி("Max:", அதிகபட்சம்(numbers));

}
statistics([10, 25, 5, 30, 15, 20])
```

### Example 4: File Processing

```
# Read, process, and write file
எனில் உள்ளது("input.txt"):
    மாறி content = படி("input.txt");
    மாறி upper_content = மேல்(content);
    எழுது("output.txt", upper_content)
    பதிப்பி("File processed successfully!");
} இல்லையெனில் {
    பதிப்பி("Input file not found!");
}
```

---

## Summary

Built-in functions save you time by providing common operations:

- **I/O**: `பதிப்பி`, `உள்ளீடு`
- **Types**: `எண்ணாக`, `தசமாக`, `சரமாக`, `வகை`
- **Collections**: `நீளம்`, `வரம்பு`, `சேர்`, `நீக்கு`, `வரிசை`, `தலைகீழ்`
- **Math**: `வர்க்கம்`, `அடி`, `தளம்`, `கூரை`, `முழுமை`, `குறைந்தபட்சம்`, `அதிகபட்சம்`, `கூட்டு`, `தற்செயல்`
- **String**: `பிரி`, `இணை`, `மேல்`, `கீழ்`, `ஒழுங்கு`, `மாற்று`, `தொடங்கு`, `முடிவு`, `உள்ளதா`
- **File I/O**: `படி`, `எழுது`, `உள்ளது`
- **System**: `வெளியேறு`

---

**Next: [Chapter 11: Lists →](11_lists.md)**
