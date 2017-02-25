## Comparisons

To compile the comparisons, just run make.

This will compare the 4 different BST - Avl Tree, Red Black Tree, Skip List, and std::set (which uses Red Black Tree) on a few large insertion and deletion tests.

This comparison depends on the other sibling folders in BST directory.

### Files

comparisons.cpp - Contains the wrappers for the different BST, and some large tests to run them.

### Comparison

On my VM:
- Avl Tree takes ~3025ms
- Red Black Tree takes ~3670ms
- Skip List takes ~7050ms
- std::set takes ~2500ms

So std::set is the fastest, with Avl tree and Red Black tree being comparable in speed. As expected, Skip List is slower.

Of course (other than std::set), these data structures are not very optimised - my implementation of Red Black tree takes ~1000ms longer than the implementation used in std::set.
