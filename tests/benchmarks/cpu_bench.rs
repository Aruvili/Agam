use std::time::Instant;

fn main() {
    let n = 10_000_000;
    let start = Instant::now();
    let mut total: f64 = 0.0;
    for i in 1..=n {
        total += (i as f64).sqrt() * (i as f64).sin();
    }
    let duration = start.elapsed();
    println!("Result: {:.4}", total);
    println!("CPU Test: {:.4} sec", duration.as_secs_f64());
}
