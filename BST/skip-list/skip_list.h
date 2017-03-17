#ifndef BST_SKIP_LIST
#define BST_SKIP_LIST

#include <cassert>
#include <iostream>
#include <random>

template <class T>
class skip_list {
public:
    skip_list();
    ~skip_list();

    // Does nothing if item already exists in tree.
    void insert(const T& item);

    // Does nothing if item is not in tree.
    void remove(const T& item);

    // Returns true iff item is in tree.
    bool find(const T& item) const;

    // Returns value of minimum item in tree.
    T minimum() const;

    size_t size() const { return num_elements; }

    void print_out(std::ostream& o = std::cout) const;

protected:
    struct Interval {
        Interval(const T& start, Interval* elementBelow)
            : start(start),
            left(nullptr),
            right(nullptr),
            elementBelow(elementBelow) {
        }

        const T start;
        // If nullptr, is smallest element at level.
        Interval* left;
        // If nullptr, is largest element at level.
        Interval* right;

        // If nullptr, is lowest level.
        Interval* elementBelow;
    };

    // First element at each level in tree.
    // index 0 is lowest, index size - 1 is highest level.
    std::vector<Interval*> start_at_level;

private:

    // Will first find where the element will go at current level, then call insert on lower level.
    // Returns nullptr if did not insert at current level, otherwise returns valid ptr.
    // IMPORTANT: interval MUST have start < item. If no intervals meet criteria at level, it should
    // be a nullptr.
    Interval* insert(Interval* interval, int level, const T& item);

    // Will first find where the element will go at current level, then call on lower level if element wasn't found
    // IMPORTANT: interval MUST have start < item. If no intervals meet criteria at level, it should
    // be a nullptr.
    bool find(Interval* interval, int level, const T& item) const;

    // Will first find where the element would be at current level, then remove lower levels.
    // IMPORTANT: interval MUST have start < item. If no intervals meet criteria at level, it should
    // be a nullptr.
    void remove(Interval* interval, int level, const T& item);

    // Will push interval along the level, until the next Interval has start > item.
    // May return nullptr, if no intervals at level have start <= item.
    Interval* advance_interval_along_level(
        Interval* interval, int level, const T& item) const;

    // Will return new interval.
    // Level argument must be non-negative.
    Interval* insert_item_after_interval_in_level(
            const T& item, Interval* interval, int level, Interval* child);

    void delete_level(Interval* interval);

    // Code to use for random from
    // http://stackoverflow.com/questions/19665818/generate-random-numbers-using-c11-random-library
    std::default_random_engine generator;
    std::uniform_int_distribution<int> dist;
    const static int INSERT = 0;

    size_t num_elements;
};

template <class T>
skip_list<T>::skip_list()
     : generator(std::random_device()()),
     dist(0, 1),
     num_elements(0) {
}

template <class T>
skip_list<T>::~skip_list() {
    for (Interval* interval : start_at_level) {
        delete_level(interval);
    }
}

template <class T>
void skip_list<T>::insert(const T& item) {

    Interval* result;
    if (!start_at_level.empty()) {
        int top_level = start_at_level.size() - 1;
        result = insert(nullptr, top_level, item);
    } else {
        // Handle case where nothing has been added specially.
        ++num_elements;
        result = new Interval(item, nullptr);
        start_at_level.push_back(result);
    }

    // Only add additional levels if inserted item up until max level.
    if (result) {
        while (dist(generator) == INSERT) {
            result = new Interval(item, result);
            start_at_level.push_back(result);
        }

    }
    
}

template <class T>
typename skip_list<T>::Interval* skip_list<T>::insert(
        Interval* interval, int level, const T& item) {
    interval = advance_interval_along_level(interval, level, item);

    // Item has already been inserted, don't do anything.
    if (interval != nullptr && interval->start == item) {
        return nullptr;
    }

    // Reached base level, so don't want to recurse any farther, just insert here.
    if (level == 0) {
        ++num_elements;
        return insert_item_after_interval_in_level(item, interval, level, nullptr);
    }

    Interval* child =
        // Start on lower interval, using child of current interval if current interval exists.
        insert((interval != nullptr ?
                    interval->elementBelow : nullptr),
               level - 1, item);

    // Must have been inserted up in lower levels and pass coinflip for this level.
    if (child != nullptr && dist(generator) == INSERT) {
        return insert_item_after_interval_in_level(item, interval, level, child);
    }

    // Will not insert this item into higher levels.
    return nullptr;
}

