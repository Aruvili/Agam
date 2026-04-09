use std::time::Instant;

fn main() {
    let n = 5_000_000;
    let start = Instant::now();
    let mut arr = Vec::with_capacity(n);
    for i in 0..n {
        arr.push((i as i64) * 2);
    }
    let total: i64 = arr.iter().sum();
    let duration = start.elapsed();
    println!("Result: {}", total);
    println!("Array Test: {:.4} sec", duration.as_secs_f64());
}
