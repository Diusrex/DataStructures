#ifndef REDBLACKTREEBASIC_H
#define REDBLACKTREEBASIC_H

// This file contains RedBlackTree's implementation for all functions that are
// not specific to rotation, insertion, or deletion.
// They are mostly just convenience functions that are used in the 3 other
// implementation headers.

#ifndef REDBLACKTREE_H
#error This file should only be included by RedBlackTree.h
#endif

template<typename T>
bool RedBlackTree<T>::IsBlack(const Node* node) const
{
    // Leaves are considered to be black.
    return node == nullptr || node->color == Node::BLACK;
}

template<typename T>
bool RedBlackTree<T>::IsRed(const Node* node) const
{
    return node != nullptr && node->color == Node::RED;
}

template<typename T>
typename RedBlackTree<T>::Node* RedBlackTree<T>::GetSibling(const Node* node) const
{
    Node* parent = node->parent;
    if (parent == nullptr)
        return nullptr;
    
    if (parent->left == node)
        return parent->right;
    else
        return parent->left;
}

template<typename T>
bool RedBlackTree<T>::Contains(const T& value) const
{
    Node* node = root;
    while (node != nullptr && node->value != value)
    {
        if (value < node->value)
            node = node->left;
        else
            node = node->right;
    }
    
    return node != nullptr;
}

// Note that this does NOT create any relation between new and old root
// Does not change oldRoot at all
// Is safe to call for any combination of nullptr and valid nodes
template<typename T>
void RedBlackTree<T>::TransferSubtreeParentship(Node* oldRoot, Node* newRoot)
{
    Node* parent = oldRoot->parent;
    
    if (parent != nullptr)
    {
        if (oldRoot == parent->left)
            SetLeftChild(parent, newRoot);
        else
            SetRightChild(parent, newRoot);
    }
    else
    {
        root = newRoot;
        
        if (newRoot != nullptr)
            newRoot->parent = nullptr;
    }
}

template<typename T>
void RedBlackTree<T>::SetLeftChild(Node* parent, Node* leftChild)
{
    if (leftChild != nullptr)
        leftChild->parent = parent;
    
    if (parent != nullptr)
        parent->left = leftChild;
}

template<typename T>
void RedBlackTree<T>::SetRightChild(Node* parent, Node* rightChild)
{
    if (rightChild != nullptr)
        rightChild->parent = parent;
    
    parent->right = rightChild;
}

template<typename T>
void RedBlackTree<T>::WriteOut(std::ostream& o) const
{
    WriteOut(o, root);
}

template<typename T>
void RedBlackTree<T>::WriteOut(std::ostream& o, const Node* node) const
{
    if (node == nullptr)
        return;
    
    o << node->value << " is " << (IsBlack(node) ? "black" : "red") << " and goes to: ";
    if (node->left == nullptr)
        o << "nullptr";
    else
        o << node->left->value;
    
    o << " and ";
    if (node->right == nullptr)
        o << "nullptr";
    else
        o << node->right->value;
    
    o << ". Parent: ";
    if (node->parent == nullptr)
        o << "nullptr";
    else
        o << node->parent->value;
    
    o << ".\n";
    WriteOut(o, node->left);
    WriteOut(o, node->right);
}

template<typename T>
void RedBlackTree<T>::RemoveSubtree(Node* current)
{
    if (current == nullptr)
        return;
    
    RemoveSubtree(current->left);
    RemoveSubtree(current->right);
    
    delete current;
}

#endif