// Will return new interval.
// Level argument must be non-negative.
template <class T>
typename skip_list<T>::Interval* skip_list<T>::insert_item_after_interval_in_level(
        const T& item, Interval* interval, int level, Interval* child) {
    Interval* new_interval = new Interval(item, child);

    new_interval->left = interval;

    // Merge into list, after interval.
    // Will update ptr for node left of new interval.
    if (interval != nullptr) {
        new_interval->right = interval->right;
        interval->right = new_interval;
    } else {
        new_interval->right = start_at_level[level];
        start_at_level[level] = new_interval;
    }

    // Update ptr for node right of new interval
    Interval* to_right = new_interval->right;
    if (to_right != nullptr) {
        to_right->left = new_interval;
    }
     
    return new_interval;
}

template <class T>
void skip_list<T>::remove(const T& item) {
    remove(nullptr, start_at_level.size() - 1, item);
}

template <class T>
void skip_list<T>::remove(Interval* interval, int level, const T& item) {
    if (level < 0) {
        return;
    }

    interval = advance_interval_along_level(interval, level, item);

    // Delete lower
    remove((interval != nullptr ?
                interval->elementBelow : nullptr),
           level - 1, item);


    // Remove this interval if it matches
    if (interval && interval->start == item) {
        if (level == 0) {
            --num_elements;
        }

        Interval* left = interval->left;
        Interval* right = interval->right;

        // Will not be any nodes in levels above either, so can remove a level.
        // Combined together, these removal of levels will ensure that no levels will be empty.
        if (left == nullptr && right == nullptr) {
            start_at_level.pop_back();
            return;
        }

        if (left != nullptr) {
            left->right = right;
        } else {
            // Need to update start at level
            start_at_level[level] = right;
        }

        if (right != nullptr) {
            right->left = left;
        }

        delete interval;
    }
}

template <class T>
bool skip_list<T>::find(const T& item) const {
    return find(nullptr, start_at_level.size() - 1, item);
}

template <class T>
bool skip_list<T>::find(Interval* interval, int level, const T& item) const {
    if (level < 0) {
        return false;
    }

    // Advance interval along level
    interval = advance_interval_along_level(interval, level, item);

    if (interval != nullptr && interval->start == item) {
        return true;
    }

    return find((interval != nullptr ?
                    interval->elementBelow : nullptr),
                level - 1, item);
}

template <class T>
typename skip_list<T>::Interval* skip_list<T>::advance_interval_along_level(
        Interval* interval, int level, const T& item) const {
    // Use interval.start <= item to ensure will advance interval to the item in level if it was already inserted.

    // Interval is before start of list. See if can move into part of list, or will stay in front.
    if (interval == nullptr && start_at_level[level]->start <= item) {
        interval = start_at_level[level];
    }

    if (interval != nullptr) {
        // Try to push along the intervals at this level until can't continue
        while (interval->right != nullptr && interval->right->start <= item) {
            interval = interval->right;
        }
    }
    return interval;
}



template <class T>
T skip_list<T>::minimum() const {
    assert(size() > 0);

    // The bottom row, first element start is minimum.
    return start_at_level[0]->start;
}

template <class T>
void skip_list<T>::delete_level(Interval* interval) {
    if (interval == nullptr) {
        return;
    }

    while (interval != nullptr) {
        Interval* next = interval->right;
        delete interval;
        interval = next;
    }
}

template <class T>
void skip_list<T>::print_out(std::ostream& o) const {
    o << "Printing out list from highest level to lowest:\n";
    int level = start_at_level.size() - 1;
    for (auto level_iterator = start_at_level.rbegin(); level_iterator != start_at_level.rend();
            ++level_iterator, --level) {
        o << "Level " << level << ":";
        for (Interval* interval = *level_iterator; interval != nullptr;
                interval = interval->right) {
            o << ' ' << interval->start;
        }
        o << '\n';
    }
}


#endif
