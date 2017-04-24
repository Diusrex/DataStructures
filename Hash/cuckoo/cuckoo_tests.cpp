#include "cuckoo.h"

#include <exception>

#include <algorithm>
#include <iostream>
#include <limits>
#include <chrono>
#include <map>
#include <string>
#include <unordered_set>
#include <sstream>
#include <vector>


using milliseconds = std::chrono::milliseconds;
using Time = std::chrono::system_clock;
using std::to_string;

const int NumElementsInserted =        1000000;
const int EveryIndexRemovedAfterInsert =    10;
const int EveryIndexRemoved =                4;


// Will stick with mapping every item to the provided index.
// Can then switch to a different hashing upon the second rehashing.
class specialized_hashing_function : public hashing_function<int> {
public:
    specialized_hashing_function(std::map<int, int> element_to_index,
            std::map<int, int> secondary_element_to_index=std::map<int, int>{})
        : number_rehashes(-1), // Make it so the initial rehash doesn't count
        element_to_index_by_rehashes{element_to_index, secondary_element_to_index}
        {
    }

    specialized_hashing_function(std::vector<std::map<int, int>> element_to_index_by_rehashes)
        : number_rehashes(-1), // Make it so the initial rehash doesn't count
        element_to_index_by_rehashes(element_to_index_by_rehashes)
        {
    }



    virtual void reset_hash(int p) override {
        ++number_rehashes;
    }

    virtual int get_hash(const int& t) const override {
        if (static_cast<size_t>(number_rehashes) >= element_to_index_by_rehashes.size()) {
            return element_to_index_by_rehashes[element_to_index_by_rehashes.size() - 1].
                find(t)->second;
        }
        return element_to_index_by_rehashes[number_rehashes].find(t)->second;
    }

    int number_rehashes;

    const std::vector<std::map<int, int>> element_to_index_by_rehashes;
};

using wrapped_specialized_hashing_function = std::unique_ptr<specialized_hashing_function>;

// Done differently than other Data Structures since want to get access to the data in the class
class cuckoo_hashing_tests : public cuckoo_hashing<int> {
public:
    cuckoo_hashing_tests(double eps=0.5)
        : cuckoo_hashing(
            std::unique_ptr<hashing_function<int>>(
                new basic_hashing_function<int>{std::mt19937{}, true}),
            std::unique_ptr<hashing_function<int>>(
                new basic_hashing_function<int>{std::mt19937{}, true}),
            eps) {
    }

    cuckoo_hashing_tests(std::unique_ptr<hashing_function<int>> first_table,
        std::unique_ptr<hashing_function<int>> second_table,
        double eps=0.5)
        : cuckoo_hashing(std::move(first_table),
                std::move(second_table),
                eps) {
    }

    size_t get_num_resize() const {
        return num_resize;
    }

    size_t get_num_rehash() const {
        return num_rehash;
    }

    int get_number_inserts_required_to_increase_tablesize() const {
        return max_number_elements - num_elements + 1;
    }

    int get_number_removes_required_to_decrease_tablesize() const {
        return num_elements -min_number_elements + 1;
    }

    void assert_is_valid() const {
        size_t number_elements =
            assert_table_is_valid(0) + assert_table_is_valid(1);
        
        if (size() != number_elements)
            throw "The size wasn't updated properly: is " +
                to_string(number_elements) + " while reports " + to_string(size());

        if (number_elements < min_number_elements)
            throw "Table should have been resized, number elements is " + to_string(number_elements) +
                " vs min of " + to_string(min_number_elements);

        if (number_elements > max_number_elements)
            throw "Table should have been resized, number elements is " + to_string(number_elements) +
                " vs max of " + to_string(max_number_elements);

        // Ensure meets the requirement of table_size >= (1 + eps) * number_elements
        size_t expected_table_size = (1 + eps) * number_elements;
        if (table_size < expected_table_size)
            throw "The table didn't resize properly, should have a table size of at least " +
                to_string(table_size) + ", rather than " + to_string(expected_table_size);

        // Ensure that the maximum number of elements will work with the table size.
        size_t required_table_size_for_max_elem = (1 + eps) * max_number_elements;
        if (table_size < required_table_size_for_max_elem)
            throw "The table size isn't large enough for the maximum number of elements: "
                " would need a size of " + to_string(required_table_size_for_max_elem) +
                "but only have a size of " + to_string(table_size);
    }

private:

