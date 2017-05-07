IMPLEMENTATION = skiplist.h skiplist.cpp
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) skiplist_tests.cpp
	g++ $(CPP_ARGS) -g -o tests skiplist.cpp skiplist_tests.cpp

clean:
	rm -f tests
