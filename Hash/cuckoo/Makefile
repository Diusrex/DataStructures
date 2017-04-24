IMPLEMENTATION = cuckoo.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) cuckoo_tests.cpp
	g++ $(CPP_ARGS) -g -o tests cuckoo_tests.cpp

clean:
	rm -f tests
