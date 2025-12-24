# அகம் (Agam) — Tamil Programming Language

![Language](https://img.shields.io/badge/built%20with-Rust-orange)
![License](https://img.shields.io/badge/license-MIT-green)

**தமிழில் நிரலாக்கம் செய்யுங்கள்!**
*Program in Tamil.*

---

## 🌟 Introduction

**அகம் (Agam)** is a modern, fast, and safe **Tamil-first programming language** designed for real-world usage.

Agam enables developers, students, and educators to write expressive programs **entirely in Tamil**, while enjoying **Python inspired simplicity** and **Rust-powered performance and safety**.

> **Created by:** **Balapriyan B**
> **Assisted by:** **Claude Sonnet 4.5**

Agam’s goal is to make programming **native, accessible, and powerful** for Tamil-speaking communities.

---

## ✨ Key Highlights

* 🇮🇳 **Tamil-first language** — write programs fully in Tamil
* 🐍 **Python inspired syntax** — simple, readable, indentation-based
* ⚡ **High performance** — powered by Rust
* 🔒 **Memory safe & type secure**
* 🖥️ **Interactive REPL**
* 🎓 **Education-ready**
* 🚀 **Designed for production use**

---

## 🚀 Quick Start

### Hello World

```agam
அச்சிடு("வணக்கம் உலகம்!")
```

**Output**

```
வணக்கம் உலகம்!
```

---

## 🧩 Language Basics

### Variables & Constants

```agam
மாறி பெயர் = "தமிழ்"
மாறி வயது = 25
மாறாத பை = 3.14159

அச்சிடு(பெயர்)
```

---

### Conditionals

```agam
மாறி மதிப்பெண் = 85

என்றால் மதிப்பெண் >= 90:
    அச்சிடு("தர நிலை: அ+")
இல்லையென்றால் மதிப்பெண் >= 80:
    அச்சிடு("தர நிலை: அ")
இல்லை:
    அச்சிடு("மேம்படுத்த வேண்டும்")
```

---

### Loops

```agam
# While loop
மாறி எண் = 1
வரை எண் <= 5:
    அச்சிடு(எண்)
    எண் = எண் + 1
```

```agam
# For loop
ஒவ்வொரு எண் உள்ள வரம்பு(1, 6):
    அச்சிடு(எண்)
```

---

### Functions

```agam
செயல் வணக்கம்(பெயர்):
    திரும்பு "வணக்கம், " + பெயர் + "!"

அச்சிடு(வணக்கம்("நண்பா"))
```

**Output**

```
வணக்கம், நண்பா!
```

---

## 📖 Supported Keywords

### Core Keywords

| Tamil           | English  | Purpose       |
| --------------- | -------- | ------------- |
| `செயல்`         | `fn`     | Function      |
| `மாறி`          | `let`    | Variable      |
| `மாறாத`         | `const`  | Constant      |
| `என்றால்`       | `if`     | Conditional   |
| `இல்லையென்றால்` | `elif`   | Else-if       |
| `இல்லை`         | `else`   | Else          |
| `வரை`           | `while`  | While loop    |
| `ஒவ்வொரு`       | `for`    | For loop      |
| `உள்ள`          | `in`     | In            |
| `திரும்பு`      | `return` | Return        |
| `நிறுத்து`      | `break`  | Break loop    |
| `தொடர்`         | `continue` | Continue loop |
| `உண்மை`         | `true`   | Boolean true  |
| `பொய்`          | `false`  | Boolean false |
| `இல்லா`         | `null`   | Null value    |
| `மற்றும்`       | `and`    | Logical AND   |
| `அல்லது`        | `or`     | Logical OR    |
| `இல்ல`          | `not`    | Logical NOT   |

### Advanced Features

| Tamil           | English  | Purpose           |
| --------------- | -------- | ----------------- |
| `கட்டமைப்பு`    | `struct` | Define struct     |
| `விருப்பம்`     | `enum`   | Define enum       |
| `பொருத்து`      | `match`  | Pattern matching  |
| `இறக்குமதி`     | `import` | Import module     |
| `இருந்து`       | `from`   | From (for imports)|
| `முயற்சி`       | `try`    | Try block         |
| `பிடி`          | `catch`  | Catch block       |
| `வீசு`          | `throw`  | Throw error       |

### Built-in Functions

| Tamil          | English     | Purpose           |
| -------------- | ----------- | ----------------- |
| `அச்சிடு`      | `print`     | Output            |
| `உள்ளீடு`      | `input`     | Input             |
| `நீளம்`        | `len`       | Get length        |
| `வகை`          | `type`      | Get type          |
| `வரம்பு`       | `range`     | Number range      |
| `வர்க்கம்`     | `sqrt`      | Square root       |
| `படி`          | `read_file` | Read file         |
| `எழுது`        | `write_file`| Write file        |

> 📚 See the [complete documentation](docs/README.md) for all 33+ built-in functions!

---

## 🖥️ Interactive REPL

Launch the REPL:

```bash
agam
```

```
╔════════════════════════════════════════════╗
║  அகம் - Agam Programming Language v0.1.0  ║
║  தமிழில் நிரலாக்கம் செய்யுங்கள்!          ║
╚════════════════════════════════════════════╝

>>> மாறி x = 10
>>> அச்சிடு(x * 2)
20
>>> வெளியேறு()
நன்றி! மீண்டும் வருக! 🙏
```

---

## 📦 Installation

### Using Prebuilt Binary (Recommended)

> [ALL DOWNLOAD LINKS INCLUDED HERE](https://github.com/Aruvili/Agam/releases)

Prebuilt binaries for **Windows, macOS, and Linux** will be available after the first GitHub release.

Users will be able to:

* Download a single executable
* Run `agam` instantly
* Use Agam without installing Rust or Cargo

---

## 🎯 Use Cases

* Learning programming in Tamil
* Schools & colleges
* Beginner-friendly scripting
* Algorithm learning
* REPL-based experimentation
* Community-driven projects


---

## 🤝 Community

Agam is **open-source and community-driven**.
Feedback, ideas, and contributions are welcome.

---

## 📜 License

[MIT License](LICENSE).

---

### 🇮🇳 **அகம் — தமிழில் நிரலாக்கத்தின் எதிர்காலம்**

Created by **Balapriyan B**
Assisted by **Claude Sonnet 4.5**

### README.md
Credits To CHATGPT

---
