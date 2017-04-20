#include "pairing_heap.h"

#include <limits>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <queue>
#include <vector>


using std::make_pair;
using std::to_string;

const int NumRandomInserted =       500000;

const int NumRandomChanged =        200000;

const int NumElementsInserted =    1000000;
const int EveryIndexChanged =            3;
const int EveryIndexRemovedAfterChange = 4;


class pairing_heap_test : public pairing_heap<std::string, int> {
public:
    int size_of_root_childlist() const {
        if (heap_root == nullptr || heap_root->left_child == nullptr) {
            return 0;
        }

        int count = 1;
        Node* current = heap_root->left_child->right_sibling;
        while (current != heap_root->left_child) {
            ++count;
            current = current->right_sibling;
        }

        return count;
    }

    void assert_is_valid() const {
        size_t total_num_nodes = assert_heap_is_valid();

        if (size() != total_num_nodes)
            throw "The size wasn't updated properly: is " +
                to_string(total_num_nodes) + " while reports " + to_string(size());
    }

private:
    // Will return number of nodes.
    size_t assert_heap_is_valid() const {
        if (heap_root == nullptr) {
            return 0;
        }

        size_t total = 0;

        // Will iterate through all nodes using a queue, since recursive will take too much memory.
        std::queue<std::pair<Node*, int>> nodes_and_parent_val;
        nodes_and_parent_val.push(make_pair(heap_root, std::numeric_limits<int>::min()));

        while (!nodes_and_parent_val.empty()) {
            Node* node = nodes_and_parent_val.front().first;
            int lowest_allowed = nodes_and_parent_val.front().second;
            nodes_and_parent_val.pop();

            if (node->right_sibling == nullptr)
                throw "The key " + node->key + " with weight " + to_string(node->weight) +
                    " doesn't have any ptr to right sibling";

            // Ensure all nodes in sibling list are valid.
            // Already checked the base_of_sibling_tree against parent.
            total += assert_heap_siblings_are_valid(node, lowest_allowed, node->parent);

            // Add all of the siblings children.
            add_heap_siblings_children_list(node, &nodes_and_parent_val);
        }

        return total;
    }

    // Will return the number of nodes in sibling list
    // Will check that all of the siblings are valid, but will not compare against their children list.
    size_t assert_heap_siblings_are_valid(Node* base_of_sibling_list, int lowest_allowed,
            Node* parent) const {
        Node* current = base_of_sibling_list;
        size_t total = 0;

        do {
            ++total;

            if (current->left_sibling == nullptr)
                throw "The key " + current->key + " with weight " + to_string(current->weight) +
                    " doesn't have any ptr to left sibling";
            if (current->right_sibling == nullptr)
                throw "The key " + current->key + " with weight " + to_string(current->weight) +
                    " doesn't have any ptr to right sibling";

            if (current->weight < lowest_allowed) {
                throw "The weight " + to_string(current->weight) + " for key " + current->key +
                    " is below that of a parent (" + to_string(lowest_allowed) + ")";
            }

            if (current->parent != parent)
                throw "The key " + current->key + " with weight " + to_string(current->weight) +
                    " doesn't point to correct current: " + node_to_string(current->parent) + " vs " +
                    node_to_string(parent);

            // Do need to check both left and right siblings, since it is possible that just one
            // of them is incorrect.
            Node* left_sibling = current->left_sibling;
            if (left_sibling->right_sibling != current)
                throw "The key " + left_sibling->key + " with weight " + to_string(current->weight) +
                    " doesn't have a correct ptr to its right sibling with key " +
                    current->key + " with weight " + to_string(current->weight);

            Node* right_sibling = current->right_sibling;
            if (right_sibling->left_sibling != current)
                throw "The key " + right_sibling->key + " with weight " + to_string(current->weight) +
                    " doesn't have a correct ptr to its left sibling with key " +
                    current->key + " with weight " + to_string(current->weight);

            current = current->right_sibling;

        } while (current != base_of_sibling_list);

        return total;
    }

    // Will just add one element from the children list into nodes_and_parent_val.
    void add_heap_siblings_children_list(Node* base_of_sibling_list,
            std::queue<std::pair<Node*, int>>* nodes_and_parent_val) const {
        Node* current = base_of_sibling_list;

        do {
            if (current->left_child != nullptr) {
                Node* child = current->left_child;
                if (child->parent != current)
                    throw "The key " + child->key + " with weight " + to_string(child->weight) +
                        " doesn't point to correct current: " + node_to_string(child->parent) + " vs " +
                        node_to_string(current);

                nodes_and_parent_val->push(make_pair(child, current->weight));
            }

            current = current->right_sibling;

        } while (current != base_of_sibling_list);
    }


