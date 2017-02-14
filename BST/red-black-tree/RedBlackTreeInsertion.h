#ifndef REDBLACKTREEINSERTION_H
#define REDBLACKTREEINSERTION_H

// This file contains RedBlackTree's implementation for all functions that are
// specific to insertion.
// For an explanation of insertion, see diusrex.com/painless-red-black-tree-implementation-insertion

#ifndef REDBLACKTREE_H
#error This file should only be included by RedBlackTree.h
#endif

template<typename T>
bool RedBlackTree<T>::Insert(const T &value) {
    // Case where the tree doesn't exist. Just set as root.
    if (root == nullptr) {
        root = new Node(Node::BLACK, value, nullptr);
        return true;
    }
    
    // First, find the parent for this node.
    Node* node = root;
    Node* parent = nullptr;
    while (node != nullptr && node->value != value) {
        parent = node;
        if (value < node->value)
            node = node->left;
        else
            node = node->right;
    }
    
    // Wasn't already in the tree, so should be added
    if (node == nullptr) {
        Node* newNode = new Node(Node::RED, value, parent);
        if (value < parent->value)
            parent->left = newNode;
        
        else
            parent->right = newNode;
        
        HandleDoubleRed(newNode, parent);
        return true;
    }
    
    return false;
}

template<typename T>
void RedBlackTree<T>::HandleDoubleRed(Node* child, Node* parent) {
    // At least one is black, so no problem
    if (IsBlack(child) || IsBlack(parent)) {
        return;
    }
    
    // Know the grandparent exists (otherwise parent would be black, since root is black)
    Node* grandparent = parent->parent;
    Node* uncle = GetSibling(parent);
    
    // Can switch parent + uncle to be black and possibly switch grandparent
    // diusrex.com/painless-red-black-tree-implementation-insertion#uncle-red
    if (IsRed(uncle)) {
        parent->color = uncle->color = Node::BLACK;
        
        // If grandparent isn't root, then should handle it being red and parent being red
        if (grandparent->parent != nullptr) {
            grandparent->color = Node::RED;
            HandleDoubleRed(grandparent, grandparent->parent);
        }
        
        return;
    }
    
    // diusrex.com/painless-red-black-tree-implementation-insertion#grandparent-rotations
    // Two main categories of shifts
    if (parent == grandparent->left) {
        // diusrex.com/painless-red-black-tree-implementation-insertion#parent-left
        // Overall, is a right shift to shift the grandparent to where the uncle used to be
            // However, which node will be moved there depends on the structure beforehand
        
        // Need to reorder the child and parent first
        if (child == parent->right) {
            LeftRotate(parent);
        }
        
        RightRotate(grandparent);
    } else { // parent == grandparent->right
        // diusrex.com/painless-red-black-tree-implementation-insertion#parent-right
        // Overall, is a right shift to shift the grandparent to where the uncle used to be
            // However, which node will be moved there depends on the structure beforehand
        
        // Need to reorder the child and parent first
        if (child == parent->left) {
            RightRotate(parent);
        }
        
        LeftRotate(grandparent);
    }
    
    // Now, need to change the colors.
    // The grandparent node will be red, as will its sibling, and their parent will be black
    grandparent->color = GetSibling(grandparent)->color = Node::RED;
    grandparent->parent->color = Node::BLACK;
}

#endif
