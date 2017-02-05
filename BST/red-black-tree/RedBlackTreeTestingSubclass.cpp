#include "RedBlackTreeTestingSubclass.h"

// This file is for the implementation of RedBlackTreeTestingSubclass 
// TODO: Additional comments in this - ParentUp vs NewNodeUp

using std::string;
using std::to_string;

int RedBlackTreeTestingSubclass::SetUp_Insert_ClockwiseShift_ParentUp() {
    int parentUp, childUp;
    
    SetUp_Insert_Shift(true, &parentUp, &childUp);
    
    return parentUp;
}

int RedBlackTreeTestingSubclass::SetUp_Insert_ClockwiseShift_NewNodeUp() {
    int parentUp, childUp;
    
    SetUp_Insert_Shift(true, &parentUp, &childUp);
    
    return childUp;
}

int RedBlackTreeTestingSubclass::SetUp_Insert_CounterClockwiseShift_ParentUp() {
    int parentUp, childUp;
    
    SetUp_Insert_Shift(false, &parentUp, &childUp);
    
    return parentUp;
}

int RedBlackTreeTestingSubclass::SetUp_Insert_CounterClockwiseShift_NewNodeUp() {
    int parentUp, childUp;
    
    SetUp_Insert_Shift(false, &parentUp, &childUp);
    
    return childUp;
}

