IMPLEMENTATION = pairing_heap.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) pairing_heap_tests.cpp
	g++ $(CPP_ARGS) -o tests pairing_heap_tests.cpp

clean:
	rm -f tests
