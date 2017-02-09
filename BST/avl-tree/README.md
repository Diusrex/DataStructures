## Avl Tree

To compile the tests, just run make.

### Files

avl_tree.h contains the full implementation of the tree, and is a standalone file.

avl_tests.cpp contains the testing implementation.

### Tests Description

There were two types of tests:

- Specific case tests
    * Tests for all of the different special cases for the tree when inserting and deleting.
    * Including things like checking the different rotations necessary for insertion/deletion.
    * Will check that elements are insert/removed properly, the expected element is at the root, and that the tree remains valid.


- Large tests
    * Meant to tests the speed and correctness of the tree by inserting a large number of elements (up to 1000000 elements added)
    * Have both insertion and deletion versions.
    * Will check that elements are inserted and removed properly, as well as ensure that the tree remains valid.
