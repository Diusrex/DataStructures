#ifndef HOLLOW_HEAP
#define HOLLOW_HEAP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <queue>

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


// Two parent advantages:
//    Don't need to change any of the children when a node becomes hollow.
//    Will expand less nodes:
//       Say node X becomes hollow, with Y being the new node holding the key.
//           In one parent, the ranked child of X with rank < r - 2 and all unranked children of X
//           would become visible when Y is eventually removed.
//           But in two parent, all children of X will not be visible until Y and X are removed from the heap.

// Two parent invarients:
//   A child list is a singly linked list
//   If a node is the second parent (for a hollow node), then it will be 
//   A hollow node with a secondary parent will never be in the root list. Only once both of its parents
//     are removed will it be considered to be added to root list.
//   The hollow node with a second parent will ALWAYS be the last node in its second parents child list.
//     It is possible that the hollow node will have its original parent be removed first, in which case
//       the hollow node will consider the second parent to be its only parent.
//   The ranked nodes will be in decreasing rank order, but there may be unranked nodes in between them.
//   Once a node gets a second parent, it will never be changed!

// Note: The actual Hollow Heap paper outlined not storing a parent ptr for a node.
// I setup the nodes to store a ptr to their parent, but this isn't optimal for
// single parent nodes, especially single root.
//    Am planning on maybe making it an option to use either, just to see how well they work.

// Note: The actual Hollow Heap paper included the process of cleaning up all of the hollow nodes.
//    This is currently not done, and should be improved in the future.

// A possible further extension (or redesign) described in their paper on pg 9 is to
// allow unranked merges to increase the rank, if the lesser ranked node won the comparison.

// Implements the base functionality used by all of the different heaps.

namespace hollow_heap {

enum class hollow_heap_type{MULTIPLE_ROOTS, SINGLE_ROOT, TWO_PARENT};

namespace internal {

template <class Key, class Weight>
struct base_node {
    base_node(const Key& key, const Weight& weight)
        : key(key),
        weight(weight),
        isHollow(false),
        parent(nullptr),
        second_parent(nullptr),
        rank(0),
        child_list(nullptr),
        right_sibling(nullptr) {
    }

    const Key key;
    Weight weight;
    bool isHollow;

    // TODO: Make this optional!
    base_node* parent;

    // TODO: Make this optional!
    // Only set in decrease key when doing two parent variant.
    // Will be the last node for that parent, so don't need to store an
    // additional ptr for it.
    base_node* second_parent;



    // Invarient:
    //  Will have rank children, with the children being in order r - 1, r - 2, ... 0
    //  Unless is hollow, in which case will have r - 1, r - 2 children.
    int rank;
    // Will point to the highest ranked node in multiple root heap.
    // Will point to rank 0 node if no unranked children, otherwise will not have any children.
    // The first rank elements will be the ranked children, in decreasing order of rank.
    // Then, these will be followed by the unranked children (if there are any).
    base_node* child_list;
    base_node* right_sibling;
};

// Handles the different algorithms needed for the different setups.
template <class Key, class Weight>
class hollow_heap_impl {
    using Node = base_node<Key, Weight>;
public:
    virtual ~hollow_heap_impl() {}

    virtual Node* get_start_of_childlist(Node* parent) const = 0;

    virtual bool reached_end_of_childlist(Node* child, Node* prev_child,
        Node* parent) const = 0;

    virtual Node* meld_together(Node* added_heap, Node* heap_root_list) const = 0;

    // TODO: Improve name!
    virtual void setup_relationship_between_hollownode_and_copy(
            Node* hollow_node, Node* new_node) const = 0;

    // Will link the two nodes, using ranked link if possible.
    // Otherwise, will use unranked linke.
    Node* ranked_link(Node* h1, Node* h2) const {
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

        loser->parent = winner;

        // Add loser to winners child list, in correct position.
        do_ranked_link(winner, loser);
        winner->rank++;

        return winner;
    }

protected:

    void update_parent_pointers(Node* new_node) const {
        // Now, update the parent pointer for moved nodes.
        for (Node *child = get_start_of_childlist(new_node), *prev_child = nullptr;
                !reached_end_of_childlist(child, prev_child, new_node);
                prev_child = child, child = child->right_sibling) {
            child->parent = new_node;
        }
    }

    virtual void do_ranked_link(Node* winner, Node* loser) const = 0;

    Node* unranked_link(Node* h1, Node* h2) const {
        assert(h1->rank != h2->rank);

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

        loser->parent = winner;

        do_unranked_link(winner, loser);

        return winner;
    }

    virtual void do_unranked_link(Node* winner, Node* loser) const = 0;

};

template <class Key, class Weight>
class hollow_heap_impl_multiroot : public hollow_heap_impl<Key, Weight> {
    using Node = base_node<Key, Weight>;
public:
    Node* get_start_of_childlist(Node* parent) const override {
        return parent->child_list;
    }

