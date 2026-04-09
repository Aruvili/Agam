#include <iostream>
#include <cstdint>

struct Node {
    int val;
    Node* next;
};

// Simple bump allocator (Arena) to mimic Agam ZPM behavior
class Arena {
public:
    Arena(size_t size) : size_(size), used_(0) {
        data_ = static_cast<uint8_t*>(std::malloc(size));
    }
    
    ~Arena() {
        std::free(data_);
    }
    
    void* alloc(size_t size) {
        // Simple align to 8
        size = (size + 7) & ~7;
        if (used_ + size > size_) {
            return nullptr; // Out of memory
        }
        void* ptr = data_ + used_;
        used_ += size;
        return ptr;
    }

private:
    uint8_t* data_;
    size_t size_;
    size_t used_;
};

int main() {
    const int count = 10000000;
    // Pre-calculate size: 10M nodes * 16 bytes (int + pointer + padding)
    Arena arena(count * 16 + 1024);
    
    Node* head = static_cast<Node*>(arena.alloc(sizeof(Node)));
    head->val = 0;
    head->next = nullptr;
    
    Node* current = head;
    for (int i = 1; i < count; ++i) {
        Node* next_node = static_cast<Node*>(arena.alloc(sizeof(Node)));
        next_node->val = i;
        next_node->next = nullptr;
        
        current->next = next_node;
        current = next_node;
    }
    
    std::cout << current->val << std::endl;
    
    return 0;
}
