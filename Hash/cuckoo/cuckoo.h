#ifndef HASH_CUCKOO_H
#define HASH_CUCKOO_H


// Used just for  setting up the hashing function.
#include <chrono>
#include <thread>

#include <iostream>
#include <memory>
#include <random>

template <class T>
class hashing_function {
public:
    virtual ~hashing_function() { }

    virtual void reset_hash(int p) = 0;

    // Will bound the hash based on the earlier provided p.
    virtual int get_hash(const T& t) const = 0;
};

// Note: This is fairly simple hashing function, and so the cuckoo hashing would
// probably perform far better with a stronger hashing function.
template <class T>
class basic_hashing_function : public hashing_function<T> {
public:
    basic_hashing_function(std::mt19937 _rng, bool should_seed_rng)
        : rng(_rng) {
        if (should_seed_rng) {
            int seed = std::random_device{}();
            rng.seed(seed);
        }
    }

    void reset_hash(int _p) override {
        std::uniform_int_distribution<int> dist{0, _p};
        p = _p;
        a = dist(rng);
        b = dist(rng);
    }

    int get_hash(const T& t) const override {
        return (a * t + b) % p;
    }

    std::mt19937 rng;
    long long a;
    long long b;
    int p;
};

// Using eps of 0.4 seems to work quite well.
// Note that the variance on time taken is quite large, probably due to the hash function
// not being optimal.

// What is interesting is that, when the hash functions were the exact same (not seeded properly),
// the time for the large test was ~343 milliseconds.

// Some intuition on sizes (just increasing), with eps=0.5
//  table dim    min size     max size     max loop
//  10           0            6            18
//  32           2            21           27
//  76           7            50           33
//  164          17           109          39

// Note: Assumes that T can be converted to an numeric.
// Could be improved with a better hashing scheme, currently 
template <class T>
class cuckoo_hashing {
public:
    cuckoo_hashing(std::unique_ptr<hashing_function<T>> first_table,
        std::unique_ptr<hashing_function<T>> second_table,
        double eps=0.4);

    ~cuckoo_hashing();

    void insert(const T& item);

    bool contains(const T& item) const;

    void remove(const T& item);

    size_t size() const { return num_elements; }

    void print_out() const;

protected:
    // A further efficiency could be, for large classes,
    // to have this struct store a pointer to them instead.
    struct ItemOr {
        ItemOr()
            : contains_item(false)
        {}

        ItemOr(T item, bool contains_item)
            : item(item),
            contains_item(contains_item)
        {}

        T item;
        bool contains_item;
    };

    // Will ping-pong items between the two tables until either:
    //   1) item.contains_item = false, which means successfully inserted.
    //   2) reached max_loop iterations, in which case a rehash is required.
    void attempt_to_insert_item(ItemOr *item);

    void resize();
    size_t num_resize;
    // Will go over maximum of the current table_size and size_for_rehash.
    // Uses size_for_rehash for updating the two hashing functions.
    void rehash(size_t size_for_rehash);
    size_t num_rehash;

    // Leads to value for max_number_elements being based off of the table size.
    // Formula is table_size >= (1 + eps) * number_elements
    // If wanting 1/3 fullness, 0.5 is the desired level.
    double eps;

    // Maximum number of times can attempt to insert a key before a rehash.
    int max_loop;

    size_t num_elements;
    size_t max_number_elements;
    size_t min_number_elements;

    size_t num_insertions_without_rehash;

    size_t table_size;
    std::vector<ItemOr> tables[2];
    std::unique_ptr<hashing_function<T>> hashes[2];
};

template <class T>
cuckoo_hashing<T>::cuckoo_hashing(
        std::unique_ptr<hashing_function<T>> first_table,
        std::unique_ptr<hashing_function<T>> second_table,
        double eps)
        : num_resize(0),
        num_rehash(0),
        eps(eps),
        num_elements(0),
        max_number_elements(0),
        min_number_elements(0),
        num_insertions_without_rehash(0),
        table_size(0) {
    hashes[0] = std::move(first_table);
    hashes[1] = std::move(second_table);

    resize();
}

template <class T>
cuckoo_hashing<T>::~cuckoo_hashing() {}

template <class T>
void cuckoo_hashing<T>::resize() {
    ++num_resize;

    // Update table size. Factor of number of elements inserted and
    // a constant factor to ensure weird stuff doesn't happen when there is a
    // small # of elements.
    const size_t new_table_size =
        2 * std::ceil(size() * (1 + eps)) + 10;

    // NOTE: If these three functions are changed, will need to update the notes
    // before the declaration of cuckoo_hashing.
    // Ensure table_size >= (1 + eps) * number_elements
    max_number_elements = new_table_size / (1 + eps);
    // Don't let the table size get too empty.
    // At this point, could revert to what the table size used to be.
    min_number_elements = std::ceil(size() / (1 + eps)) / 2;

    // max_loop = 3 log1+ε(table_size)
    max_loop = 3 * std::ceil(log(new_table_size) / log(1 + eps));

    //std::cout << "Resize: " << new_table_size << ' ' << min_number_elements << ' '
    //    << max_number_elements << ' ' << max_loop << ' ' << size() << '\n';


    // Add the size to the tables now.
    if (new_table_size > table_size) {
        tables[0].resize(new_table_size);
        tables[1].resize(new_table_size);
    }

    // Do a rehash
    rehash(new_table_size);

    // Resize the tables if necessary.
    if (new_table_size < table_size) {
        tables[0].resize(new_table_size);
        tables[0].shrink_to_fit();
        tables[1].resize(new_table_size);
        tables[1].shrink_to_fit();
    }

    table_size = new_table_size;
}