    bool reached_end_of_childlist(Node* child, Node* /*prev_child*/, Node* /*parent*/) const override {
        return child == nullptr;
    }

    Node* meld_together(Node* added_heap, Node* heap_root_list) const override {
        // Just set the original root list to be the right sibling of the added heap
        added_heap->right_sibling = heap_root_list;
        return added_heap;
    }

    void do_ranked_link(Node* winner, Node* loser) const override {
        // Have loser be start of childlist.
        loser->right_sibling = winner->child_list;
        winner->child_list = loser;
    }

    void do_unranked_link(Node* winner, Node* loser) const override {
        // Invalid state, shouldn never do an unranked link.
        assert(false);
    }

    void setup_relationship_between_hollownode_and_copy(
            Node* hollow_node, Node* new_node) const override {
        if (hollow_node->child_list == nullptr) {
            // Not going to transfer any children.
            return;
        }

        // Never keep less than one
        Node* last_node_not_transferred = hollow_node->child_list;
        // Keep two.
        if (hollow_node->rank >= 2) {
            last_node_not_transferred = last_node_not_transferred->right_sibling;
        }

        new_node->child_list = last_node_not_transferred->right_sibling;
        last_node_not_transferred->right_sibling = nullptr;

        this->update_parent_pointers(new_node);
    }
};


template <class Key, class Weight>
class hollow_heap_impl_singleroot_single_parent : public hollow_heap_impl<Key, Weight> {
    using Node = base_node<Key, Weight>;
public:
    Node* get_start_of_childlist(Node* parent) const override {
        if (parent->child_list == nullptr) {
            return nullptr;
        }

        return parent->child_list->right_sibling;
    }


    bool reached_end_of_childlist(Node* child, Node* prev_child, Node* parent) const override {
        return child == nullptr || prev_child == parent->child_list;
    }

    Node* meld_together(Node* added_heap, Node* heap_root_list) const override {
        // Merging two non-empty heaps. Can do a ranked link if they have equal rank.
        // Assume heap_root_list was already just a single element.
        // heap_root_list will always be heap_root_min
        return force_link(heap_root_list, added_heap);
    }

    // TODO: Protected???

    void do_ranked_link(Node* winner, Node* loser) const override {
        if (winner->child_list == nullptr) {
            // Setup the circular child list
            loser->right_sibling = loser;
            winner->child_list = loser;
        } else {
            // Add to the initial element of child list.
            // Since parent points to last element of circlar list.
            loser->right_sibling = winner->child_list->right_sibling;
            winner->child_list->right_sibling = loser;
        }
    }

    void do_unranked_link(Node* winner, Node* loser) const override {
        if (winner->child_list == nullptr) {
            // Setup circular list.
            loser->right_sibling = loser;
            winner->child_list = loser;
        } else {
            // Add the loser to end of list, so it should be pointed to by 
            // child_list

            // Keep circular aspect
            loser->right_sibling = winner->child_list->right_sibling;;
            winner->child_list->right_sibling = loser;

            // Set loser to now be end of list.
            winner->child_list = loser;
        }
    }



    void setup_relationship_between_hollownode_and_copy(
            Node* hollow_node, Node* new_node) const override {
        if (hollow_node->child_list == nullptr) {
            // Not going to transfer any children.
            return;
        }

        // TODO: Clean this up!
        Node* start_of_node_childlist = nullptr;
        Node* end_of_node_childlist = nullptr;

        Node* start_of_new_node_childlist = nullptr;
        Node* end_of_new_node_childlist = nullptr;

        // Transfer entire child list
        if (hollow_node->rank == 0) {
            start_of_new_node_childlist = get_start_of_childlist(hollow_node);
            end_of_new_node_childlist = hollow_node->child_list;
        } else {
            // It keeps the start. Only keeps 1 additional node if its rank is >= 2
            start_of_node_childlist = get_start_of_childlist(hollow_node);
            end_of_node_childlist = start_of_node_childlist;
            if (hollow_node->rank >= 2) {
                end_of_node_childlist = end_of_node_childlist->right_sibling;
            }

            // Only transfer nodes if it has more than the last two nodes.
            // And in that case, transfer everything including the end of the original list.
            if (end_of_node_childlist != hollow_node->child_list) {
                start_of_new_node_childlist = end_of_node_childlist->right_sibling;
                end_of_new_node_childlist = hollow_node->child_list;
            }
        }

        // Update the lists pts and keep them circular.
        hollow_node->child_list = end_of_node_childlist;
        if (hollow_node->child_list != nullptr) {
            hollow_node->child_list->right_sibling = start_of_node_childlist;
        }

        new_node->child_list = end_of_new_node_childlist;
        if (new_node->child_list != nullptr) {
            new_node->child_list->right_sibling = start_of_new_node_childlist;
        }

        this->update_parent_pointers(new_node);
    }

private:
    Node* force_link(Node* h1, Node* h2) const {
        if (h1->rank == h2->rank) {
            return this->ranked_link(h1, h2);
        } else {
            return this->unranked_link(h1, h2);
        }
    }
};

template <class Key, class Weight>
class hollow_heap_impl_singleroot_two_parent : public hollow_heap_impl<Key, Weight> {
    using Node = base_node<Key, Weight>;
public:
    Node* get_start_of_childlist(Node* parent) const override {
        return parent->child_list;
    }