    size_t assert_table_is_valid(int table_num) const {
        // Count the number of elements, ensure each element is in the correct spot,
        // and make sure it isn't in the other spot in table for itself.

        size_t number_elements = 0;
        for (size_t i = 0; i < table_size; ++i) {
            if (!tables[table_num][i].contains_item) {
                continue;
            }

            ++number_elements;

            int item_stored = tables[table_num][i].item;

            size_t hash_using_tables_hash = hashes[table_num]->get_hash(item_stored);
            std::flush(std::cout);
            if (i != hash_using_tables_hash)
                throw "Invalid index to store value " + to_string(item_stored) + ": " +
                   " in index " + to_string(i) + " but hashes to " + to_string(hash_using_tables_hash) +
                   " in table " + to_string(table_num);
        }

        return number_elements;
    }

};

void CheckNumberResize(const cuckoo_hashing_tests& cuckoo, size_t expected_count) {
    if (cuckoo.get_num_resize() != expected_count)
        throw "Unexpected number of resize, expected " + to_string(expected_count) +
            " got " + to_string(cuckoo.get_num_resize());
}

// NOTE: This should ONLY be used when the results for the hashing scheme are deterministic.
// Otherwise is possible for a bad random to cause additional hashes required.
// If isn't deterministic, should use CheckMinNumberRehash
void CheckNumberRehash(const cuckoo_hashing_tests& cuckoo, size_t expected_count) {
    if (cuckoo.get_num_rehash() != expected_count)
        throw "Unexpected number of rehashes, expected " + to_string(expected_count) +
            " got " + to_string(cuckoo.get_num_rehash());
}

// NOTE: Should only be used for non-deterministic tests.
void CheckMinNumberRehash(const cuckoo_hashing_tests& cuckoo, size_t expected_count) {
    if (cuckoo.get_num_rehash() < expected_count)
        throw "Unexpected number of rehashes, expected at least " + to_string(expected_count) +
            " got " + to_string(cuckoo.get_num_rehash());
}

void CheckContainsElement(const cuckoo_hashing_tests& cuckoo, int val) {
    if (!cuckoo.contains(val))
        throw "Expected cuckoo to contain " + to_string(val);
}

void CheckDoesntContainElement(const cuckoo_hashing_tests& cuckoo, int val) {
    if (cuckoo.contains(val))
        throw "Expected cuckoo to not contain " + to_string(val);
}

void CheckNumberElements(const cuckoo_hashing_tests& cuckoo, size_t expected_size) {
    if (cuckoo.size() != expected_size)
        throw "Cuckoo size is wrong: expected " + to_string(expected_size) + " got " + to_string(cuckoo.size());
}

void CheckCorrectNumberRehashes(specialized_hashing_function* hashing_table,
        int number_rehashes) {
    if (hashing_table->number_rehashes != number_rehashes)
        throw "Number of hashes incorrect, expected " + to_string(number_rehashes) +
            " got " + to_string(hashing_table->number_rehashes);
}

void SimpleInsertion() {
    cuckoo_hashing_tests cuckoo;

    cuckoo.insert(5);
    cuckoo.insert(6);
    cuckoo.insert(7);

    try {
        CheckContainsElement(cuckoo, 5);
        CheckContainsElement(cuckoo, 6);
        CheckContainsElement(cuckoo, 7);

        CheckDoesntContainElement(cuckoo, 4);
        CheckDoesntContainElement(cuckoo, 9);

        CheckNumberElements(cuckoo, 3);

        // Only the initial setup when creating the object
        CheckNumberResize(cuckoo, 1);
        CheckMinNumberRehash(cuckoo, 1);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionNoResizeNecessary: " << s << '\n';
        throw s;
    }
}

void InsertionWithCollisions() {
    // 0 and 1 will collide, so 0 will be placed into other table.
    cuckoo_hashing_tests cuckoo{
        std::unique_ptr<hashing_function<int>>(new specialized_hashing_function{
            std::map<int, int>{{0, 0}, {1, 0}}}),
        std::unique_ptr<hashing_function<int>>(new specialized_hashing_function{
            std::map<int, int>{{0, 5}, {1, 2}}})
    };

    cuckoo.insert(0);
    cuckoo.insert(1);

    try {
        CheckContainsElement(cuckoo, 0);
        CheckContainsElement(cuckoo, 1);

        CheckDoesntContainElement(cuckoo, 2);

        CheckNumberElements(cuckoo, 2);

        // Only the initial setup when creating the object
        CheckNumberResize(cuckoo, 1);
        CheckNumberRehash(cuckoo, 1);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionWithCollisions: " << s << '\n';
        throw s;
    }
}

