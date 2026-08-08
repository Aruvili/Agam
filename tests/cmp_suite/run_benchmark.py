#!/usr/bin/env python3
import subprocess
import time
import statistics
import os
import sys

def build_targets():
    print("==================================================")
    print(" Compiling Benchmark Targets...")
    print("==================================================")

    # 1. Compile C++ with -O3 -march=native
    print("[1/4] Compiling C++ (GCC -O3 -march=native)...")
    res = subprocess.run(["g++", "-O3", "-march=native", "tests/cmp_suite/prime_bench.cpp", "-o", "tests/cmp_suite/prime_cpp"], capture_output=True, text=True)
    if res.returncode != 0:
        print(f"C++ Compilation Error:\n{res.stderr}")
        sys.exit(1)

    # 2. Compile Rust with -C opt-level=3 -C target-cpu=native
    print("[2/4] Compiling Rust (opt-level=3 target-cpu=native)...")
    res = subprocess.run(["rustc", "-C", "opt-level=3", "-C", "target-cpu=native", "tests/cmp_suite/prime_bench.rs", "-o", "tests/cmp_suite/prime_rust"], capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Rust Compilation Error:\n{res.stderr}")
        sys.exit(1)

    # 3. Ensure Agam compiler build exists
    if not os.path.exists("./build/bin/agamc"):
        print("Building Agam compiler...")
        subprocess.run(["cmake", "--build", "build", "--config", "Release"], check=True)

    # 4. Compile Agam Native Binary (-O3)
    print("[4/4] Compiling Agam Native Release Binary (-O3 -march=native)...")
    res1 = subprocess.run(["./build/bin/agamc", "-O3", "tests/cmp_suite/prime_bench.agam", "-o", "tests/cmp_suite/prime_agam.o"], capture_output=True, text=True)
    if res1.returncode != 0:
        print(f"Agam Codegen Error:\n{res1.stderr}")
        sys.exit(1)

    res2 = subprocess.run(["gcc", "-no-pie", "-O3", "-flto", "-march=native", "tests/cmp_suite/prime_agam.o", "src/runtime/agam_runtime.c", "src/runtime/agam_entry.c", "-o", "tests/cmp_suite/prime_agam_bin", "-lm"], capture_output=True, text=True)
    if res2.returncode != 0:
        print(f"Agam Link Error:\n{res2.stderr}")
        sys.exit(1)

    print("Compilation Complete!\n")

def run_bench(name, cmd, num_runs=5):
    print(f"Benchmarking {name} ({num_runs} runs)...")
    times = []
    output_val = None

    for r in range(num_runs):
        start = time.perf_counter()
        res = subprocess.run(cmd, capture_output=True, text=True)
        end = time.perf_counter()

        if res.returncode != 0:
            print(f"Error running {name}:\n{res.stderr}")
            return None

        elapsed_ms = (end - start) * 1000.0
        times.append(elapsed_ms)
        output_val = res.stdout.strip()

    avg_ms = statistics.mean(times)
    min_ms = min(times)
    std_dev = statistics.stdev(times) if len(times) > 1 else 0.0

    return {
        "name": name,
        "output": output_val,
        "avg_ms": avg_ms,
        "min_ms": min_ms,
        "std_dev": std_dev,
        "runs": times
    }

def main():
    build_targets()

    targets = [
        ("C++ (GCC -O3 -march=native)", ["./tests/cmp_suite/prime_cpp"]),
        ("Rust (-C opt-level=3)", ["./tests/cmp_suite/prime_rust"]),
        ("Agam (Native Release Binary -O3)", ["./tests/cmp_suite/prime_agam_bin"]),
        ("Agam (LLVM 17 -O3 JIT)", ["./build/bin/agamc", "-O3", "--run", "tests/cmp_suite/prime_bench.agam"]),
        ("Python (CPython 3.14)", ["python3", "tests/cmp_suite/prime_bench.py"]),
    ]

    results = []
    for name, cmd in targets:
        res = run_bench(name, cmd, num_runs=5)
        if res:
            results.append(res)

    print("\n==================================================================================")
    print("                 BENCHMARK RESULTS (Prime Count up to 100,000)")
    print("==================================================================================")
    print(f"{'Language / Implementation':<34} | {'Result':<7} | {'Min Time (ms)':<14} | {'Avg Time (ms)':<14} | {'Rel vs C++':<10}")
    print("-" * 92)

    cpp_avg = results[0]["avg_ms"] if results else 1.0

    for r in results:
        rel = r["avg_ms"] / cpp_avg
        print(f"{r['name']:<34} | {r['output']:<7} | {r['min_ms']:>12.2f} ms | {r['avg_ms']:>12.2f} ms | {rel:>8.2f}x")

    print("==================================================================================")

if __name__ == "__main__":
    main()
