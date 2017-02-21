## Pairing Heap

To compile the tests, just run `make`.  To run tests, run `./tests`.

To compare Dijkstras, first compile using `make dijkstra`.
- To run the re-insertion version of dijkstras, run `./dijkstra reinsert`.
- To run the key update version of dijkstras, run `./dijkstra updatekey`.
- To run the dijkstras without any graph output, instead run it as `./dijkstra {reinsert/updatekey} less_out`

* As well, you will need to pipe in the graph itself - the options stored in data/ are graph250.in, graph1000.in, and graph4000.in.
    All of the graphs are quite dense, and were from http://www.info.univ-angers.fr/pub/porumbel/graphs/ (dsjc250.5, dsjc1000.1 and 4000.5).
    Note that 4000.5 has a large amount of input, so loading input from file takes most of the running time...
    

### Details source

Used https://brilliant.org/wiki/pairing-heap/ for the outline for implementation.

More specifically, it outlines a simple two pass (left->right then right->left) merge
when handling removing a node.

### Files

pairing_heap.h contains the full implementation of the heap, and is a standalone file.

pairing_heap_tests.cpp contains all the tests for the pairing heap.

dijkstra.cpp contains two different implementations for Dijkstra's algorithm.
    Key change contains the implementation using the pairing heap, where the keys can be updated.
    Reinsert uses the built-in C++ priority_queue, and will reinsert each node when its weight needs to be changed.

    It then contains a few different tests comparing the running time of the two over different graphs.


### Tests Description

There were two types of tests:

- Specific case tests
    * Tests for all of the different special cases for a pairing heap when inserting and deleting.
    * Includes cases like the number of children of the deleted root.
    * Will check inserting keys, changing keys weights, and removing keys.


- Large test
    * Mean to test the speed and correctness of the heap by inserting a large number of elements (up to 1000000 elements)
    * Each includes inserting, changing weights, and removing the min element.


### Dijkstras

graph4000.in:
- Update Key: ~36ms
- Reinsert key: ~150ms

(Timing ignores time to load the data, and is ran with less out so not additional output is generated).

The other graphs are essentially instant (<1ms), so not recording any results here.
