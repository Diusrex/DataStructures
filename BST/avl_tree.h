#ifndef BST_AVL_TREE
#define BST_AVL_TREE

#include <cassert>
#include <cmath>
#include <iostream>

template <class T>
class avl_tree {
public:
    avl_tree();
    ~avl_tree();

    // Does nothing if item already exists in tree.
    void insert(const T& item);

    // Does nothing if item is not in tree.
    void remove(const T& item);

    // Returns true if item is in tree.
    bool find(const T& item) const;

    // Returns value of minimum item in tree.
    T minimum() const;

    int size() const;


    void print_out(std::ostream& o = std::cout) const;

// Protected to make testing easier.
protected:

    int num_elements;

    struct Node {
        Node(const T& value, Node* parent)
            : value(value),
            height(0),
            lhs(nullptr),
            rhs(nullptr),
            parent(parent) {
        }

        T value;
        // Height is treated as the distance from a leaf.
        // So nullptr nodes will have height -1.
        int height;
        Node* lhs;
        Node* rhs;
        Node* parent;
    };

    Node* root;

    // Insert will only need to be reotated once, while its possible that
    // delete will need to be propogated up.
    void balance(Node* current, bool only_rotate_once);

    // Updates the height of the node based on its two children.
    // The children MUST already have correct height
    void update_height(Node* node) const;

    // If lhs is taller, will be positive. Otherwise negative.
    int subtree_difference(const Node* node) const;

    // Gets the height of a node, with nullptr being -1.
    int height(const Node* node) const;

    // Rotates the subtree that has this node as base. Will update the heights
    // for the nodes that are changed.
    void left_rotate(Node* node);
    void right_rotate(Node* node);

    void set_left_child(Node* parent, Node* new_child) const;
    void set_right_child(Node* parent, Node* new_child) const;

    // Will ensure the subtree ownership is completely transfered from
    // old_root to new_root.
    // Is able to handle new_root being nullptr, but will crash if old_root is
    // nullptr.
    void transfer_subtree_parentship(Node* old_root, Node* new_root);

    // Based on the rules of a BST, will return the 'best' node to be deleted,
    // based on wanting to remove the value in given node.
    Node* get_removed_node(Node* node_with_removed_value);

    void print_out(std::ostream& o, Node* node) const;

    // Will delete the sub-tree in O(n) time, where n is the number of nodes in
    // subtree.
    void delete_subtree(Node* node);
};

template <class T>
avl_tree<T>::avl_tree() 
    : num_elements(0),
    root(nullptr) {
}

template <class T>
avl_tree<T>::~avl_tree() {
    delete_subtree(root);
}

template <class T>
void avl_tree<T>::delete_subtree(Node* node) {
    if (node == nullptr)
        return;

    delete_subtree(node->lhs);
    delete_subtree(node->rhs);

    delete node;
}

template <class T>
void avl_tree<T>::insert(const T& item) {
    // Different cases in insert_balance function from
    // https://www.tutorialspoint.com/data_structures_algorithms/avl_tree_algorithm.htm
    // Case where the tree doesn't exist. Just set as root.
    if (root == nullptr) {
        ++num_elements;
        root = new Node(item, nullptr);
        update_height(root);
        return;
    }

    // First, find the parent for this node.
    Node* node = root;
    Node* parent = nullptr;

    while (node != nullptr && node->value != item) {
        parent = node;
        if (item < node->value)
            node = node->lhs;
        else
            node = node->rhs;
    }

    // Wasn't already in the tree, so should be added.
    if (node == nullptr) {
        ++num_elements;
        Node* new_node = new Node(item, parent);
        if (item < parent->value)
            parent->lhs = new_node;
        else
            parent->rhs = new_node;

        // This node will definitely be balanced, since was just added.
        // May need to balance a parent node.
        balance(parent, /*only_rotate_once=*/true);
    }
}


template <class T>
void avl_tree<T>::remove(const T& item) {
    // Cases from https://courses.cs.washington.edu/courses/cse332/10sp/lectures/lecture8.pdf

    // Find the node with the value that is being removed.
    Node* node = root;
    while (node != nullptr && node->value != item) {
        if (node->value > item)
            node = node->lhs;
        else
            node = node->rhs;
    }

    // It didn't exist in the first place
    if (node == nullptr) {
        return;
    }

    Node* node_to_remove = get_removed_node(node);

    if (node_to_remove != node)
        node->value = node_to_remove->value;

    // At most one will not be a nullptr
    Node* moving_up = (node_to_remove->lhs != nullptr) ?
        node_to_remove->lhs : node_to_remove->rhs;

    transfer_subtree_parentship(node_to_remove, moving_up);

    // May need to rotate multiple times.
    balance(node_to_remove->parent, /*only_rotate_once=*/false);

    delete node_to_remove;
    --num_elements;
}


