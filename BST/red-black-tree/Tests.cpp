#include "RedBlackTreeTestingSubclass.h"

#include <iostream>
#include <set>

using namespace std;

// These tests could have been setup better by using a testing framework instead
// of running like a regular C++ program.
// I would like to switch them, but don't have enough time...

// There are a lot of different cases, especially for deletion....

void TestInsertNode_ParentAndUncleRed();
void TestInsertNode_ClockwiseShift();
void TestInsertNode_CounterClockwiseShift();

void TestDeleteElementNotIn();
void TestDeleteNode_OnlyRoot();
void TestDeleteNode_IsRed();
void TestDeleteNode_HasRedChild();
void TestDeleteNode_SiblingIsRed();

void TestDeleteNode_SiblingAndChildrenBlack();
void TestDeleteNode_SiblingAndChildrenBlack_ParentIsRed();

void TestDeleteNode_DeletedLeft_SiblingChildRed();
void TestDeleteNode_DeletedRight_SiblingChildRed();
void TestDeleteNode_DeletedLeft_SiblingChildRed_ParentIsRed();
void TestDeleteNode_DeletedRight_SiblingChildRed_ParentIsRed();



void RunRandomInsertTest();
void RunLargeInsertTest();
void RunLargeCompleteDeleteTest();
void RunLargeDeleteTest();

const int MostInserted = 1000000;

// In large test, will delete multiples of this immediately
const int EveryDeletedImmediately = 5;

// In large test, will delete multiples of this after everything else was inserted
const int EveryDeletedAfter = 3;

int main() {
    // All the basic insertion cases
    TestInsertNode_ParentAndUncleRed();
    TestInsertNode_ClockwiseShift();
    TestInsertNode_CounterClockwiseShift();
    
    // All the basic deletion cases
    TestDeleteElementNotIn();
    TestDeleteNode_OnlyRoot();
    TestDeleteNode_IsRed();
    TestDeleteNode_HasRedChild();
    
    // Handle all the different cases for this
    TestDeleteNode_DeletedLeft_SiblingChildRed();
    TestDeleteNode_DeletedRight_SiblingChildRed();
    TestDeleteNode_DeletedLeft_SiblingChildRed_ParentIsRed();
    TestDeleteNode_DeletedRight_SiblingChildRed_ParentIsRed();
    
    
    //TestDeleteNode_SiblingIsRed();
    
    
    TestDeleteNode_SiblingAndChildrenBlack();
    TestDeleteNode_SiblingAndChildrenBlack_ParentIsRed();
    
    srand(0);
    RunRandomInsertTest();
    srand(55);
    RunRandomInsertTest();
    srand(100);
    RunRandomInsertTest();
    
    RunLargeInsertTest();
    RunLargeCompleteDeleteTest();
    RunLargeDeleteTest();
}

// Testing utilities
void InsertThenDelete(RedBlackTreeTestingSubclass &tree, int num, const string &testName);
void EnsureInsert(RedBlackTreeTestingSubclass &tree, int num, const string &testName);
void EnsureDelete(RedBlackTreeTestingSubclass &tree, int num, const string &testName);

void EnsureValid(const RedBlackTreeTestingSubclass & tree, const string &testName);

void PrintOutError(const string & errorMessage, const string &testName);



void TestInsertNode_ParentAndUncleRed() {
    string testname = "TestInsertNode_ParentAndUncleRed";
    RedBlackTreeTestingSubclass tree;
    // So base is 5
    tree.Insert(5);
    
    // Parent is 3, uncle is 6
    // Will not have done any special operations yet
    tree.Insert(3);
    tree.Insert(6);
    
    // Main operation. Add a child to 3, which should invert the color of 3 and 6
    tree.Insert(2);
    
    EnsureValid(tree, testname);
}

void TestInsertNode_ClockwiseShift() {
    string testname = "TestInsertNode_ClockwiseShift";
    RedBlackTreeTestingSubclass tree;
    
    // Try when parent shifted up
    int toInsert = tree.SetUp_Insert_ClockwiseShift_ParentUp();
    tree.Insert(toInsert);
    EnsureValid(tree, testname);
    
    // Try when new node shifted up
    toInsert = tree.SetUp_Insert_ClockwiseShift_NewNodeUp();
    
    tree.Insert(toInsert);
    EnsureValid(tree, testname);
}

