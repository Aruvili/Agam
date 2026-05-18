# Chapter 3: Hello World (வணக்கம் உலகம்)

## Your First Program

Let's write your first program in Tamil!

### Step 1: Create the File

Create a file named `hello.agam`:

```
பதிப்பி("வணக்கம் உலகம்!")
```

### Step 2: Run It

```bash
cd d:\agam\Language
cargo run --release -- hello.agam
```

### Step 3: See the Output

```
வணக்கம் உலகம்!
```

🎉 **You just wrote your first Tamil program!**

---

## Understanding the Code

Let's break down `பதிப்பி("வணக்கம் உலகம்!")`:

| Part | Meaning |
|------|---------|
| `பதிப்பி` | The "print" function - displays text |
| `(` | Opening parenthesis - starts the arguments |
| `"வணக்கம் உலகம்!"` | The text to display (a "string") |
| `)` | Closing parenthesis - ends the arguments |

---

## More Examples

### Print Multiple Lines

```
பதிப்பி("வணக்கம்!")
பதிப்பி("எப்படி இருக்கிறீர்கள்?")
பதிப்பி("நலமா?")
```

Output:
```
வணக்கம்!
எப்படி இருக்கிறீர்கள்?
நலமா?
```

### Print Numbers

```
பதிப்பி(42)
பதிப்பி(3.14159)
```

Output:
```
42
3.14159
```

### Print Multiple Items

```
பதிப்பி("Number:", 42)
பதிப்பி("Name:", "agam", "Version:", 1)
```

Output:
```
Number: 42
Name: agam Version: 1
```

### Print with Expressions

```
பதிப்பி(10 + 5)
பதிப்பி(100 / 4)
பதிப்பி("Result:", 7 * 8)
```

Output:
```
15
25
Result: 56
```

---

## Comments

Use `#` to write notes that are ignored by the computer:

```
# This is a comment - it does nothing!
பதிப்பி("Hello")  # This is an inline comment

# You can use comments to explain your code:
# The line below prints a greeting
பதிப்பி("வணக்கம்!")
```

---

## Common Mistakes

### ❌ Missing Quotes

```
பதிப்பி(வணக்கம்)  # Error! Text needs quotes
```

### ✅ Correct

```
பதிப்பி("வணக்கம்")  # Correct!
```

---

### ❌ Wrong Parentheses

```
பதிப்பி "வணக்கம்"  # Error! Missing parentheses
```

### ✅ Correct

```
பதிப்பி("வணக்கம்")  # Correct!
```

---

## Try It Yourself!

### Exercise 1

Write a program that prints:
```
என் பெயர் அகம்
நான் தமிழில் பேசுவேன்
```

<details>
<summary>Solution</summary>

```
பதிப்பி("என் பெயர் அகம்")
பதிப்பி("நான் தமிழில் பேசுவேன்")
```
</details>

### Exercise 2

Write a program that calculates and prints `25 + 17`:

<details>
<summary>Solution</summary>

```
பதிப்பி(25 + 17)
# Or with a message:
பதிப்பி("25 + 17 =", 25 + 17)
```
</details>

---

## Summary

| What You Learned | Example |
|-----------------|---------|
| Print text | `பதிப்பி("Hello")` |
| Print numbers | `பதிப்பி(42)` |
| Print calculations | `பதிப்பி(10 + 5)` |
| Comments | `# This is a comment` |

---

**Next: [Chapter 4: Variables →](04_variables.md)**
