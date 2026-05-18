# Chapter 9: Functions (செயல்கள்)

## What is a Function?

A function is a reusable block of code that performs a specific task.

```
செயல் greet() {
    பதிப்பி("Hello!");
}

greet()   # Call the function
# Output: Hello!
```

---

## Defining Functions

Use `செயல்` (meaning "function" or "action"):

```
செயல் say_hello() {
    பதிப்பி("வணக்கம்!");
    பதிப்பி("Welcome to agam");
}
# Call the function
say_hello()
```

### Syntax

```
செயல் function_name(parameters) {
    # code block
    # (indented with 4 spaces)
}
```

---

## Parameters (அளவுருக்கள்)

Functions can accept input values:

```
செயல் greet(name) {
    பதிப்பி("வணக்கம், " + name + "!");
}
greet("Tamil")      # Output: வணக்கம், Tamil!
greet("agam")    # Output: வணக்கம், agam!
```

### Multiple Parameters

```
செயல் add(a, b) {
    மாறி result = a + b;
    பதிப்பி(result);
}
add(5, 3)    # Output: 8
add(10, 20)  # Output: 30
```

---

## Return Values (விடை)

Use `விடை` to return a value:

```
செயல் add(a, b) {
    விடை a + b;

}
மாறி result = add(5, 3);
பதிப்பி(result)   # Output: 8;

# Use directly in expressions
பதிப்பி(add(10, 20) * 2)   # Output: 60;
```

### Multiple Returns

```
செயல் get_min_max(numbers) {
    மாறி min = numbers[0];
    மாறி max = numbers[0];
    
    சுற்று (num உள் numbers) {
        எனில் (num < min) {
            min = num;
        }
        எனில் (num > max) {
            max = num;
        }
    }
    விடை [min, max];
}
மாறி result = get_min_max([5, 2, 8, 1, 9]);
பதிப்பி("Min:", result[0])   # Output: Min: 1;
பதிப்பி("Max:", result[1])   # Output: Max: 9;
```

---

## Early Return

Return can exit a function early:

```
செயல் find_first_negative(numbers) {
    சுற்று (num உள் numbers) {
        எனில் (num < 0) {
            விடை num;    # Exit immediately
        }
    }
    விடை இல்லா;         # No negative found
}
பதிப்பி(find_first_negative([1, 2, -3, 4]))   # Output: -3;
பதிப்பி(find_first_negative([1, 2, 3, 4]))    # Output: இல்லா;
```

---

## Recursion (மீளுறு)

A function can call itself:

```
செயல் factorial(n) {
    எனில் (n <= 1) {
        விடை 1;
    } இல்லையெனில் {
        விடை n * factorial(n - 1);
    }
}
பதிப்பி(factorial(5))   # Output: 120;
# 5! = 5 × 4 × 3 × 2 × 1 = 120
```

### Fibonacci Example

```
செயல் fibonacci(n) {
    எனில் (n <= 0) {
        விடை 0;
    } இல்லையெனில் (n == 1) {
        விடை 1;
    } இல்லையெனில் {
        விடை fibonacci(n - 1) + fibonacci(n - 2);
    }
}
சுற்று (i உள் வரம்பு(10)) {
    பதிப்பி(fibonacci(i));
}
# Output: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
```

---

## Practical Examples

### Example 1: Check if Prime

```
செயல் is_prime(n) {
    எனில் (n < 2) {
        விடை பொய்;
    }
    
    சுற்று (i உள் வரம்பு(2, n)) {
        எனில் (n % i == 0) {
            விடை பொய்;
        }
    }
    விடை உண்மை;
}
# Test
சுற்று (num உள் வரம்பு(1, 20)) {
    எனில் (is_prime(num)) {
        பதிப்பி(சரமாக(num) + " is prime");
    }
}
```

### Example 2: Calculate Average

```
செயல் average(numbers) {
    மாறி total = 0;
    
    சுற்று (num உள் numbers) {
        total = total + num;
    
    }
    விடை total / நீளம்(numbers);

}
மாறி scores = [85, 90, 78, 92, 88];
பதிப்பி("Average:", average(scores));
# Output: Average: 86.6
```

### Example 3: String Reversal

```
செயல் reverse(text) {
    மாறி result = "";
    
    சுற்று (char உள் text) {
        result = char + result;
    
    }
    விடை result;

}
பதிப்பி(reverse("Tamil"))     # Output: limaT;
பதிப்பி(reverse("வணக்கம்"))    # Output: ம்கக்ணவ;
```

### Example 4: Temperature Converter

```
செயல் celsius_to_fahrenheit(c) {
    விடை (c * 9 / 5) + 32;

}
செயல் fahrenheit_to_celsius(f) {
    விடை (f - 32) * 5 / 9;

}
பதிப்பி("25°C =", celsius_to_fahrenheit(25), "°F");
பதிப்பி("77°F =", fahrenheit_to_celsius(77), "°C");
```

---

## Function Best Practices

### 1. Use Descriptive Names

```
# Good ✅
செயல் calculate_area(length, width) {
    விடை length * width;

}
# Bad ❌
செயல் calc(l, w) {
    விடை l * w;
}
```

### 2. One Function, One Task

```
# Good ✅ - Separate functions
செயல் validate_input(data) {
    # validation logic

}
செயல் process_data(data) {
    # processing logic

}
# Bad ❌ - Too much in one function
செயல் do_everything(data) {
    # validation + processing + output
}
```

### 3. Keep Functions Short

- Ideally under 20 lines
- If longer, consider splitting into smaller functions

---

## Lambda Functions (செயலி)

As of version 0.1.2, Agam supports anonymous functions (lambda functions). These are useful for passing functions as arguments or defining short helper functions.

### Syntax

There are three ways to define a lambda function:

1. **Tamil Keyword**: `செயலி(params): expr`
2. **English Keyword**: `lambda(params): expr`
3. **Arrow Syntax**: `(params) => expr`

### Examples

```
# Define square function
மாறி sq = செயலி(x): x * x;
பதிப்பி(sq(5))  # Output: 25;

# English keyword
மாறி add = lambda(a, b): a + b;
பதிப்பி(add(10, 20))  # Output: 30;

# Arrow syntax
மாறி hello = (name) => "Hello, " + name;
பதிப்பி(hello("Agam"))  # Output: Hello, Agam;
```

### Passing to Functions

Lambdas are commonly used with higher-order functions:

```
# Custom map function
செயல் map(list, func) {
    மாறி result = [];
    சுற்று (item உள் list) {
        result = result + [func(item)];
    }
    விடை result;

}
மாறி numbers = [1, 2, 3];
மாறி doubled = map(numbers, (x) => x * 2);
பதிப்பி(doubled)  # Output: [2, 4, 6];
```

---

## Summary

| Concept | Syntax |
|---------|--------|
| Define | `செயல் name() { ... }` |
| Parameters | `செயல் name(param) { ... }` |
| Return | `விடை value;` |
| Call | `name()` or `name(arg)` |

---

**Next: [Chapter 10: Built-in Functions →](10_builtins.md)**
