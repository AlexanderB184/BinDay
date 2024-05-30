#pragma once

#include "gc.h"

// Hash Table Item
// An Item for a Hash Table with seperate chaining
typedef struct mmap_item {
  struct gc_item* old_address;
  struct gc_item* new_address;
  struct mmap_item* next;
} mmap_item;

// Hash Table
// A Hash Table with seperate chaining
// Stores items in a bump allocator stored at "buffer"
// essentially two seperate parts, a vector storing items
// and a hash table which refers to items in the vector
typedef struct mmap {
  struct mmap_item* buckets[MEMORY_MAP_MAX_BUCKETS];
  struct mmap_item buffer[MEMORY_MAP_CAPACITY];
  size_t size;
} mmap;

// Creates an empty hash table with the specified capacity
void mmapinit(mmap* hashTable);

// Deletes a hash table and frees its memory
void mmapfree(mmap* hashTable);

// Clears the items from a hash table, does not free memory
void mmapclear(mmap* hashTable);

// Inserts a key-item pair into the hash table
void mmapinsert(mmap* hashTable, void* old_address,
                void* new_address);

// Searches for a key in the hash table, returns a pointer to the corresponding
// item, if the key does not exist in the table it returns NULL
void* mmaplookup(const mmap* hashTable,
                           void* old_address);

// hash function
int hash(void* x);
