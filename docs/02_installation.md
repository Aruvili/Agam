# Chapter 2: Installation (நிறுவுதல்)

## Prerequisites

Before installing agam, you need:

1. **C++17 Compiler** (GCC 9+ / Clang 10+ / MSVC 2019+)
2. **CMake** (version 3.20 or later)
3. **LLVM** (version 17 or later)

---

## Installing Agam

### Option 1: Quick Install Scripts

**Linux/macOS**
```bash
curl -sSL https://dl.aruvili.com/install.sh | bash
```

**Windows (PowerShell)**
```powershell
irm https://dl.aruvili.com/install.ps1 | iex
```

### Option 2: Build from Source (Recommended for Contributors)

```bash
# Step 1: Clone the repository and navigate to it
git clone https://github.com/Aruvili/Agam.git
cd Agam

# Step 2: Configure the build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Step 3: Build the project
cmake --build build --config Release

# Step 4: Run the test suite to verify
cd build && ctest --output-on-failure
```

The compiled binary will be located in the `build/` directory (or `build/Release/` on Windows).

---

## Running agam

### Command Line Interface

```bash
# Verify installation
agamc --version

# Run a file interactively (JIT execution)
agamc run examples/hello.agam

# Compile to an executable
agamc examples/hello.agam -o hello
```

### Specifying Standard Library Path

If your standard library is located in a custom directory, you can pass it to the compiler using the `--lib-path` flag:

```bash
agamc hello.agam --lib-path /path/to/agam/std
```

---

## Testing Your Installation

### Step 1: Create a Test File

Create a file called `test.agam`:

```
பதிப்பி("agam நிறுவல் வெற்றி!")
பதிப்பி("Installation successful!")
```

### Step 2: Run It

```bash
agamc run test.agam
```

### Expected Output

```
agam நிறுவல் வெற்றி!
Installation successful!
```

🎉 **Congratulations!** agam is working!

---

## Troubleshooting

### "Command not found"

If you installed via the quick script, make sure the installation directory is added to your system's PATH.

### Build Errors (LLVM not found)

Ensure LLVM 17 is installed and reachable by CMake. 
On Windows, make sure you have LLVM installed (via the official installer or winget) and its `bin` directory is in your PATH.
On macOS/Linux, `llvm-config` from LLVM 17 must be available in your PATH.

### Unicode Display Issues

Make sure your terminal supports UTF-8. On Windows, try:
```powershell
chcp 65001
```

---

**Next: [Chapter 3: Hello World →](03_hello_world.md)**
