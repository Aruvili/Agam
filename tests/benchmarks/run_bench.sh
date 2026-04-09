#!/bin/bash

# Configuration
AGAMC="./build/bin/agamc"
RUNTIME_C="src/runtime/agam_runtime.c"
RUNTIME_O="tests/benchmarks/agam_runtime.o"
export AGAM_STD_PATH=$(pwd -W)

# 1. Ensure compiler is built
echo "Building Agam..."
cmake --build build --target agamc > /dev/null 2>&1

# 2. Build the C runtime
echo "Compiling C runtime..."
gcc -O3 -c $RUNTIME_C -o $RUNTIME_O

# Function to run a benchmark set
run_benchmark() {
    local name=$1
    local agam_file="tests/benchmarks/${name}.agam"
    local cpp_file="tests/benchmarks/${name}.cpp"
    local rs_file="tests/benchmarks/${name}.rs"
    local py_file="tests/benchmarks/${name}.py"
    local obj_file="tests/benchmarks/${name}_agam.o"
    local bin_agam="tests/benchmarks/${name}_agam_aot"
    local bin_cpp="tests/benchmarks/${name}_cpp"
    local bin_rs="tests/benchmarks/${name}_rs"

    echo ""
    echo "=========================================="
    echo "   Benchmark: ${name^^}"
    echo "=========================================="

    # Compile others
    g++ -O3 $cpp_file -o $bin_cpp
    rustc -O $rs_file -o $bin_rs

    echo "Running Agam (JIT)..."
    echo "------------------------------------------"
    time $AGAMC $agam_file --run

    echo ""
    echo "Running Agam (AOT)..."
    echo "------------------------------------------"
    $AGAMC $agam_file -O3 -o $obj_file
    g++ -O3 $obj_file $RUNTIME_O -o $bin_agam -lm
    time ./$bin_agam

    echo ""
    echo "Running C++ (Optimized)..."
    echo "------------------------------------------"
    time ./$bin_cpp

    echo ""
    echo "Running Rust (Optimized)..."
    echo "------------------------------------------"
    time ./$bin_rs

    echo ""
    echo "Running Python..."
    echo "------------------------------------------"
    time python $py_file
}

# Run all benchmarks
run_benchmark "node_alloc"
run_benchmark "cpu_bench"
run_benchmark "array_bench"
run_benchmark "prime_bench"

echo ""
echo "=========================================="
echo "Benchmarks Complete."
echo "=========================================="
