fn main() {
    let limit: i32 = 100000;
    let mut count: i32 = 0;
    
    for num in 2..limit {
        let mut is_prime = true;
        let max_i = (num as f64).sqrt() as i32;
        for i in 2..=max_i {
            if num % i == 0 {
                is_prime = false;
                break;
            }
        }
        if is_prime {
            count += 1;
        }
    }
    
    println!("{}", count);
}
