#include "skiplist.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

using std::to_string;
using internal::get_first_node_in_tour;

void assert_expected_number_elements(Node* node, size_t expected) {
    size_t actual = 0;
    node = get_first_node_in_tour(node);

    for (; node != nullptr; node = node->right) {
        ++actual;
    }

    if (actual != expected)
        throw "The number of elements didn't match up: expected " + to_string(expected) +
            " got " + to_string(actual);
}

void assert_all_connected_to_are_valid(Node* node) {
    if (node == nullptr) {
        return;
    }

    // Go to the left-most side of of the list.
    node = get_first_node_in_tour(node);

    // Go through from left to right, keeping track of the expected points at each height.
    // Which 
    // Expect to see this node for next ->left on same height.
    std::vector<Node*> node_to_left_at_height;
    // Expect to see this node for next stack reaching the given height.
    //    From ->right
    std::vector<Node*> expected_node_to_right_at_height;

    int index = 0;
    for (Node* next = node->right; node != nullptr;
            node = next, next = (next ? next->right : nullptr), ++index) {

        Node* prev_vertical = nullptr;
        for (size_t h = 0; node != nullptr;
               ++h, prev_vertical = node, node = node->up) {
            // Check the node itself
            if (node->down != prev_vertical) {
                if (prev_vertical == nullptr)
                    throw "Node " + to_string(node->element) + " doesn't point down to nullptr, instead at " + to_string(node->down->element);
                else if (node->down == nullptr )
                    throw "Node " + to_string(node->element) + " points to nullptr when should be " + to_string(prev_vertical->element) + '\n';

                else
                    throw "Node " + to_string(node->element) + " points to " + to_string(node->down->element) + " when should be " + to_string(prev_vertical->element);
            }

            if (prev_vertical != nullptr && node->element != prev_vertical->element)
                throw "Node doesn't contain the same element as the node below it";

            if (h < node_to_left_at_height.size()) {
                if (node->left != node_to_left_at_height[h])
                    throw "Node did not point to correct left node at this height.";
            } else {
                if (node->left != nullptr)
                    throw "Node elem " + to_string(node->element) + " index " + to_string(index) +
                        " expected something to left, but there wasn't anything there";

                node_to_left_at_height.push_back(node);
            }
            node_to_left_at_height[h] = node;

            if (h < expected_node_to_right_at_height.size()) {
                if (node != expected_node_to_right_at_height[h])
                    throw "Node did not point to correct right node at this height.";
            } else {
                expected_node_to_right_at_height.push_back(nullptr);
            }
            expected_node_to_right_at_height[h] = node->right;
        }
    }

    // Ensure that the expected to right is all nullptr, since they are the end at their height.
    for (size_t h = 0; h < expected_node_to_right_at_height.size(); ++h) {
        if (expected_node_to_right_at_height[h] != nullptr)
            throw "At height " + to_string(h) + " had reached end of list but still expected someone after";
    }
}

void assert_correct_node(Node* base, int elem, int height) {
    if (base->element != elem)
        throw "The node with element " + to_string(base->element) + " was expected to have elem " +
            to_string(elem);

    int orig = height;
    while (base->up != nullptr) {
        --height;
        base = base->up;
    }
    // Include the count for current node.
    --height;

    if (height != 0)
        throw "Height for element " + to_string(base->element) + " was incorrect."
            " Expected " + to_string(orig) + " got " + to_string(orig - height);
}

// Generation is pretty simple.
void GenerationTest() {
    std::vector<int> elements{1, 2, 1, 3, 4, 1, 3};
    std::vector<int> heights {1, 3, 1, 2, 1, 2, 4};
    EulerTourSkiplist tour;
    std::vector<Node*> nodes = 
        tour.euler_tour_to_skiplist(elements, heights);

    try {
        // The actually selected node doesn't matter!
        Node* arbitrary_node = nodes[0];
        assert_expected_number_elements(arbitrary_node, elements.size());

        for (size_t elem = 0; elem < heights.size(); ++elem) {
            assert_correct_node(nodes[elem], elements[elem], heights[elem]);
        }

        assert_all_connected_to_are_valid(arbitrary_node);
    } catch (std::string& s) {
        std::cout << "Error in GenerationTest: " << s << '\n';
        throw s;
    } catch (const char* s) {
        std::cout << "Error in GenerationTest: " << s << '\n';
        throw s;
    }
}