void InsertionForcedRehash() {
    // 0, 1, and 2 will collide an infinite number of times, so will rehash.
    // 2 can then be placed into table 0 pos 1.
    wrapped_specialized_hashing_function wrapped_hash_1{
        new specialized_hashing_function{
            std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
            std::map<int, int>{{0, 0}, {1, 0}, {2, 1}}}};
    wrapped_specialized_hashing_function wrapped_hash_2{
        new specialized_hashing_function{
            std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
            std::map<int, int>{{0, 0}, {1, 0}, {2, 0}}}};
    specialized_hashing_function* actual_hash_1 = wrapped_hash_1.get();
    specialized_hashing_function* actual_hash_2 = wrapped_hash_2.get();
    cuckoo_hashing_tests cuckoo{
        std::move(wrapped_hash_1),
        std::move(wrapped_hash_2)};

    cuckoo.insert(0);
    cuckoo.insert(1);
    cuckoo.insert(2);

    try {
        CheckCorrectNumberRehashes(actual_hash_1, 1);
        CheckCorrectNumberRehashes(actual_hash_2, 1);

        CheckContainsElement(cuckoo, 0);
        CheckContainsElement(cuckoo, 1);
        CheckContainsElement(cuckoo, 2);

        CheckDoesntContainElement(cuckoo, 3);

        CheckNumberElements(cuckoo, 3);

        // Initial setup when creating the object, and an additional rehash
        CheckNumberResize(cuckoo, 1);
        CheckNumberRehash(cuckoo, 2);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionForcedRehash: " << s << '\n';
        throw s;
    }
}

void InsertionForcedMultipleRehash() {
    // 0, 1, and 2 will collide an infinite number of times, in first two hash
    // choices, so will need to rehash multiple times.
    wrapped_specialized_hashing_function wrapped_hash_1{
        new specialized_hashing_function{
            std::vector<std::map<int, int>>{
                std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
                std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
                std::map<int, int>{{0, 0}, {1, 0}, {2, 1}}}}};
    wrapped_specialized_hashing_function wrapped_hash_2{
        new specialized_hashing_function{
            std::vector<std::map<int, int>>{
                std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
                std::map<int, int>{{0, 0}, {1, 0}, {2, 0}},
                std::map<int, int>{{0, 0}, {1, 0}, {2, 1}}}}};
    specialized_hashing_function* actual_hash_1 = wrapped_hash_1.get();
    specialized_hashing_function* actual_hash_2 = wrapped_hash_2.get();
    cuckoo_hashing_tests cuckoo{
        std::move(wrapped_hash_1),
        std::move(wrapped_hash_2)};

    cuckoo.insert(0);
    cuckoo.insert(1);
    cuckoo.insert(2);

    try {
        CheckCorrectNumberRehashes(actual_hash_1, 2);
        CheckCorrectNumberRehashes(actual_hash_2, 2);

        CheckContainsElement(cuckoo, 0);
        CheckContainsElement(cuckoo, 1);
        CheckContainsElement(cuckoo, 2);

        CheckDoesntContainElement(cuckoo, 3);

        CheckNumberElements(cuckoo, 3);

        // Initial setup when creating the object, and a two additional rehashes
        CheckNumberResize(cuckoo, 1);
        CheckNumberRehash(cuckoo, 3);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionForcedMultipleRehash: " << s << '\n';
        throw s;
    }
}

void InsertionAlreadyContainsItem() {
    // Will only be inserting 0, and want to make sure it only contains it once.
    cuckoo_hashing_tests cuckoo{
        std::unique_ptr<hashing_function<int>>(new specialized_hashing_function{
            std::map<int, int>{{0, 0}}}),
        std::unique_ptr<hashing_function<int>>(new specialized_hashing_function{
            std::map<int, int>{{0, 0}}})
    };

    cuckoo.insert(0);
    cuckoo.insert(0);
    // At this point, would get an infinite loop if doesn't handle 0 properly.
    cuckoo.insert(0);

    try {
        CheckContainsElement(cuckoo, 0);
        CheckNumberElements(cuckoo, 1);
        cuckoo.assert_is_valid();

        cuckoo.remove(0);

        CheckDoesntContainElement(cuckoo, 0);
        CheckNumberElements(cuckoo, 0);

        // Initial setup when creating the object
        CheckNumberResize(cuckoo, 1);
        CheckNumberRehash(cuckoo, 1);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionAlreadyContainsItem: " << s << '\n';
        throw s;
    }
}

