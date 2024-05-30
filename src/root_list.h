#pragma once

#include "gc.h"

struct gc_item;
typedef volatile void* external_ref_t;

size_t add_roots(size_t);

void remove_roots(size_t);

void set_roots_back(size_t);

size_t get_root_count();

external_ref_t* item_from_root(size_t);