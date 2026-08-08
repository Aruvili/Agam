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

**அகம் (Agam)** is a modern, statically-typed, Tamil-first programming language built on **C++17** and **LLVM 17**. It combines Python-like simplicity with native performance and memory safety.

---

## Why Agam?

- Write programs **entirely in Tamil**
- Compiles to native machine code via LLVM
- Memory-safe with the Zone-Pulse Memory (ZPM) model
- Simple, readable syntax — beginner-friendly
- Interactive REPL — no setup required
- Designed for education and real-world use

---

## Quick Example

```
பதிப்பி("வணக்கம் உலகம்!");
```

```
வணக்கம் உலகம்!
```

```
செயல் கூட்டல்(அ: எண், ஆ: எண்): எண் {
    விடை அ + ஆ;
}

செயல் மைய(): எண் {
    மாறி முடிவு: எண் = கூட்டல்(3, 4);
    எனில் (முடிவு > 5) {
        விடை முடிவு;
    }
    விடை 0;
}
```

---

## Language Basics

### Variables & Constants

```
மாறி பெயர்: சரம் = "தமிழ்";
மாறி வயது: எண் = 25;
மாறி நிலைமாறிலி பை: தசமம்64 = 3.14159;
```

### Conditionals

```
மாறி மதிப்பெண்: எண் = 85;

எனில் (மதிப்பெண் >= 90) {
    பதிப்பி("தர நிலை: அ+");
} இல்லையெனில் (மதிப்பெண் >= 80) {
    பதிப்பி("தர நிலை: அ");
} இல்லையெனில் {
    பதிப்பி("மேம்படுத்த வேண்டும்");
}
```

### Loops

```
# While loop
மாறி நிலை i: எண் = 1;
வரை (i <= 5) {
    பதிப்பி(i);
    i = i + 1;
}

# For loop
சுற்று (i உள் 6) {
    பதிப்பி(i);
}
```

### Functions

```
செயல் வணக்கம்(பெயர்: சரம்): சரம் {
    விடை "வணக்கம், " + பெயர் + "!";
}

பதிப்பி(வணக்கம்("நண்பா"));
```

---

## Keywords Reference

### Core

| Tamil | English | Purpose |
|---|---|---|
| `செயல்` | `fn` | Function |
| `மாறி` | `let` | Variable |
| `நிலைமாறிலி` | `const` | Constant |
| `எனில்` | `if` | Conditional |
| `இல்லையெனில்` | `else` | Else |
| `வரை` | `while` | While loop |
| `சுற்று` | `for` | For loop |
| `உள்` | `in` | In |
| `விடை` | `return` | Return |
| `உண்மை` | `true` | Boolean true |
| `பொய்` | `false` | Boolean false |
| `இல்லை` | `null` | Null |

### Advanced

| Tamil | English | Purpose |
|---|---|---|
| `அமைப்பு` | `struct` | Define struct |
| `பட்டியல்` | `enum` | Define enum |
| `பண்பு` | `trait` | Define trait |
| `செயல்படுத்து` | `impl` | Implement methods |
| `பொருத்து` | `match` | Pattern matching |
| `இறக்குமதி` | `import` | Import module |
| `வெளி` | `extern` | Extern FFI declaration |
| `நிலை` | `mut` | Mutable reference |
| `மண்டலம்` | `zone` | Memory zone |
| `கடன்` | `borrow` | Borrow reference |
| `பகிர்வு` | `shared` | Shared reference |
| `தப்பித்தல்` | `escape` | Escape memory zone |
| `புதிய` | `new` | Instantiate |
| `நீக்கு` | `delete` | Free memory |
| `ஒதுக்கீடு` | `alloc` | Allocate memory |
| `ஆக` | `as` | Alias/Cast |

### Built-in Functions

| Tamil | English | Purpose |
|---|---|---|
| `பதிப்பி` | `print` | Output |
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

---

## Compiler CLI & Debugging

```bash
agamc --version

# Compile to optimized native binary (-O3 SIMD auto-vectorization)
agamc hello.agam -o hello

# Compile with DWARF debug symbols for GDB / LLDB step-debugging
agamc -g hello.agam -o hello_debug

# Run with JIT execution engine
agamc run hello.agam

# Emit intermediate representations
agamc hello.agam --emit-ast     # Abstract Syntax Tree
agamc hello.agam --emit-hir     # High-Level IR
agamc hello.agam --emit-mir     # Mid-Level IR
agamc hello.agam --emit-llvm    # LLVM IR
```