void InsertionForceTableResize() {
    cuckoo_hashing_tests cuckoo;

    int to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();

    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    try {
        for (int i = 0; i < to_insert; ++i) {
            CheckContainsElement(cuckoo, i);
        }

        CheckDoesntContainElement(cuckoo, to_insert);

        CheckNumberElements(cuckoo, to_insert);

        // Initial setup when creating the object, and an additional resize + rehash
        CheckNumberResize(cuckoo, 2);
        // May have been extra rehashes for a bad hashing function.
        CheckMinNumberRehash(cuckoo, 2);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in InsertionForceTableResize: " << s << '\n';
        throw s;
    }


}

void RemoveItemSimple() {
    cuckoo_hashing_tests cuckoo;

    cuckoo.insert(0);
    cuckoo.insert(3);
    cuckoo.insert(5);

    cuckoo.remove(3);

    try {
        CheckContainsElement(cuckoo, 0);
        CheckContainsElement(cuckoo, 5);

        CheckDoesntContainElement(cuckoo, 3);

        CheckNumberElements(cuckoo, 2);

        // Initial setup when creating the object.
        CheckNumberResize(cuckoo, 1);
        // Could have had additional rehashes due to bad hash functions chosen.
        CheckMinNumberRehash(cuckoo, 1);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveItemSimple: " << s << '\n';
        throw s;
    }
}

void RemoveItemsWhenHadManyBefore() {
    cuckoo_hashing_tests cuckoo;

    int to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();

    // Insert enough items to force it to resize.
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }


    int to_remove = cuckoo.get_number_removes_required_to_decrease_tablesize();
    // Remove enough items to force it to decrease in size.
    for (int i = 0; i < to_remove; ++i) {
        cuckoo.remove(i);
    }

    try {
        for (int i = 0; i < to_remove; ++i) {
            CheckDoesntContainElement(cuckoo, i);
        }
        for (int i = to_remove; i < to_insert; ++i) {
            CheckContainsElement(cuckoo, i);
        }


        CheckNumberElements(cuckoo, to_insert - to_remove);

        // Initial setup when creating the object, additional increase
        // additional decrease.
        CheckNumberResize(cuckoo, 3);
        // Could have had additional rehashes due to bad hash functions chosen.
        CheckMinNumberRehash(cuckoo, 3);

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveItemsWhenHadManyBefore: " << s << '\n';
        throw s;
    }
}

void RemoveAllItemsWhenHadMany() {
    cuckoo_hashing_tests cuckoo;

    int to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    // Increase size once more
    to_insert += cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    // By this point, will have done two extra resizes.

    // Now, remove all elements
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.remove(i);
    }

    // Check that everything is not included
    try {
        for (int i = 0; i < to_insert; ++i) {
            CheckDoesntContainElement(cuckoo, i);
        }

        CheckNumberElements(cuckoo, 0);

        // Don't know how many resizes + rehashes there were.

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveAllItemsWhenHadMany: " << s << '\n';
        throw s;
    }


    // Now, add back some more elements to force it to resize again.
    to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    try {
        for (int i = 0; i < to_insert; ++i) {
            CheckContainsElement(cuckoo, i);
        }

        CheckNumberElements(cuckoo, to_insert);

        // Don't know how many resizes + rehashes there were.

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveAllItemsWhenHadMany: " << s << '\n';
        throw s;
    }


}

// Exact same as RemoveAllItemsWhenHadMany, but it had failed in past when had an eps of 0.4,
// so added this test to check for that case
void RemoveAllItemsWhenHadManyDifferentEps() {
    cuckoo_hashing_tests cuckoo(/*eps=*/0.4);

    int to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    // Increase size once more
    to_insert += cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    try {
        cuckoo.assert_is_valid();

    } catch (std::string& s) {
        std::cout << "Error in RemoveAllItemsWhenHadManyDifferentEps Inserts: " << s << '\n';
        throw s;
    }

    // By this point, will have done two extra resizes.

    // Now, remove all elements
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.remove(i);
    }

    // Check that everything is not included
    try {
        cuckoo.assert_is_valid();

        for (int i = 0; i < to_insert; ++i) {
            CheckDoesntContainElement(cuckoo, i);
        }

        CheckNumberElements(cuckoo, 0);

        // Don't know how many resizes + rehashes there were.

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveAllItemsWhenHadManyDifferentEps First: " << s << '\n';
        throw s;
    }

    // Now, add back some more elements to force it to resize again.
    to_insert = cuckoo.get_number_inserts_required_to_increase_tablesize();
    for (int i = 0; i < to_insert; ++i) {
        cuckoo.insert(i);
    }

    try {
        for (int i = 0; i < to_insert; ++i) {
            CheckContainsElement(cuckoo, i);
        }

        CheckNumberElements(cuckoo, to_insert);

        // Don't know how many resizes + rehashes there were.

        cuckoo.assert_is_valid();
    } catch (std::string& s) {
        std::cout << "Error in RemoveAllItemsWhenHadManyDifferentEps Second: " << s << '\n';
        throw s;
    }


}



