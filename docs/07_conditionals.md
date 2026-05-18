# Chapter 7: Conditionals (நிபந்தனைகள்)

## Making Decisions

Conditionals let your program make decisions based on conditions.

```
எனில் condition:
    # do something if condition is true
```

---

## The எனில் Statement (if)

Use `எனில்` to execute code only when a condition is true:

```
மாறி age = 20;

எனில் age >= 18:;
    பதிப்பி("You are an adult!");
```

> **Important:** Don't forget the `:` after the condition and indent the code inside!

---

## எனில்...இல்லை (if...else)

Use `இல்லை` to execute code when the condition is false:

```
மாறி age = 15;

எனில் age >= 18:;
    பதிப்பி("You are an adult");
} இல்லையெனில் {
    பதிப்பி("You are a minor");

}
# Output: You are a minor
```

---

## எனில்...இல்லையெனில்...இல்லை (if...elif...else)

Use `இல்லையெனில்` to check multiple conditions:

```
மாறி score = 85;

எனில் score >= 90:;
    பதிப்பி("Grade: A");
இல்லையெனில் score >= 80:;
    பதிப்பி("Grade: B");
இல்லையெனில் score >= 70:;
    பதிப்பி("Grade: C");
இல்லையெனில் score >= 60:;
    பதிப்பி("Grade: D");
} இல்லையெனில் {
    பதிப்பி("Grade: F");

}
# Output: Grade: B
```

---

## Keyword Reference

| Tamil | English | Usage |
|-------|---------|-------|
| `எனில்` | `if` | First condition |
| `இல்லையெனில்` | `elif` | Additional conditions |
| `இல்லை` | `else` | When no condition matches |

---

## Compound Conditions

Combine conditions using logical operators:

### மற்றும் (AND) - Both must be true

```
மாறி age = 25;
மாறி has_id = உண்மை;

எனில் age >= 18 மற்றும் has_id:;
    பதிப்பி("Entry allowed");
} இல்லையெனில் {
    பதிப்பி("Entry denied");
}
```

### அல்லது (OR) - At least one must be true

```
மாறி is_member = பொய்;
மாறி has_pass = உண்மை;

எனில் is_member அல்லது has_pass:
    பதிப்பி("Welcome!");
```

### இல்ல (NOT) - Reverse the condition

```
மாறி is_closed = பொய்;

எனில் இல்ல is_closed:
    பதிப்பி("The shop is open");
```

---

## Nested Conditionals

You can put conditions inside conditions:

```
மாறி age = 25;
மாறி is_student = உண்மை;

எனில் age >= 18:;
    பதிப்பி("Adult");
    
    எனில் is_student:
        பதிப்பி("Student discount: 20%");
    } இல்லையெனில் {
        பதிப்பி("Regular price");
} இல்லையெனில் {
    பதிப்பி("Minor - free entry!");
}
```

---

## Practical Examples

### Example 1: Login Check

```
மாறி username = "admin";
மாறி password = "secret123";

எனில் username == "admin" மற்றும் password == "secret123":;
    பதிப்பி("✅ Login successful!");
} இல்லையெனில் {
    பதிப்பி("❌ Invalid credentials");
}
```

### Example 2: Number Classification

```
மாறி num = -5;

எனில் num > 0:
    பதிப்பி("Positive number");
இல்லையெனில் num < 0:
    பதிப்பி("Negative number");
} இல்லையெனில் {
    பதிப்பி("Zero");

}
# Output: Negative number
```

### Example 3: Ticket Pricing

```
மாறி age = 10;
மாறி is_weekend = உண்மை;

மாறி price = 100  # Base price;

எனில் age < 12:
    price = 50  # Kids discount;
இல்லையெனில் age >= 60:;
    price = 60  # Senior discount;

எனில் is_weekend:
    price = price + 20  # Weekend surcharge;

பதிப்பி("Ticket price:", price);
# Output: Ticket price: 70
```

---

## Try It Yourself!

### Exercise 1: Temperature Check

Write a program that prints:
- "Cold" if temperature < 15
- "Comfortable" if temperature is 15-25
- "Hot" if temperature > 25

<details>
<summary>Solution</summary>

```
மாறி temperature = 22;

எனில் temperature < 15:
    பதிப்பி("Cold");
இல்லையெனில் temperature <= 25:;
    பதிப்பி("Comfortable");
} இல்லையெனில் {
    பதிப்பி("Hot");
}
```
</details>

### Exercise 2: Leap Year Check

A year is a leap year if:
- Divisible by 4 AND not divisible by 100
- OR divisible by 400

<details>
<summary>Solution</summary>

```
மாறி year = 2024;

எனில் (year % 4 == 0 மற்றும் year % 100 != 0) அல்லது (year % 400 == 0):;
    பதிப்பி(சரமாக(year) + " is a leap year");
} இல்லையெனில் {
    பதிப்பி(சரமாக(year) + " is not a leap year");
}
```
</details>

---

## Summary

| Pattern | Usage |
|---------|-------|
| `எனில் condition:` | Single condition |
| `எனில்...இல்லை:` | Two choices |
| `எனில்...இல்லையெனில்...இல்லை:` | Multiple choices |

---

**Next: [Chapter 8: Loops →](08_loops.md)**
