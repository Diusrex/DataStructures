#include "../avl-tree/avl_tree.h"
#include "../red-black-tree/RedBlackTree.h"
#include "../skip-list/skip_list.h"

#include <chrono>
#include <limits>
#include <iostream>
#include <set>

using namespace std;

class Wrapper {
public:
    virtual ~Wrapper() {} 

    virtual void insert(int item) = 0;

    virtual void remove(int item) = 0;

    virtual bool find(int item) const = 0;

    virtual Wrapper* CopyWrapper() const = 0;
};

class AvlWrapper : public Wrapper {
public:
    void insert(int item) override {
        tree.insert(item);
    }

    void remove(int item) override {
        tree.remove(item);
    }

    bool find(int item) const override {
        return tree.find(item);
    }

    Wrapper* CopyWrapper() const override {
        return new AvlWrapper();
    }

private:
    avl_tree<int> tree;
};

class RedBlackWrapper : public Wrapper {
public:
    void insert(int item) override {
        tree.Insert(item);
    }

    void remove(int item) override {
        tree.Delete(item);
    }

    bool find(int item) const override {
        return tree.Contains(item);
    }

    Wrapper* CopyWrapper() const override {
        return new RedBlackWrapper();
    }


private:
    RedBlackTree<int> tree;
};

class SkipListWrapper : public Wrapper {
public:
    void insert(int item) override {
        list.insert(item);
    }

    void remove(int item) override {
        list.remove(item);
    }

    bool find(int item) const override {
        return list.find(item);
    }

    Wrapper* CopyWrapper() const override {
        return new SkipListWrapper();
    }


private:
    skip_list<int> list;
};

class StandardSetWrapper : public Wrapper {
public:
    void insert(int item) override {
        s.insert(item);
    }

    void remove(int item) override {
        s.erase(item);
    }
    
    bool find(int item) const override {
        return s.find(item) != s.end();
    }

    Wrapper* CopyWrapper() const override {
        return new StandardSetWrapper();
    }

private:
    set<int> s;
};

const int MostInserted =      1000000;

const int LargestRandomNum = 10000000;
const int NumRandomInserted =  500000;

// In large test, will delete multiples of this immediately
const int EveryDeletedImmediately = 5;

// In large test, will delete multiples of this after everything else was inserted
const int EveryDeletedAfter = 3;

// Returns junk
int RandomInsertTest(Wrapper& tree) {
    srand(0);

    std::set<int> s;
    for (int i = 0; i < NumRandomInserted; ++i) {
        int num = rand() % LargestRandomNum;
        tree.insert(num);
        s.insert(num);
    }

    // This is just meant to ensure the finds won't be removed.
    int sum = 0;
    for (int i = 0; i < LargestRandomNum; ++i) {
        sum += tree.find(i);
    }
    return sum;
}

// Returns junk
int InsertTest(Wrapper& tree) {
    for (int i = 0; i < MostInserted; ++i)
        tree.insert(i);

    // This is just meant to ensure the finds won't be removed.
    int sum = 0;
    for (int i = 0; i < MostInserted; ++i)
        sum += tree.find(i);
    return sum;
}

// Returns junk
int CompleteDeleteTest(Wrapper& tree) {
    for (int i = 0; i < MostInserted; ++i) {
        tree.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            tree.remove(i);
        }
    }

    int sum = 0;
    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately != 0) {
            sum += tree.find(i);

            tree.remove(i);
        }
    }
    return sum;
}

// Returns junk
int PartialDeleteTest(Wrapper& tree) {
    for (int i = 0; i < MostInserted; ++i) {
        tree.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            tree.remove(i);
        }
    }

    for (int i = 0; i < MostInserted; i += EveryDeletedAfter) {
        tree.remove(i);
    }

    int sum = 0;
    for (int i = 0; i < MostInserted; ++i) {
        sum += tree.find(i);
    }
    return sum;
}

chrono::milliseconds GetTime() {
    return chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch());
}


// Returns junk
int RunTestAndPrintTime(const string tree_name, const Wrapper& base_tree) {
    // Allocate before running the test.
    Wrapper* random = base_tree.CopyWrapper();
    Wrapper* insert = base_tree.CopyWrapper();
    Wrapper* completeDelete = base_tree.CopyWrapper();
    Wrapper* partialDelete = base_tree.CopyWrapper();

    int sum = 0;
    chrono::milliseconds before = GetTime();

    sum += RandomInsertTest(*random);
    
    sum += InsertTest(*insert);

    sum += CompleteDeleteTest(*completeDelete);

    sum += PartialDeleteTest(*partialDelete);

    chrono::milliseconds after = GetTime();

    cout << tree_name << " took " << (after - before).count() << "ms \n\n";

    // Deallocate after, since not part of the test.
    delete random;
    delete insert;
    delete completeDelete;
    delete partialDelete;

    return sum;
}

int main() {
    int sum = 0;
    sum += RunTestAndPrintTime("Avl Tree", AvlWrapper{});
    sum += RunTestAndPrintTime("Red Black Tree", RedBlackWrapper{});
    sum += RunTestAndPrintTime("Skip List", SkipListWrapper{});
    sum += RunTestAndPrintTime("std::set", StandardSetWrapper{});

    cout << "In total, " << sum << " elements were found throughout the progression.\n";
}
