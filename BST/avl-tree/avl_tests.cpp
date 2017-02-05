#include "avl_tree.h"

#include <limits>
#include <iostream>
#include <set>

using namespace std;

const int MostInserted =      1000000;

const int LargestRandomNum = 10000000;
const int NumRandomInserted =  500000;

// In large test, will delete multiples of this immediately
const int EveryDeletedImmediately = 5;

// In large test, will delete multiples of this after everything else was inserted
const int EveryDeletedAfter = 3;


class avl_test_tree : public avl_tree<int> {
public:
    int root_val() const {
        return root->value;
    }

    void assert_is_valid_avl_tree() const {
        if (root == nullptr)
            return;

        if (root->parent != nullptr)
            throw "The root thinks it has a parent";

        assert_is_valid_avl_tree(root, numeric_limits<int>::min(),
                numeric_limits<int>::max());

        if (size() != count_size(root))
            throw "The size wasn't updated properly: is " +
                to_string(count_size(root)) + " while reports " + to_string(size());
    }

    // Will return height of subtree.
    int assert_is_valid_avl_tree(Node* node, int min_val, int max_val) const {
        if (node == nullptr)
            return -1; // nullptrs are -1, so leaves are 0.

        if (node->value <= min_val || node->value >= max_val)
            throw "The value " + to_string(node->value) +
                " is outside the bounds (" + to_string(min_val) + ", " +
                to_string(max_val) + ")";

        if (node->lhs != nullptr && node->lhs->parent != node)
            throw "The node " + to_string(node->lhs->value) + 
                "does not have the right parent";

        if (node->rhs != nullptr && node->rhs->parent != node)
            throw "The node " + to_string(node->rhs->value) + 
                "does not have the right parent";

        int num_on_left = assert_is_valid_avl_tree(node->lhs, min_val, node->value);
        int num_on_right = assert_is_valid_avl_tree(node->rhs, node->value, max_val);

        if (abs(num_on_left - num_on_right) > 1)
            throw "The node " + to_string(node->value) +
                " has a large gap in number of nodes on either side (" +
                to_string(num_on_left) + " vs " + to_string(num_on_right) + ".";

        int expected_height = 1 + max(num_on_left, num_on_right);
        if (expected_height != node->height)
            throw "The node " + to_string(node->value) + " has height " +
                to_string(node->height) + " while should have " +
                to_string(expected_height) + ".";

        return expected_height;
    }

    int count_size(const Node* node) const
    {
        if (node == nullptr)
            return 0;

        return 1 + count_size(node->lhs) + count_size(node->rhs);
    }
};

bool CheckRoot(const avl_test_tree& tree, int expected, const std::string& test_id) {
    if (tree.root_val() != expected) {
        std::cout << "ERROR in " << test_id << ": Root is " << tree.root_val()
            << " expected " << expected << '\n';
        return false;
    }
    return true;
}

bool CheckMinimum(const avl_test_tree& tree, int expected, const std::string& test_id) {
    if (tree.minimum() != expected) {
        std::cout << "ERROR in " << test_id << ": Minimum is " << tree.minimum()
            << " expected " << expected << '\n';
        return false;
    }
    return true;
}

bool CheckIsValid(const avl_test_tree& tree, const std::string& test_id) {
     try {
        tree.assert_is_valid_avl_tree();
    } catch (string s) {
        std::cout << "ERROR in " << test_id << ": " << s << '\n';
        return false;
    }
     return true;
}

bool InsertRightRotate() {
    avl_test_tree tree;

    tree.insert(3);
    tree.insert(1);
    tree.insert(0);

    bool valid = CheckRoot(tree, 1, "InsertRightRotate");
    valid &= CheckMinimum(tree, 0, "InsertRightRotate");
    valid &= CheckIsValid(tree, "InsertRightRotate");
    return valid;
}

bool InsertLeftRightRotate() {
    avl_test_tree tree;

    tree.insert(3);
    tree.insert(1);
    tree.insert(2);

    bool valid = CheckRoot(tree, 2, "InsertLeftRightRotate");
    valid &= CheckMinimum(tree, 1, "InsertLeftRightRotate");
    valid &= CheckIsValid(tree, "InsertLeftRightRotate");
    return valid;
}

bool InsertLeftRotate() {
    avl_test_tree tree;

    tree.insert(3);
    tree.insert(5);
    tree.insert(7);

    bool valid = CheckRoot(tree, 5, "InsertLeftRotate");
    valid &= CheckMinimum(tree, 3, "InsertLeftRotate");
    valid &= CheckIsValid(tree, "InsertLeftRotate");
    return valid;
}

bool InsertRightLeftRotate() {
    avl_test_tree tree;

    tree.insert(3);
    tree.insert(5);
    tree.insert(4);

    bool valid = CheckRoot(tree, 4, "InsertRightLeftRotate");
    valid &= CheckMinimum(tree, 3, "InsertRightLeftRotate");
    valid &= CheckIsValid(tree, "InsertRightLeftRotate");
    return valid;
}

bool DeleteRightRotate() {
    avl_test_tree tree;

    // 6 will be root, with 3 (and its child 1) on left, 8 on right side.
    tree.insert(6);
    tree.insert(3);
    tree.insert(8);
    tree.insert(1);
    
    tree.remove(8);

    if (!tree.size() == 3)
        throw "Failure in DeleteRightRotate: size is wrong. Misconfigured?";

    bool valid = CheckRoot(tree, 3, "DeleteRightRotate");
    valid &= CheckMinimum(tree, 1, "DeleteRightRotate");
    valid &= CheckIsValid(tree, "DeleteRightRotate");
    return valid;
}

