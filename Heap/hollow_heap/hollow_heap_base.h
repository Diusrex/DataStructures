#ifndef HOLLOW_HEAP
#define HOLLOW_HEAP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <queue>

// TODO: This should be split into 3 subclasses rather than have the if else statements everywhere.

// Multiple root invarients:
//   heap_root_list will include heap_root_min
//   Child list is singly linked list, pointing at left most element (node w/ highest rank).
//   A node u w/ rank r has exactly r children w/ ranks r - 1, ..., 0
//      Can have any number of unranked children, who are stored after ranked.
//   On decrease-key, will move all r - 2 children


// Single root advantages:
//   Single root, which makes some things simpler.
//   Will have more nodes being children of others,
//     which may lead to less merges.

// Single root invarients:
//   heap_root_list will always be heap_root_min
//   A child list is a circular singly linked list, pointing at the right most element
//      (rank 0 node or most recently added unranked child).
//   A node u w/ rank r has exactly r ranked children w/ ranks r - 1, ..., 0
//      Can have any number of unranked children, who are stored after ranked.
//   On decrease-key, will move all unranked children, and r - 2 ranked children.

// A possible further extension (or redesign) described in their paper on pg 9 is to
// allow unranked merges to increase the rank, if the lesser ranked node won the comparison.

// Implements the base functionality used by all of the different heaps.
template <class Key, class Weight>
class hollow_heap_base {
public:
    hollow_heap_base(bool allow_multiple_roots);
    ~hollow_heap_base();

    void insert(const Key& key, const Weight& weight);

    bool inHeap(const Key& key) {
        return key_to_node.find(key) != key_to_node.end();
    }

    std::pair<Key, Weight> find_min() const;

    std::pair<Key, Weight> extract_min();

    // Fails an assert if element not in heap.
    void decrease_key(const Key& key, const Weight& weight);

    size_t size() const { return num_elem; }

    bool empty() const { return size() == 0; }

    // Note that the print_out may fail on sufficiently large heaps due to
    // the recusive calls being too deep (> 36000 deep).
    void print_out(std::ostream& o = std::cout) const;

protected:

    struct Node {
        Node(const Key& key, const Weight& weight)
            : key(key),
            weight(weight),
            isHollow(false),
            parent(nullptr),
            secondParent(nullptr),
            rank(0),
            child_list(nullptr),
            right_sibling(nullptr) {
        }

        const Key key;
        Weight weight;
        bool isHollow;
        Node* parent;
        // Only set in decrease key when doing two parent variant.
        // Will be the last node for that parent, so don't need to store an
        // additional ptr for it.
        Node* secondParent;

        // Invarient:
        //  Will have rank children, with the children being in order r - 1, r - 2, ... 0
        //  Unless is hollow, in which case will have r - 1, r - 2 children.
        int rank;
        // Will point to the highest ranked node in multiple root heap.
        // Will point to rank 0 node if no unranked children, otherwise will not have any children.
        // The first rank elements will be the ranked children, in decreasing order of rank.
        // Then, these will be followed by the unranked children (if there are any).
        Node* child_list;
        Node* right_sibling;
    };

    // Will always be a single linked list.
    Node* heap_root_list;
    // The smallest weighted Node in heap_root_list list.
    Node* heap_root_min;

    bool allow_unranked_links() const { return !allow_multiple_roots; }
    bool uses_circular_childlist() const { return !allow_multiple_roots; }

    const bool allow_multiple_roots;

    Node* get_start_of_childlist(Node* parent) const;

private:
    // May update heap_root_list + heap_root_min as necessary.
    void meld_into_root(Node* heap);

    // Will link the two nodes, using ranked link if possible.
    // Otherwise, will use unranked linke.
    Node* force_link(Node* n1, Node* n2);

    // Links are only used in delete-min operation.
    // Will assert that their rank is equal
    Node* ranked_link(Node* n1, Node* n2);

    // Should write out the invarients for the different versions.
    Node* unranked_link(Node* n1, Node* n2);

    void decrease_key(Node* node, const Weight& new_weight);

    // Will just remove the given element, handling its children as necessary.
    void delete_node(Node* node);

    // Returns the list of nodes to be merged together for the final rootlist.
    Node* remove_all_hollow_roots();

