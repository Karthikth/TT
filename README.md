# TT
TT Coding Problem
/*******************************************************************************
 *                                  README
 *******************************************************************************
 * LRUCache Implementation: LRUCache.cpp
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
