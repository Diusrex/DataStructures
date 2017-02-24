IMPLEMENTATION = pairing_heap.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) avl_tests.cpp
	g++ $(CPP_ARGS) -o tests avl_tests.cpp

clean:
	rm -f tests
