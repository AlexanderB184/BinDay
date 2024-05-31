#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "root_list.h"

typedef unsigned char byte;

#define MEMORY_MAP_CAPACITY 0x1000
#define MEMORY_MAP_MAX_BUCKETS MEMORY_MAP_CAPACITY * 13 / 10
#define GC_STACK_SIZE 0x1000
#define GC_MAX_MEMORY 0x8000
#define GC_ROOT_LIST_SIZE 0x1000

typedef volatile void* external_ref_t;

typedef struct root_list_t root_list_t;

struct root_list_t {
  external_ref_t* list_of_pointers;
  size_t count;
  size_t capacity;
};

typedef enum block_type block_type;

enum block_type { mem_avail, mem_used };

typedef struct gc_head gc_head;

struct gc_head {
  size_t nptrs;
  int mark;
};

typedef struct mem_block mem_block;

struct mem_block {
  size_t size;
  block_type type;
  union {
    mem_block* next_free;
    gc_head header;
  } contents;
  byte data[];
};

/*

// future interface should change to this

struct mem_block {
  size_t size;
  block_type type;
  union {
    mem_block* next_free;
    struct gc_head {
      type_t data_type; // memory block acts as an array of type data_type
      size_t amount;
    } header;
  } contents;
  byte data[];
};

typedef size_t type_t;

struct type_obj {
  size_t pointer_count;
  size_t pointer_indices[];
};

type_obj* types_list;

type_info = types_list[data_type];

*/

typedef struct garbage_collector_t garbage_collector_t;

struct garbage_collector_t {
  void* memory;
  size_t capacity;
  mem_block* free_list_head;
  root_list_t root_list;
  int isactive;
  int mark;
};

extern garbage_collector_t garbage_collector;

void init();

void gc();

// SAFETY:
// CANNOT SAVE POINTER DIRECTLY INTO OBJECT, MUST SAVE IT IN A INTERMEDIATE
// LOCATION THEN MOVE THIS IS BECAUSE GC MAY BE CALLED DURING ALLOC SO THE
// TARGET BEING SAVED TO CAN MOVE AND CAUSE THE POINTER TO ALLOCATED MEMORY TO
// BE SAVED AT A RANDOM LOCATION.
// IT IS RECOMMENDED TO USE "alloc_into_object", "alloc_into_root", OR
// "alloc_into_attr" INSTEAD.
external_ref_t alloc(size_t, size_t);

#define get_ref(root) (*item_from_root(root))

#define get_val(type, root) (*(type*)get_ref(root))

void alloc_into_object(external_ref_t, size_t, size_t, size_t);

void alloc_into_root(size_t, size_t, size_t);

#define alloc_into_attr(type, attr, ptr, sz, nptrs) \
  alloc_into_object((ptr), offsetof(type, attr), (sz), (nptrs))
  