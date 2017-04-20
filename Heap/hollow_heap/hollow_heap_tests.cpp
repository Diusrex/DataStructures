#include "hollow_heap_base.h"

#include <exception>

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <set>
#include <sstream>
#include <vector>


using std::to_string;

const int NumRandomInserted =       500000;

const int NumRandomChanged =        200000;

const int NumElementsInserted =    1000000;
const int EveryIndexChanged =            3;
const int EveryIndexRemovedAfterChange = 4;

class hollow_heap_test : public hollow_heap_base<std::string, int> {
public:
    hollow_heap_test(bool allow_multiple_roots)
        : hollow_heap_base(allow_multiple_roots)
    {}

    void assert_is_valid(bool most_recent_was_delete) const {
        size_t total_num_nodes = 0;
        
        if (heap_root_list != nullptr) {
            total_num_nodes = assert_heap_is_valid(most_recent_was_delete);
        }

        if (size() != total_num_nodes)
            throw "The size wasn't updated properly: is " +
                to_string(total_num_nodes) + " while reports " + to_string(size());
    }

    bool allows_multiple_roots() const { return allow_multiple_roots; }

    void assert_root_has_child_key(const std::string& key, int index) const {
        const int wanted_index = index;
        Node* c_node = heap_root_list->child_list;
        if (uses_circular_childlist()) {
            // Get to start of list.
            c_node = c_node->right_sibling;
        }

        while (index > 0 && c_node != nullptr) {
            --index;
            std::cout << c_node->key;
            c_node = c_node->right_sibling;
        }

        if (c_node == nullptr)
            throw "Root didn't have enough children to contain key " + key;

        if (key != c_node->key)
            throw "Root didn't have child node " + key + " at index " + to_string(wanted_index) + "." +
                " Instead had key " + c_node->key;
    }

private:
    // Will return number of nodes.
    size_t assert_heap_is_valid(bool most_recent_was_delete) const {
        if (heap_root_list == nullptr)
            return 0;

	size_t total = 0;

        // Will iterate through all nodes using a queue, since recursive will take too much memory.
        std::queue<Node*> parent_of_subtree;

        if (allow_multiple_roots) {
            check_multiroot_heap_list(most_recent_was_delete, &parent_of_subtree);

        } else {
            if (heap_root_list->right_sibling != nullptr)
                throw "The heap root list does not have a single root. Keys " +
                    heap_root_list->key + " " + heap_root_list->right_sibling->key;

            if (heap_root_list != heap_root_min)
                throw "The heap root min doens't point at the only element in root list.";

            parent_of_subtree.push(heap_root_list);
        }

        while (!parent_of_subtree.empty()) {

            Node* parent = parent_of_subtree.front();
            parent_of_subtree.pop();

            // Hollow notes don't count for element count.
            if (!parent->isHollow) {
                ++total;
            }

            // Ensure all nodes in sibling list are valid.
            assert_heap_children_are_valid(parent);

            // Add all of the siblings children.
            add_heap_children_list(parent, &parent_of_subtree);

            // TODO: Check to see, if allowing two parents, that it is being done correctly.
            // Otherwise, ensure it doesn't use the secondParent var.
        }

        return total;
    }

    void check_multiroot_heap_list(bool most_recent_was_delete, std::queue<Node*>* parent_of_subtree) const {
        // If most_recent_was_delete is true, then the rank of the nodes in root list must be unique.
        // Otherwise their ranks can be anything.
        std::set<int> rankEncountered;
        bool found_heap_root_min = false;

        for (Node* current = heap_root_list; current != nullptr;
                current = current->right_sibling) {
            if (most_recent_was_delete && rankEncountered.find(current->weight) != rankEncountered.end())
                throw "The rank " + to_string(current->weight) + " had already been encountered " +
                    "in rootlist and a delete should have linked them.";

            rankEncountered.insert(current->weight);
            parent_of_subtree->push(current);

            if (current->weight < heap_root_min->weight)
                throw "Element with weight " + to_string(current->weight) + " which is lower " +
                    "than heap_root_min's weight: " + to_string(heap_root_min->weight);

            found_heap_root_min |= current == heap_root_min;
        }

        if (!found_heap_root_min)
            throw "Didn't find the heap root min in the heap root list...";
    }

