IMPLEMENTATION = pairing_heap.h
CPP_ARGS = --std=c++11 -Wall -O3

tests: $(IMPLEMENTATION) pairing_heap_tests.cpp
	g++ $(CPP_ARGS) -o tests pairing_heap_tests.cpp

dijkstra: $(IMPLEMENTATION) dijkstra.cpp
	g++ $(CPP_ARGS) -o dijkstra dijkstra.cpp

clean:
	rm -f dijkstra tests
