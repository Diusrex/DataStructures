## Skip list

To compile the tests, just run make.

I checked my understanding of a skip list using [Wikipedia](https://en.wikipedia.org/wiki/Skip_list), although I only really needed the diagram.

### Files

skip_list.h contains the full implementation of the BST, and is a standalone file.

skip_list_tests.cpp contains the testing implementation.

### Tests Description

There were two types of tests:

- Specific case tests
    * Tests for the different cases with insertion, deletion and finding an element.
    * Including things like checking the different possible order for elements to be inserted.
    * Will check that elements are insert/removed properly for find, and that the skip list remains valid.


- Large tests
    * Meant to tests the speed and correctness of the skip list by inserting a large number of elements (up to 1000000 elements added)
    * Have both insertion and deletion versions.
    * Will check that elements are inserted and removed properly, as well as ensure that the tree remains valid.

The tests (especially large tests, are copied from avl_tree implementation).