    bool reached_end_of_childlist(Node* child, Node* prev_child, Node* parent) const override {
            // No child, or went past the second parent child, which would be at the end of the childlist
        return child == nullptr ||
            (prev_child != nullptr && prev_child->second_parent == parent);
        
    }

    Node* meld_together(Node* added_heap, Node* heap_root_list) const override {
        // Merging two non-empty heaps. Can do a ranked link if they have equal rank.
        return force_link(heap_root_list, added_heap);
    }

    void do_ranked_link(Node* winner, Node* loser) const override {
        // Add to start of childlist.
        loser->right_sibling = winner->child_list;
        winner->child_list = loser;
    }


    void do_unranked_link(Node* winner, Node* loser) const override {
        // Add to start of childlist
        loser->right_sibling = winner->child_list;
        winner->child_list = loser;
    }

    void setup_relationship_between_hollownode_and_copy(
            Node* hollow_node, Node* new_node) const override {
        hollow_node->second_parent = new_node;
        new_node->child_list = hollow_node;
    }

private:
    Node* force_link(Node* h1, Node* h2) const {
        if (h1->rank == h2->rank) {
            return this->ranked_link(h1, h2);
        } else {
            return this->unranked_link(h1, h2);
        }
    }
};


}  // namespace internal


template <class Key, class Weight>
class hollow_heap_base {
    using Node = internal::base_node<Key, Weight>;
public:
    hollow_heap_base(hollow_heap_type type);
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

    // Will always be a single linked list.
    Node* heap_root_list;
    // The smallest weighted Node in heap_root_list list.
    Node* heap_root_min;

    bool allow_multiple_roots() const {
        return type == hollow_heap_type::MULTIPLE_ROOTS; }
    bool allow_unranked_links() const {
        return type == hollow_heap_type::SINGLE_ROOT || type == hollow_heap_type::TWO_PARENT; }
    bool uses_circular_childlist() const {
        return type == hollow_heap_type::SINGLE_ROOT; }

    const hollow_heap_type type;

    // TODO: Use this!
    void clean_out_hollow_nodes();

    // Handles the specialized algorithms for the class.
    std::unique_ptr<internal::hollow_heap_impl<Key, Weight>> impl;

    Node* get_start_of_childlist(Node* parent) const {
        return impl->get_start_of_childlist(parent);
    }

    // NOTE: Must be used with get_start_of_childlist
    bool reached_end_of_childlist(Node* child, Node* prev_child, Node* parent) const {
        return impl->reached_end_of_childlist(child, prev_child, parent);
    }

private:
    // May update heap_root_list + heap_root_min as necessary.
    void meld_into_root(Node* heap);

    void decrease_key(Node* node, const Weight& new_weight);

    // Will just remove the given element, handling its children as necessary.
    void delete_node(Node* node);

    // Returns the list of nodes to be merged together for the final rootlist.
    Node* get_rootlist_candidates();

    void regenerate_root_list(Node* list_of_candidates);

    void print_out_childlist(Node* parent, std::ostream& o,
            const std::string& offset) const;
    void print_out_for_node(Node* node, Node* parent, std::ostream& o,
            const std::string& offset) const;


    std::unordered_map<Key, Node*> key_to_node;

    size_t num_elem;

    hollow_heap_base(const hollow_heap_base& other) = delete;
    hollow_heap_base& operator=(const hollow_heap_base& other) = delete;
};

template <class Key, class Weight>
hollow_heap_base<Key, Weight>::hollow_heap_base(hollow_heap_type type)
    : heap_root_list(nullptr),
    type(type),
    num_elem(0) {

    switch (type) {
    case hollow_heap_type::MULTIPLE_ROOTS:
        impl.reset(new internal::hollow_heap_impl_multiroot<Key, Weight>{});
        break;

    case hollow_heap_type::SINGLE_ROOT:
        impl.reset(new internal::hollow_heap_impl_singleroot_single_parent<Key, Weight>{});
        break;

    case hollow_heap_type::TWO_PARENT:
        impl.reset(new internal::hollow_heap_impl_singleroot_two_parent<Key, Weight>{});
        break;

    default:
        // Should never reach here.
        assert(false);

    }
}

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

