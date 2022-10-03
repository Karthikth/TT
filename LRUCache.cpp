/*******************************************************************************
 *                                  README
 *******************************************************************************
 * LRUCache Implementation:
 *
 * This file contains LRUCache class as well as the underlying implementations.
 *
 * The implementation adheres to the LRU cache constaints as follows:
 *   1. LRUCache(int capacity):
 *          Initializes the LRU cache with positive size capacity.
 *   2. int get(int key):
 *          Returns the value of the key if the key exists, otherwise returns -1.
 *   3. void put(int key, int value):
 *          Updates the value of the key if the key exists. Otherwise, adds the 
 *      key-value pair to the cache. If the number of keys exceeds the capacity
 *      from this operation, evicts the least recently used key from the cache.
 *
 * Exception Safety:
 *   1. LRUCache(int capacity): When initialised with negative size, it throws
 *      exception of type InvalidCapacity that is derived from std::exception
 *   2. int get(int key): Doesn't throw
 *   3. void put(int key, int value): May throw a bad_alloc, but, leaves the 
 *      underlying data structures in the previous stable state.
 *
 * Thread-Safety: Not added. Can be incorporated easily as and when needed.
 *
 * Usage:
 *   1. This code can be run from any online C++ compiler or by generating a
 *      binary output file by using a stand-alone compiler
 *   2. If the implementation of LRUCache is to be linked with other, just
 *      comment out the main function at the end of the file and go ahead
 *
 * Refactoring:
 *      This file can be refactored into multiple header and implementation
 * files by providing a makefile to build the binary as needed. Need local setup
 *
 * Input: It takes an optional input for the capacity, defaulting to 2 otherwise
 *
 * Output when run as such:
 *   It will outpt the results of the set of pre-defined test cases as written
 * in the TEST_* functions at the bottom of the file, just above the main.
 * They can be tweaked to add/remove more cases and the needed TEST_* functions
 * can be commented/uncommented in the run_and_test_lru_cache_impl() function
 * just above the main function at the bottom.
 *******************************************************************************/
#ifndef _LRUCACHE_H_
#define _LRUCACHE_H_ 1

#include <iostream>
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <chrono>
#include <memory>

using namespace std;

constexpr auto KEY_NOT_FOUND_RET_VAL = -1;

// A simple Node struct for Two Way linked list implementation
// A combination of shared_ptr and weak_ptr could have been used
// for prev/next pointers. But felt it could be an overkill given the
// no. of copies to be made when Nodes are moved around
struct TwoWayListNode
{
    int _key{0};
    int _value{0};
    TwoWayListNode *_prev{nullptr};
    TwoWayListNode *_next{nullptr};

    TwoWayListNode(int key, int value) noexcept
     : _key(key),
       _value(value),
       _prev(nullptr),
       _next(nullptr)
    { }
};

// A Custom-made doubly linked list that provides the operations needed
// for implementing the LRU cache constrains
// It either adds a new Node to the front, or rolls over the found Node
// to the front, such that:
//   The front of the list is MRU - i.e. the most recently used
//   The back of the list is LRU - i.e. the least recently used 
class LRUTwoWayList
{
    size_t _size{0};

    TwoWayListNode *_front{nullptr}; // head of the Two Way list
    TwoWayListNode *_back{nullptr}; // tail of the Two Way list

    // if either the front or back is empty, the list is empty
    constexpr bool empty() const noexcept
    {
        return nullptr == _back;
    }

  public:
    explicit LRUTwoWayList() noexcept
     : _front(nullptr),
       _back(nullptr)
    { }

    // Allocates a new node and adds to the front of the list and returns the same
    TwoWayListNode* add_to_front(int key, int value)
    {
        TwoWayListNode *new_node = new TwoWayListNode(key, value);

        // At the very begining - case of the first node being added
        if ( (nullptr == _front) && (nullptr == _back) )
        {
            _front = _back = new_node;
        }
        else // for subsequent adds, push the current front behind the newly added Node
        {
            new_node->_next = _front;
            _front->_prev = new_node;
            _front = new_node; // and the new node is now front
        }

        ++_size; // housekeep the no. of items in the two way list

        return new_node; // return the new Node so it can be added to the map
    }

