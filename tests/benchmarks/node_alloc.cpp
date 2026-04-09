#include <iostream>

struct Node {
    int val;
    Node* next;
};

int main() {
    const int count = 10000000;
    
    // Standard Heap Allocation (Global Allocator)
    Node* head = new Node{0, nullptr};
    Node* current = head;
    
    for (int i = 1; i < count; ++i) {
        Node* next_node = new Node{i, nullptr};
        current->next = next_node;
        current = next_node;
    }
    
    std::cout << current->val << std::endl;
    
    return 0;
}
