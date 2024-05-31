#include "mem_page.h"

mem_page* new_page(size_t min_data) {
  size_t size = min_data; // TODO: Align size with page size
  mem_page* page = malloc(size);
  memset(page, 0, sizeof(*page));
  *page = (mem_page){.next_page = NULL, .size = size - sizeof(mem_page)};
  return page;
}

void delete_page(mem_page* page) { free(page); }