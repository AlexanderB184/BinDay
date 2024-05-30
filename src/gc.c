
#include "gc.h"

#include <assert.h>
#include <stdio.h>

#include "mmap.h"

garbage_collector_t garbage_collector;

#define get_item_header(item) (mem_block*)((item) - offsetof(mem_block, data))

void recursive_mark();
void mark_object(void*);
void sweep();
void defragment();

void print_memory() {
  void* item;
  mem_block* header;
  size_t upto = 0;
  printf("current mark:%d\n", garbage_collector.mark);
  for (size_t root = 0; root < garbage_collector.root_list.count; root++) {
    printf("root: %zu points to: %p\n", root,
           garbage_collector.root_list.list_of_pointers[root]);
  }
  while (upto < garbage_collector.capacity) {
    header = (mem_block*)(garbage_collector.memory + upto);
    item = &header->data;

    size_t block_size = header->size;
    printf("%p: (size: %zu) used: %d ", item, block_size,
           header->type == mem_used);

    if (header->type == mem_used) {
      printf("mark: %d ", header->contents.header.mark);
      printf("points to: ");
      if (header->contents.header.nptrs > 10) {
        fprintf(stderr, "too many pointers in nptrs\n");
        return;
      }
      for (size_t i = 0; i < header->contents.header.nptrs; i++) {
        void** pptr = (void**)(item) + i;
        printf(" %p", *pptr);
      }
    }

    printf("\n");
    upto += block_size;
  }
}

void init() {
  static byte buffer[GC_MAX_MEMORY];
  static external_ref_t root_list[GC_ROOT_LIST_SIZE];

  garbage_collector = (garbage_collector_t){
      .memory = &buffer,
      .capacity = GC_MAX_MEMORY,
      .root_list =
          (root_list_t){.list_of_pointers = (external_ref_t*)&root_list,
                        .count = 0,
                        .capacity = GC_ROOT_LIST_SIZE},

      .isactive = 0,
      .mark = 0,
      .free_list_head = (mem_block*)&buffer};

  memset(garbage_collector.free_list_head, 0, sizeof(mem_block));

  *garbage_collector.free_list_head = (mem_block){
      .size = GC_MAX_MEMORY, .type = mem_avail, .contents.next_free = NULL};
}

void mark_object(void* object) {
  if (object == NULL) {
    return;
  }
  mem_block* header = get_item_header(object);

  if ((size_t)header < (size_t)garbage_collector.memory ||
      (size_t)header >=
          garbage_collector.capacity + (size_t)garbage_collector.memory) {
    fprintf(stderr, "ERROR: pointer points outside gc\n");
    return;
  }
  if (header->type == mem_avail) {
    fprintf(stderr, "ERROR: reference to freed memory\n");
    return;
  }
  if (header->type != mem_used) {
    fprintf(stderr, "ERROR: invalid mem_block type\n");
    return;
  }

  if (header->contents.header.mark == garbage_collector.mark) {
    return;
  }

  header->contents.header.mark = garbage_collector.mark;

  for (size_t i = 0;
       i < header->contents.header.nptrs && i < header->size / sizeof(void*);
       i++) {
    mark_object(*((void**)(object) + i));
  }
}

void mark() {
  // invert mark
  garbage_collector.mark = ~garbage_collector.mark;
  // iterate through roots to find reachable objects
  for (size_t root = 0; root < garbage_collector.root_list.count; root++) {
    if (*item_from_root(root)) mark_object((void*)*item_from_root(root));
  }
}

void defragment() {
  size_t gc_iterator = 0;
  size_t bump_pos = 0;

  external_ref_t* pointer_queue[GC_STACK_SIZE];
  size_t pointer_count = 0;

  mmap memory_map;
  mmapinit(&memory_map);

  while (gc_iterator < garbage_collector.capacity) {
    mem_block* block = (mem_block*)(garbage_collector.memory + gc_iterator);
    external_ref_t item = &block->data;

    size_t block_size = block->size;

    if (block->type == mem_avail) {
      gc_iterator += block_size;
      continue;
    }

    mem_block* new_block = (mem_block*)(garbage_collector.memory + bump_pos);
    external_ref_t new_item = &new_block->data;
    mmapinsert(&memory_map, (void*)item, (void*)new_item);
    memmove(new_block, block, block_size);
    for (size_t i = 0; i < new_block->contents.header.nptrs; i++) {
      external_ref_t* pptr = (external_ref_t*)(new_item) + i;
      if (*pptr) {
        pointer_queue[pointer_count++] = pptr;
      }
    }

    bump_pos += block_size;
    gc_iterator += block_size;
  }

  mem_block* remainder = (mem_block*)(garbage_collector.memory + bump_pos);
  *remainder = (mem_block){.size = garbage_collector.capacity - bump_pos,
                           .type = mem_avail,
                           .contents.next_free = NULL};
  garbage_collector.free_list_head = remainder;

  for (size_t i = 0; i < pointer_count; i++) {
    external_ref_t* itemptr = pointer_queue[i];  // pop stack
    *itemptr = mmaplookup(&memory_map, (void*)*itemptr);
  }

  for (size_t root = 0; root < garbage_collector.root_list.count; root++) {
    volatile void** itemptr =
        &garbage_collector.root_list.list_of_pointers[root];
    if (*itemptr) *itemptr = mmaplookup(&memory_map, *(void**)itemptr);
  }
}

