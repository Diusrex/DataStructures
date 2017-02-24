#include "skip_list.h"

#include <limits>
#include <iostream>
#include <set>

using namespace std;

const int MostInserted =      1000000;

const int LargestRandomNum = 10000000;
const int NumRandomInserted =  500000;

// In large test, will delete multiples of this immediately
const int EveryDeletedImmediately = 5;

// In large test, will delete multiples of this after everything else was inserted
const int EveryDeletedAfter = 3;

class skip_list_test : public skip_list<int> {
public:
    void assert_is_valid() const {
        if (start_at_level.empty()) {
            if (size() != 0)
                throw "The skip_list thinks it still has " + to_string(size()) + " elements when it has none";
            return;
        }

        int highest_level = start_at_level.size() - 1;
        assert_is_valid_skip_list(highest_level, set<const Interval*>());

        if (size() != get_size_of_level(start_at_level[0]))
            throw "The size wasn't updated properly: is " +
                to_string(get_size_of_level(start_at_level[0])) + " while reports " + to_string(size());
    }

    void assert_is_valid_skip_list(int level, set<const Interval*> all_expected_intervals) const {
        if (level == -1) {
            return;
        }

        set<const Interval*> expected_intervals_in_lower_level;

        //std::cout << "Level: " << level << ":\n";
        int previous = numeric_limits<int>::min();
        for (const Interval* interval = start_at_level[level];
                interval != nullptr; interval = interval->right) {
            //std::cout << "Hello " << interval << ' ' << interval->start << ' ' << interval->elementBelow << '\n';
            if (interval->start < previous) {
                throw "Element " + to_string(interval->start) +
                    " was less than the previous " + to_string(previous);
            }
            if (interval->start == previous) {
                throw "Element " + to_string(interval->start) +
                    " was equal to previous";;
            }

            if (interval->left != nullptr && interval->left->right != interval) {
                throw "There is an issue with left right ptrs, nodes " +
                    to_string(interval->left->start) + " and " + to_string(interval->start);
            }

            if (interval->right != nullptr && interval->right->left != interval) {
                throw "There is an issue with right left ptrs, nodes " +
                    to_string(interval->right->start) + " and " + to_string(interval->start);
            }


            // Isn't the last level, so should point to next element.
            if (level != 0) {
                if (interval->elementBelow == nullptr)
                    throw "Interval " + to_string(interval->start) + " on " +
                        to_string(level) + " did not point to a child";

                if (interval->elementBelow->start != interval->start)
                    throw "Interval " + to_string(interval->start) + " child has value " +
                        to_string(interval->elementBelow->start);


            } else {
                if (interval->elementBelow != nullptr)
                    throw "Interval " + to_string(interval->start) +
                        " on bottom level pointed to interval below...";
            }
            all_expected_intervals.erase(interval);

            expected_intervals_in_lower_level.insert(interval->elementBelow);
            previous = interval->start;
        }

        if (start_at_level[level] == nullptr)
            throw "Level " + to_string(level) + " doesn't have any nodes in it...";

        if (!all_expected_intervals.empty())
            throw "Level " + to_string(level) + " was missing " +
                to_string(all_expected_intervals.size()) + " required intervals, including interval start " +
                to_string((*all_expected_intervals.begin())->start);

        assert_is_valid_skip_list(level - 1, expected_intervals_in_lower_level);
    }
    
    size_t get_size_of_level(Interval* interval) const {
        if (interval == nullptr)
            return 0;

        return 1 + get_size_of_level(interval->right);
    }
};

bool CheckSize(const skip_list_test& skip_list, size_t expected_size, const string& test_id) {
    if (skip_list.size() != expected_size) {
        std::cout << "ERROR in " << test_id << ": Size is " << skip_list.size()
            << " expected " << expected_size << '\n';
        return false;
    }
    return true;
}

bool CheckMinimum(const skip_list_test& skip_list, int expected, const string& test_id) {
    if (skip_list.minimum() != expected) {
        std::cout << "ERROR in " << test_id << ": Minimum is " << skip_list.minimum()
            << " expected " << expected << '\n';
        return false;
    }
    return true;
}

