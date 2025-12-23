# Chapter 14: Error Messages (பிழைச் செய்திகள்)

## Understanding Errors

Errors help you find and fix problems in your code. agam provides error messages in Tamil with English explanations.

---

## Types of Errors

### 1. Lexer Errors (சொற்பிழை)

Problems with individual characters or tokens:

```
சொற்பிழை (Lexer Error) [1:5]: எதிர்பாராத எழுத்து '@'
```

**Common causes:**
- Invalid characters
- Unterminated strings

**Example:**
```
மாறி x = @invalid    # Error: unexpected character
மாறி text = "hello   # Error: unterminated string
```

---

### 2. Parser Errors (தொடரியல் பிழை)

Problems with code structure:

```
தொடரியல் பிழை (Syntax Error) [2:1]: உள்தள்ளுதல் எதிர்பார்க்கப்படுகிறது
```

**Common causes:**
- Missing colons (`:`)
- Missing indentation
- Mismatched brackets

**Example:**
```
# Missing colon
என்றால் x > 5     # Error: ':' expected
    அச்சிடு(x)

# Missing indentation
என்றால் x > 5:
அச்சிடு(x)        # Error: indentation expected
```

---

### 3. Runtime Errors (இயக்க பிழை)

Problems during program execution:

```
இயக்க பிழை (Runtime Error) [5:10]: வரையறுக்கப்படாத மாறி 'name'
```

**Common causes:**
- Undefined variables
- Division by zero
- Index out of range
- Type mismatches

---

## Common Errors and Solutions

### "வரையறுக்கப்படாத மாறி" (Undefined Variable)

**Error:** You're using a variable that doesn't exist.

```
அச்சிடு(பெயர்)   # Error: பெயர் is not defined
```

**Solution:** Define the variable first.

```
மாறி பெயர் = "Tamil"
அச்சிடு(பெயர்)   # Works!
```

---

### "பூஜ்ஜியத்தால் வகுக்க இயலாது" (Division by Zero)

**Error:** Cannot divide by zero.

```
மாறி result = 10 / 0   # Error!
```

**Solution:** Check before dividing.

```
மாறி divisor = 0
என்றால் divisor != 0:
    மாறி result = 10 / divisor
இல்லை:
    அச்சிடு("Cannot divide by zero!")
```

---

### "குறியீட்டு வரம்பிற்கு வெளியே" (Index Out of Range)

**Error:** Accessing an index that doesn't exist.

```
மாறி list = [1, 2, 3]
அச்சிடு(list[10])   # Error: index 10 is out of range
```

**Solution:** Check the list length.

```
மாறி index = 10
என்றால் index < நீளம்(list):
    அச்சிடு(list[index])
இல்லை:
    அச்சிடு("Index out of range!")
```

---

### "':' எதிர்பார்க்கப்படுகிறது" (Expected ':')

**Error:** Missing colon after condition or function.

```
என்றால் x > 5    # Missing colon!
    அச்சிடு(x)
```

**Solution:** Add the colon.

```
என்றால் x > 5:   # Correct!
    அச்சிடு(x)
```

---

### "மாறாத மாறி, மாற்ற இயலாது" (Cannot Modify Constant)

**Error:** Trying to change a constant value.

```
மாறாத PI = 3.14
PI = 3.14159   # Error: cannot modify constant
```

**Solution:** Use `மாறி` instead if the value needs to change.

```
மாறி pi = 3.14    # Use variable instead
pi = 3.14159      # Now it's allowed
```

---

## Reading Error Messages

Error format:
```
பிழை வகை [வரி:நெடுவரிசை]: செய்தி
```

Example:
```
இயக்க பிழை [5:10]: வரையறுக்கப்படாத மாறி 'x'
     ↑        ↑ ↑                        ↑
   type    line col                  message
```

- **வரி (Line):** Which line has the error
- **நெடுவரிசை (Column):** Which character on that line
- **செய்தி (Message):** What went wrong

---

## Tips for Debugging

1. **Read the error message carefully** - it tells you exactly what's wrong

2. **Check the line number** - go to that specific line

3. **Look for common issues:**
   - Missing quotes around strings
   - Missing colons after `என்றால்`, `வரை`, `செயல்`
   - Incorrect indentation
   - Typos in variable names

4. **Use print statements** to debug:
   ```
   அச்சிடு("Debug: x =", x)
   ```

5. **Test small pieces** of code in the REPL

---

## Summary

| Error Type | Tamil | Common Cause |
|------------|-------|--------------|
| Lexer | சொற்பிழை | Invalid characters |
| Syntax | தொடரியல் பிழை | Missing `:` or indentation |
| Runtime | இயக்க பிழை | Undefined variables, division by 0 |

---

**🎉 Congratulations!**

You've completed The agam Book! You now know:
- ✅ Variables and data types
- ✅ Operators and expressions
- ✅ Conditionals and loops
- ✅ Functions
- ✅ Lists and dictionaries
- ✅ Error handling

**Happy coding in Tamil! தமிழில் நிரலாக்கம் செய்யுங்கள்!** 🇮🇳
