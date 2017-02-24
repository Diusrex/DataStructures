IMPLEMENTATION = skip_list.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) skip_list_tests.cpp
	g++ $(CPP_ARGS) -o tests skip_list_tests.cpp

clean:
	rm -f tests
