<p align="center">
  <img src="assets/agam-banner.svg" alt="Agam Tamil Programming Language" />
</p>

<p align="center">
  <img src="https://img.shields.io/github/v/release/Aruvili/Agam" />
  <img src="https://img.shields.io/badge/docs-agam.aruvili.com-brightgreen" />
  <img src="https://img.shields.io/github/license/Aruvili/Agam" />
  <img src="https://img.shields.io/github/stars/Aruvili/Agam?style=social" />
</p>

---

# அகம் (Agam) — Tamil Programming Language

> **Program in Tamil. Build for the real world.**

**அகம் (Agam)** is a modern, statically-typed, Tamil-first programming language built on **C++17**, **Flex**, **Bison**, and **LLVM 17**. It combines Python-like simplicity with native performance and memory safety.

---

## Why Agam?

- 🇮🇳 Write programs **entirely in Tamil**
- ⚡ Compiles to native machine code via LLVM
- 🔒 Memory-safe with the Zone-Pulse Memory (ZPM) model
- 🐍 Simple, readable syntax — beginner-friendly
- 🖥️ Interactive REPL — no setup required
- 🎓 Designed for education and real-world use

---

## Quick Example

```agam
அச்சிடு("வணக்கம் உலகம்!")
```

```
வணக்கம் உலகம்!
```

```agam
செயல் கூட்டல்(அ: முழு, ஆ: முழு): முழு {
    திரும்பு அ + ஆ
}

செயல் முதன்மை(): முழு {
    மாறி முடிவு: முழு = கூட்டல்(3, 4)
    என்றால் (முடிவு > 5) {
        திரும்பு முடிவு
    }
    திரும்பு 0
}
```

---

## Language Basics

### Variables & Constants

```agam
மாறி பெயர் = "தமிழ்"
மாறி வயது = 25
மாறாத பை = 3.14159
```

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

### Loops

```agam
# While loop
மாறி எண் = 1
வரை எண் <= 5:
    அச்சிடு(எண்)
    எண் = எண் + 1

# For loop
ஒவ்வொரு எண் உள்ள வரம்பு(1, 6):
    அச்சிடு(எண்)
```

### Functions

```agam
செயல் வணக்கம்(பெயர்):
    திரும்பு "வணக்கம், " + பெயர் + "!"

அச்சிடு(வணக்கம்("நண்பா"))
```

---

## Keywords Reference

### Core

| Tamil | English | Purpose |
|---|---|---|
| `செயல்` | `fn` | Function |
| `மாறி` | `let` | Variable |
| `மாறாத` | `const` | Constant |
| `என்றால்` | `if` | Conditional |
| `இல்லையென்றால்` | `elif` | Else-if |
| `இல்லை` | `else` | Else |
| `வரை` | `while` | While loop |
| `ஒவ்வொரு` | `for` | For loop |
| `உள்ள` | `in` | In |
| `திரும்பு` | `return` | Return |
| `நிறுத்து` | `break` | Break |
| `தொடர்` | `continue` | Continue |
| `உண்மை` | `true` | Boolean true |
| `பொய்` | `false` | Boolean false |
| `இல்லா` | `null` | Null |
| `மற்றும்` | `and` | Logical AND |
| `அல்லது` | `or` | Logical OR |
| `இல்ல` | `not` | Logical NOT |

### Advanced

| Tamil | English | Purpose |
|---|---|---|
| `கட்டமைப்பு` | `struct` | Define struct |
| `விருப்பம்` | `enum` | Define enum |
| `பொருத்து` | `match` | Pattern matching |
| `இறக்குமதி` | `import` | Import module |
| `இருந்து` | `from` | From |
| `முயற்சி` | `try` | Try block |
| `பிடி` | `catch` | Catch block |
| `வீசு` | `throw` | Throw error |

### Built-in Functions

| Tamil | English | Purpose |
|---|---|---|
| `அச்சிடு` | `print` | Output |
| `உள்ளீடு` | `input` | Input |
| `நீளம்` | `len` | Get length |
| `வகை` | `type` | Get type |
| `வரம்பு` | `range` | Number range |
| `வர்க்கம்` | `sqrt` | Square root |
| `படி` | `read_file` | Read file |
| `எழுது` | `write_file` | Write file |

> See the [complete documentation](https://agam.aruvili.com) for all 33+ built-in functions.

---

## Installation

**Linux/macOS**
```bash
curl -sSL https://dl.aruvili.com/install.sh | bash
```

**Windows (PowerShell)**
```powershell
irm https://dl.aruvili.com/install.ps1 | iex
```

---

## Building from Source

### Prerequisites

| Tool | Version |
|---|---|
| C++17 | GCC 9+ / Clang 10+ / MSVC 2019+ |
| CMake | 3.20+ |
| LLVM | 17+ |

### Build Steps

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cd build && ctest --output-on-failure
```

---

## Compiler CLI

```bash
agamc --version

# Compile to executable
agamc hello.agam -o hello

# Run with JIT
agamc run hello.agam

# Emit intermediate representations
agamc hello.agam --emit-ast     # Abstract Syntax Tree
agamc hello.agam --emit-llvm    # LLVM IR
```

---

## Project Structure

```
agam/
├── include/agam/
│   ├── lexer/          # Token definitions
│   ├── ast/            # AST node hierarchy
│   ├── semantic/       # Type checker, symbol table
│   └── codegen/        # LLVM IR generator
├── src/
│   ├── lexer/          # Flex specification
│   ├── parser/         # Bison grammar
│   ├── semantic/       # Semantic analysis
│   ├── codegen/        # Code generation
│   └── main.cpp
├── std/                # Standard library
├── tests/
├── scripts/            # install.sh / install.ps1
├── docs/
└── CMakeLists.txt
```

---

## Documentation

🌐 **[https://agam.aruvili.com](https://agam.aruvili.com)**

---

## Credits

| Role | Contributor |
|---|---|
| Language & Compiler | [Balapriyan B](https://github.com/BalaPriyan) |
| AI Assistance | Claude Sonnet 4.5 |
| Language Testing | [Sriram G](https://github.com/GGSriram) |
| Documentation | [Bagavathisingh B](https://github.com/Bagavathisingh) |

---

## Contributing

Agam is open-source and community-driven.  
Contributions, issues, and ideas are always welcome.

⭐ Star the repo if you find it useful — it helps the Tamil developer ecosystem grow.

---

## License

MIT License © [Aruvili](https://github.com/Aruvili)

---

🇮🇳 **அகம் — தமிழில் நிரலாக்கத்தின் எதிர்காலம்**