    // Will check that all of the siblings are valid, but will not compare against their children list.
    void assert_heap_children_are_valid(Node* parent) const {
        int rank = parent->rank - 1;

        int final_wanted_rank = 0;
        if (parent->isHollow) {
            final_wanted_rank = std::max(0, parent->rank - 2);
        }

        if (parent->child_list == nullptr) {
            if (rank != -1) {
                throw "Node " + parent->key + " should have had children due to non-zero rank " +
                    to_string(parent->rank);
            }
            // Will just run into issues if continue past here.
            return;
        }

        Node* current = parent->child_list;
        if (uses_circular_childlist()) {
            current = current->right_sibling;
        }
        Node* previous_child = nullptr;
        for (; rank >= final_wanted_rank;
                previous_child = current, current = current->right_sibling, --rank) {

            if (current == nullptr)
                throw "For parent " + parent->key + " didn't have enough children. Made it to rank " +
                    to_string(rank);

            if (current->parent != parent)
                throw "The key " + current->key + " with weight " + to_string(current->weight) +
                    " doesn't point to correct parent: " + node_to_string(current->parent) + " vs " +
                    node_to_string(parent);

            if (current->weight < parent->weight) {
                throw "The weight " + to_string(current->weight) + " for key " + current->key +
                    " is below that of a parent (" + to_string(parent->weight) + ")";
            }

            // Finally, check the rank.
            if (current->rank != rank)
                throw "The key " + current->key + " with weight " + to_string(current->weight) +
                    " doesn't have correct rank " + to_string(rank) + ". Instead it has rank " +
                    to_string(current->rank) + ". Parents rank is " + to_string(parent->rank);
        }

        if (!allow_unranked_links()) {
            if (current != nullptr)
                throw "Had additional nodes in child list under " + parent->key;
        } else {
            // TODO: This is also fine if allow multiple parents.
            if (!parent->isHollow) {
                // Can have unranked children, they should also follow heap rule and have correct
                // parent.
                
                // Until previous_child isn't the end of parents childlist.
                for (; previous_child != parent->child_list;
                        previous_child = current, current = current->right_sibling) {
                    if (current->weight < parent->weight) {
                        throw "The weight " + to_string(current->weight) + " for key " + current->key +
                            " is below that of a parent (" + to_string(parent->weight) + ")";
                    }

                    if (current->parent != parent)
                        throw "The key " + current->key + " with weight " + to_string(current->weight) +
                            " doesn't point to correct parent: " + node_to_string(current->parent) + " vs " +
                            node_to_string(parent);
                }
            } else {
                // Shouldn't have unranked children, so make sure already hit end of the childlist.
                if (previous_child != parent->child_list)
                    throw "Hollow node has unranked children still.";
            }
        }
    }

    // Will just add one element from the children list into nodes_and_parent_val.
    void add_heap_children_list(Node* parent, std::queue<Node*>* parent_of_subtree) const {
        if (parent->child_list == nullptr) {
            return;
        }

        Node* current = parent->child_list;
        do {
            parent_of_subtree->push(current);

            current = current->right_sibling;
        } while (current != parent->child_list && current != nullptr);
    }

