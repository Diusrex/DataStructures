default: main tests

TREE_IMPLEMTATION= RedBlackTreeBasic.h RedBlackTree.h RedBlackTreeDeletion.h \
		   RedBlackTreeInsertion.h RedBlackTreeRotation.h
CPP_FLAGS = --std=c++11 -Wall -O3

main: Main.cpp $(TREE_IMPLEMENTATION)
	g++ $(CPP_FLAGS) -o main Main.cpp

tests: Tests.cpp RedBlackTreeTestingSubclass.cpp RedBlackTreeTestingSubclass.h $(TREE_IMPLEMENTATION)
	g++ $(CPP_FLAGS) -o tests Tests.cpp RedBlackTreeTestingSubclass.cpp


clean:
	rm main tests
