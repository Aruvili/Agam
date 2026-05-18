# Chapter 13: Keywords Reference (முக்கிய சொற்கள்)

## Complete Keyword List

agam supports both Tamil and English keywords. You can use either!

---

## Declaration Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `செயல்` | `fn` | Define a function | `செயல் greet() { }` |
| `மாறி` | `let` | Declare a variable | `மாறி x = 10;` |
| `நிலைமாறிலி` | `const` | Declare a constant | `நிலைமாறிலி PI = 3.14;` |

---

## Control Flow Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `எனில்` | `if` | If condition | `எனில் (x > 5) { }` |
| `இல்லையெனில்` | `else` | Else condition | `இல்லையெனில் { }` |
| `வரை` | `while` | While loop | `வரை (x < 10) { }` |
| `சுற்று` | `for` | For loop | `சுற்று (i உள் items) { }` |
| `உள்` | `in` | In operator | `x உள் list` |
| `விடை` | `return` | Return value | `விடை result;` |

---

## Boolean & Data Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `உண்மை` | `true` | Boolean true | `மாறி flag = உண்மை;` |
| `பொய்` | `false` | Boolean false | `மாறி flag = பொய்;` |
| `இல்லை` | `null` | Null value | `மாறி data = இல்லை;` |

---

## Module Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `இறக்குமதி` | `import` | Import a module | `இறக்குமதி math_utils;` |
| `வெளி` | `extern` | Extern FFI declaration | `வெளி செயல் print();` |

---

## Type Definition Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `அமைப்பு` | `struct` | Define a structure | `அமைப்பு Person { }` |
| `பட்டியல்` | `enum` | Define an enumeration | `பட்டியல் Color { }` |

---

## Trait & Implementation Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `பண்பு` | `trait` | Define a trait | `பண்பு Animal { }` |
| `செயல்படுத்து` | `impl` | Implement trait/methods | `செயல்படுத்து Animal for Dog { }` |

---

## Pattern Matching Keywords

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `பொருத்து` | `match` | Match expression | `பொருத்து (value) { }` |

---

## Memory Management Keywords (ZPM)

agam features the **Zone-based Pulse Memory (ZPM)** system. These keywords are fundamental for safe memory control:

| Tamil | English | Description | Example |
|-------|---------|-------------|---------|
| `நிலை` | `mut` | Mutable reference | `கடன் நிலை(x)` |
| `மண்டலம்` | `zone` | Define a Memory zone | `மண்டலம் my_zone { }` |
| `கடன்` | `borrow` | Borrow a reference | `கடன்(x)` |
| `பகிர்வு` | `shared` | Shared reference | `பகிர்வு(x)` |
| `தப்பித்தல்` | `escape` | Escape a value out of a zone | `தப்பித்தல் value;` |
| `புதிய` | `new` | Instantiate memory | `புதிய Person()` |
| `நீக்கு` | `delete` | Free memory manually | `நீக்கு(ptr);` |
| `ஒதுக்கீடு` | `alloc` | Allocate raw memory | `ஒதுக்கீடு(size);` |
| `ஆக` | `as` | Alias/Cast | `x ஆக y` |

---

## Logical & Operators Note

Unlike older versions, Agam V1.0.1+ natively supports standard C-style logical operators instead of Tamil keywords for brevity and parity with other systems programming languages:

| Operator | Description |
|----------|-------------|
| `&&` | Logical AND |
| `\|\|` | Logical OR |
| `!` | Logical NOT |
| `==` | Equal |
| `!=` | Not equal |

---

## Built-in Data Types

| Tamil | English | Description |
|-------|---------|-------------|
| `எண்` | `int` | Integer |
| `தசமம்` | `float` | Float |
| `சரம்` | `string` | String |
| `மெய்மை` | `bool` | Boolean |
| `வெற்று` | `void` | Void / No return |

*Note: Sized variations like `எண்8`, `எண்32`, `தசமம்64`, etc. also exist!*

---

**Next: [Chapter 14: Error Messages →](14_errors.md)**
