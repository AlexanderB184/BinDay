#include <stddef.h>

typedef size_t type_t;

typedef struct type_obj type_obj;

struct type_obj {
  size_t size;
  size_t pointer_count;
  size_t pointer_indices[];
};

typedef struct type_list type_list;

struct type_list {
  type_obj* items;
  size_t count, capacity;
};

type_list list = {0};

type_obj types[256];
