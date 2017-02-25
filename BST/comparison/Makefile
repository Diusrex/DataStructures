IMPLEMENTATION = ../avl-tree/avl_tree.h ../red-black-tree/RedBlackTree.h ../skip-list/skip_list.h
CPP_ARGS = --std=c++11 -Wall -O3

compare: $(IMPLEMENTATION) comparisons.cpp
	g++ $(CPP_ARGS) -o compare comparisons.cpp

clean:
	rm -f compare