    // Move the given node to the front by making necessary re-links
    void move_to_front(TwoWayListNode *given_node)
    {
        if ( _front == given_node ) // already at the front, so no-op
        {
            return;
        }

        if ( _back == given_node) // if back node to be moved, curtail it to it's prev
        {
            _back = _back->_prev;
            _back->_next = nullptr;
        }
        else // unplug an internal node and link it's neighbours to be the adjacents
        {
            given_node->_prev->_next = given_node->_next;
            given_node->_next->_prev = given_node->_prev;
        }

        // do the necessary for making the given node as front
        given_node->_next = _front;
        given_node->_prev = nullptr;
        _front->_prev = given_node;
        _front = given_node;
    }

    constexpr TwoWayListNode* front() const noexcept
    {
        return _front;
    }

    constexpr TwoWayListNode* back() const noexcept
    {
        return _back;
    }
    
    constexpr size_t size() const noexcept
    {
        return _size;
    }

    // just our operator friend to write to cout for testing output
    friend ostream& operator<< (ostream& os, const LRUTwoWayList& list);
};

// Iterate over the list from front to back and write to output stream
ostream& operator<< (ostream& os, const LRUTwoWayList& list)
{
    os << "{";

    auto iter = list.front(); // iterator-like only
    // To make this loop range based, an iterator for our custom list could be implemented
    for ( ; (iter != nullptr && iter->_next != nullptr); iter = iter->_next  )
    {
        os << iter->_key << "=" << iter->_value << ", ";
    }

    if ( iter != nullptr )
        os << iter->_key << "=" << iter->_value;

    return os << "}";
}

// For throwing when the LRUCache's capacity is initialised with a negative size
class InvalidCapacity : public std::exception
{
	virtual const char* what() const noexcept override
	{
		return "LRUCache's capacity has to be initialised with a positive value";
	}
} InvalidCapacityException;

/* Attempt at trying to use pimpl idiom pattern
 * or rather pointer to an absract interface can be used to provide polymorphic behavior
 * with the varying concrete implementation of the underlying data structures
class LRUCache
{
    class LRUCacheImpl;
    std::unique_ptr<LRUCacheImpl> uptr_to_lru_impl;

  public:
    ~LRUCache();
    explicit LRUCache(int capacity) throw();
    {
        if ( 0 >= capacity )
        {
            throw InvalidCapacityException;
        }
        
        uptr_to_lru_impl.reset(new LRUCacheImpl(capacity));
    }

    int get(int key) noexcept
    {
        return uptr_to_lru_impl->get(key);
    }
    
    void put(int key, int value)
    {
        uptr_to_lru_impl->put(key, value);
    }

    friend ostream& operator<< (ostream& os, const LRUCache& cache);
};

ostream& operator<< (ostream& os, const LRUCache& cache)
{
    os << *(cache.uptr_to_lru_impl) << endl;
} */

// LRUCache implementation using the LRUTwoWayList defined above and unordered_map 
// hash container from STL as dictionary for storing the <key,Node(key,value)> pair
// The LRUTwoWayList and the TwoWayListNode could as well be nested within this 
// class in order to make it more encapsulated - however, left it this way for now 
class LRUCache
{
    int _capacity{-1};
    LRUTwoWayList _lru_list; // our custom two way list that contains MRU to LRU
    // unordered hash map to hold the key and the corresponding Node in the list
    // Again shared_ptr could have been used here, skipped for now
    std::unordered_map<int, TwoWayListNode*> _lru_cache_map;

  public:
    explicit LRUCache(int capacity)
     : _capacity(capacity)
    {
        if ( 0 >= capacity )
        {
            throw InvalidCapacityException; // capacity cannot be negative
        }
    }

