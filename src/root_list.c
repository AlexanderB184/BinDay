#include "root_list.h"

size_t add_roots(garbage_collector_t* gc, size_t count) {
  if (gc->root_list.count + count >=
      gc->root_list.capacity) {
    return 0;
  }

  memset(gc->root_list.list_of_pointers +
             gc->root_list.count,
         0, count * sizeof(mem_block*));
  size_t root = gc->root_list.count;
  gc->root_list.count += count;
  return root;
}

void remove_roots(garbage_collector_t* gc, size_t count) {
  gc->root_list.count -= count;
}

void set_roots_back(garbage_collector_t* gc, size_t count) {
  gc->root_list.count = count;
}

size_t get_root_count(garbage_collector_t* gc) {
  return gc->root_list.count;
}

external_ref_t* item_from_root(garbage_collector_t* gc, size_t root) {
  return &gc->root_list.list_of_pointers[root];
}