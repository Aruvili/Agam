# Chapter 8: Loops (வளையங்கள்)

## What are Loops?

Loops let you repeat code multiple times without writing it over and over.

---

## The வரை Loop (while)

Repeats while a condition is true:

```
மாறி count = 1;

வரை (count <= 5) {
    பதிப்பி(count);
    count = count + 1;

}
# Output:
# 1
# 2
# 3
# 4
# 5
```

### How it works:
1. Check condition `count <= 5`
2. If true, run the code inside
3. Go back to step 1
4. If false, exit the loop

---

## The ஒவ்வொரு Loop (for)

Iterates over each item in a collection:

```
ஒவ்வொரு number உள்ள [1, 2, 3, 4, 5]:
    பதிப்பி(number)

# Output:
# 1
# 2
# 3
# 4
# 5
```

### With Strings

```
ஒவ்வொரு letter உள்ள "Tamil":
    பதிப்பி(letter)

# Output:
# T
# a
# m
# i
# l
```

### With வரம்பு (range)

```
# Numbers 0 to 4
ஒவ்வொரு i உள்ள வரம்பு(5):
    பதிப்பி(i)
# Output: 0, 1, 2, 3, 4

# Numbers 1 to 5
ஒவ்வொரு i உள்ள வரம்பு(1, 6):
    பதிப்பி(i)
# Output: 1, 2, 3, 4, 5

# Even numbers 2 to 10
ஒவ்வொரு i உள்ள வரம்பு(2, 11, 2):
    பதிப்பி(i)
# Output: 2, 4, 6, 8, 10
```

---

## Loop Keywords Reference

| Tamil | English | Meaning |
|-------|---------|---------|
| `வரை` | `while` | Repeat while condition is true |
| `ஒவ்வொரு` | `for` | Iterate over items |
| `உள்ள` | `in` | In / belonging to |
| `நிறுத்து` | `break` | Exit the loop |
| `தொடர்` | `continue` | Skip to next iteration |

---

## நிறுத்து (break)

Exit the loop early:

```
ஒவ்வொரு num உள்ள வரம்பு(1, 10):
    எனில் num == 5:
        பதிப்பி("Found 5! Stopping.")
        நிறுத்து
    பதிப்பி(num)

# Output:
# 1
# 2
# 3
# 4
# Found 5! Stopping.
```

---

## தொடர் (continue)

Skip the current iteration and continue with the next:

```
ஒவ்வொரு num உள்ள வரம்பு(1, 6):
    எனில் num == 3:
        தொடர்   # Skip 3
    பதிப்பி(num)

# Output:
# 1
# 2
# 4
# 5
```

---

## Practical Examples

### Example 1: Sum of Numbers

```
மாறி total = 0;

சுற்று (num உள் வரம்பு(1, 11)) {
    total = total + num;

}
பதிப்பி("Sum 1-10:", total);
# Output: Sum 1-10: 55
```

### Example 2: Countdown

```
மாறி count = 5;

வரை (count > 0) {
    பதிப்பி(count);
    count = count - 1;

}
பதிப்பி("🚀 Blast off!");

# Output:
# 5
# 4
# 3
# 2
# 1
# 🚀 Blast off!
```

### Example 3: Find First Even Number

```
மாறி numbers = [1, 3, 7, 8, 9, 12];

சுற்று (num உள் numbers) {
    எனில் num % 2 == 0:;
        பதிப்பி("First even number:", num);
        நிறுத்து

}
# Output: First even number: 8
```

### Example 4: Multiplication Table

```
மாறி number = 5;

பதிப்பி("Multiplication table for", number);
பதிப்பி("-" * 20);

சுற்று (i உள் வரம்பு(1, 11)) {
    மாறி result = number * i;
    பதிப்பி(சரமாக(number) + " x " + சரமாக(i) + " = " + சரமாக(result));

}
# Output:
# Multiplication table for 5
# --------------------
# 5 x 1 = 5
# 5 x 2 = 10
# ... etc
```

### Example 5: Password Retry

```
நிலைமாறிலி CORRECT_PASSWORD = "secret";
மாறி attempts = 3;

வரை (attempts > 0) {
    மாறி guess = உள்ளீடு("Password: ");
    
    எனில் guess == CORRECT_PASSWORD:;
        பதிப்பி("✅ Access granted!");
        நிறுத்து
    } இல்லையெனில் {
        attempts = attempts - 1;
        பதிப்பி("❌ Wrong. Attempts left:", attempts);

    }
எனில் attempts == 0:;
    பதிப்பி("🔒 Account locked!");
```

---

## Nested Loops

Loops inside loops:

```
ஒவ்வொரு i உள்ள வரம்பு(1, 4):
    ஒவ்வொரு j உள்ள வரம்பு(1, 4):
        பதிப்பி(சரமாக(i) + "x" + சரமாக(j) + "=" + சரமாக(i*j))
    பதிப்பி("")   # Empty line between rows

# Output:
# 1x1=1
# 1x2=2
# 1x3=3
# 
# 2x1=2
# ... etc
```

### Pattern Printing

```
ஒவ்வொரு i உள்ள வரம்பு(1, 6):
    பதிப்பி("*" * i)

# Output:
# *
# **
# ***
# ****
# *****
```

---

## When to Use Which Loop?

| Use `வரை` (while) when... | Use `ஒவ்வொரு` (for) when... |
|---------------------------|------------------------------|
| You don't know how many times | You know exactly how many times |
| Based on a condition | Iterating over a collection |
| User input validation | Processing each item in a list |

---

## Summary

| Loop | Syntax |
|------|--------|
| While | `வரை condition:` |
| For | `ஒவ்வொரு item உள்ள collection:` |
| Range | `ஒவ்வொரு i உள்ள வரம்பு(start, end):` |
| Break | `நிறுத்து` |
| Continue | `தொடர்` |

---

**Next: [Chapter 9: Functions →](09_functions.md)**
