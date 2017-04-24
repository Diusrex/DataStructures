# Cuckoo Hashing Implementation

Implementation of a cuckoo hash using C++11, using the [Cuckoo Hashing paper](http://resources.mpi-inf.mpg.de/departments/d1/teaching/ws14/AlgoDat/materials/cuckoo.pdf) as a reference. A visualization has been made by [László Kozma](http://www.lkozma.net/cuckoo_hashing_visualization/).

### Comparison against std::unordered_set

In cuckoo_tests.cpp, RunLargeTest will compare cuckoo hashing vs std::unordred when adding and removing 1000000 elements.
Is not an exhasutive test, but should do a reasonable job for comparison.

Results:
- <b>~320 ms cuckoo</b>  - Note that the difference between identical runs (with different hashing function) is up to 100 ms.
- <b>~92 ms std::unordered_set</b> - Has very little change between runs.


### Files

cuckoo.h contains the full implementation of the hashing scheme, including the default hashing scheme, and is a standalone file.

cuckoo_tests.cpp contains the testing implementation.

### Extensions

For this implementation to truly be used, the internal hashing scheme (implemented by the class hashing_function) would need to be extended.

Further experimentation could also be done with the best epsilon value to be used.


### Tests Description

There were two types of tests:

- Specific case tests
    * Tests for most of the different special cases that the hashing scheme needed to be able to handle.
    * Includes thing like resizing the table, rehashing, and resizing/rehashing multiple times.

- Large test
    * Mean to test the speed and correctness of the hashing scheme by inserting a large number of elements (1000000 elements)
    * Focuses on a mix of insertion and deletion
    * Ensures that the data structure remains valid throughout the tests
    * Also compares against the speed of std::unordered_set.
