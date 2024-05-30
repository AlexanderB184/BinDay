#include "mmap.h"

#include <string.h>

void mmapinit(mmap* hashTable) {
  // for (size_t i = 0; i < MEMORY_MAP_MAX_BUCKETS; i++) {
  //   hashTable->buckets[i] = NULL;
  // }
  (void)memset(hashTable->buckets, 0, sizeof(hashTable->buckets));
  hashTable->size = 0;
}

void mmapclear(mmap* hashTable) {
  // for (size_t i = 0; i < MEMORY_MAP_MAX_BUCKETS; i++) {
  //   hashTable->buckets[i] = NULL;
  // }
  (void)memset(hashTable->buckets, 0, sizeof(hashTable->buckets));
  hashTable->size = 0;
}

void mmapinsert(mmap* hashTable, void* old_address, void* new_address) {
  int index = (size_t)old_address % MEMORY_MAP_MAX_BUCKETS;
  hashTable->buffer[hashTable->size] =
      (mmap_item){.old_address = old_address,
                  .new_address = new_address,
                  .next = hashTable->buckets[index]};
  hashTable->buckets[index] = &hashTable->buffer[hashTable->size];
  hashTable->size++;
}

void* mmaplookup(const mmap* hashTable, void* old_address) {
  size_t index = (size_t)old_address % MEMORY_MAP_MAX_BUCKETS;
  mmap_item* currentNode = hashTable->buckets[index];
  while (currentNode) {
    if (currentNode->old_address == old_address) {
      return currentNode->new_address;
    }
    currentNode = currentNode->next;
  }
  return NULL;
}
