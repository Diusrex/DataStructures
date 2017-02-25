#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include <limits>
#include <iostream>

// Complete RedBlackTree. Implementation is spread among 4 different files -
// RedBlackTreeBasic.h, RedBlackTreeRotate.h, RedBlackTreeInsertion.h, and
// RedBlackTreeDeletion.h to make it more obvious how each part is implemented.

// Note that this class isn't optimised for memory or speed, but is rather
// a simple example.
template<typename T>
class RedBlackTree
{
public:
    RedBlackTree()
        : root(nullptr)
    {}
    
    ~RedBlackTree()
    {
        RemoveSubtree(root);
    }
    
    bool Contains(const T &value) const;
    
    bool Insert(const T &value);
    
    bool Delete(const T & value);
    
    void WriteOut(std::ostream& o) const;
    
protected:
    // Just have this protected for possible children
    struct Node {
        enum Color {BLACK, RED};
        Node(Color color, const T &value, Node*parent)
            : color(color),
            value(value),
            parent(parent),
            left(nullptr),
            right(nullptr)
        {}
        
        Color color;
        T value;

        Node* parent;

        Node* left;
        Node* right;
    };
    
    Node* root;

    // This function will remove all nodes in the subtree. May imbalance the
    // tree if called on anything other than the root.
    void RemoveSubtree(Node* current);

    bool IsBlack(const Node* node) const;
    bool IsRed(const Node* node) const;
    
private:
    
    void SetLeftChild(Node* parent, Node* leftChild);
    void SetRightChild(Node* parent, Node* rightChild);
    Node* GetSibling(const Node* node) const;
    
    void HandleDoubleRed(Node* child, Node* parent);
    
    // Assumes that the right child of baseChanged exists.
    // Will update all references for the changed nodes, including to parent.
    void LeftRotate(Node* baseChanged);

    // Assumes that the left child of baseChanged exists.
    // Will update all references for the changed nodes, including to parent.
    void RightRotate(Node* baseChanged);
    
    void TransferSubtreeParentship(Node* oldRoot, Node* newRoot);
    
    void HandleDoubleBlack(Node* doubleBlackChild);
    Node* GetRemovedNode(Node* nodeToRemove) const;
    
    void WriteOut(std::ostream& o, const Node* node) const;
};

// All the code needs to be in the header due to it being a template

#include "RedBlackTreeBasic.h"
#include "RedBlackTreeRotation.h"
#include "RedBlackTreeDeletion.h"
#include "RedBlackTreeInsertion.h"

#endif