// Small class to make comparing Cuckoo vs unordered_set
class hash_wrapper {
public:
    virtual void insert(int item) = 0;

    virtual bool contains(int item) const = 0;

    virtual void remove(int item) = 0;

    virtual void check_valid() const = 0;
};

class cuckoo_wrapper : public hash_wrapper {
public:
    cuckoo_wrapper()
    {}

    cuckoo_wrapper(std::unique_ptr<hashing_function<int>> first_hash,
            std::unique_ptr<hashing_function<int>> second_hash)
        : cuckoo(std::move(first_hash),
                std::move(second_hash)) {
    }

    void insert(int item) override { cuckoo.insert(item); }
    
    bool contains(int item) const override { return cuckoo.contains(item); }

    void remove(int item) override { cuckoo.remove(item); }
    
    void check_valid() const override { cuckoo.assert_is_valid(); }

private:
    cuckoo_hashing_tests cuckoo;
};


class unordered_set_wrapper : public hash_wrapper {
public:
    void insert(int item) override { s.insert(item); }

    bool contains(int item) const override { return s.find(item) != s.end(); }

    void remove(int item) override { s.erase(item); }

    // Don't check anything
    void check_valid() const override {}

private:
    std::unordered_set<int> s;
};


milliseconds GetCurrentTime() {
    return std::chrono::duration_cast<milliseconds >(
            Time::now().time_since_epoch());
}

milliseconds RunLargeTest(hash_wrapper* hash) {
    milliseconds start_time = GetCurrentTime();

    for (int i = 0; i < NumElementsInserted; ++i) {
        hash->insert(i);
        if (!hash->contains(i))
            throw "Something funky happened, lost element " + to_string(i) + " during insertion";
        if (i % EveryIndexRemovedAfterInsert == 0)
            hash->remove(i);
    }

    // Search for all elements, to add time taken.
    for (int i = 0; i < NumElementsInserted; ++i) {
        // Wasn't supposed to be removed.
        if (i % EveryIndexRemovedAfterInsert != 0 && !hash->contains(i))
            throw "Hash didn't include " + to_string(i);
    }

    milliseconds operations_time = GetCurrentTime() - start_time;

    // Check it is valid. Don't count this time
    try {
        hash->check_valid();
    } catch (std::string& s) {
        std::cout << "Error in RunLargeTest: " << s << '\n';
        throw s;
    }


    // Then do further deletions.
    start_time = GetCurrentTime();


    operations_time += GetCurrentTime() - GetCurrentTime();

    // Check it is valid. Don't count this time
    try {
        hash->check_valid();
    } catch (std::string& s) {
        std::cout << "Error in RunLargeTest: " << s << '\n';
        throw s;
    }

    return operations_time;
}



int main() {
    SimpleInsertion();
    InsertionWithCollisions();
    InsertionForcedRehash();
    InsertionForcedMultipleRehash();
    InsertionAlreadyContainsItem();
    InsertionForceTableResize();

    RemoveItemSimple();
    RemoveItemsWhenHadManyBefore();
    RemoveAllItemsWhenHadMany();
    RemoveAllItemsWhenHadManyDifferentEps();

    cuckoo_wrapper cuckoo;
    milliseconds time_for_default_cuckoo =
        RunLargeTest(&cuckoo);

    unordered_set_wrapper unordered_set;
    milliseconds time_for_unordered_set =
        RunLargeTest(&unordered_set);

    std::cout << "Time for cuckoo: " << time_for_default_cuckoo.count() << " ms.\n" <<
        "Time for unordered_set: " << time_for_unordered_set.count() << "ms.\n";
}


