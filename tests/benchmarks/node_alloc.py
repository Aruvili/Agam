import time

class Node:
    def __init__(self, val, next):
        self.val = val
        self.next = next

def main():
    count = 10000000
    start_time = time.time()
    
    head = Node(0, None)
    current = head
    
    for i in range(1, count):
        next_node = Node(i, None)
        current.next = next_node
        current = next_node
        
    end_time = time.time()
    print(current.val)
    # print(f"Python Time: {end_time - start_time:.4f}s")

if __name__ == "__main__":
    main()
