#ifndef GRAPH_CONNECTIVITY_SPECIALISED_SKIPLIST
#define GRAPH_CONNECTIVITY_SPECIALISED_SKIPLIST

#include <iostream>

#include <stdlib.h>
#include <set>
#include <vector>

// Nodes will still have random height when inserted, but will not be altered after that.
// Will always return the LOWEST node in the stack for an element.

struct Node {
    Node(int element)
        : up(nullptr),
        down(nullptr),
        left(nullptr),
        right(nullptr),
        element(element)
    {}

    Node* up;
    Node* down;
    Node* left;
    Node* right;

    const int element;
};



// Supports specialized functionality using the model of a skiplist
// to allow logN cut and merge of two separate euler tours.
// All Node* will be cleaned up on final delete.
// WARNING: DO NOT allow it to be deconstructed until done with all Node*.
class EulerTourSkiplist {
public:
    EulerTourSkiplist() {}
    ~EulerTourSkiplist();

    // Returns list with size = elements.size.
    // Each provided element will be transformed into a stack of nodes with random height
    std::vector<Node*> euler_tour_to_skiplist(std::vector<int> elements);


    // Returns list with size = elements.size.
    // Each provided element will be transformed into a stack of nodes with height provided.
    // NOTE: Should only be used when testing!
    std::vector<Node*> euler_tour_to_skiplist(std::vector<int> elements,
            std::vector<int> heights);

    // Returns true if they are part of the same tour.
    bool nodes_are_connected(Node* n1, Node* n2) const;


    // If new_height is -1, will be randomly set. Otherwise will use the provided height, which must be >= 1
    // Will return the additional node containing the same element as second.
    Node* insert_tour_after_node(Node* inserted,
            Node* second, int new_after_node_height=-1);


    // Assumes that the tour was surrounded by two nodes with same element value.
    // Will delete one of those nodes, then return the other untounced node.
    Node* cut_out_tour(Node* tour_start, Node* tour_end);



private:
    void link_horizontal(Node* left_node, Node* right_node) const;

    // Stores the lowest node in the stack.
    std::set<Node*> all_allocated_stacks;
};

// Should only be used by implementation/testing.
namespace internal {

// Will go up in heights, then go down until reached the end.
// Should be ~O(lg n) - log(n) heights), and each height will let us skip enough nodes for
// it to also be lg n.
Node* get_first_node_in_tour(Node* node);

}  // namespace internal


#endif // GRAPH_CONNECTIVITY_SPECIALISED_SKIPLIST
