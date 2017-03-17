#ifndef PAIRING_HEAP
#define PAIRING_HEAP

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <queue>

// Handling of differnent functionality from https://brilliant.org/wiki/pairing-heap/.
// Specifically, will use simple two-pass (left->right then right->left) to handle 
// deleting an element.

// Is a min-heap, although that could be changed.
template <class Key, class Weight>
class pairing_heap {
public:
    pairing_heap();
    ~pairing_heap();

    // Asserts that key isn't already in heap
    void insert(const Key& key, const Weight& weight);

    bool inHeap(const Key& key) {
        return key_to_node.find(key) != key_to_node.end();
    }

    // Fails an assert if no elements in heap
    std::pair<Key, Weight> find_min() const;

    std::pair<Key, Weight> extract_min();

    // Fails an assert if element not in heap.
    void decrease_key(const Key& key, const Weight& weight);

    size_t size() const { return num_elem; }

    void print_out(std::ostream& o = std::cout) const;

protected:

    struct Node {
        Node(const Key& key, const Weight& weight)
            : key(key),
            weight(weight),
            parent(nullptr),
            left_child(nullptr) {
            left_sibling = this;
            right_sibling = this;
        }

        const Key key;
        Weight weight;
        Node* parent;
        Node* left_child;
        Node* left_sibling;
        Node* right_sibling;
    };

    Node* heap_root;

    std::unordered_map<Key, Node*> key_to_node;

private:
    // Will remove this node, run two pass on its children, then merge them with the root.
    // Will call delete on node.
    void delete_node(Node* node);

    Node* run_two_pass_merge(Node* leftmost_node);
    Node* run_left_then_right_merge(Node* first, Node* second, Node* leftmost_node);

    // Will return pointer to root of merge.
    // Able to handle either one being a nullptr.
    Node* merge_heaps(Node* h1, Node* h2);

    void add_second_to_children(Node* heap, Node* new_child);

    // Will also change the node to point to itself as left + right sibling.
    void remove_from_sibling_list(Node* node);

    // May update the root of this heap.
    void decrease_key(Node* node, const Weight& weight);

    // Completely obliterates the heap - O(n)
    void delete_heap(Node* heap);

    void print_out(Node* heap, std::ostream& o, const std::string& offset) const;

    size_t num_elem;
};

#define pairing_heap_node typename pairing_heap<Key, Weight>::Node

template <class Key, class Weight>
pairing_heap<Key, Weight>::pairing_heap()
    : heap_root(nullptr),
    num_elem(0)
{ }

