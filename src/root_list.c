#include "root_list.h"

size_t add_roots(garbage_collector_t* collector, size_t count) {
  if (collector->root_list.count + count >= collector->root_list.capacity) {
    collector->root_list.capacity =
        (collector->root_list.count + count) * 3 / 2;
    collector->root_list.list_of_pointers = realloc(
        collector->root_list.list_of_pointers, collector->root_list.capacity);
  }

  memset(collector->root_list.list_of_pointers + collector->root_list.count, 0,
         count * sizeof(mem_block*));
  size_t root = collector->root_list.count;
  collector->root_list.count += count;
  return root;
}

void remove_roots(garbage_collector_t* collector, size_t count) {
  collector->root_list.count -= count;
}

void set_roots_back(garbage_collector_t* collector, size_t count) {
  collector->root_list.count = count;
}

size_t get_root_count(garbage_collector_t* collector) {
  return collector->root_list.count;
}

external_ref_t* item_from_root(garbage_collector_t* collector, size_t root) {
  return &(collector->root_list.list_of_pointers[root]);
}