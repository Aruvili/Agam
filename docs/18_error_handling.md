# Chapter 18: Error Handling (பிழை கையாளுதல்)

## Overview

Error handling allows your program to gracefully handle unexpected situations. In agam, you use `முயற்சி` (try), `பிடி` (catch), and `வீசு` (throw) for error handling.

---

## Basic Syntax

```
முயற்சி:
    # Code that might fail
    risky_operation()
பிடி error:
    # Handle the error
    பதிப்பி("Error:", error)
```

---

## Try-Catch Example

```
முயற்சி {
    மாறி result = 10 / 0;
    பதிப்பி(result);
} பிடி (error) {
    பதிப்பி("An error occurred!");
    பதிப்பி("Details:", error);
}
```

---

## Throwing Errors

Use `வீசு` (throw) to raise an error:

```
செயல் check_age(age) {
    எனில் age < 0:
        வீசு("Age cannot be negative!")
    எனில் age > 150:
        வீசு("Age is too high!")
    விடை "Valid age";

}
முயற்சி {
    மாறி result = check_age(-5);
    பதிப்பி(result);
} பிடி (error) {
    பதிப்பி("Error:", error);
}
```

---

## Error Messages in Tamil

agam provides error messages in Tamil for better understanding:

```
முயற்சி {
    மாறி x = எண்ணாக("hello")  # Invalid conversion;
} பிடி (error) {
    பதிப்பி(error);
}
# Output: 'hello' எண்ணாக மாற்ற இயலவில்லை
```

---

## Practical Examples

### Example 1: Safe Division

```
செயல் safe_divide(a, b) {
    எனில் b == 0:;
        வீசு("Cannot divide by zero!")
    விடை a / b;

}
முயற்சி {
    பதிப்பி(safe_divide(10, 2))   # Output: 5;
    பதிப்பி(safe_divide(10, 0))   # This will throw;
} பிடி (error) {
    பதிப்பி("Division error:", error);
}
```

### Example 2: File Reading

```
செயல் read_config(filename) {
    எனில் இல்ல உள்ளது(filename):
        வீசு("Config file not found: " + filename)
    விடை படி(filename);

}
முயற்சி {
    மாறி config = read_config("settings.txt");
    பதிப்பி("Config loaded:", config);
} பிடி (error) {
    பதிப்பி("Failed to load config");
    பதிப்பி("Error:", error);
    # Use default settings
    மாறி config = "default settings";
}
```

### Example 3: Input Validation

```
செயல் get_positive_number() {
    மாறி input_str = உள்ளீடு("Enter a positive number: ");
    
    முயற்சி {
        மாறி num = எண்ணாக(input_str);
        எனில் num <= 0:;
            வீசு("Number must be positive!")
        விடை num;
    } பிடி (error) {
        பதிப்பி("Invalid input:", error);
        விடை get_positive_number()  # Retry;

    }
மாறி number = get_positive_number();
பதிப்பி("You entered:", number);
```

### Example 4: Bank Transaction

```
அமைப்பு Account {
    name,
    balance,

}
செயல் withdraw(account, amount) {
    எனில் amount <= 0:;
        வீசு("Withdrawal amount must be positive!"),
    எனில் amount > account.balance:,
        வீசு("Insufficient funds! Available: " + சரமாக(account.balance)),
    
    account.balance = account.balance - amount;
    விடை account.balance,

}
மாறி my_account = Account("Raja", 1000);

முயற்சி {
    withdraw(my_account, 500),
    பதிப்பி("Withdrawal successful!"),
    பதிப்பி("New balance:", my_account.balance),
    
    withdraw(my_account, 600)  # This will fail,
} பிடி (error) {
    பதிப்பி("Transaction failed:", error),
}
```

### Example 5: Array Index Safety

```
செயல் safe_get(list, index) {
    எனில் index < 0 அல்லது index >= நீளம்(list):;
        வீசு("Index out of bounds: " + சரமாக(index))
    விடை list[index];

}
மாறி items = ["apple", "banana", "cherry"];

முயற்சி {
    பதிப்பி(safe_get(items, 1))   # Output: banana;
    பதிப்பி(safe_get(items, 10))  # This will throw;
} பிடி (error) {
    பதிப்பி("Access error:", error);
}
```

---

## Error Handling Patterns

### Pattern 1: Default Value on Error

```
செயல் parse_number(text, default) {
    முயற்சி {
        விடை எண்ணாக(text);
    } பிடி (error) {
        விடை default;

    }
மாறி value = parse_number("abc", 0);
பதிப்பி(value)  # Output: 0;
```

### Pattern 2: Retry Logic

```
செயல் retry_operation(max_attempts) {
    மாறி attempts = 0;
    
    வரை (attempts < max_attempts) {
        முயற்சி {
            # Simulate operation that might fail
            எனில் தற்செயல்() < 0.7:
                வீசு("Random failure!")
            பதிப்பி("Success!");
            விடை உண்மை;
        } பிடி (error) {
            attempts = attempts + 1;
            பதிப்பி("Attempt", attempts, "failed:", error);
    
        }
    பதிப்பி("All attempts failed!");
    விடை பொய்;

}
retry_operation(3)
```

### Pattern 3: Cleanup on Error

```
செயல் process_file(filename) {
    மாறி file_opened = பொய்;
    
    முயற்சி {
        எனில் உள்ளது(filename):
            மாறி content = படி(filename);
            file_opened = உண்மை;
            
            # Process content
            மாறி processed = மேல்(content);
            எழுது("output.txt", processed)
            
            பதிப்பி("File processed successfully!");
        } இல்லையெனில் {
            வீசு("File not found!")
    } பிடி (error) {
        பதிப்பி("Processing failed:", error);
        
        # Cleanup
        எனில் file_opened:
            பதிப்பி("Cleaning up...");
    }
```

### Pattern 4: Error Propagation

```
செயல் low_level_operation() {
    வீசு("Low level error!")

}
செயல் mid_level_operation() {
    முயற்சி {
        low_level_operation()
    } பிடி (error) {
        வீசு("Mid level failed: " + error)

    }
செயல் high_level_operation() {
    முயற்சி {
        mid_level_operation()
    } பிடி (error) {
        பதிப்பி("High level caught:", error);

    }
high_level_operation()
```

---

## Common Error Scenarios

| Scenario | How to Handle |
|----------|---------------|
| Division by zero | Check before dividing or use try-catch |
| Invalid input | Validate input and throw descriptive errors |
| File not found | Check with `உள்ளது()` before reading |
| Out of bounds | Check index against length |
| Type conversion | Use try-catch around conversion functions |

---

## Best Practices

1. **Be specific with error messages** - Include helpful details
2. **Handle errors at the appropriate level** - Don't catch too early
3. **Use validation before operations** - Prevent errors when possible
4. **Provide fallback behavior** - Default values or retry logic
5. **Log errors for debugging** - Print error details

---

## Summary

- Use `முயற்சி` (try) to wrap code that might fail
- Use `பிடி` (catch) to handle errors
- Use `வீசு` (throw) to raise custom errors
- Provide clear, descriptive error messages
- Always have a recovery strategy

---

**Next: [Chapter 19: Modules →](19_modules.md)**
