#include "../src/gc.h"

int main(int argc, char const* argv[]) {
  garbage_collector_t collector;

  init(&collector, 4000);

  size_t r = add_roots(&collector, 1);

  alloc_into_root(&collector, r, sizeof(int*), 1);

  collect(&collector);

  alloc_into_object(&collector, get_ref(&collector, r), 0, sizeof(int), 0);

  collect(&collector);

  *get_val(&collector, int*, r) = 5;

  collect(&collector);

  printf("number: %d\n", *get_val(&collector, int*, r));

  remove_roots(&collector, 1);

  collect(&collector);
  
  release(&collector);

  return 0;
}