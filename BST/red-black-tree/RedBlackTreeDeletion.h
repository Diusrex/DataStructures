#ifndef REDBLACKTREEDELETION_H
#define REDBLACKTREEDELETION_H

// This file contains RedBlackTree's implementation for all functions that are
// specific to deletion.

#ifndef REDBLACKTREE_H
#error This file should only be included by RedBlackTree.h
#endif

template<typename T>
bool RedBlackTree<T>::Delete(const T& value)
{
    // Find the node with the value that is being removed.
    Node* node = root;
    while (node != nullptr && node->value != value) {
        if (value < node->value)
            node = node->left;
        else
            node = node->right;
    }
    
    // It didn't exist in the first place
    if (node == nullptr) {
        return false;
    }
    
    // Determine the node that will be removed.
    Node* nodeBeingRemoved = GetRemovedNode(node);
    
    // At most one will not be a nullptr
    Node* movingUp = nodeBeingRemoved->left == nullptr ? nodeBeingRemoved->right : nodeBeingRemoved->left;
    
    bool movingUpWasNull = (movingUp == nullptr);
    // Only time this can happen is if it has no children
    if (movingUpWasNull) {
        // temp value, will be removed later
        movingUp = new Node(Node::BLACK, -1, nodeBeingRemoved);
    }
    
    // Update currents value to be the value of the node being removed
    // if node is not itself being removed
    // This way, don't remove the other value from the tree
    if (nodeBeingRemoved != node)
        node->value = nodeBeingRemoved->value;
    
    // Transfer the ownership, which also cuts out nodeBeingRemoved from the tree
    // Does not change the references for nodeBeingRemoved
    TransferSubtreeParentship(nodeBeingRemoved, movingUp);
    
    
    if (IsRed(movingUp) || IsRed(nodeBeingRemoved)) {
        movingUp->color = Node::BLACK;
    } else {
        HandleDoubleBlack(movingUp);
    }
    
    // Clean up movingUp if it was null
    if (movingUpWasNull) {
        // Replace moving up with nullptr
        TransferSubtreeParentship(movingUp, nullptr);
        delete movingUp;
    }
    
    // Finally, delete nodeBeingRemoved
    delete nodeBeingRemoved;
    
    return true;
}

// nodeToRemove must start off with the node with value to be deleted
// Will return the node that is to be removed
// 
template<typename T>
typename RedBlackTree<T>::Node* RedBlackTree<T>::GetRemovedNode(Node* nodeToRemove) const {
    if (nodeToRemove->left == nullptr) {
        // Right child will be replacing nodeToRemove.
        return nodeToRemove;
    } else if (nodeToRemove->right == nullptr) {
        // Left child will be replacing nodeToRemove.
        return nodeToRemove;
    }
    
    // In this case, will be removing the next smallest (or biggest node), and moving its value into current
    Node* nextSmallest = nodeToRemove->left;
    while (nextSmallest->right != nullptr)
        nextSmallest = nextSmallest->right;
    
    // Operation is significantly easier if removing a red node or the removed
    // node has red child.
    if (IsRed(nextSmallest) || IsRed(nextSmallest->left)) {
        return nextSmallest;
    }
    
    Node* nextBiggest = nodeToRemove->right;
    while (nextBiggest->left != nullptr)
        nextBiggest = nextBiggest->left;
    
    // Hope nextBiggest or its right child is red.
    return nextBiggest;
}

// doubleBlackChild cannot be null
// If it is, should replace it with a blank node, then remove and delete it after
template<typename T>
void RedBlackTree<T>::HandleDoubleBlack(Node* doubleBlackChild) {
    if (doubleBlackChild->parent == nullptr) {
        // At the root, so can change it freely from double black to black
        return;
    }
    
    // Know that current doubleBlackChild previously had a black relative
    // So this means that sibling must exist
    Node* parent = doubleBlackChild->parent;
    Node* sibling = GetSibling(doubleBlackChild);
    Node* siblingLeft = sibling->left;
    Node* siblingRight = sibling->right;
    
    bool oneSiblingChildIsRed = IsRed(siblingLeft) || IsRed(siblingRight);
    
    // Handle the different situations for a double black node.
    if (IsRed(sibling)) {
        // Rotate it in the direction of the double black.
        if (doubleBlackChild == parent->left) {
            LeftRotate(parent);
        } else {
            RightRotate(parent);
        }
        
        // The color of sibling and parent before and after are locked
        sibling->color = Node::BLACK;
        parent->color = Node::RED;
        
        // However, the doubleBlackChild remains double black, just shifted the red around
        // This case makes it easier to finish.
        HandleDoubleBlack(doubleBlackChild);
    } else if (oneSiblingChildIsRed) {
        // Rotate the subtree, centered at parent, towards the double black child.
        // If the siblings non-red child would be rotated to be the child of new subtree root,
        // do a double rotate to ensure the red node would be rotated there.
        if (doubleBlackChild == parent->left) {
            if (IsBlack(siblingRight)) {
                RightRotate(sibling);
            }
            
            LeftRotate(parent);
        } else { // Is on right
            if (IsBlack(siblingLeft)) {
                LeftRotate(sibling);
            }
        
            RightRotate(parent);
        }
        
        // New subtree root is parent of original parent
        Node* newParent = parent->parent;
        
        // Its color should be that of the old parent
        newParent->color = parent->color;
        
        // As well, the color of nodes to left and right of new parent should be black
        newParent->left->color = newParent->right->color = Node::BLACK;
    } else { // Sibling and children are black
        // Shift black up, removing from sibling
        sibling->color = Node::RED;
        if (IsBlack(doubleBlackChild->parent)) {
            HandleDoubleBlack(doubleBlackChild->parent);
        } else {
            doubleBlackChild->parent->color = Node::BLACK;
        }
    }
}

#endif