void RedBlackTreeTestingSubclass::SetUp_Insert_Shift(bool clockwise, int* parentUp, int* childUp) {
    // Reset
    RemoveSubtree(root);
    root = nullptr;
    
    // Add root.
    Insert(5);
    
    int parent;
    if (clockwise)
        parent = 2;
    else
        parent = 7;
    
    // Parent
    Insert(parent);
    
    // If parent - 1 is inserted, parent would be rotated into root.
    *parentUp = parent - 1;
    // If parent + 1 is inserted, child would be rotated into root.
    *childUp = parent + 1;
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack() {
    int left, right;
    
    SetUp_Delete_SiblingAndChildrenBlack(&left, &right);
    
    return left;
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingAndChildrenBlack(){
    int left, right;
    
    SetUp_Delete_SiblingAndChildrenBlack(&left, &right);
    
    return right;
}

void RedBlackTreeTestingSubclass::SetUp_Delete_SiblingAndChildrenBlack(int* leftVal, int* rightVal) {
    Node* base = new Node(Node::BLACK, 4, nullptr);
    *leftVal = base->value - 1;
    *rightVal = base->value + 1;
    base->left = new Node(Node::BLACK, *leftVal, base);
    base->right = new Node(Node::BLACK, *rightVal, base);
    
    UpdateRoot(base);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack_ParentIsRed() {
    int left, right;
    
    SetUp_Delete_SiblingAndChildrenBlack_ParentIsRed(&left, &right);
    
    return left;
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingAndChildrenBlack_ParentIsRed() {
    int left, right;
    
    SetUp_Delete_SiblingAndChildrenBlack_ParentIsRed(&left, &right);
    
    return left;
}

void RedBlackTreeTestingSubclass::SetUp_Delete_SiblingAndChildrenBlack_ParentIsRed(int* leftVal, int* rightVal) {
    Node* base = new Node(Node::BLACK, 4, nullptr);
    base->left = new Node(Node::BLACK, 2, base);
    base->right = new Node(Node::RED, 7, base);
    
    Node* parent = base->right;
    
    *leftVal = parent->value - 1;
    *rightVal = parent->value + 1;
    
    parent->left = new Node(Node::BLACK, *leftVal, parent);
    parent->right = new Node(Node::BLACK, *rightVal, parent);
    
    UpdateRoot(base);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingLeftIsRed() {
    return SetUp_Delete_OneSiblingChildRed(true, true);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingLeftIsRed() {
    return SetUp_Delete_OneSiblingChildRed(false, true);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingRightIsRed() {
    return SetUp_Delete_OneSiblingChildRed(true, false);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingRightIsRed() {
    return SetUp_Delete_OneSiblingChildRed(false, false);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_OneSiblingChildRed(bool deletedLeftOfParent, bool redSiblingNodeLeft) {
    // Reset
    RemoveSubtree(root);
    root = nullptr;
    
    Insert(5);
    int left = 3, right = 7;
    
    Insert(left);
    Insert(right);
    
    // Adds to opposite side
    int base = deletedLeftOfParent ? right : left;
    

    if (redSiblingNodeLeft)
        Insert(base - 1);
    else
        Insert(base + 1);
        
    return deletedLeftOfParent ? left : right;
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingLeftIsRed_ParentIsRed() {
    return SetUp_Delete_OneSiblingChildRed_ParentIsRed(true, true);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingLeftIsRed_ParentIsRed() {
    return SetUp_Delete_OneSiblingChildRed_ParentIsRed(false, true);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_LeftOfParent_SiblingRightIsRed_ParentIsRed() {
    return SetUp_Delete_OneSiblingChildRed_ParentIsRed(true, false);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_RightOfParent_SiblingRightIsRed_ParentIsRed() {
    return SetUp_Delete_OneSiblingChildRed_ParentIsRed(false, false);
}

int RedBlackTreeTestingSubclass::SetUp_Delete_OneSiblingChildRed_ParentIsRed(bool deletedLeftOfParent, bool redSiblingNodeLeft) {
    // Build the standard part of the tree
    // Is difficult to set this type of tree up with just using regular insertion
    Node* base = new Node(Node::BLACK, 4, nullptr);
    base->left = new Node(Node::BLACK, 2, base);
    base->right = new Node(Node::RED, 20, base);
    
    Node* parent = base->right;
    parent->left = new Node(Node::BLACK, parent->value - 5, parent);
    parent->right = new Node(Node::BLACK, parent->value + 5, parent);
    
    Node* sibling;
    int toDelete;
    
    if (deletedLeftOfParent) {
        sibling = parent->right;
        toDelete = parent->left->value;
    } else {
        sibling = parent->left;
        toDelete = parent->right->value;
    }
    
    if (redSiblingNodeLeft) {
        sibling->left = new Node(Node::RED, sibling->value - 1, sibling);
    } else {
        sibling->right = new Node(Node::RED, sibling->value + 1, sibling);
    }
    
    UpdateRoot(base);
    
    return toDelete;
}

void RedBlackTreeTestingSubclass::UpdateRoot(Node* newRoot) {
    RemoveSubtree(root);
    root = newRoot;
    
    try {
        AssertMeetsConditions();
    } catch (string error) {
        std::cout << "ERROR: " << error << " when updating root.\n";
    }
}

void RedBlackTreeTestingSubclass::AssertMeetsConditions() const {
    if (root == nullptr)
        return;
    
    if (root->parent != nullptr)
        throw "The root thinks it has a parent";
    
    if (IsRed(root))
        throw "The root is not black";
    
    AssertIsBinaryTree(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    AssertIsRedBlackTree(root);
}

void RedBlackTreeTestingSubclass::AssertIsBinaryTree(const Node* node, int minimum, int maximum) const {
    if (node == nullptr)
        return;
    
    if (node->value <= minimum || node->value >= maximum)
        throw "The value " + to_string(node->value) + " is outside the bounds ("
            + to_string(minimum) + ", " + to_string(maximum) + ")";
    
    if (node->left != nullptr && node->left->parent != node)
        throw "The node " + to_string(node->left->value) +
            " does not have the right parent";
    
    if (node->right != nullptr && node->right->parent != node)
        throw "The node " + to_string(node->right->value) +
            " does not have the right parent";
    
    
    AssertIsBinaryTree(node->left, minimum, node->value);
    AssertIsBinaryTree(node->right, node->value, maximum);
}

int RedBlackTreeTestingSubclass::AssertIsRedBlackTree(Node* node) const {
    if (node == nullptr)
        return 0;
    
    int numBlackOnLeft = AssertIsRedBlackTree(node->left);
    int numBlackOnRight = AssertIsRedBlackTree(node->right);
    
    if (numBlackOnLeft != numBlackOnRight)
        throw "The node " + to_string(node->value) + 
            " does not have an equal number of black nodes to leaves";
    
    if (IsRed(node)) {
        // A red node must have black children
        if (IsRed(node->left))
            throw "The node " + to_string(node->value) +
                " should not have a red child";
    
        if (IsRed(node->right))
            throw "The node " + to_string(node->value) +
                " should not have a red child";
    }
    
    return numBlackOnLeft + (IsBlack(node) ? 1 : 0);
}
