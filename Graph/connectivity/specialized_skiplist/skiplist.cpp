#include "skiplist.h"

#include <assert.h>

namespace internal {

Node* get_first_node_in_tour(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    // Remember that we want to stay/be as high as possible before going to the left.
    // Go as far up and to the left as possible, to skip over as many nodes as possible.
    while (node->left != nullptr || node->up != nullptr) {
        // Go up as far as possible on this node.
        while (node->up != nullptr) {
            node = node->up;
        }

        if (node->left != nullptr) {
            node = node->left;
        }
    }

    // Now get to the very start of the list, at level 0.
    while (node->left != nullptr || node->down != nullptr) {
        while (node->left != nullptr) {
            node = node->left;
        }

        // Go down a level, since will need to be able to reach height 0.
        if (node->down != nullptr) {
            node = node->down;
        }
    }

    return node;
}

}

using internal::get_first_node_in_tour;

// Utility functions for skiplist.
// Returns nullptr if no node with a higher level considering this node
// and all nodes to the right.
Node* find_next_higher_level_to_right(Node* node, Node* farthest_right=nullptr) {
    while (node != nullptr && node->up == nullptr && node != farthest_right) {
        node = node->right;
    }

    if (node == nullptr) {
        return nullptr;
    }

    return node->up;
}

// Returns nullptr if no node with a higher level considering this node
// and all nodes to the left.
Node* find_next_higher_level_to_left(Node* node, Node* farthest_left=nullptr) {
    while (node != nullptr && node->up == nullptr && node != farthest_left) {
        node = node->left;
    }

    if (node == nullptr) {
        return nullptr;
    }

    return node->up;
}

// Will go up in heights, then go down until reached the end.
// Should be ~O(lg n) - log(n) heights), and each height will let us skip enough nodes for
// it to also be lg n.
Node* get_last_node_in_list(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    // Remember that we want to stay/be as high as possible before going to the right.
    // Go as far up and to the right as possible, to skip over as many nodes as possible.
    while (node->right != nullptr || node->up != nullptr) {
        // Go up as far as possible on this node.
        while (node->up != nullptr) {
            node = node->up;
        }

        if (node->right != nullptr) {
            node = node->right;
        }
    }

    // Now get to the very start of the list, at level 0.
    while (node->right != nullptr || node->down != nullptr) {
        while (node->right != nullptr) {
            node = node->right;
        }

        // Go down a level, since will need to be able to reach height 0.
        if (node->down != nullptr) {
            node = node->down;
        }
    }

    return node;
}





EulerTourSkiplist::~EulerTourSkiplist() {
   for (Node* node : all_allocated_stacks) {
       // Delete upwards
       for (Node* up_next = node->up; node != nullptr;
               node = up_next) {
           up_next = node->up;
           delete node;
       }
   } 
}

// Returns list with size = elements.size.
// Each provided element will be transformed into a stack of nodes with random height
std::vector<Node*> EulerTourSkiplist::euler_tour_to_skiplist(std::vector<int> elements) {
    // Everything starts with height >= 1
    std::vector<int> random_heights(elements.size(), 1);
    for (size_t i = 0; i < elements.size(); ++i) {
        // 50% chance to continue increasing the height.
        while (rand() % 2 == 1) {
            ++random_heights[i];
        }
    }

    return euler_tour_to_skiplist(elements, random_heights);
}

// Returns a list of nodes with size = elements.size.
// Each provided element will be transformed into a stack of nodes with height equal
// to the provided height.
// Heights MUST BE >= 1
std::vector<Node*> EulerTourSkiplist::euler_tour_to_skiplist(std::vector<int> elements,
        std::vector<int> heights) {
    // Keep track of tallest node*
    std::vector<Node*> all_nodes;
    all_nodes.reserve(elements.size());
    std::vector<Node*> most_recent_node_with_height;

    for (size_t i = 0; i < elements.size(); ++i) {
        const int elem = elements[i];
        const size_t height = static_cast<size_t>(heights[i]);
        Node* prev;

        // Setup base node.
        {
        Node* node = new Node{elem};
        all_nodes.push_back(node);

        if (!most_recent_node_with_height.empty()) {
            link_horizontal(most_recent_node_with_height[0], node);

            most_recent_node_with_height[0] = node;
        } else {
            most_recent_node_with_height.push_back(node);
        }

        prev = node;
        }

        assert(height >= 1);


        // Add for random height.
        for (size_t h = 1; h < height; ++h) {
            Node* next = new Node{elem};
            next->down = prev;
            prev->up = next;

            // Already included a node stack with this height - link to it.
            if (most_recent_node_with_height.size() > h) {
                link_horizontal(most_recent_node_with_height[h], next);

                most_recent_node_with_height[h] = next;
            } else {
                most_recent_node_with_height.push_back(next);
            }

            prev = next;
        }
    }

    return all_nodes;
}

bool EulerTourSkiplist::nodes_are_connected(Node* n1, Node* n2) const {
    return get_first_node_in_tour(n1) == get_first_node_in_tour(n2);
}