// selected_to_insert can be arbitrary, so long as it is a valid element index.
void RunInsertTestForInsertionPositions(
        const std::vector<int>& elements, const std::vector<int>& heights,
        const std::vector<int>& inserted_elements, const std::vector<int>& inserted_heights,
        const int height_for_new_node, const std::string& test_name,
        size_t insert_after, size_t selected_to_insert=0) {

    EulerTourSkiplist tour;
    std::vector<Node*> nodes =
        tour.euler_tour_to_skiplist(elements, heights);

    std::vector<Node*> inserted_nodes =
        tour.euler_tour_to_skiplist(inserted_elements, inserted_heights);

    // Everything from second nodelist.
    tour.insert_tour_after_node(inserted_nodes[selected_to_insert],
            nodes[insert_after], height_for_new_node);

    try {
        // Check the nodes are in correct order.
        Node* current = nodes[0];
        // First check the nodes before where it was inserted.
        for (size_t i = 0; i <= insert_after; ++i, current = current->right) {
            assert_correct_node(current, elements[i], heights[i]);
        }
        // Check the inserted nodes
        for (size_t i = 0; i < inserted_elements.size(); ++i, current = current->right) {
            assert_correct_node(current, inserted_elements[i], inserted_heights[i]);
        }
        // Check the newly added node which contains the inserted elements
        assert_correct_node(current, elements[insert_after], height_for_new_node);
        current = current->right;
        // Now check the nodes that were after where it was inserted
        for (size_t i = insert_after + 1; i < elements.size(); ++i, current = current->right) {
            assert_correct_node(current, elements[i], heights[i]);
        }

        // The actually selected node doesn't matter!
        Node* arbitrary_node = nodes[0];
        // Size of two sets, plus the additionally added node.
        assert_expected_number_elements(arbitrary_node,
                elements.size() + inserted_elements.size() + 1);

        assert_all_connected_to_are_valid(arbitrary_node);
    } catch (std::string& s) {
        std::cout << "Error in " << test_name << ": " << s << '\n';
        throw s;
    } catch (const char* s) {
        std::cout << "Error in " << test_name << ": " << s << '\n';
        throw s;
    }
}

void InsertionTestSimpleNewNode() {
    // Make sure that, no matter the node we are merging from, will completely merge.
    const std::vector<int> elements{1, 2, 3, 4};
    const std::vector<int> heights {1, 3, 2, 1};

    const std::vector<int> inserted_elements{5, 6, 7};
    const std::vector<int> inserted_heights {1, 2, 1};

    const int height_for_new_node = 1;

    for (size_t insert_after = 0; insert_after < elements.size(); ++insert_after) {
        for (size_t selected_to_insert = 0; selected_to_insert < inserted_elements.size();
                ++selected_to_insert) {

            RunInsertTestForInsertionPositions(elements, heights,
                    inserted_elements, inserted_heights,
                    height_for_new_node, "InsertionTestSimpleNewNode",
                    insert_after, selected_to_insert);
        }
    }
}


void InsertionTestAllSameMaxHeight() {
    // Ensure that insertion is correct when the max height for the inserted into, inserted, and
    // new node are all equal.

    // Make sure that, no matter the node we are merging from, will completely merge.
    const std::vector<int> elements{1, 2, 3, 4};
    const std::vector<int> heights {1, 3, 2, 1};

    const std::vector<int> inserted_elements{5, 6, 7};
    const std::vector<int> inserted_heights {3, 2, 3};

    const int height_for_new_node = 3;

    for (size_t insert_after = 0; insert_after < elements.size(); ++insert_after) {

        RunInsertTestForInsertionPositions(elements, heights,
                inserted_elements, inserted_heights,
                height_for_new_node, "InsertionTestAllSameMaxHeight",
                insert_after);
    }
}

void InsertionOriginalAndNewNodeTaller() {
    const std::vector<int> elements{1, 2, 3, 4};
    const std::vector<int> heights {2, 5, 1, 3};

    const std::vector<int> inserted_elements{4, 5, 6};
    const std::vector<int> inserted_heights {1, 2, 3};

    const int height_for_new_node = 4;

    for (size_t insert_after = 0; insert_after < elements.size(); ++insert_after) {

        RunInsertTestForInsertionPositions(elements, heights,
                inserted_elements, inserted_heights,
                height_for_new_node, "InsertionTestOriginalAndNewNodeTaller",
                insert_after);
    }
}