        for (Node *child = get_start_of_childlist(remove), *prev_child = nullptr, *next;
                !reached_end_of_childlist(child, prev_child, remove);
                prev_child = child, child = next) {
            next = child->right_sibling;

            // TODO: This may need to be refactored later.
            // Child if child has only 1 parent remaining
            if (child->second_parent == nullptr) {
                // If child has only one parent (this guy), delete child.
                toBeDeleted.push(child);
            } else {
                // Otherwise, mark the child as only having one parent remaining.
                child->second_parent = nullptr;
                // And mark it as being the last child in list.
                child->right_sibling = nullptr;
            }
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
        Node* root_candidates_list = get_rootlist_candidates();

        regenerate_root_list(root_candidates_list);
    }
}


template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::clean_out_hollow_nodes() {
    // Add the root list first.
    std::queue<Node*> nodes_to_go_through;
    for (Node* current = heap_root_list; current != nullptr; current = current->right_sibling) {
        nodes_to_go_through.push(current);
    }

    // Go through ALL nodes.
    Node* list = nullptr;
    while (!nodes_to_go_through.empty()) {
        Node* current = nodes_to_go_through.front();
        nodes_to_go_through.pop();

        // Add to list if it isn't a hollow node.
        if (!current->isHollow) {
            current->right_sibling = list;
            list = current;
        }

        // Add all of its children to the queue.
        for (Node *child = get_start_of_childlist(current), *prev_child = nullptr;
                !reached_end_of_childlist(child, prev_child, current);
                prev_child = child, child = child->right_sibling) {
            nodes_to_go_through.push(child);
        }
    }

    regenerate_root_list(list);
}

// Should have been updated for double parent.
template <class Key, class Weight>
typename hollow_heap_base<Key, Weight>::Node* hollow_heap_base<Key, Weight>::get_rootlist_candidates() {
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
            // Will be deleting this node, but first need to add all of its children.

            for (Node *child = get_start_of_childlist(current), *prev_child = nullptr, *next;
                    !reached_end_of_childlist(child, prev_child, current);
                    prev_child = child, child = next) {
                next = child->right_sibling;

                // TODO: May need to refactor this later.
                if (child->parent == nullptr || child->second_parent == nullptr) {
                    child->parent = child->second_parent = nullptr;
                    toMergeWithHeapRoot.push(child);
                } else {
                    if (current == child->second_parent) {
                        child->second_parent = nullptr;
                        // Have reached the end of this nodes list.
                        next = nullptr;
                    } else {
                        child->parent = nullptr;
                        child->right_sibling = nullptr;
                    }
                }
            }

            // This node can now be safely removed.
            delete current;
        } else {
            current->right_sibling = list;
            list = current;
        }
    }

    return list;
}


template <class Key, class Weight>
void hollow_heap_base<Key, Weight>::regenerate_root_list(
        Node* list_of_candidates) {
    // Should be extra space.
    int size_for_ranks = 5;
    if (size() > 0) {
        size_for_ranks += 2 * std::log2(size());
    }
    std::vector<Node*> ranks(size_for_ranks, nullptr);

    Node* current_node = list_of_candidates;

    // The root candidates list will be a singly linked list, so this works.
    while (current_node != nullptr) {
        current_node->parent = nullptr;
        // current_node may change during the following while loop.
        Node* next_node = current_node->right_sibling;

        while (ranks[current_node->rank] != nullptr) {
            int rank = current_node->rank;
            current_node = impl->ranked_link(current_node, ranks[rank]);
            ranks[rank] = nullptr;
        }
        ranks[current_node->rank] = current_node;

        current_node = next_node;
    }

    heap_root_list = nullptr;
    heap_root_min = nullptr;

    for (size_t i = 0; i < ranks.size(); ++i) {
        if (ranks[i] != nullptr) {
            meld_into_root(ranks[i]);
        }
    }
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

    impl->setup_relationship_between_hollownode_and_copy(node, new_node);

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

    heap_root_list = impl->meld_together(heap, heap_root_list);
    if (heap->weight < heap_root_min->weight) {
        heap_root_min = heap;
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

    for (Node *child = get_start_of_childlist(parent), *prev_child = nullptr;
            !reached_end_of_childlist(child, prev_child, parent);
            prev_child = child, child = child->right_sibling) {
        print_out_for_node(child, parent, o, offset);
    }
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
    o << " ptr " << node << " parent " << node->parent << " other: " << node->second_parent;

    // It is its other parent, so don't print out anything else for node.
    if (node->parent != parent &&
            node->second_parent != nullptr) {
        o << ", but is other child so will print more at different spot!\n";
        return;
    }

    o << ", with children:\n";
    print_out_childlist(node, o, offset + "    ");
}

}  // namespace hollow_heap

#endif  // HOLLOW_HEAP
