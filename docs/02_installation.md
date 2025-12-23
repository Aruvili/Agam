# Chapter 2: Installation (நிறுவுதல்)

## Prerequisites

Before installing agam, you need:

1. **Rust** (version 1.70 or later)
2. **Cargo** (comes with Rust)

---

## Installing Rust

### Windows

1. Download the installer from [rustup.rs](https://rustup.rs)
2. Run `rustup-init.exe`
3. Follow the prompts
4. Restart your terminal

### Linux / macOS

Open terminal and run:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Verify Installation

```bash
rustc --version
cargo --version
```

You should see version numbers for both.

---

## Installing agam

### Option 1: Build from Source (Recommended)

```bash
# Step 1: Navigate to the Language directory
cd d:\agam\Language

# Step 2: Build the release version
cargo build --release

# Step 3: The binary is now at:
# Windows: target\release\agam.exe
# Linux/Mac: target/release/agam
```

### Option 2: Add to PATH (Optional)

To run `agam` from anywhere:

**Windows (PowerShell as Admin):**
```powershell
# Add to user PATH
$env:Path += ";D:\agam\Language\target\release"
[Environment]::SetEnvironmentVariable("Path", $env:Path, "User")
```

**Linux/macOS:**
```bash
# Add to ~/.bashrc or ~/.zshrc
export PATH="$PATH:/path/to/agam/target/release"
```

---

## Running agam

### Method 1: Using Cargo (Easiest)

```bash
# Navigate to project directory
cd d:\agam\Language

# Run a file
cargo run --release -- examples/hello.agam

# Start REPL (interactive mode)
cargo run --release
```

### Method 2: Using the Binary Directly

```bash
# Windows
.\target\release\agam.exe examples/hello.agam

# Linux/Mac
./target/release/agam examples/hello.agam
```

### Method 3: After Adding to PATH

```bash
agam examples/hello.agam
agam --help
```

---

## Testing Your Installation

### Step 1: Create a Test File

Create a file called `test.agam`:

```
அச்சிடு("agam நிறுவல் வெற்றி!")
அச்சிடு("Installation successful!")
```

### Step 2: Run It

```bash
cargo run --release -- test.agam
```

### Expected Output

```
agam நிறுவல் வெற்றி!
Installation successful!
```

🎉 **Congratulations!** agam is working!

---

## The REPL (Interactive Mode)

Start the REPL to try code interactively:

```bash
cargo run --release
```

You'll see:
```
╔══════════════════════════════════════════════════════════════╗
║     அகம் - agam Programming Language v0.1.0           ║
║     தமிழில் நிரலாக்கம் செய்யுங்கள்!                           ║
╚══════════════════════════════════════════════════════════════╝

>>> 
```

Try these commands:
```
>>> அச்சிடு("வணக்கம்!")
வணக்கம்!
>>> மாறி x = 10
>>> அச்சிடு(x * 2)
20
>>> வெளியேறு()
```

---

## Command Line Options

| Command | Description |
|---------|-------------|
| `agam` | Start REPL |
| `agam file.agam` | Run a file |
| `agam --help` | Show help |
| `agam --version` | Show version |

---

## Troubleshooting

### "Command not found"

Make sure you're in the correct directory:
```bash
cd d:\agam\Language
cargo run --release -- examples/hello.agam
```

### Build Errors

Update Rust:
```bash
rustup update
```

### Unicode Display Issues

Make sure your terminal supports UTF-8. On Windows, try:
```powershell
chcp 65001
```

---

**Next: [Chapter 3: Hello World →](03_hello_world.md)**