    std::string node_to_string(const Node* node) const {
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

void CheckMinValue(const pairing_heap_test& heap, int expected_weight) {
    std::pair<std::string, int> actual = heap.find_min();

    if (expected_weight != actual.second)
        throw "Min weight is wrong: expected " + to_string(expected_weight) +
            " got " + to_string(actual.second);
}



void CheckMinValue(const pairing_heap_test& heap, const std::string& expected_key, int expected_weight) {
    std::pair<std::string, int> actual = heap.find_min();

    if (expected_key != actual.first)
        throw "Min key is wrong: expected " + expected_key + " got " + actual.first;

    if (expected_weight != actual.second)
        throw "Min weight is wrong: expected " + to_string(expected_weight) +
            " got " + to_string(actual.second);
}

void CheckHeapSize(const pairing_heap_test& heap, size_t expected_size) {
    if (heap.size() != expected_size)
        throw "Heap size is wrong: expected " + to_string(expected_size) + " got " + to_string(heap.size());
}

void InsertionUnchangedRootTest() {
    pairing_heap_test heap;

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);

    try {
        CheckMinValue(heap, "A", 5);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionUnchangedRootTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void InsertionChangesRootTest() {
    pairing_heap_test heap;

    heap.insert("A", 5);
    // Becomes root
    heap.insert("B", 4);
    // Becomes root
    heap.insert("C", 3);

    try {
        CheckMinValue(heap, "C", 3);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionUnchangedRootTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void ChangeWeightOrderingStaysTest() {
    pairing_heap_test heap;

    heap.insert("A", 5);
    heap.insert("B", 12);
    heap.insert("C", 11);

    heap.decrease_key("B", 7);
    heap.decrease_key("C", 5);

    try {
        CheckMinValue(heap, "A", 5);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightOrderingStaysTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void ChangeWeightOrderingChangedNoSiblingsTest() {
    pairing_heap_test heap;

    heap.insert("A", 5);
    heap.insert("B", 12);

    heap.decrease_key("B", 4);

    try {
        CheckMinValue(heap, "B", 4);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightOrderingStaysTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}


void ChangeWeightOrderingChangedHasSiblingsTest() {
    pairing_heap_test heap;

    heap.insert("A", 5);
    heap.insert("B", 12);
    heap.insert("C", 6);

    heap.decrease_key("B", 4);

    try {
        CheckMinValue(heap, "B", 4);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightOrderingStaysTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void ChangeWeightOfRootTest() {
    pairing_heap_test heap;

    heap.insert("A", 3);

    heap.decrease_key("A", 1);

    try {
        CheckMinValue(heap, "A", 1);
        CheckHeapSize(heap, 1);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightOfRootTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void RemoveRootNoChildrenTest() {
    pairing_heap_test heap;
    heap.insert("A", 5);

    heap.extract_min();

    try {
        CheckHeapSize(heap, 0);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveRootNoChildrenTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void RemoveRootOneChildTest() {
    pairing_heap_test heap;
    heap.insert("A", 5);
    heap.insert("B", 6);

    heap.extract_min();

    try {
        CheckMinValue(heap, "B", 6);
        CheckHeapSize(heap, 1);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveRootOneChild: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void RemoveRootTwoChildrenTest() {
    pairing_heap_test heap;
    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);

    heap.extract_min();

    try {
        CheckMinValue(heap, "B", 6);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveRootTwoChildrenTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void RemoveRootThreeChildrenTest() {
    pairing_heap_test heap;
    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 8);
    heap.insert("D", 7);

    heap.extract_min();

    try {
        CheckMinValue(heap, "B", 6);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveRootThreeChildrenTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

// This one is probably a little excessive, but could be nice to have
void RemoveRootFourChildrenTest() {
    pairing_heap_test heap;
    heap.insert("A", 5);
    heap.insert("B", 7);
    heap.insert("C", 7);
    heap.insert("D", 8);
    heap.insert("E", 6);

    heap.extract_min();

    try {
        CheckMinValue(heap, "E", 6);
        CheckHeapSize(heap, 4);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveRootFourChildrenTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

std::vector<std::string> GetAllNames(int numElements) {
    std::vector<std::string> names(numElements);
    for (int i = 0; i < numElements; ++i) {
        std::string& name = names[i];
        name = "";
        int temp = i + 1;
        while (temp) {
            name += 'A' + (temp % 26) - 1;
            temp /= 26;
        }
    }

    return names;
}

void ChangeCount(std::map<int, int>& weight_to_counts, int weight, int change) {
    weight_to_counts[weight] += change;
    if (weight_to_counts[weight] == 0) {
        weight_to_counts.erase(weight);
    }
}

void LargeRandomTest() {
    std::cout << "Starting large random test.\n\n";
    pairing_heap_test heap;

    srand(0);

    std::vector<std::string> names = GetAllNames(NumRandomInserted);
    std::vector<int> weights(NumRandomInserted);
    std::map<int, int> weight_to_counts;
    std::unordered_map<std::string, int> name_to_index;


    for (int i = 0; i < NumRandomInserted; ++i) {
        weights[i] = rand();

        heap.insert(names[i], weights[i]);

        weight_to_counts[weights[i]]++;
        name_to_index[names[i]] = i;
    }

    try {
        CheckMinValue(heap, weight_to_counts.begin()->first);
        CheckHeapSize(heap, NumRandomInserted);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in LargeRandomTest insertion: " << s << '\n';
        heap.print_out();
        throw s;
    }

    int total = NumRandomInserted;

    std::vector<bool> removed(NumRandomInserted, false);
    try {
        // Now, randomly remove and reduce weight of elements.
        // Mostly just reduce the weight.
        for (int i = 0; i < NumRandomChanged; ++i) {
            int change = rand() % 4;
            if (change == 0) {
                // Removing min element
                // First check it is the correct minimum.
                CheckMinValue(heap, weight_to_counts.begin()->first);

                std::pair<std::string, int> top = heap.extract_min();

                int index = name_to_index[top.first];

                if (removed[index]) {
                    throw "The key " + top.first + " weight " + to_string(top.second) +
                        " should have already been removed...";
                }

                removed[index] = true;

                ChangeCount(weight_to_counts, top.second, -1);
                --total;
            } else {
                int weight_change = rand() % 1000;
                int index = rand() % NumRandomInserted;

                while (removed[index]) {
                    index = rand() % NumRandomInserted;
                }

                ChangeCount(weight_to_counts, weights[index], -1);
                
                weights[index] -= weight_change;
                ChangeCount(weight_to_counts, weights[index], 1);

                heap.decrease_key(names[index], weights[index]);
            }
        }


        CheckMinValue(heap, weight_to_counts.begin()->first);
        CheckHeapSize(heap, total);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in LargeRandomTest change: " << s << '\n';
        heap.print_out();
        throw s;
    }

    try {
        // Now, clear out the heap.
        while (total != 0) {
            // First check it is the correct minimum.
            CheckMinValue(heap, weight_to_counts.begin()->first);

            std::pair<std::string, int> top = heap.extract_min();
            --total;

            int index = name_to_index[top.first];

            if (removed[index]) {
                throw "The key " + top.first + " weight " + to_string(top.second) +
                    " should have already been removed...";
            }

            removed[index] = true;

            ChangeCount(weight_to_counts, top.second, -1);
        }

        CheckHeapSize(heap, 0);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in LargeRandomTest clear: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void LargeTest() {
    std::cout << "Starting large test.\n\n";
    pairing_heap_test heap;

    std::vector<std::string> names = GetAllNames(NumElementsInserted);
    for (int i = 0; i < NumElementsInserted; ++i) {
        heap.insert(names[i], i);
    }

    try {
        CheckHeapSize(heap, NumElementsInserted);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in LargeTest insert: " << s << '\n';
        heap.print_out();
        throw s;
    }

    try {
        int total = NumElementsInserted;
        for (int i = 1, count = 0; i < NumElementsInserted; i += EveryIndexChanged, ++count) {
            heap.decrease_key(names[i], -i);
            if (count % EveryIndexRemovedAfterChange == 0) {
                std::pair<std::string, int> top = heap.extract_min();
                if (top.first != names[i]) {
                    throw "Unexpected key removed - expected " + names[i] + " got " + top.first;
                }
                --total;
            }
        }
        
        CheckHeapSize(heap, total);
        heap.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in LargerTest decrease: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

int main() {
    InsertionUnchangedRootTest();
    InsertionChangesRootTest();


    ChangeWeightOrderingStaysTest();
    ChangeWeightOrderingChangedNoSiblingsTest();
    ChangeWeightOfRootTest();

    RemoveRootNoChildrenTest();
    RemoveRootOneChildTest();
    RemoveRootTwoChildrenTest();
    RemoveRootThreeChildrenTest();
    RemoveRootFourChildrenTest();

    LargeRandomTest();
    LargeTest();
}