    // Returns the value for the key, if the key exists. otherwise, returns -1.
    // While doing so, moves the Node of the found key to the front of the list
    // thus making it the MRU
    int get(int key) noexcept
    {
        auto const iter = _lru_cache_map.find(key); // find the key in the map

        if (  iter == _lru_cache_map.end() ) // when the key is not found in the map
        {
            return KEY_NOT_FOUND_RET_VAL; // return -1
        }

        // When the key is found, pass the associated Node to the list to make it front
        _lru_list.move_to_front(iter->second);

        return iter->second->_value; // return the value from the node
    }

    // Updates the value of the key, if it already exists in the map.
    // If the key doesn't exist in the map, it adds the key and makes it the MRU
    // when the set capacity hasn't been reached yet
    // If the set capacity has been reached, evicts the LRU key from map, and then
    // adds the new key and value (i.e. Node) to map and makes the Node as front
    void put(int key, int value)
    {
        auto iter = _lru_cache_map.find(key); // find the key in the map

        if ( iter != _lru_cache_map.end() ) // when the key is found in the map
        {
            iter->second->_value = value; // just update the value
            _lru_list.move_to_front(iter->second); // make the corresponding node MRU in the list
            return;
        }

        TwoWayListNode* new_node{nullptr}; // to capture the new node to add to the map later

        if ( _lru_list.size() == _capacity ) // when the size has reached the capacity limits
        {
            new_node = _lru_list.back(); // get the Node at the back, which will be the LRU
            _lru_cache_map.erase(new_node->_key); //remove the key from the map, but retain the node
            new_node->_key = key; // update the new key to the removed node 
            // set the corresponding value for the new key in the Node (reused)
            // deleting the old value may be needed here, when the value type is a pointer
            new_node->_value = value;
            _lru_list.move_to_front(new_node); // make the new node the MRU in the list
        }
        else // when the key not found and the size has not reached the capacity limits
        {
            try // try adding a new node for the key to the front of the list
            {
                new_node = _lru_list.add_to_front(key, value);
            }
            catch(std::exception &except) // bad_alloc may be thrown, catch and propagate
            {
                cout << "LRUCache::put - add_to_front threw: " << except.what() << endl;
                throw except;
            }
            catch(...)
            {
                cout << "LRUCache::put - Unknown exception" << endl;
                throw;
            }
        }

        _lru_cache_map[key] = new_node; // add the key and it's corresponding node to the map
    }

    constexpr size_t capacity() const noexcept
    {
        return _capacity;
    }

    constexpr size_t size() const noexcept
    {
        return _lru_list.size(); // return the size of the underlying list    
    }

    void clear() noexcept
    {
        for ( auto const &iter : _lru_cache_map ) // iterate through the map
        {
            delete iter.second; // clean all the Nodes
        }

        _lru_cache_map.clear(); // clear the map
    }

    ~LRUCache()
    {
        this->clear();
    }

    // again, to write the internal state to the output stream to check results
    friend ostream& operator<< (ostream& os, const LRUCache& cache);
};

// just call out to the underlying list to do the job
ostream& operator<< (ostream& os, const LRUCache& cache)
{
    return os << cache._lru_list;
/*
    os << "{";

    for ( auto const &iter : cache._lru_cache_map  )
    {
        os << "[" << iter.first << "=" << iter.second->_value << "], ";
    }

    return os << "}";
*/
}

// A decorator-like testing class to test the LRUCache
class LoggedOrTimedOpsTester
{
    LRUCache& _cache;

  public:
    explicit LoggedOrTimedOpsTester(LRUCache& cache) noexcept
     : _cache(cache) // glue it to the given LRUCache object
    { }

    // logs the internal state of the cache before and after the call to put
    void logged_put(int key, int value)
    {
        //cout << "Before:\t" << _cache << endl;
        _cache.put(key, value);
        log_capacity();
        cout << "After Put(" << key << "," << value << "):\t" << _cache << endl;
    }

    // logs the internal state of the cache before and after the call to get
    int logged_get(int key) noexcept
    {
        //cout << "Before:\t" << _cache << endl;
        int value = _cache.get(key);
        cout << "Get(" << key << ") Returned:\t" << value << endl;
        log_capacity();
        cout << "After Get(" << key << "):\t" << _cache << endl;
        return value;
    }

