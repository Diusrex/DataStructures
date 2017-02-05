default: main tests

TREE_IMPLEMTATION= RedBlackTreeBasic.h RedBlackTree.h RedBlackTreeDeletion.h \
		   RedBlackTreeInsertion.h RedBlackTreeRotation.h

main: Main.cpp $(TREE_IMPLEMENTATION)
	g++ --std=c++11 -o main Main.cpp

tests: Tests.cpp RedBlackTreeTestingSubclass.cpp RedBlackTreeTestingSubclass.h $(TREE_IMPLEMENTATION)
	g++ --std=c++11 -o tests Tests.cpp RedBlackTreeTestingSubclass.cpp


clean:
	rm main tests