    std::string node_to_string(const Node* node) const {
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// TODO: Remove later.
void my_terminate()
{
    std::cout << "I sure hope this gets caught\n";
    int* temp = nullptr;
    *temp = 5;
}

void CheckMinValue(const hollow_heap_test& heap, int expected_weight) {
    std::pair<std::string, int> actual = heap.find_min();

    if (expected_weight != actual.second)
        throw "Min weight is wrong: expected " + to_string(expected_weight) +
            " got " + to_string(actual.second);
}

void CheckMinValue(const hollow_heap_test& heap, const std::string& expected_key, int expected_weight) {
    std::pair<std::string, int> actual = heap.find_min();

    if (expected_key != actual.first)
        throw "Min key is wrong: expected " + expected_key + " got " + actual.first;

    if (expected_weight != actual.second)
        throw "Min weight is wrong: expected " + to_string(expected_weight) +
            " got " + to_string(actual.second);
}

void CheckHeapSize(const hollow_heap_test& heap, size_t expected_size) {
    if (heap.size() != expected_size)
        throw "Heap size is wrong: expected " + to_string(expected_size) + " got " + to_string(heap.size());
}

void InsertionUnchangedMinTest(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);

    try {
        CheckMinValue(heap, "A", 5);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in InsertionUnchangedMinTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void InsertionChangesMinTest(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    // Becomes root
    heap.insert("B", 4);
    // Becomes root
    heap.insert("C", 3);

    try {
        CheckMinValue(heap, "C", 3);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in InsertionChangesMinTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void NoRankedWithUnrankedChangeMinTest(hollow_heap_test heap) {
    // Specialized test to ensure single root updating is handled correctly.
    if (heap.allows_multiple_roots()) {
        return;
    }

    assert(heap.empty());

    heap.insert("A", 5);
    // A has B assigned as child with ranked link.
    heap.insert("B", 12);

    // C has A added with unranked link.
    heap.insert("C", 3);

    // D now has C as a child.
    heap.insert("D", 2);

    // C can't have D as a parent, so is broken off.
    heap.decrease_key("C", 1);

    try {
        heap.assert_root_has_child_key("A", 0);
        heap.assert_root_has_child_key("D", 1);

        CheckMinValue(heap, "C", 1);
        CheckHeapSize(heap, 4);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in NoRankedWithUnrankedChangeMinTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void OneRankedWithUnrankedChangedMinTest(hollow_heap_test heap) {
    // Specialized test to ensure single root updating is handled correctly.
    if (heap.allows_multiple_roots()) {
        return;
    }

    assert(heap.empty());

    heap.insert("A", 5);
    // A has B assigned as child with ranked link.
    heap.insert("B", 12);

    // A has C assigned as child with unranked link.
    heap.insert("C", 10);

    // D now has A as a child.
    heap.insert("D", 4);

    // A can't have D as a parent, so is broken off.
    heap.decrease_key("A", 2);

    try {
        heap.assert_root_has_child_key("D", 0);
        heap.assert_root_has_child_key("C", 1);

        CheckMinValue(heap, "A", 2);
        CheckHeapSize(heap, 4);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in OneRankedWithUnrankedChangeMinTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void ChangeWeightMinUnchangedTest(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 12);
    heap.insert("C", 11);

    heap.decrease_key("B", 7);
    heap.decrease_key("C", 5);

    try {
        CheckMinValue(heap, "A", 5);
        CheckHeapSize(heap, 3);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightMinUnchangedTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void DeleteMinTest(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);

    heap.extract_min();

    try {
        CheckMinValue(heap, "B", 6);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid(/*most_recent_was_delete=*/true);
    } catch (std::string& s) {
        std::cout << "Error in DeleteMinTest: " << s << '\n';
        heap.print_out();
        throw s;
    }

}

void DeleteMinMultipleLinks(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);
    heap.insert("D", 8);
    heap.insert("E", 9);
    
    // B becomes root and has rank 2.
    heap.extract_min();

    try {
        CheckMinValue(heap, "B", 6);
        CheckHeapSize(heap, 4);
        heap.assert_is_valid(/*most_recent_was_delete=*/true);
    } catch (std::string& s) {
        std::cout << "Error in DeleteMinMultipleLinks: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void ChangeWeightMinChangedTest(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 12);

    heap.decrease_key("B", 4);

    try {
        CheckMinValue(heap, "B", 4);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightMinChangedTest: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void ChangeWeightOfChildMustMove(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 7);

    // B becomes root, with C as child.
    heap.extract_min();

    // C becomes part of min heap.
    heap.decrease_key("C", 5);

    try {
        CheckMinValue(heap, "C", 5);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in ChangeWeightOfChildMustMove: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void MultipleHollowNodesInRootlist(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);
    heap.insert("B", 6);
    heap.insert("C", 10);
    
    // B (rank 1) becomes root, with child C (rank 0)
    heap.extract_min();

    heap.insert("A", 5);
    heap.insert("D", 8);
    heap.insert("E", 9);

    // B (rank 2) becomes root, with children:
    // C (rank 0).
    // D (rank 1) with child E (rank 0).
    heap.extract_min();

    // Now change D's weight to make it be moved away from parent A.
    // Makes one of the children of B be hollow node.
    heap.decrease_key("D", 4);

    // Remove D from rootlist.
    heap.extract_min();

    // Remove B from rootlist. Will cause the hollow node that is B's child
    // to be moved into root list.
    // The hollow nodes child E must be added to root list, and will be the
    // min node.
    heap.extract_min();

    try {
        CheckMinValue(heap, "E", 9);
        CheckHeapSize(heap, 2);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in : MultipleHollowNodesInRootlist" << s << '\n';
        heap.print_out();
        throw s;
    }
}

void RemoveAllNodes(hollow_heap_test heap) {
    assert(heap.empty());

    heap.insert("A", 5);

    heap.extract_min();

    heap.insert("A", 6);

    try {
        CheckMinValue(heap, "A", 6);
        CheckHeapSize(heap, 1);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s){
        std::cout << "Error in RemoveAllNodes: " << s << '\n';
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

void LargeRandomTest(hollow_heap_test heap) {
    assert(heap.empty());
    std::cout << "Starting large random test.\n\n";

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
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
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
        bool last_was_delete = false;
        for (int i = 0; i < NumRandomChanged; ++i) {
            int change = rand() % 4;
            if (change == 0) {
                last_was_delete = true;
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
                last_was_delete = false;
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
        heap.assert_is_valid(last_was_delete);
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
        heap.assert_is_valid(/*most_recent_was_delete=*/true);
    } catch (std::string& s) {
        std::cout << "Error in LargeRandomTest clear: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void LargeTest(hollow_heap_test heap) {
    assert(heap.empty());
    std::cout << "Starting large test.\n\n";

    std::vector<std::string> names = GetAllNames(NumElementsInserted);
    for (int i = 0; i < NumElementsInserted; ++i) {
        heap.insert(names[i], i);
    }

    try {
        CheckHeapSize(heap, NumElementsInserted);
        heap.assert_is_valid(/*most_recent_was_delete=*/false);
    } catch (std::string& s) {
        std::cout << "Error in LargeTest insert: " << s << '\n';
        heap.print_out();
        throw s;
    }

    try {
        int total = NumElementsInserted;
        bool last_was_delete = false;
        for (int i = 1, count = 0; i < NumElementsInserted; i += EveryIndexChanged, ++count) {
            heap.decrease_key(names[i], -i);
            last_was_delete = false;
            if (count % EveryIndexRemovedAfterChange == 0) {
                last_was_delete = true;
                std::pair<std::string, int> top = heap.extract_min();
                if (top.first != names[i]) {
                    throw "Unexpected key removed - expected " + names[i] + " got " + top.first;
                }
                --total;
            }
        }
        
        CheckHeapSize(heap, total);
        heap.assert_is_valid(last_was_delete);
    } catch (std::string& s) {
        std::cout << "Error in LargerTest decrease: " << s << '\n';
        heap.print_out();
        throw s;
    }
}

void run_hollow_heap_tests(const hollow_heap_test& heap) {
    InsertionUnchangedMinTest(heap);
    InsertionChangesMinTest(heap);

    DeleteMinTest(heap);
    DeleteMinMultipleLinks(heap);

    ChangeWeightMinUnchangedTest(heap);
    ChangeWeightMinChangedTest(heap);
    ChangeWeightOfChildMustMove(heap);


    // Specialized tests.
    NoRankedWithUnrankedChangeMinTest(heap);
    OneRankedWithUnrankedChangedMinTest(heap);
    
    MultipleHollowNodesInRootlist(heap);

    LargeRandomTest(heap);
    LargeTest(heap);
}




int main() {
    std::set_terminate([](){my_terminate(); });
    std::cout << "Running heap with multiple roots\n";
    run_hollow_heap_tests(hollow_heap_test{true});
    std::cout << "\nRunning heap with single root\n";
    run_hollow_heap_tests(hollow_heap_test{false});
}