    // Measures the time taken for a put opertion
    void timed_put(int key, int value)
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        _cache.put(key, value);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        log_capacity();
        cout << "Time Taken for Put(" << key << "," << value << ") is:\t" << time_taken.count() << " ns" << endl;
    }

    // Measures the time taken for a get opertion
    int timed_get(int key) noexcept
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        int value = _cache.get(key);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        log_capacity();
        cout << "Time Taken for Get(" << key << ") that Returned " << value << " is:\t" << time_taken.count() << " ns" << endl;
        return value;
    }

    inline void log_capacity() const noexcept
    {
        cout << "LRUCache(" << _cache.capacity() << "): ";
    }
    
    inline void log_initial_cache() const noexcept
    {
        log_capacity();
        cout << "Initial Cache:\t" << _cache << endl;;
    }

    void populate_to_capacity() // populate the cache to it's full capacity for load test
    {
        for ( int i=1; i <= _cache.capacity(); ++i )
            _cache.put(i, i);
        assert( _cache.size() == _cache.capacity() );
        cout << "LRUCache(" << _cache.capacity() << "): ";
        cout << "Size is now at: " << _cache.size() << endl;
    }

    // Measures the time taken for a set of put and get operations for the given load no. of times
    void time_test_load(int load)
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < load; ++i)
        {
            _cache.put(i, i);
            _cache.get(i % (load / 50));
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        assert( _cache.size() == _cache.capacity() );
        log_capacity();
        cout << "Time Taken for Put and Get for " << load << " times is:\t" << time_taken.count() << " ms" << endl;
    }
};

// Test the LRUCache with logging
void TEST_LOGGED(LRUCache& cache)
{
    LoggedOrTimedOpsTester tester(cache);

    cout << "\nTEST_LOGGED:" << endl;

    tester.log_initial_cache();

    tester.logged_put(1, 1); // cache is {1=1}
    tester.logged_put(2, 2); // cache is {1=1, 2=2}
    tester.logged_get(1);    // return 1
    tester.logged_put(3, 3); // LRU key was 2, evicts key 2, cache is {1=1, 3=3}
    tester.logged_get(2);    // returns -1 (not found)
    tester.logged_put(4, 4); // LRU key was 1, evicts key 1, cache is {4=4, 3=3}
    tester.logged_get(1);    // return -1 (not found)
    tester.logged_get(3);    // return 3
    tester.logged_get(4);    // return 4
}

// Test the LRUCache for the time taken for each operation and also load avg time
void TEST_TIMED_AND_LOADED(LRUCache& cache, int load)
{
    LoggedOrTimedOpsTester tester(cache);

    cout << "\nTEST_TIMED_AND_LOADED:" << endl;
 
    cout << "\nStarting with empty cache:" << endl;
    tester.timed_put(1,1);
    tester.timed_get(1);
    tester.timed_put(2,2);
    tester.timed_get(2);
    tester.timed_put(3,3);
    tester.timed_get(4);
    tester.timed_get(4);

    cout << "\nPopulate to full capacity and time the Get and Put:" << endl;
    tester.populate_to_capacity();
    tester.timed_put(-1,-2);
    tester.timed_get(-1);
    tester.timed_get(-2);
    tester.timed_put(1,2);

    cout << "\nLoad tested and timed for " << load << " times of Get and Put:" << endl;
    tester.time_test_load(load);
}

// Just to run and test the LRUCache, so as to keep the main short and sweet!
int run_and_test_lru_cache_impl()
{
    try
    {
        int capacity{2};
        cout << "\nEnter LRUCache's Capacity: ";
        cin >> capacity;
        
        auto sp_obj = std::make_shared<LRUCache>(capacity);
        TEST_LOGGED(*sp_obj);
        TEST_TIMED_AND_LOADED(*sp_obj, 10000);
    }
    catch(std::exception &excp)
    {
        cout << "main: LRUCache threw: " << excp.what() << endl;
        return -1;
    }
    catch(...)
    {
        cout << "main: Unknown Exception" << endl;
        return -1;
    }

    return 0;
}

#endif // _LRUCACHE_H_

/*
int main()
{
    return run_and_test_lru_cache_impl();
} */