IMPLEMENTATION = hollow_heap_base.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) hollow_heap_tests.cpp
	g++ $(CPP_ARGS) -g -o tests hollow_heap_tests.cpp

clean:
	rm -f tests