---

## Package Manager (`agamp`) & Central Registry

`agamp` is the official Cargo-inspired package manager for Agam. It manages dependencies, builds isolated `./target/debug` and `./target/release` output artifacts, and maintains deterministic `pk.lock` lockfiles.

```bash
# Create a new binary package project
agamp new my_project

# Compile debug or release binaries
agamp build
agamp build --release

# Compile & run
agamp run

# Perform type-checking without building
agamp check

# Install dependencies (Checks Central Registry -> Standard Library -> Git Fallback)
agamp add valaiccevaiyagam
agamp add https://github.com/user/custom_pkg.git

# Update installed modules & clean build directory
agamp update
agamp clean
```

---

## Enterprise Standard Library Ecosystem (18 Modules)

Agam features 18 enterprise production standard library packages in `std/`:

| Module | Scope & Tamil Interfaces |
| :--- | :--- |
| **`std/io.agam`** | Standard Output and String Printing (`வரியிறக்கி_பதிப்பி`). |
| **`std/math.agam`** | Trigonometry, Logarithms, Square Roots (`வர்க்கமூலம்`, `சைன்`). |
| **`std/net.agam`** | Sockets & HTTP Network Layer (`சாக்கெட்_கேள்`, `சாக்கெட்_ஏற்றுக்கொள்`). |
| **`std/vector.agam`** | Generic Dynamic Array Allocation. |
| **`std/string.agam`** | String Transformations & Manipulation (`சரம்_நீளம்`, `சரம்_மாற்று`). |
| **`std/fs.agam`** | File System & I/O Operations (`கோப்பு_முழுவதும்_வாசி`, `கோப்பு_முழுவதும்_எழுது`). |
| **`std/random.agam`** | Pseudo-Random Range & Float Generators (`சீரற்ற_எண்`, `சீரற்ற_தசமம்`). |
| **`std/hashmap.agam`** | Generic Key-Value HashMap Dictionaries (`வரைபடம்_சேர்`). |
| **`std/json.agam`** | JSON Serialization & Formatting (`ஜேசான்_எண்`, `ஜேசான்_சரம்`). |
| **`std/os.agam`** | Process Environment & Shell Execution (`இயங்குதளம்_பெயர்`). |
| **`std/time.agam`** | High-Precision Timers (`தற்போதைய_நேரம்`, `உறங்கு_மில்லி`). |
| **`std/thread.agam`** | Multi-Threading Concurrency (`இழை_தொடங்கு`, `இழை_காத்திரு`). |
| **`std/crypto.agam`** | Crypto Hashing & Encoding (`பேஸ்64_குறியாக்கு`, `பேஸ்64_டிகோட்`, `ஷாஹா256`). |
| **`std/regex.agam`** | Regular Expression Engine (`சீரான_வெளிப்பாடு_தேடு`, `சீரான_வெளிப்பாடு_மாற்று`). |
| **`std/datetime.agam`** | Calendar & Date Formatting (`தற்போதைய_தேதி`, `தேதி_வடிவமைப்பு`). |
| **`std/cli.agam`** | CLI Flag & Option Parsing (`கொடி_உள்ளதா`, `விருப்பம்_பெறு`). |
| **`std/sqlite.agam`** | Relational Database Persistence (`தரவுத்தளம்_திற`, `தரவுத்தளம்_இயக்கு`). |
| **`packages/வலைச்சேவையகம்/`** | Standalone FastAPI-Level Tamil Web Server Framework (`வலை_ஜேசான்_பதில்`). |

---

## Project Structure

```
agam/
├── include/agam/       # Core compiler headers (lexer, ast, hir, thir, mir, codegen)
├── src/                # Implementation sources
│   ├── agamp/          # Package manager implementation
│   └── codegen/        # LLVM IR & JIT code generator
├── registry/           # Centralized Package Registry Index (index.json)
├── packages/           # Standalone Enterprise Packages (வலைச்சேவையகம்)
├── std/                # 18 Standard Library Modules
├── tests/              # CTest unit suite and check_all.sh integration runner
├── docs/               # Language documentation
└── CMakeLists.txt
```

---

## Documentation

**[https://agam.aruvili.com](https://agam.aruvili.com)**

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

Star the repo if you find it useful — it helps the Tamil developer ecosystem grow.

---

## License

MIT License © [Aruvili](https://github.com/Aruvili)

---

 **அகம் — தமிழில் நிரலாக்கத்தின் எதிர்காலம்**