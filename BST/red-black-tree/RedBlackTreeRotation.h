#ifndef REDBLACKTREEROTATION_H
#define REDBLACKTREEROTATION_H

// This file contains RedBlackTree's implementation for all functions that are
// specific to rotation.

#ifndef REDBLACKTREE_H
#error This file should only be included by RedBlackTree.h
#endif

template<typename T>
void RedBlackTree<T>::LeftRotate(Node* baseChanged)
{
    // baseChanged will become the left child of newBase (its right child)
    // newBase will also become the 'owner' of the subtree
    Node* newBase = baseChanged->right;
    TransferSubtreeParentship(baseChanged, newBase);
    
    SetRightChild(baseChanged, newBase->left);
    SetLeftChild(newBase, baseChanged);
}


template<typename T>
void RedBlackTree<T>::RightRotate(Node* baseChanged)
{
    // baseChanged will become the right child of newBase (its left child)
    // newBase will also become the 'owner' of the subtree
    Node* newBase = baseChanged->left;
    TransferSubtreeParentship(baseChanged, newBase);
    
    SetLeftChild(baseChanged, newBase->right);
    SetRightChild(newBase, baseChanged);
}

#endif