void TestInsertNode_CounterClockwiseShift() {
    string testname = "TestInsertNode_CounterClockwiseShift";
    RedBlackTreeTestingSubclass tree;
    
    // Try when parent shifted up
    int toInsert = tree.SetUp_Insert_CounterClockwiseShift_ParentUp();
    tree.Insert(toInsert);
    EnsureValid(tree, testname);
    
    // Try when new node shifted up
    toInsert = tree.SetUp_Insert_CounterClockwiseShift_NewNodeUp();
    
    tree.Insert(toInsert);
    EnsureValid(tree, testname);
}


void TestDeleteElementNotIn() {
    string testname = "TestDeleteElementNotIn";
    RedBlackTreeTestingSubclass tree;
    tree.Insert(1);
    
    tree.Delete(2);
    
    EnsureValid(tree, testname);
}

void TestDeleteNode_OnlyRoot() {
    string testname = "TestDeleteNode_OnlyRoot";
    RedBlackTreeTestingSubclass tree;
    InsertThenDelete(tree, 1, testname);
    
    // Try giving it a new root, then delete that
    InsertThenDelete(tree, 2, testname);
    
    EnsureValid(tree, testname);
}

void TestDeleteNode_IsRed() {
    string testname = "TestDeleteNode_IsRed";
    RedBlackTreeTestingSubclass tree;
    tree.Insert(1);
    
    InsertThenDelete(tree, 2, testname);
    InsertThenDelete(tree, 0, testname);
    
    EnsureValid(tree, testname);
}

void TestDeleteNode_HasRedChild() {
    string testname = "TestDeleteNode_HasRedChild";
    RedBlackTreeTestingSubclass tree;
    tree.Insert(1);
    tree.Insert(2);
    
    tree.Delete(1);
    
    EnsureValid(tree, testname);
}

void TestDeleteNode_SiblingAndChildrenBlack() {
    string testname = "TestDeleteNode_SiblingAndChildrenBlack";
    RedBlackTreeTestingSubclass tree;
    
    int toDelete = tree.SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try when sibling right is red
    toDelete = tree.SetUp_Delete_RightOfParent_SiblingAndChildrenBlack();
    toDelete = tree.SetUp_Delete_RightOfParent_SiblingAndChildrenBlack();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}

void TestDeleteNode_DeletedLeft_SiblingChildRed() {
    string testname = "TestDeleteNode_DeletedLeft_SiblingChildRed";
    RedBlackTreeTestingSubclass tree;
    
    // Try when sibling left is red
    int toDelete = tree.SetUp_Delete_LeftOfParent_SiblingLeftIsRed();
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try when sibling right is red
    toDelete = tree.SetUp_Delete_LeftOfParent_SiblingRightIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}

void TestDeleteNode_DeletedRight_SiblingChildRed() {
    string testname = "TestDeleteNode_DeletedRight_SiblingChildRed";
    RedBlackTreeTestingSubclass tree;
    
    // Try when sibling left is red
    int toDelete = tree.SetUp_Delete_RightOfParent_SiblingLeftIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try when sibling right is red
    toDelete = tree.SetUp_Delete_RightOfParent_SiblingRightIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}

void TestDeleteNode_DeletedLeft_SiblingChildRed_ParentIsRed() {
    string testname = "TestDeleteNode_DeletedLeft_SiblingChildRed_ParentIsRed";
    RedBlackTreeTestingSubclass tree;
    
    // Try when sibling left is red
    int toDelete = tree.SetUp_Delete_LeftOfParent_SiblingLeftIsRed_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try when sibling right is red
    toDelete = tree.SetUp_Delete_LeftOfParent_SiblingRightIsRed_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}

void TestDeleteNode_DeletedRight_SiblingChildRed_ParentIsRed() {
    string testname = "TestDeleteNode_DeletedRight_SiblingChildRed_ParentIsRed";
    RedBlackTreeTestingSubclass tree;
    
    // Try when sibling left is red
    int toDelete = tree.SetUp_Delete_RightOfParent_SiblingLeftIsRed_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try when sibling right is red
    toDelete = tree.SetUp_Delete_RightOfParent_SiblingRightIsRed_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}

