use std::time::Instant;

fn is_prime(num: i32) -> bool {
    if num < 2 { return false; }
    let limit = (num as f64).sqrt() as i32;
    for i in 2..=limit {
        if num % i == 0 { return false; }
    }
    true
}

fn main() {
    let limit = 20000;
    let start = Instant::now();
    let mut count = 0;
    for num in 2..limit {
        if is_prime(num) {
            count += 1;
        }
    }
    let duration = start.elapsed();
    println!("Result: {}", count);
    println!("Prime Test: {:.4} sec", duration.as_secs_f64());
}