template <class T>
void cuckoo_hashing<T>::rehash(size_t size_for_rehash) {
    ++num_rehash;

    // Reset the hashes for tables
    hashes[0]->reset_hash(size_for_rehash);
    hashes[1]->reset_hash(size_for_rehash);

    // Now, for each item that isn't in its correct table entry, attempt to re-insert it.
    // Only need to look at the initial table_size

    for (int table = 0; table < 2; ++table) {
        // Go through all elements in the current tables size.
        // May need to all elements in the tables, not just the stored size of the table.
        size_t larger_table_size = std::max(size_for_rehash, table_size);

        for (size_t index = 0; index < larger_table_size; ++index) {
            // Don't do any calculations if it doesn't have an entry.
            if (!tables[table][index].contains_item) {
                continue;
            }

            size_t hashed_index_in_table =
                hashes[table]->get_hash(tables[table][index].item);

            // Already in a valid index.
            if (hashed_index_in_table == index) {
                continue;
            }

            // Should be in a different spot!
            ItemOr item = tables[table][index];
            // Mark this current spot as not containing an element.
            tables[table][index].contains_item = false;

            // Now rehash this current item.
            attempt_to_insert_item(&item);

            // Didn't manage to place the item back in, so will quite this rehash.
            if (item.contains_item) {

                // If it wasn't successful, need to rehash everything.
                while (item.contains_item) {
                    rehash(size_for_rehash);
                    attempt_to_insert_item(&item);
                }

                // Don't bother any farther with the current re-hash attempt.
                return;
            }
        }
    }
}

template <class T>
void cuckoo_hashing<T>::insert(const T& item) {
    if (contains(item)) {
        return;
    }

    ++num_elements;

    if (num_elements > max_number_elements) {
        resize();
        num_insertions_without_rehash = 0;
    }

    ++num_insertions_without_rehash;
    if (num_insertions_without_rehash > table_size * table_size) {
        rehash(table_size);
        num_insertions_without_rehash = 1;
    }

    ItemOr current{item, true};

    attempt_to_insert_item(&current);

    // We exceeded max_loop, since otherwise it would have been changed to not contain a key.
    while (current.contains_item) {
        rehash(table_size);
        attempt_to_insert_item(&current);
        num_insertions_without_rehash = 0;
    }
}

// Will not update any counter variables. Those should be updated outside this function.
template <class T>
void cuckoo_hashing<T>::attempt_to_insert_item(ItemOr* current) {
    // Ensure that it considers itself to have an item.
    current->contains_item = true;

    // Always starts with the first table.
    int current_table = 0;

    for (int num_loops = 0; num_loops < max_loop && current->contains_item;
            ++num_loops, current_table = 1 - current_table) {
        // Try to put the item into the table.
        int index = hashes[current_table]->get_hash(current->item);

        // Put this itemor into the table. Will swap the entries, so will try to rehash
        // if necessary.
        std::swap(tables[current_table][index], *current);
    }
}


template <class T>
void cuckoo_hashing<T>::remove(const T& item) {
    if (!contains(item)) {
        return;
    }

    --num_elements;

    // Remove from the table.
    int first_index = hashes[0]->get_hash(item);
    if (tables[0][first_index].contains_item &&
            tables[0][first_index].item == item) {
        tables[0][first_index].contains_item = false;
    } else {
        int second_index = hashes[1]->get_hash(item);
        tables[1][second_index].contains_item = false;
    }

    // Resize table if necessary.
    if (num_elements < min_number_elements) {
        resize();
        num_insertions_without_rehash = 0;
    }
}

template <class T>
bool cuckoo_hashing<T>::contains(const T& item) const {
    int first_index = hashes[0]->get_hash(item);
    if (tables[0][first_index].contains_item &&
            tables[0][first_index].item == item)
        return true;

    int second_index = hashes[1]->get_hash(item);
    return tables[1][second_index].contains_item &&
        tables[1][second_index].item == item;
}

template <class T>
void cuckoo_hashing<T>::print_out() const {
    for (int i = 0; i < table_size; ++i) {
        std::cout << i << ": ";
        if (tables[0][i].contains_item) {
            std::cout << tables[0][i].item << " (alt: " << hashes[1]->get_hash(tables[0][i].item) << ")   ";
        } else {
            std::cout << "        ";
        }

        if (tables[1][i].contains_item) {
            std::cout << tables[1][i].item << " (alt: " << hashes[0]->get_hash(tables[1][i].item) << ")   ";
        } else {
            std::cout << "     ";
        }

        std::cout << '\n';
    }
}

#endif  // HASH_CUCKOO_H
