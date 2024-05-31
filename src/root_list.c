#include "root_list.h"

size_t add_roots(size_t count) {
  if (garbage_collector.root_list.count + count >=
      garbage_collector.root_list.capacity) {
    return 0;
  }

  memset(garbage_collector.root_list.list_of_pointers +
             garbage_collector.root_list.count,
         0, count * sizeof(mem_block*));
  size_t root = garbage_collector.root_list.count;
  garbage_collector.root_list.count += count;
  return root;
}

void remove_roots(size_t count) { garbage_collector.root_list.count -= count; }

void set_roots_back(size_t count) { garbage_collector.root_list.count = count; }

size_t get_root_count() { return garbage_collector.root_list.count; }

external_ref_t* item_from_root(size_t root) {
  return &garbage_collector.root_list.list_of_pointers[root];
}