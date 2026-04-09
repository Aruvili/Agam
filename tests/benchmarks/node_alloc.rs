struct Node {
    val: i32,
    next: Option<Box<Node>>,
}

// In Rust, we need to be careful with stack depth for 10M nodes
// Standard Box<Node> would cause stack overflow during drop()
// For a fair "allocation" benchmark, we use a loop to build the chain

fn main() {
    let count = 10000000;
    
    // Initial head
    let mut head = Box::new(Node {
        val: 0,
        next: None,
    });
    
    // We cannot easily do current = head.next for a long chain due to ownership
    // Instead, we just measure pure allocation of 10M boxes in a loop 
    // and link them if possible.
    
    // To fairly match the Agam benchmark layout:
    let mut current = &mut *head;
    for i in 1..count {
        current.next = Some(Box::new(Node {
            val: i,
            next: None,
        }));
        current = current.next.as_mut().unwrap();
    }
    
    println!("{}", current.val);
    
    // Explicitly leak or skip drop to prevent 10M stack-depth recursion during cleanup
    std::mem::forget(head);
}