bool CheckFindContains(const skip_list_test& skip_list, int element, const string& test_id) {
    if (!skip_list.find(element)) {
        std::cout << "ERROR in " << test_id << ": Expected would contain " << element << '\n';
        return false;
    }
    return true;
}


bool CheckFindNotIn(const skip_list_test& skip_list, int element, const string& test_id) {
    if (skip_list.find(element)) {
        std::cout << "ERROR in " << test_id << ": Expected wouldn't contain " << element << '\n';
        return false;
    }
    return true;
}

bool CheckIsValid(const skip_list_test& skip_list, const string& test_id) {
    try {
        skip_list.assert_is_valid();
    } catch (string s) {
        std::cout << "ERROR in " << test_id << ": " << s << '\n';
        return false;
    }
    return true;
}


bool InsertElementsAfter() {
    skip_list_test skip_list;

    skip_list.insert(1);
    skip_list.insert(2);
    skip_list.insert(3);

    const string id = "InsertElementsAfter";
    bool valid = CheckSize(skip_list, 3, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindContains(skip_list, 2, id);
    valid &= CheckFindContains(skip_list, 3, id);
    return valid;
}

bool InsertElementBetween() {
    skip_list_test skip_list;

    skip_list.insert(1);
    skip_list.insert(3);
    skip_list.insert(2);

    const string id = "InsertElementBetween";
    bool valid = CheckSize(skip_list, 3, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindContains(skip_list, 2, id);
    valid &= CheckFindContains(skip_list, 3, id);
    return valid;
}

bool InsertElementsBefore() {
    skip_list_test skip_list;

    skip_list.insert(3);
    skip_list.insert(2);
    skip_list.insert(1);

    const string id = "InsertElementsBefore";
    bool valid = CheckSize(skip_list, 3, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindContains(skip_list, 2, id);
    valid &= CheckFindContains(skip_list, 3, id);
    return valid;
}

bool FindChecks() {
    skip_list_test skip_list;
    // Some simple checks that can be easily done.

    skip_list.insert(1);
    skip_list.insert(3);

    const string id = "FindChecks";
    bool valid = CheckSize(skip_list, 2, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);

    // Important part! Check for both elements, between them, before and after.
    valid &= CheckFindNotIn(skip_list, 0, id);
    valid &= CheckFindContains(skip_list, 1, id);
    valid &= CheckFindNotIn(skip_list, 2, id);
    valid &= CheckFindContains(skip_list, 3, id);
    valid &= CheckFindNotIn(skip_list, 4, id);
    return valid;
}


bool RemoveElementAfter() {
    skip_list_test skip_list;

    skip_list.insert(1);
    skip_list.insert(2);
    skip_list.insert(3);

    skip_list.remove(3);

    const string id = "RemoveElementAfter";
    bool valid = CheckSize(skip_list, 2, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindContains(skip_list, 2, id);
    valid &= CheckFindNotIn(skip_list, 3, id);
    return valid;
}

bool RemoveElementBetween() {
    skip_list_test skip_list;

    skip_list.insert(1);
    skip_list.insert(2);
    skip_list.insert(3);


    skip_list.remove(2);

    const string id = "RemoveElementBetween";
    bool valid = CheckSize(skip_list, 2, id);
    valid &= CheckMinimum(skip_list, 1, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindContains(skip_list, 1, id);
    valid &= CheckFindContains(skip_list, 3, id);
    valid &= CheckFindNotIn(skip_list, 2, id);
    return valid;
}

bool RemoveElementBefore() {
    skip_list_test skip_list;

    skip_list.insert(3);
    skip_list.insert(2);
    skip_list.insert(1);

    skip_list.remove(1);

    const string id = "RemoveElementsBefore";
    bool valid = CheckSize(skip_list, 2, id);
    valid &= CheckMinimum(skip_list, 2, id);
    valid &= CheckIsValid(skip_list, id);
    valid &= CheckFindNotIn(skip_list, 1, id);
    valid &= CheckFindContains(skip_list, 2, id);
    valid &= CheckFindContains(skip_list, 3, id);
    return valid;
}





bool Contains(const std::set<int>& s, int num) {
    return s.find(num) != s.end();
}

void LargeRandomInsertTest() {
    std::cout << "Starting large random insert. "
        << "If this takes longer than ~20 seconds, there is a balancing issue\n";
    skip_list_test skip_list;

    srand(0);

    std::set<int> s;
    for (int i = 0; i < NumRandomInserted; ++i) {
        int num = rand() % LargestRandomNum;
        skip_list.insert(num);
        s.insert(num);
    }

    for (int i = 0; i < LargestRandomNum; ++i)
        if (skip_list.find(i) != Contains(s, i))
            std::cout << "\nERROR in LargeRandomInsertTest: item " << i <<
                " reported by set as " << Contains(s, i) << " avl reports " <<
                skip_list.find(i) << '\n';

    CheckIsValid(skip_list, "LargeRandomInsertTest");

    std::cout << "Completed large random insert\n\n";
}


void LargeInsertTest() {
    std::cout << "Starting large insert."
        << "If this takes longer than ~5 seconds, there is a balancing issue\n";
    skip_list_test skip_list;

    for (int i = 0; i < MostInserted; ++i)
        skip_list.insert(i);

    for (int i = 0; i < MostInserted; ++i)
        if (!skip_list.find(i))
            std::cout << "\nERROR in LargeInsertTest: item " << i << " missing";

    CheckIsValid(skip_list, "LargeInsertTest");

    std::cout << "Completed large insert\n\n";
}

void RunLargeCompleteDeleteTest() {
    std::cout << "Starting large complete delete\n";    
    
    skip_list_test skip_list;
    for (int i = 0; i < MostInserted; ++i) {
        skip_list.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            skip_list.remove(i);
        }
    }

    // No point in continuing if not valid.
    if (!CheckIsValid(skip_list, "LargeCompleteDeleteTest"))
        return;

    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately != 0) {
            if (!skip_list.find(i)) {
                std::cout << "ERROR in LargeCompleteDeleteTest: value " << i <<
                    " not found in skip_list\n";
            }

            skip_list.remove(i);
        }
    }

    for (int i = 0; i < MostInserted; ++i) {
        if (skip_list.find(i)) {
            std::cout << "ERROR in LargeCompleteDeleteTest: value " << i <<
                " is still in skip_list\n";
        }
    }

    CheckIsValid(skip_list, "LargeCompleteDeleteTest");

    std::cout << "Finished large complete delete\n\n";
}

void RunLargeDeleteTest() {
    std::cout << "Starting large delete test\n";

    skip_list_test skip_list;

    for (int i = 0; i < MostInserted; ++i) {
        skip_list.insert(i);
        if (i % EveryDeletedImmediately == 0) {
            skip_list.remove(i);
        }
    }

    // No point in continuing if invalid already.
    if (!CheckIsValid(skip_list, "LargeDeleteTest"))
        return;

    for (int i = 0; i < MostInserted; i += EveryDeletedAfter) {
        skip_list.remove(i);
    }

    for (int i = 0; i < MostInserted; ++i) {
        if (i % EveryDeletedImmediately == 0 || i % EveryDeletedAfter == 0) {
            // Should be deleted.
            if (skip_list.find(i))
                std::cout << "ERROR in LargeDeleteTest: Contains " << i << '\n';
        } else if (!skip_list.find(i)) {
            std::cout << "ERROR in LargeDeleteTest: Doesn't contain " << i << '\n';
        }
    }
    std::cout << "Finished large delete test\n";
}


int main() {
    bool insert_fine = InsertElementsAfter();
    insert_fine &= InsertElementBetween();
    insert_fine &= InsertElementsBefore();

    insert_fine &= FindChecks();

    bool remove_fine = RemoveElementAfter();
    remove_fine &= RemoveElementBetween();
    remove_fine &= RemoveElementBefore();

    std::cout << "Completed small tests\n\n";
    if (insert_fine) {
        LargeInsertTest();
        LargeRandomInsertTest();
    }

    if (insert_fine && remove_fine) {
        RunLargeCompleteDeleteTest();
        RunLargeDeleteTest();
    }
}
