# Chapter 3: Hello World (வணக்கம் உலகம்)

## Your First Program

Let's write your first program in Tamil!

### Step 1: Create the File

Create a file named `hello.agam`:

```
அச்சிடு("வணக்கம் உலகம்!")
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

Let's break down `அச்சிடு("வணக்கம் உலகம்!")`:

| Part | Meaning |
|------|---------|
| `அச்சிடு` | The "print" function - displays text |
| `(` | Opening parenthesis - starts the arguments |
| `"வணக்கம் உலகம்!"` | The text to display (a "string") |
| `)` | Closing parenthesis - ends the arguments |

---

## More Examples

### Print Multiple Lines

```
அச்சிடு("வணக்கம்!")
அச்சிடு("எப்படி இருக்கிறீர்கள்?")
அச்சிடு("நலமா?")
```

Output:
```
வணக்கம்!
எப்படி இருக்கிறீர்கள்?
நலமா?
```

### Print Numbers

```
அச்சிடு(42)
அச்சிடு(3.14159)
```

Output:
```
42
3.14159
```

### Print Multiple Items

```
அச்சிடு("Number:", 42)
அச்சிடு("Name:", "agam", "Version:", 1)
```

Output:
```
Number: 42
Name: agam Version: 1
```

### Print with Expressions

```
அச்சிடு(10 + 5)
அச்சிடு(100 / 4)
அச்சிடு("Result:", 7 * 8)
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
அச்சிடு("Hello")  # This is an inline comment

# You can use comments to explain your code:
# The line below prints a greeting
அச்சிடு("வணக்கம்!")
```

---

## Common Mistakes

### ❌ Missing Quotes

```
அச்சிடு(வணக்கம்)  # Error! Text needs quotes
```

### ✅ Correct

```
அச்சிடு("வணக்கம்")  # Correct!
```

---

### ❌ Wrong Parentheses

```
அச்சிடு "வணக்கம்"  # Error! Missing parentheses
```

### ✅ Correct

```
அச்சிடு("வணக்கம்")  # Correct!
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
அச்சிடு("என் பெயர் அகம்")
அச்சிடு("நான் தமிழில் பேசுவேன்")
```
</details>

### Exercise 2

Write a program that calculates and prints `25 + 17`:

<details>
<summary>Solution</summary>

```
அச்சிடு(25 + 17)
# Or with a message:
அச்சிடு("25 + 17 =", 25 + 17)
```
</details>

---

## Summary

| What You Learned | Example |
|-----------------|---------|
| Print text | `அச்சிடு("Hello")` |
| Print numbers | `அச்சிடு(42)` |
| Print calculations | `அச்சிடு(10 + 5)` |
| Comments | `# This is a comment` |

---

**Next: [Chapter 4: Variables →](04_variables.md)**