void InsertionInsertedAndNewNodeTaller() {
    const std::vector<int> elements{1, 2, 3, 4};
    const std::vector<int> heights {2, 3, 1, 3};

    const std::vector<int> inserted_elements{4, 5, 6};
    const std::vector<int> inserted_heights {1, 5, 3};

    const int height_for_new_node = 6;

    for (size_t insert_after = 0; insert_after < elements.size(); ++insert_after) {

        RunInsertTestForInsertionPositions(elements, heights,
                inserted_elements, inserted_heights,
                height_for_new_node, "InsertionInsertedAndNewNodeTaller",
                insert_after);
    }
}

// Assumes that element is in elements.
// Will check that both cut lists are valid, then delete them.
void RunRemoveElementTest(int element, const std::vector<int>& elements,
        const std::vector<int>& heights, const std::string& test_name) {
    EulerTourSkiplist tour;

    std::vector<Node*> nodes =
        tour.euler_tour_to_skiplist(elements, heights);

    int first_occurance = std::find(elements.begin(), elements.end(), element)
        - elements.begin();
    // Equivalent to: Size - 1 - distance_from_end
    int last_occurance = elements.rend() - std::find(elements.rbegin(), elements.rend(), element) - 1;

    tour.cut_out_tour(nodes[first_occurance], nodes[last_occurance]);

    Node* arbitrary_node_uncut = nodes[0];
    if (first_occurance == 0) {
        // Note that the last element
        size_t past_index = last_occurance + 2;
        arbitrary_node_uncut = nodes.size() > past_index ? nodes[past_index] : nullptr;
    }

    Node* arbitrary_node_cut = nodes[first_occurance];

    try {
        // Check both lists - first that they are both in correct order.
        // Cut first.
        Node* current = get_first_node_in_tour(arbitrary_node_cut);
        for (int i = first_occurance; i <= last_occurance; ++i, current = current->right) {
            assert_correct_node(current, elements[i], heights[i]);
        }

        // Now check uncut.
        current = get_first_node_in_tour(arbitrary_node_uncut);
        // Part before it was cut.
        for (int i = 0; i < first_occurance; ++i, current = current->right) {
            assert_correct_node(current, elements[i], heights[i]);
        }
        // Part after it was cut (which should be connected.
        for (size_t i = last_occurance + 2; i < elements.size(); ++i, current = current->right) {
            assert_correct_node(current, elements[i], heights[i]);
        }

        // Check that their size is correct.
        size_t expected_size_for_cut = last_occurance - first_occurance + 1;
        size_t expected_size_for_uncut = elements.size() - expected_size_for_cut;
        if (expected_size_for_uncut > 1)
            --expected_size_for_uncut;
        assert_expected_number_elements(arbitrary_node_cut,
                expected_size_for_cut);
        assert_expected_number_elements(arbitrary_node_uncut,
                expected_size_for_uncut);

        // Now check they are both valid.
        assert_all_connected_to_are_valid(arbitrary_node_cut);
        assert_all_connected_to_are_valid(arbitrary_node_uncut);

    } catch (std::string& s) {
        std::cout << "Error in " << test_name << ": " << s << '\n';
        throw s;
    } catch (const char* s) {
        std::cout << "Error in " << test_name << ": " << s << '\n';
        throw s;
    }
}

void DeletionTests() {
    const std::vector<int> example_tour{1, 2, 3, 5, 6, 5, 3, 2, 4, 2, 1};
    const std::vector<int> heights     {1, 3, 1, 2, 3, 1, 2, 2, 3, 4, 1};


    // Test removing the 1s - has everything inside of it.
    RunRemoveElementTest(1, example_tour, heights, "DeletionTests");

    // Test removing the 2s - has a few inside of it.
    RunRemoveElementTest(2, example_tour, heights, "DeletionTests");

    // Test removing the 3s - has a few inside of it.
    RunRemoveElementTest(3, example_tour, heights, "DeletionTests");

    // Test removing the 5s - has just the 6 inside of it
    RunRemoveElementTest(5, example_tour, heights, "DeletionTests");

    // Test removing the 6s - is just one element.
    RunRemoveElementTest(6, example_tour, heights, "DeletionTests");
}

int main() {
    GenerationTest();

    InsertionTestSimpleNewNode();
    InsertionTestAllSameMaxHeight();
    InsertionOriginalAndNewNodeTaller();
    InsertionInsertedAndNewNodeTaller();

    DeletionTests();
}