void TestDeleteNode_SiblingAndChildrenBlack_ParentIsRed() {
    string testname = "TestDeleteNode_SiblingAndChildrenBlack_ParentIsRed";
    RedBlackTreeTestingSubclass tree;
    
    // Try deleting left version
    int toDelete = tree.SetUp_Delete_LeftOfParent_SiblingAndChildrenBlack_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
    
    // Try delete right version
    toDelete = tree.SetUp_Delete_RightOfParent_SiblingAndChildrenBlack_ParentIsRed();
    
    tree.Delete(toDelete);
    EnsureValid(tree, testname);
}


void RunRandomInsertTest() {
    string testname = "RunRandomInsertTest";
    RedBlackTreeTestingSubclass tree;
    set<int> includedElements;
    
    // At much more than this, and it will start failing to add new numbers
    // So will take forever
    while (includedElements.size() < 20000) {
        int num = rand();
        
        tree.Insert(num);
        includedElements.insert(num);
    }
    
    for (int num : includedElements) {
        if (!tree.Contains(num))
            PrintOutError("Doesn't contain " + to_string(num), testname);
    }
    
    EnsureValid(tree, testname);
}

void RunLargeInsertTest() {
    string testname = "RunLargeInsertTest";
    cout << "Starting large insert\n";
    RedBlackTreeTestingSubclass tree;
    for (int i = 0; i < MostInserted; ++i) {
        EnsureInsert(tree, i, testname);
    }
    
    for (int i = 0; i < MostInserted; ++i) {
        if (!tree.Contains(i))
            PrintOutError("Doesn't contain " + to_string(i), testname);
    }
        
    
    EnsureValid(tree, testname);
    cout << "Finished large insert\n";
}

void RunLargeCompleteDeleteTest() {
    string testname = "RunLargeCompleteDeleteTest";
    
    cout << "Starting large complete delete\n";
    RedBlackTreeTestingSubclass tree;
    for (int i = 0; i < MostInserted; ++i) {
        EnsureInsert(tree, i, testname);
        if (i % EveryDeletedImmediately == 0) {
            EnsureDelete(tree, i, testname);
        }
    }
    
    EnsureValid(tree, testname);
    
    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately != 0)
            EnsureDelete(tree, i, testname);
    }
    
    for (int i = 0; i < MostInserted; ++i) {
        if (tree.Contains(i))
            PrintOutError("Contains " + to_string(i), testname);
    }
        
    
    EnsureValid(tree, testname);
    cout << "Finished large complete delete\n";
}

void RunLargeDeleteTest() {
    string testname = "RunLargeDeleteTest";
    
    cout << "Starting large delete\n";
    RedBlackTreeTestingSubclass tree;
    for (int i = 0; i < MostInserted; ++i) {
        EnsureInsert(tree, i, testname);
        if (i % EveryDeletedImmediately == 0) {
            EnsureDelete(tree, i, testname);
        }
    }
    
    EnsureValid(tree, testname);
    
    for (int i = 0; i < MostInserted; i += EveryDeletedAfter) {
        if (i % EveryDeletedImmediately != 0)
            EnsureDelete(tree, i, testname);
    }
    
    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately == 0 || i % EveryDeletedAfter == 0) {
            // Should be deleted
            if (tree.Contains(i))
                PrintOutError("Contains " + to_string(i), testname);
        } else if (!tree.Contains(i)) {
            PrintOutError("Doesn't contain " + to_string(i), testname);
        }
    }
        
    
    EnsureValid(tree, testname);
    cout << "Finished large delete\n";
}

void InsertThenDelete(RedBlackTreeTestingSubclass &tree, int num, const string &testName) {
    tree.Insert(num);
    tree.Delete(num);
    
    if (tree.Contains(num))
        PrintOutError("Deleting " + to_string(num) + " did not remove it from tree", testName);
}

void EnsureInsert(RedBlackTreeTestingSubclass &tree, int num, const string &testName) {
    if (!tree.Insert(num))
        PrintOutError("Value " + to_string(num) + " was not inserted", testName);
}

void EnsureDelete(RedBlackTreeTestingSubclass &tree, int num, const string &testName) {
    if (!tree.Delete(num))
        PrintOutError("Value " + to_string(num) + " was not inserted", testName);
}


void EnsureValid(const RedBlackTreeTestingSubclass & tree, const string &testName) {
    try {
        tree.AssertMeetsConditions();
    } catch (string error) {
        PrintOutError(error, testName);
    }
}

void PrintOutError(const string & errorMessage, const string &testName) {
    cout << "\n\nERROR in " << testName << ": " << errorMessage << "\n\n\n";
}
