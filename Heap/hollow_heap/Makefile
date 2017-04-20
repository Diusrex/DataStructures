IMPLEMENTATION = hollow_heap_base.h
CPP_ARGS = --std=c++11 -Wall -O3 -g

tests: $(IMPLEMENTATION) hollow_heap_tests.cpp
	g++ $(CPP_ARGS) -o tests hollow_heap_tests.cpp

dijkstra: $(IMPLEMENTATION) dijkstra.cpp
	g++ $(CPP_ARGS) -o dijkstra dijkstra.cpp

clean:
	rm -f dijkstra tests
