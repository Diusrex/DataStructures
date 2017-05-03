# Heaps

This directory contains a few different heap implementations that have different complexity bounds.

## Comparison

Using comparison, there are two standard input files, large_graph.in, medium_graph.in, and small_graph.in. Ran each 10 times, and took the average.

Note that the times are just approximate, since they were calculated on my VM without being able to control any other possible programs.

For small graph, on my VM:
- std::insert: 216 ms (only one that will reinsert a key every time it is updated).
- Pairing Heap: 68 ms
- Hollow Heap Multi-root: 96 ms
- Hollow Heap Single-root: 104 ms
- Hollow Heap Two-parent: 93 ms

For medium graph, on my VM:
- std::insert: 2700 ms (only one that will reinsert a key every time it is updated).
- Pairing Heap: 1110 ms
- Hollow Heap Multi-root: 1700 ms
- Hollow Heap Single-root: 1700 ms
- Hollow Heap Two-parent: 1700 ms

For large graph, on my VM:
- std::insert: 12200 ms (only one that will reinsert a key every time it is updated).
- Pairing Heap: 4900 ms
- Hollow Heap Multi-root: 6500 ms
- Hollow Heap Single-root: 6800 ms
- Hollow Heap Two-parent: 7000 ms


There are probably two things slowing down all the versions of Hollow Heap:
1) Excessive hollow nodes, since I haven't added any functionality to clean up all hollow nodes. There is a function that has been implemented for this, but it isn't tested and isn't being called yet.

2) Inefficient memory useage, since the heap allocates a lot more nodes than pairing heap, so it will be hurt by the increased allocation requirements.

Two parent is likely slower than the other versions since for multi- and single-root the parent for most nodes will be decreased over time - if a node Y has X as a parent, then when the weight for X is decreased, the node Y will likely point to the decreased weight version of X. This would allow the weight for Y to be decreased further without needing to create a new node - it is very likely the memory allocation is what is slowing two-parent down.