template <class T>
typename avl_tree<T>::Node* avl_tree<T>::get_removed_node(Node* node_with_removed_value) {
    if (node_with_removed_value->lhs == nullptr ||
            node_with_removed_value->rhs == nullptr) {
        // Can remove this node, since one of its children don't exist.
        return node_with_removed_value;
    } else {
        // Will need to remove (and swap with) node with largest value still smaller,
        // or node with smallest values still larger. Will remove from the larger of
        // sub-trees.
        Node* node_to_remove;
        if (subtree_difference(node_with_removed_value) > 0) {
            // lhs is larger, remove node with largest values still smaller.
            node_to_remove = node_with_removed_value->lhs;
            while (node_to_remove->rhs != nullptr) {
                node_to_remove = node_to_remove->rhs;
            }
        } else {
            // rhs is larger, remove node with smallest value still larger.
            node_to_remove = node_with_removed_value->rhs;
            while (node_to_remove->lhs != nullptr) {
                node_to_remove = node_to_remove->lhs;
            }
        }
        return node_to_remove;
    }
}

template <class T>
void avl_tree<T>::balance(Node* current, bool only_rotate_once) {
    if (current == nullptr) {
        return;
    }

    int diff = subtree_difference(current); 
    if (std::abs(diff) > 1) {
        // Balance subtrees from current.
        if (diff > 0) { // Move height to right side.
            // In this case, need to do a double rotate.
            if (subtree_difference(current->lhs) < 0) {
                left_rotate(current->lhs);
            }
            right_rotate(current);
        } else { // More to left side.
            if (subtree_difference(current->rhs) > 0) {
                right_rotate(current->rhs);
            }
            left_rotate(current);
        }

        if (only_rotate_once) {
            return;
        }
    }

    update_height(current);

    // Balance on parent of current.
    balance(current->parent, only_rotate_once);
}

template <class T>
void avl_tree<T>::left_rotate(Node* node) {
    // node becomes the left child of new_base, with new_base's left child
    // becoming the right child of node.
    // new_base will become the owner of the subtree, which may update root.
    Node* new_base = node->rhs;
    transfer_subtree_parentship(node, new_base);
    
    set_right_child(node, new_base->lhs);
    set_left_child(new_base, node);  

    update_height(node);
    update_height(new_base);
}

template <class T>
void avl_tree<T>::right_rotate(Node* node) {
    // node becomes the right child of new_base, with new_base's right child
    // becoming the left child of node.
    // new_base will become the owner of the subtree, which may update root.
    Node* new_base = node->lhs;
    transfer_subtree_parentship(node, new_base);
    
    set_left_child(node, new_base->rhs);
    set_right_child(new_base, node);  

    update_height(node);
    update_height(new_base);
}

template <class T>
void avl_tree<T>::set_left_child(Node* parent, Node* new_child) const {
    if (new_child != nullptr)
        new_child->parent = parent;

    parent->lhs = new_child;
}

template <class T>
void avl_tree<T>::set_right_child(Node* parent, Node* new_child) const {
    if (new_child != nullptr)
        new_child->parent = parent;

    parent->rhs = new_child;
}

template <class T>
void avl_tree<T>::transfer_subtree_parentship(Node* old_root, Node* new_root) {
    Node* parent = old_root->parent;

    if (parent != nullptr) {
        if (old_root == parent->lhs)
            set_left_child(parent, new_root);
        else
            set_right_child(parent, new_root);
    } else {
        root = new_root;
        if (new_root != nullptr)
            new_root->parent = nullptr;
    }
}

template <class T>
bool avl_tree<T>::find(const T& item) const {
    Node* current = root;

    while (current != nullptr && current->value != item) {
        if (current->value > item) {
            current = current->lhs;
        } else {
            current = current->rhs;
        }
    }

    return current != nullptr && current->value == item;
}

template <class T>
T avl_tree<T>::minimum() const {
    assert(size() > 0);

    Node* current = root;
    while (current->lhs) {
        current = current->lhs;
    }
    return current->value;
}

template <class T>
void avl_tree<T>::update_height(Node* node) const {
    node->height = 1 + std::max(height(node->lhs), height(node->rhs));
}

template <class T>
int avl_tree<T>::subtree_difference(const Node* node) const {
    return height(node->lhs) - height(node->rhs);
}

template <class T>
int avl_tree<T>::height(const Node* node) const {
    if (node == nullptr) {
        return -1;
    }

    return node->height;
}

template <class T>
int avl_tree<T>::size() const {
    return num_elements;
}

template <class T>
void avl_tree<T>::print_out(std::ostream& o) const {
    print_out(o, root);
}

template <class T>
void avl_tree<T>::print_out(std::ostream& o, Node* node) const {
    if (node == nullptr)
        return;

    o << node->value << " height " << node->height << " and goes to: ";
    if (node->lhs == nullptr)
        o << "nullptr";
    else
        o << node->lhs->value;

    o << " and ";
    if (node->rhs == nullptr)
        o << "nullptr";
    else
        o << node->rhs->value;

    o << ". Parent: ";
    if (node->parent == nullptr)
        o << "nullptr";
    else
        o << node->parent->value;

    o << ".\n";
    print_out(o, node->lhs);
    print_out(o, node->rhs);
}

#endif