// If new_height is -1, will be randomly set. Otherwise will use the provided height, which must be >= 1
Node* EulerTourSkiplist::insert_tour_after_node(Node* inserted,
        Node* second, int new_after_node_height) {
    assert(inserted != nullptr);
    // Need to update the first and last pointer for each level that the inserted
    // list contains.

    // Setup the first level.
    Node* before_inserted = second;
    Node* after_inserted = second->right;

    if (new_after_node_height == -1) {
        new_after_node_height = 1;
        while (rand() % 2 == 1) {
            ++new_after_node_height;
        }
    }
    assert(new_after_node_height >= 1);

    Node* new_second = new Node{second->element};
    Node* const base_new_second = new_second;
    all_allocated_stacks.insert(new_second);

    {
    Node* temp = new_second;
    for (int h = 1; h < new_after_node_height; ++h) {
        Node* next = new Node{second->element};

        temp->up = next;
        next->down = temp;

        temp = next;
    }

    }

    Node* inserted_start = get_first_node_in_tour(inserted);
    Node* inserted_end = get_last_node_in_list(inserted);


    // Setup the remaining levels, until have gone through all levels for
    // inserted_start/end or until have gone through all levels for before/after_inserted.

    while (inserted_start != nullptr && // If inserted_{start/end} is nullptr, other is also nullptr.
            (before_inserted != nullptr || after_inserted != nullptr) // Their values are not related.
            ) {
        // Connect everything up at this level.
        // before_inserted -> inserted_start ... inserted_end -> additional second (if it is high enough)
        // -> after_inserted

        link_horizontal(before_inserted, inserted_start);
        if (new_second != nullptr) {
            link_horizontal(inserted_end, new_second);
            link_horizontal(new_second, after_inserted);
        }  else {
            link_horizontal(inserted_end, after_inserted);
        }


        // Now advance levels for everything.

        if (new_second != nullptr) {
            new_second = new_second->up;
        }

        Node* temp_inserted_start = inserted_start;
        // Don't let it go past the end of inserted.
        inserted_start = find_next_higher_level_to_right(inserted_start, inserted_end);
        // Don't let it go past the end of inserted
        inserted_end = find_next_higher_level_to_left(inserted_end, temp_inserted_start);

        // Before/after inserted are related, because they would have been directly connected
        // to each other.
        // If there is a node in before_inserted that is of sufficient height,
        // after_inserted will be its right sibling. Otherwise need to find after_inserted
        // using the previous value of after_inserted.
        before_inserted = find_next_higher_level_to_left(before_inserted);
        if (before_inserted != nullptr) {
            after_inserted = before_inserted->right;
        } else {
            // If it doesn't exist, find using what we just put after the inserted
            // block on previous level.
            after_inserted = find_next_higher_level_to_right(after_inserted);
        }

        assert((inserted_start == nullptr) == (inserted_end == nullptr));
    }

    // May just need to do additional work for the additional second node.
    // Note that at this point, everything is on a level that hasn't yet been inserted.

    // Know that before + after inserted are nullptrs.
    while (new_second != nullptr && inserted_end != nullptr) {
        // Link. Note that before_inserted and after_inesrted are nullptr.
        // So just link inserted_end to 
        link_horizontal(inserted_end, new_second);

        // Advance up one level.
        new_second = new_second->up;

        Node* temp_inserted_start = inserted_start;
        // Don't let it go past the end of inserted.
        inserted_start = find_next_higher_level_to_right(inserted_start, inserted_end);
        // Don't let it go past the end of inserted
        inserted_end = find_next_higher_level_to_left(inserted_end, temp_inserted_start);
    }

    // Know that inserted_end and inserted_start are nullptr.
    // So need to link before+after_inserted to new_second
    while (new_second != nullptr && 
            (before_inserted != nullptr || after_inserted != nullptr)) {
        link_horizontal(before_inserted, new_second);
        link_horizontal(new_second, after_inserted);


        // Advance up one level.
        new_second = new_second->up;

        // Before/after inserted are related, because they would have been directly connected
        // to each other.
        // If there is a node in before_inserted that is of sufficient height,
        // after_inserted will be its right sibling. Otherwise need to find after_inserted
        // using the previous value of after_inserted.
        before_inserted = find_next_higher_level_to_left(before_inserted);
        if (before_inserted != nullptr) {
            after_inserted = before_inserted->right;
        } else {
            // If it doesn't exist, find using what we just put after the inserted
            // block on previous level.
            after_inserted = find_next_higher_level_to_right(after_inserted);
        }
    }

    return base_new_second;
}

Node* EulerTourSkiplist::cut_out_tour(Node* tour_start, Node* tour_end) {
    Node* before_tour = tour_end->right;
    Node* after_tour = tour_end->right;

    assert(before_tour->element == after_tour->element);

    // Remove the cut start + end from the list they are already in, but going up levels,
    // and connected left + right together.
    while (tour_start != nullptr) {
        // Link what is to left of tour_start with what is to right of tour_end.
        link_horizontal(tour_start->left, tour_end->right);

        tour_start->left = nullptr;
        tour_end->right = nullptr;

        Node* temp_tour_start = tour_start;
        // Don't let it go past the end of what is to be cut.
        tour_start = find_next_higher_level_to_right(tour_start, tour_end);
        // Don't let it go past the start of what is to be cut.
        tour_end = find_next_higher_level_to_left(tour_end, temp_tour_start);
    }

    // Remove the duplicated node.
    all_allocated_stacks.erase(after_tour);
    for (Node* next; after_tour != nullptr; after_tour = next) {
        next = after_tour->up;

        // Reconnect what it was connecting
        link_horizontal(after_tour->left, after_tour->right);
    }

    return before_tour;
}



// Able to handle either node, or even both, being a nullptr.
inline void EulerTourSkiplist::link_horizontal(Node* left_node, Node* right_node) const {
    if (left_node != nullptr) {
        left_node->right = right_node;
    }

    if (right_node != nullptr) {
        right_node->left = left_node;
    }
}