    void print_out_childlist(Node* parent, std::ostream& o,
            const std::string& offset) const;
    void print_out_for_node(Node* node, Node* parent, std::ostream& o,
            const std::string& offset) const;


    std::unordered_map<Key, Node*> key_to_node;

    size_t num_elem;
};

#define hollow_heap_node typename hollow_heap_base<Key, Weight>::Node

template <class Key, class Weight>
hollow_heap_base<Key, Weight>::hollow_heap_base(bool allow_multiple_roots)
    : heap_root_list(nullptr),
    allow_multiple_roots(allow_multiple_roots),
    num_elem(0)
{ }

template <class Key, class Weight>
hollow_heap_base<Key, Weight>::~hollow_heap_base() {
    std::queue<Node*> toBeDeleted;
    // Add all nodes from heap list.
    while (heap_root_list != nullptr) {
        toBeDeleted.push(heap_root_list);
        heap_root_list = heap_root_list->right_sibling;
    }


    while (!toBeDeleted.empty()) {
        Node* remove = toBeDeleted.front();
        toBeDeleted.pop();

        if (remove->child_list != nullptr) {
            Node* child = remove->child_list;
            do {
                // Child if child has only 1 parent remaining
                if (child->parent == nullptr || child->secondParent == nullptr) {
                    // If child has only one parent (this guy), delete child.
                    toBeDeleted.push(child);
                } else {
                    // Otherwise, mark the child as only having one parent remaining.
                    child->parent = nullptr;
                }

                child = child->right_sibling;
            } while (child != nullptr && child != remove->child_list);
        }
        
        delete remove;
    }
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::insert(const Key& key, const Weight& weight) {
    assert(!inHeap(key));

    ++num_elem;

    Node* new_node = new Node(key, weight);
    key_to_node[key] = new_node;

    // Just merge this new node with existing heap.
    meld_into_root(new_node);
}

template <class Key, class Weight>
std::pair<Key, Weight> hollow_heap_base<Key, Weight>::find_min() const {
    assert(heap_root_list != nullptr);

    // Just return minroot of heap
    return std::make_pair(heap_root_min->key, heap_root_min->weight);
}

template <class Key, class Weight>
std::pair<Key, Weight> hollow_heap_base<Key, Weight>::extract_min() {
    const std::pair<Key, Weight> val = find_min();

    delete_node(heap_root_min);

    return val;
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::delete_node(Node* node) {
    --num_elem;

    // Mark the node as no longer containing the original key.
    node->isHollow = true;
    key_to_node.erase(node->key); // Have removed the key from the heap.

    // Only need to remove the node if it is the minimum element, otherwise can be lazy.
    if (node == heap_root_min) {
        // node will be deleted in this function, no action needs to be taken.
        Node* root_candidates_list = remove_all_hollow_roots();

        // TODO: Refactor this as necessary, at LEAST into a separate function.

        // Should be extra space.
        int size_for_ranks = 5;
        if (size() > 0) {
            size_for_ranks += 2 * std::log2(size());
        }
        std::vector<Node*> ranks(size_for_ranks, nullptr);

        Node* current_node = root_candidates_list;

        // The root candidates list will be a singly linked list, so this works.
        while (current_node != nullptr) {
            current_node->parent = nullptr;
            // current_node may change during the following while loop.
            Node* next_node = current_node->right_sibling;

            while (ranks[current_node->rank] != nullptr) {
                int rank = current_node->rank;
                current_node = ranked_link(current_node, ranks[rank]);
                ranks[rank] = nullptr;
            }
            ranks[current_node->rank] = current_node;

            current_node = next_node;
        }

        for (size_t i = 0; i < ranks.size(); ++i) {
            if (ranks[i] != nullptr) {
                meld_into_root(ranks[i]);
            }
        }
    }
}

template <class Key, class Weight>
hollow_heap_node* hollow_heap_base<Key, Weight>::remove_all_hollow_roots() {
    std::queue<Node*> toMergeWithHeapRoot;
    for (Node* current = heap_root_list; current != nullptr; current = current->right_sibling) {
        toMergeWithHeapRoot.push(current);
    }

    Node* list = nullptr;
    heap_root_list = nullptr;
    heap_root_min = nullptr;

    while (!toMergeWithHeapRoot.empty()) {
        Node* current = toMergeWithHeapRoot.front();
        toMergeWithHeapRoot.pop();

        if (current->isHollow) {
            if (current->child_list != nullptr) {
                Node* child = current->child_list;
                do {
                    toMergeWithHeapRoot.push(child);

                    child = child->right_sibling;
                } while (child != nullptr && child != current->child_list);
            }

            // This node can now be safely removed.
            delete current;
        } else {
            current->parent = nullptr;
            current->right_sibling = nullptr;

            if (list == nullptr) {
                list = current;
                continue;
            }

            if (list->right_sibling == nullptr) {
                list->right_sibling = current;
                continue;
            }

            current->right_sibling = list;
            list = current;
        }
    }

    return list;
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::decrease_key(const Key& key, const Weight& new_weight) {
    assert(inHeap(key));

    decrease_key(key_to_node[key], new_weight);
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::decrease_key(Node* node, const Weight& new_weight) {
    if (node->parent == nullptr) {
        // Is part of the root list already.
        node->weight = new_weight;

        // So may just need to update heap_root_min
        if (node != heap_root_min && heap_root_min->weight > node->weight) {
            heap_root_min = node;
        }
        return;
    } else if (node->parent->weight <= new_weight) {
        // Is fine, nothing needs to be done.
        node->weight = new_weight;
        return;
    }

    // Create new node, transfering the original nodes key to the new node.
    Node* new_node = new Node(node->key, new_weight);
    key_to_node[new_node->key] = new_node;
    node->isHollow = true;

    new_node->rank = std::max(node->rank - 2, 0);

    // Only transfer children when not allowing multiple parents.
    if (true) {
        if (node->child_list != nullptr) {
            if (allow_unranked_links()) {
                Node* start_of_node_childlist = nullptr;
                Node* end_of_node_childlist = nullptr;

                Node* start_of_new_node_childlist = nullptr;
                Node* end_of_new_node_childlist = nullptr;

                // Transfer entire child list
                if (node->rank == 0) {
                    start_of_new_node_childlist = get_start_of_childlist(node);
                    end_of_new_node_childlist = node->child_list;
                } else {
                    // It keeps the start. Only keeps 1 additional node if its rank is >= 2
                    start_of_node_childlist = get_start_of_childlist(node);
                    end_of_node_childlist = start_of_node_childlist;
                    if (node->rank >= 2) {
                        end_of_node_childlist = end_of_node_childlist->right_sibling;
                    }

                    // Only transfer nodes if it has more than the last two nodes.
                    // And in that case, transfer everything including the end of the original list.
                    if (end_of_node_childlist != node->child_list) {
                        start_of_new_node_childlist = end_of_node_childlist->right_sibling;
                        end_of_new_node_childlist = node->child_list;
                    }
                }

                // Update the lists pts and keep them circular.
                node->child_list = end_of_node_childlist;
                if (node->child_list != nullptr) {
                    node->child_list->right_sibling = start_of_node_childlist;
                }

                new_node->child_list = end_of_new_node_childlist;
                if (new_node->child_list != nullptr) {
                    new_node->child_list->right_sibling = start_of_new_node_childlist;
                }

            } else {
                // Never keep less than one
                Node* last_node_not_transferred = node->child_list;
                if (node->rank >= 2) {
                    last_node_not_transferred = last_node_not_transferred->right_sibling;
                }

                new_node->child_list = last_node_not_transferred->right_sibling;
                last_node_not_transferred->right_sibling = nullptr;
            }

            // Now, update the parent pointer for moved nodes.
            if (new_node->child_list != nullptr) {
                Node* current = new_node->child_list;
                do {
                    current->parent = new_node;

                    current = current->right_sibling;
                    // Until go through all nodes in the list once.
                } while (current != nullptr && current != new_node->child_list);
            }
        }
    }

    meld_into_root(new_node);
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::meld_into_root(Node* heap) {
    heap->parent = nullptr;
    heap->right_sibling = nullptr;

    if (heap_root_list == nullptr) {
        heap_root_list = heap_root_min = heap;
        return;
    }

    if (allow_multiple_roots) {
        // If heap is a multi root, add heap to root list.

        // Put it into heap_root_list, as the second element since that is easiest.
        heap->right_sibling = heap_root_list->right_sibling;
        heap_root_list->right_sibling = heap;

        // Update the min element as necessary.
        if (heap->weight < heap_root_min->weight) {
            heap_root_min = heap;
        }
    } else {
        // Merging two non-empty heaps. Can do a ranked link if they have equal rank.
        // Assume heap_root_list was already just a single element.
        // heap_root_list will always be heap_root_min
        heap_root_list = heap_root_min = force_link(heap_root_list, heap);
    }
}

template <class Key, class Weight>
hollow_heap_node* hollow_heap_base<Key, Weight>::force_link(Node* h1, Node* h2) {
    if (h1->rank == h2->rank)
        return ranked_link(h1, h2);
    else
        return unranked_link(h1, h2);
}


template <class Key, class Weight>
hollow_heap_node* hollow_heap_base<Key, Weight>::ranked_link(Node* h1, Node* h2) {
    assert(h1->rank == h2->rank);

    // Only allow links between roots.
    assert(h1->parent == nullptr);
    assert(h2->parent == nullptr);

    Node* winner;
    Node* loser;
    if (h1->weight < h2->weight) {
        winner = h1;
        loser = h2;
    } else {
        winner = h2;
        loser = h1;
    }
    
    // Add loser to winners child list, in correct position.
    loser->parent = winner;

    if (allow_multiple_roots) {
        loser->right_sibling = winner->child_list;
        winner->child_list = loser;
    } else {
        if (winner->child_list != nullptr) {
            // Multi root child is last on end, and adding ranked to start of list
            loser->right_sibling = winner->child_list->right_sibling;
            winner->child_list->right_sibling = loser;
        } else {
            // Setup circular list.
            winner->child_list = loser;
            loser->right_sibling = loser;
        }
        
        // Will have the child's right sibling be the loser.
        // If winner didn't previously have children, then loser will point to itself.
    }

    winner->rank++;

    return winner;
}

template <class Key, class Weight>
hollow_heap_node* hollow_heap_base<Key, Weight>::unranked_link(Node* h1, Node* h2) {
    assert(h1->rank != h2->rank);
    assert(allow_unranked_links());

    // Only allow links between roots.
    assert(h1->parent == nullptr);
    assert(h2->parent == nullptr);

    Node* winner;
    Node* loser;
    if (h1->weight < h2->weight) {
        winner = h1;
        loser = h2;
    } else {
        winner = h2;
        loser = h1;
    }

    // Add loser to winners child list, in correct position.
    // It will be added to the very end of the list, so it would become
    // the child of the winner.
    loser->parent = winner;

    if (winner->child_list != nullptr) {
        // Add the loser to end of list, so between the previous_end and what was
        // its right sibling.
        Node* previous_end = winner->child_list;
        loser->right_sibling = previous_end->right_sibling;
        previous_end->right_sibling = loser;
        winner->child_list = loser;
    } else {
        // Setup circular list.
        winner->child_list = loser;
        loser->right_sibling = loser;
    }

    return winner;
}


template <class Key, class Weight>
hollow_heap_node* hollow_heap_base<Key, Weight>::get_start_of_childlist(Node* parent) const {
    if (parent->child_list == nullptr)
        return nullptr;

    if (uses_circular_childlist()) {
        return parent->child_list->right_sibling;
    } else {
        return parent->child_list;
    }
}


template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::print_out(std::ostream& o) const {
    for (Node* current = heap_root_list; current != nullptr;
            current = current->right_sibling) {
        print_out_for_node(current, nullptr, o, "");
    }
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::print_out_childlist(Node* parent, std::ostream& o,
        const std::string& offset) const {
    Node* start = get_start_of_childlist(parent);
    if (start == nullptr) {
        return;
    }

    Node* current = start;
    do {
        print_out_for_node(current, parent, o, offset);

        current = current->right_sibling;
    } while (current != nullptr && current != start);
}

template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::print_out_for_node(Node* node, Node* parent, std::ostream& o,
        const std::string& offset) const {
    if (node->isHollow) {
        o << offset << "Hollow ";
    } else {
        o << offset << "Key " << node->key << " ";
    }

    o << "weight " << node->weight << " rank " << node->rank;

    // It is its other parent, so don't print out anything else for node.
    if (node->parent != parent) {
        o << ", but is other child so will print more at different spot!\n";
        return;
    }

    o << ", with children:\n";
    print_out_childlist(node, o, offset + "    ");
}


#endif  // HOLLOW_HEAP
