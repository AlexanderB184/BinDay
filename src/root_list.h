#pragma once

#include "gc.h"

typedef volatile void* external_ref_t;
typedef struct garbage_collector_t garbage_collector_t;
size_t add_roots(garbage_collector_t*, size_t);

void remove_roots(garbage_collector_t*, size_t);

void set_roots_back(garbage_collector_t*, size_t);

size_t get_root_count(garbage_collector_t*);

external_ref_t* item_from_root(garbage_collector_t*, size_t);