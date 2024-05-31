#pragma once

#include "gc.h"

typedef unsigned char byte;

typedef struct mem_page mem_page;

struct mem_page {
  mem_page* next_page;
  size_t size;
  byte data[];
};



#define iterate_mem_blocks(collector, page, iter)      \
  for (mem_page* page = (collector)->first_page; page; \
       page = (page)->next_page)                       \
    for (size_t iter = 0; iter < (page)->size;    \
         iter += ((mem_block*)&(page)->data[iter])->size)

mem_page* new_page(size_t min_data);

void delete_page(mem_page*);