template <class Key, class Weight>
pairing_heap<Key, Weight>::~pairing_heap() {
    delete_heap(heap_root);
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::insert(const Key& key, const Weight& weight) {
    assert(!inHeap(key));

    ++num_elem;

    Node* new_node = new Node(key, weight);
    key_to_node[key] = new_node;

    // Just merge this new node with existing heap.
    heap_root = merge_heaps(heap_root, new_node);
}

template <class Key, class Weight>
std::pair<Key, Weight> pairing_heap<Key, Weight>::find_min() const {
    assert(heap_root != nullptr);

    // Just return root of heap.
    return std::make_pair(heap_root->key, heap_root->weight);
}

template <class Key, class Weight>
std::pair<Key, Weight> pairing_heap<Key, Weight>::extract_min() {
    const std::pair<Key, Weight> val = find_min();

    delete_node(heap_root);

    return val;
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::delete_node(Node* node) {
    // First, remove the node from child list.
    remove_from_sibling_list(node);

    Node* children_subtree = run_two_pass_merge(node->left_child);

    if (children_subtree != nullptr)
        children_subtree->parent = nullptr;

    // Ensure don't try to merge with this node.
    if (heap_root == node)
        heap_root = nullptr;

    // Merge children subtree with root
    heap_root = merge_heaps(heap_root, children_subtree);

    delete node;
    --num_elem;
}

template <class Key, class Weight>
pairing_heap_node* pairing_heap<Key, Weight>::run_two_pass_merge(Node* leftmost_node) {
    // One or zero nodes in list, so already done.
    if (leftmost_node == nullptr || leftmost_node->right_sibling == leftmost_node) {
        return leftmost_node;
    }

    return run_left_then_right_merge(leftmost_node, leftmost_node->right_sibling, leftmost_node);
}

template <class Key, class Weight>
pairing_heap_node* pairing_heap<Key, Weight>::run_left_then_right_merge(
        Node* first, Node* second, Node* leftmost_node) {

    Node* first_to_right = second->right_sibling;
    Node* second_to_right = first_to_right->right_sibling;

    // First, reset their siblings, then merge
    first->left_sibling = first->right_sibling = first;
    second->left_sibling = second->right_sibling = second;
    Node* merged = merge_heaps(first, second);

    // Have gone around tree.
    if (first_to_right == leftmost_node) {
        return merged;
    }


    // Almost gone around tree, just need to merge in first_to_right
    if (second_to_right == leftmost_node) {
        // First reset its siblings though.
        first_to_right->left_sibling = first_to_right->right_sibling = first_to_right;
        return merge_heaps(merged, first_to_right);
    }

    // Need to continue with sibling list, and merge that with what we have here.
    return merge_heaps(merged, run_left_then_right_merge(first_to_right, second_to_right, leftmost_node));
}


template <class Key, class Weight>
void pairing_heap<Key, Weight>::decrease_key(const Key& key, const Weight& new_weight) {
    assert(inHeap(key));

    decrease_key(key_to_node[key], new_weight);
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::decrease_key(Node* node, const Weight& new_weight) {
    node->weight = new_weight;

    // Is fine, nothing needs to be done.
    if (node->parent == nullptr || node->parent->weight <= new_weight) {
        return;
    }

    remove_from_sibling_list(node);
    node->parent = nullptr;

    // Ensure heap root is updated.
    heap_root = merge_heaps(heap_root, node);
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::remove_from_sibling_list(Node* node) {
    Node* left = node->left_sibling;
    Node* right = node->right_sibling;

    Node* parent = node->parent;

    // Was only element in the child list
    if (parent != nullptr && parent->left_child == node) {
        if (left == node) // This node is the only one in child list
            parent->left_child = nullptr;
        else
            parent->left_child = left;
    }

    // Remove node from list.
    left->right_sibling = right;
    right->left_sibling = left;

    // Have node point to itself
    node->left_sibling = node->right_sibling = node;
}

template <class Key, class Weight>
typename pairing_heap<Key, Weight>::Node* pairing_heap<Key, Weight>::merge_heaps(Node* h1, Node* h2) {
    if (h1 == nullptr)
        return h2;
    if (h2 == nullptr)
        return h1;

    Node* smaller;
    Node* larger;
    if (h2->weight > h1->weight) {
        smaller = h1;
        larger = h2;
    } else {
        smaller = h2;
        larger = h1;
    }

    add_second_to_children(smaller, larger);
    return smaller;
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::add_second_to_children(Node* heap, Node* new_child) {
    new_child->parent = heap;
    if (heap->left_child == nullptr) {
        heap->left_child = new_child;
        new_child->left_sibling = new_child->right_sibling = new_child;
        return;
    }

    Node* left = heap->left_child;
    Node* right = left->right_sibling;

    // Update the 4 pointers - two into node, two out from it.
    left->right_sibling = new_child;
    new_child->left_sibling = left;

    right->left_sibling = new_child;
    new_child->right_sibling = right;
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::delete_heap(Node* heap) {
    if (heap == nullptr) {
        return;
    }

    // Will delete all siblings in iteration.
    std::queue<Node*> toDelete;
    toDelete.push(heap);

    while (!toDelete.empty()) {
        Node* base_of_sibling_list = toDelete.front();
        toDelete.pop();
        Node* current = base_of_sibling_list;

        do {
            if (current->left_child != nullptr) {
                toDelete.push(current->left_child);
            }

            Node* prev = current;
            current = current->right_sibling;

            delete prev;

        } while (current != base_of_sibling_list);
    }
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::print_out(std::ostream& o) const {
    print_out(heap_root, o, "");
}

template <class Key, class Weight>
void pairing_heap<Key, Weight>::print_out(Node* current, std::ostream& o, const std::string& offset) const {
    if (current == nullptr) {
        return;
    }

    const std::string next_offset = offset + "    ";
    o << offset << "Key " << current->key << " weight " << current->weight << ". Children:" << std::endl;

    Node* base_child = current->left_child;
    if (base_child == nullptr) {
        o << offset << "  None";
    } else {
        Node* child = base_child;

        print_out(child, o, next_offset);
        child = child->right_sibling;

        while (child != base_child) {
            print_out(child, o, next_offset);
            child = child->right_sibling;
        }
    }
    
    o << "\n";
}


#endif  // PAIRING_HEAP