bool DeleteLeftRightRotate() {
    avl_test_tree tree;

    // 6 will be root, with 3 (and its child 4) on left, 8 on right side.
    tree.insert(6);
    tree.insert(3);
    tree.insert(8);
    tree.insert(4);
    
    tree.remove(8);

    bool valid = CheckRoot(tree, 4, "DeleteLeftRightRotate");
    valid &= CheckMinimum(tree, 3, "DeleteLeftRightRotate");
    valid &= CheckIsValid(tree, "DeleteLeftRightRotate");
    return valid;
}

bool DeleteLeftRotate() {
    avl_test_tree tree;

    // 6 will be root, with 8 (and its child 9) on right, 3 on right side.
    tree.insert(6);
    tree.insert(3);
    tree.insert(8);
    tree.insert(9);
    
    tree.remove(3);

    if (!tree.size() == 3)
        throw "Failure in DeleteRightRotate: size is wrong. Misconfigured?";

    bool valid = CheckRoot(tree, 8, "DeleteLeftRotate");
    valid &= CheckMinimum(tree, 6, "DeleteLeftRotate");
    valid &= CheckIsValid(tree, "DeleteLeftRotate");
    return valid;
}

bool DeleteRightLeftRotate() {
    avl_test_tree tree;

    // 6 will be root, with 8 (and its child 7) on right, 3 on right side.
    tree.insert(6);
    tree.insert(3);
    tree.insert(8);
    tree.insert(7);
    
    tree.remove(3);

    if (!tree.size() == 3)
        throw "Failure in DeleteRightRotate: size is wrong. Misconfigured?";

    bool valid = CheckRoot(tree, 7, "DeleteRightLeftRotate");
    valid &= CheckMinimum(tree, 6, "DeleteRightLeftRotate");
    valid &= CheckIsValid(tree, "DeleteRihgtLeftRotate");
    return valid;
}

bool Contains(const std::set<int>& s, int num) {
    return s.find(num) != s.end();
}

void LargeRandomInsertTest() {
    std::cout << "Starting large random insert. "
        << "If this takes longer than ~20 seconds, there is a balancing issue\n";
    avl_test_tree tree;

    srand(0);

    std::set<int> s;
    for (int i = 0; i < NumRandomInserted; ++i) {
        int num = rand() % LargestRandomNum;
        tree.insert(num);
        s.insert(num);
    }

    for (int i = 0; i < LargestRandomNum; ++i)
        if (tree.find(i) != Contains(s, i))
            std::cout << "\nERROR in LargeRandomInsertTest: item " << i <<
                " reported by set as " << Contains(s, i) << " avl reports " <<
                tree.find(i) << '\n';

    CheckIsValid(tree, "LargeRandomInsertTest");

    std::cout << "Completed large random insert\n\n";
}


void LargeInsertTest() {
    std::cout << "Starting large insert."
        << "If this takes longer than ~5 seconds, there is a balancing issue\n";
    avl_test_tree tree;

    for (int i = 0; i < MostInserted; ++i)
        tree.insert(i);

    for (int i = 0; i < MostInserted; ++i)
        if (!tree.find(i))
            std::cout << "\nERROR in LargeInsertTest: item " << i << " missing";

    CheckIsValid(tree, "LargeInsertTest");

    std::cout << "Completed large insert\n\n";
}

void RunLargeCompleteDeleteTest() {
    std::cout << "Starting large complete delete\n";    
    
    avl_test_tree tree;
    for (int i = 0; i < MostInserted; ++i) {
        tree.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            tree.remove(i);
        }
    }

    // No point in continuing if not valid.
    if (!CheckIsValid(tree, "LargeCompleteDeleteTest"))
        return;

    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately != 0) {
            if (!tree.find(i)) {
                std::cout << "ERROR in LargeCompleteDeleteTest: value " << i <<
                    " not found in tree\n";
            }

            tree.remove(i);
        }
    }

    for (int i = 0; i < MostInserted; ++i) {
        if (tree.find(i)) {
            std::cout << "ERROR in LargeCompleteDeleteTest: value " << i <<
                " is still in tree\n";
        }
    }

    CheckIsValid(tree, "LargeCompleteDeleteTest");

    std::cout << "Finished large complete delete\n\n";
}

void RunLargeDeleteTest() {
    std::cout << "Starting large delete test\n";

    avl_test_tree tree;

    for (int i = 0; i < MostInserted; ++i) {
        tree.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            tree.remove(i);
        }
    }

    // No point in continuing if invalid already.
    if (!CheckIsValid(tree, "LargeDeleteTest"))
        return;

    for (int i = 0; i < MostInserted; i += EveryDeletedAfter) {
        tree.remove(i);
    }

    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately == 0 || i % EveryDeletedAfter == 0) {
            // Should be deleted.
            if (tree.find(i))
                std::cout << "ERROR in LargeDeleteTest: Contains " << i << '\n';
        } else if (!tree.find(i)) {
            std::cout << "ERROR in LargeDeleteTest: Doesn't contain " << i << '\n';
        }
    }
    std::cout << "Finished large delete test\n";
}


int main() {
    bool insert_fine = InsertRightRotate();
    insert_fine &= InsertLeftRightRotate();
    insert_fine &= InsertLeftRotate();
    insert_fine &= InsertRightLeftRotate();

    bool delete_fine = DeleteRightRotate();
    delete_fine &= DeleteLeftRightRotate();
    delete_fine &= DeleteLeftRotate();
    delete_fine &= DeleteRightLeftRotate();

    std::cout << "Completed small tests\n\n";
    if (insert_fine) {
        LargeInsertTest();
        LargeRandomInsertTest();
    }

    if (insert_fine && delete_fine) {
        RunLargeCompleteDeleteTest();
        RunLargeDeleteTest();
    }
}
