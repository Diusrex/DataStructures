#include <iostream>

#include "RedBlackTree.h"

using namespace std;

int main() {
    RedBlackTree<int> tree;

    cout << "Please enter one of 3 actions:" << endl
         << "  i <num> (insert integer num into the set)" << endl
         << "  s <num> (print out if num is in the set" << endl
         << "  d <num> (delete num from the set)." << endl
         << "You may enter as many commands as wanted. (Press ctrl-D to finish entering)" << endl << endl;

    char action;
    int num;

    while (cin >> action >> num) {
        if (action == 'i') {
            if (tree.Insert(num))
                cout << "Inserted " << num << endl;
        } else if (action == 'd') {
            if (tree.Delete(num))
                cout << "Removed " << num << endl;
        } else if (action == 's') {
            if (tree.Contains(num))
                cout << "Contains " << num << endl;
            else
                cout << "Does not contain " << num << endl;
        } else {
            cout << "Action " << action << " is invalid and ignored " << endl;
        }
    }

    cout << "________________________________\n"
         << "Final tree:\n";
    tree.WriteOut(cout);
}
