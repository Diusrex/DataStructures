#pragma once

#include "RedBlackTree.h"

// This file contains the class that makes testing the RedBlackTree easier.
// It will purposefully set up certain situations, and return the int
// to be added to the tree to test a certain case.

// This code style is pretty medicore (what the tests are doing are so hidden,
// and could easily be wrong without being able to tell), but it works and I
// don't have enough time to improve it right now.
// There are a huge number of special cases that should be tested unfortunately.

// This class is meant to be used for testing the RedBlackTree
class RedBlackTreeTestingSubclass : public RedBlackTree<int>
{
public:
    // Following SetUp functions are for more complicated cases to set up
    // Or for cases where there are multiple similar versions

    // ParentUp means rotate the parent up into the root of the sub-tree, while
    // NewNodeUp means that returned int will become the sub-tree root.
    
    // Return value to insert
    int SetUp_Insert_ClockwiseShift_ParentUp();
    int SetUp_Insert_ClockwiseShift_NewNodeUp();
    int SetUp_Insert_CounterClockwiseShift_ParentUp();
    int SetUp_Insert_CounterClockwiseShift_NewNodeUp();
    
    // Return value to delete
    int SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack();
    int SetUp_Delete_RightOfParent_SiblingAndChildrenBlack();
    int SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack_ParentIsRed();
    int SetUp_Delete_RightOfParent_SiblingAndChildrenBlack_ParentIsRed();
    
    int SetUp_Delete_LeftOfParent_SiblingLeftIsRed();
    int SetUp_Delete_RightOfParent_SiblingLeftIsRed();
    int SetUp_Delete_LeftOfParent_SiblingRightIsRed();
    int SetUp_Delete_RightOfParent_SiblingRightIsRed();
    
    int SetUp_Delete_LeftOfParent_SiblingLeftIsRed_ParentIsRed();
    int SetUp_Delete_RightOfParent_SiblingLeftIsRed_ParentIsRed();
    int SetUp_Delete_LeftOfParent_SiblingRightIsRed_ParentIsRed();
    int SetUp_Delete_RightOfParent_SiblingRightIsRed_ParentIsRed();
    
    
    // Conditions are:
        // 1) Is a valid binary tree, with all nodes in left less than node,
            // and all nodes in right greater than node
        // 2) If a node is red, then both its children are black
        // 3) Every path from node to any descendent nullptr node contains the
            // same number of black nodes
        // 4) If a node A has a child B, then B's parent is A (more of a sanity check)
        // Will throw if a requirement is not met
    void AssertMeetsConditions() const;
    
private:
    void SetUp_Insert_Shift(bool clockwise, int* parentUp, int* childUp);

    void SetUp_Delete_SiblingAndChildrenBlack(int* leftVal, int* rightVal);
    void SetUp_Delete_SiblingAndChildrenBlack_ParentIsRed(int* leftVal, int* rightVal);
    
    int SetUp_Delete_OneSiblingChildRed(bool deletedLeftOfParent, bool redSiblingNodeLeft);
    int SetUp_Delete_OneSiblingChildRed_ParentIsRed(bool deletedLeftOfParent, bool redSiblingNodeLeft);
    
    
    void UpdateRoot(Node* newRoot);
    
    // If doesn't meet requirements of binary tree, then throws an exception
        // Since there shouldn't be any duplicate nodes, is an exclusive range
    void AssertIsBinaryTree(const Node* node, int minimum, int maximum) const;

    // If doesn't meet requirements of binary tree, then throws an exception
        // Since there shouldn't be any duplicate nodes, is an exclusive range
        // 2) If a node is red, then both its children are black
        // 3) Every path from node to any descendent nullptr node contains the same number of black nodes
        // Returns the number of black nodes
    int AssertIsRedBlackTree(Node* node) const;
};