void sweep() {
  size_t gc_iterator = 0;
  size_t max_free = 0;
  size_t total_free = 0;
  mem_block* prev_free = NULL;
  mem_block* prev_block = NULL;
  while (gc_iterator < garbage_collector.capacity) {
    mem_block* curr_block =
        (mem_block*)(garbage_collector.memory + gc_iterator);
    size_t bytes_to_next_block = curr_block->size;

    switch (curr_block->type) {
      case mem_used: {
        if (curr_block->contents.header.mark == garbage_collector.mark) {
          break;  // memory still in use DO NOT FREE
        }
        total_free += curr_block->size;
        curr_block->type = mem_avail;
        curr_block->contents.next_free = NULL;
        if (prev_free == NULL) {
          garbage_collector.free_list_head = curr_block;
          prev_free = curr_block;
        } else if (prev_block->type == mem_avail) {
          prev_block->size += curr_block->size;
          curr_block = prev_block;
        } else {
          // curr_block->contents.next_free = prev_free->contents.next_free;
          prev_free->contents.next_free = curr_block;
          prev_free = curr_block;
        }
        if (curr_block->size > max_free) {
          max_free = curr_block->size;
        }
        curr_block->contents.next_free = NULL;
      } break;
      case mem_avail: {
        total_free += curr_block->size;
        if (curr_block->size > max_free) {
          max_free = curr_block->size;
        }

        if (!prev_free) {
          garbage_collector.free_list_head = curr_block;
          prev_free = curr_block;
          curr_block->contents.next_free = NULL;
        } else if (prev_block && prev_block->type == mem_avail) {
          // coallesce with previous free block
          prev_block->size += curr_block->size;
          curr_block = prev_block;
        } else {
          prev_free->contents.next_free = curr_block;
          prev_free = curr_block;
          curr_block->contents.next_free = NULL;
        }

      } break;
      default:
        fprintf(stderr, "invalid mem_block type!\n");
    }
    prev_block = curr_block;
    gc_iterator += bytes_to_next_block;
  }

  double fragmentation = (double)(total_free - max_free) / (double)total_free;

  if (fragmentation > 0.75) {
    defragment();
  }
}

void gc() {
  if (garbage_collector.isactive) {
    fprintf(stderr, "gc called while active");
    exit(1);
  }

  garbage_collector.isactive = 1;

  mark();

  sweep();

  garbage_collector.isactive = 0;
}

void alloc_into_root(size_t root, size_t size, size_t nptrs) {
  external_ref_t allocation = alloc(size, nptrs);
  *item_from_root(root) = allocation;
}

void alloc_into_object(external_ref_t obj, size_t offset, size_t size,
                       size_t nptrs) {
  size_t t = add_roots(1);
  *item_from_root(t) = obj;
  external_ref_t allocation = alloc(size, nptrs);
  external_ref_t moved_obj = *item_from_root(t);
  *(external_ref_t*)(moved_obj + offset) = allocation;
  remove_roots(1);
}

external_ref_t alloc(size_t size, size_t nptrs) {
  static bool gc_called = false;

  size_t blocksize = size + sizeof(mem_block);
  mem_block* prev = NULL;

  for (mem_block* block = garbage_collector.free_list_head; block;
       block = block->contents.next_free) {
    if (block->type != mem_avail) {
      fprintf(stderr, "allocated block in free list\n");
      exit(1);
    }

    // insufficient space
    if (block->size < blocksize) {
      prev = block;
      continue;
    }

    // consume whole block
    if (block->size >= blocksize &&
        block->size < blocksize + sizeof(mem_block)) {
      blocksize = block->size;  // round up block size to entire block

      if (prev) {
        prev->contents.next_free = block->contents.next_free;
      } else {
        garbage_collector.free_list_head = block->contents.next_free;
      }

      memset(block, 0, blocksize);

      *block = (mem_block){.size = blocksize,
                           .type = mem_used,
                           .contents.header = (gc_head){
                               .mark = garbage_collector.mark, .nptrs = nptrs}};

      gc_called = false;
      return (external_ref_t)&block->data;
    }

    // split block
    if (block->size > blocksize) {
      mem_block* free_block = (mem_block*)((void*)block + blocksize);

      *free_block =
          (mem_block){.size = block->size - blocksize,
                      .type = mem_avail,
                      .contents.next_free = block->contents.next_free};

      if (prev) {
        prev->contents.next_free = free_block;
      } else {
        garbage_collector.free_list_head = free_block;
      }

      memset(block, 0, blocksize);

      *block = (mem_block){.size = blocksize,
                           .type = mem_used,
                           .contents.header = (gc_head){
                               .mark = garbage_collector.mark, .nptrs = nptrs}};

      gc_called = false;
      return (external_ref_t)&block->data;
    }
  }

  if (!gc_called) {
    // try gc
    gc();
    gc_called = true;
    return alloc(size, nptrs);
  }

  // no block with sufficient space
  fprintf(stderr, "alloc error: out of memory\n");

  exit(1);
}
