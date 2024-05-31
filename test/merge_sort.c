#include <stdio.h>
#include <time.h>

#include "../src/gc.h"

struct vector {
  volatile int* buffer;
  size_t size;
  size_t capacity;
};
typedef struct vector vector;

void print_vec(garbage_collector_t* gc, size_t vec_r) {
  for (int i = 0; i < get_val(gc,vector, vec_r).size; i++) {
    printf("%d ", get_val(gc,vector, vec_r).buffer[i]);
  }
  printf("\n");
}


void init_vec(garbage_collector_t* gc,size_t vec_r, size_t starting_capacity) {
  // printf("init vec %zu\n", starting_capacity);

  alloc_into_root(gc,vec_r, sizeof(vector), 1);
  // alloc_into(get_ref(gc,vec_r), offsetof(vector, buffer),
  //            starting_capacity * sizeof(int), 0);
  alloc_into_attr(gc,vector, buffer, get_ref(gc,vec_r),
                  starting_capacity * sizeof(int), 0);
  // volatile int* b = alloc(starting_capacity * sizeof(int), 0);
  // get_val(gc,vector, vec_r).buffer = b;
  get_val(gc,vector, vec_r).size = 0;
  get_val(gc,vector, vec_r).capacity = starting_capacity;
}

void push(garbage_collector_t* gc,size_t vec_r, int value) {
  size_t sz = get_val(gc,vector, vec_r).size;
  size_t cp = get_val(gc,vector, vec_r).capacity;
  volatile int* buffer = get_val(gc,vector, vec_r).buffer;
  (void)buffer;
  (void)sz;
  (void)cp;
  if (get_val(gc,vector, vec_r).size >= get_val(gc,vector, vec_r).capacity) {
    size_t new_buf = add_roots(gc,1);

    get_val(gc,vector, vec_r).capacity = get_val(gc,vector, vec_r).capacity * 2 + 1;

    alloc_into_root(gc,new_buf, get_val(gc,vector, vec_r).capacity * sizeof(int), 0);

    for (size_t i = 0; i < get_val(gc,vector, vec_r).size; i++) {
      ((int*)get_ref(gc,new_buf))[i] = get_val(gc,vector, vec_r).buffer[i];
    }

    get_val(gc,vector, vec_r).buffer = (int*)get_ref(gc,new_buf);

    remove_roots(gc,1);
  }
  get_val(gc,vector, vec_r).buffer[get_val(gc,vector, vec_r).size++] = value;
}

void slice(garbage_collector_t* gc,size_t slice_r, size_t vec_r, size_t start, size_t end) {
  // printf("slice %zu, %zu\n", start, end);
  init_vec(gc,slice_r, end - start);
  volatile vector* v = get_ref(gc,slice_r);
  volatile vector* v2 = get_ref(gc,vec_r);
  (void)v;
  (void)v2;
  for (size_t i = start; i < end; i++) {
    push(gc,slice_r, get_val(gc,vector, vec_r).buffer[i]);
  }
  // printf("slice ");
  // print_vec(slice_r);
}

void merged(garbage_collector_t* gc,size_t merged_r, size_t left_r, size_t right_r) {
  // printf("merge ");
  // print_vec(left_r);
  // printf("with ");
  // print_vec(right_r);

  init_vec(gc,merged_r,
           get_val(gc,vector, left_r).size + get_val(gc,vector, right_r).size);

  size_t index = add_roots(gc,2);

  alloc_into_root(gc,index, sizeof(size_t), 0);
  alloc_into_root(gc,index+1, sizeof(size_t), 0);


  while (get_val(gc,size_t, index) < get_val(gc,vector, left_r).size &&
         get_val(gc,size_t, index + 1) < get_val(gc,vector, right_r).size) {
    if (get_val(gc,vector, left_r).buffer[get_val(gc,size_t, index)] <
        get_val(gc,vector, right_r).buffer[get_val(gc,size_t, index + 1)]) {
      push(gc,merged_r, get_val(gc,vector, left_r).buffer[get_val(gc,size_t, index)++]);
    } else {
      push(gc,merged_r,
           get_val(gc,vector, right_r).buffer[get_val(gc,size_t, index + 1)++]);
    }
  }
  while (get_val(gc,size_t, index) < get_val(gc,vector, left_r).size) {
    push(gc,merged_r, get_val(gc,vector, left_r).buffer[get_val(gc,size_t, index)++]);
  }
  while (get_val(gc,size_t, index + 1) < get_val(gc,vector, right_r).size) {
    push(gc,merged_r,
         get_val(gc,vector, right_r).buffer[get_val(gc,size_t, index + 1)++]);
  }
  remove_roots(gc,2);
  // printf("merged ");
  // print_vec(merged_r);
}

void sorted(garbage_collector_t* gc,size_t sorted_r, size_t vec_r) {
  // printf("( sort ");
  // print_vec(vec_r);
  volatile vector* v = get_ref(gc,vec_r);
  (void)v;
  if (get_val(gc,vector, vec_r).size == 0) {
    slice(gc,sorted_r, vec_r, 0, 0);
    return;
  }
  if (get_val(gc,vector, vec_r).size == 1) {
    slice(gc,sorted_r, vec_r, 0, 1);
    return;
  }
  if (get_val(gc,vector, vec_r).size == 2) {
    init_vec(gc,sorted_r, 2);
    if (get_val(gc,vector, vec_r).buffer[0] < get_val(gc,vector, vec_r).buffer[1]) {
      push(gc,sorted_r, get_val(gc,vector, vec_r).buffer[0]);
      push(gc,sorted_r, get_val(gc,vector, vec_r).buffer[1]);
    } else {
      push(gc,sorted_r, get_val(gc,vector, vec_r).buffer[1]);
      push(gc,sorted_r, get_val(gc,vector, vec_r).buffer[0]);
    }
    return;
  }
  size_t temp_r = add_roots(gc,4);
  slice(gc,temp_r, vec_r, 0, get_val(gc,vector, vec_r).size / 2);
  slice(gc,temp_r + 1, vec_r, get_val(gc,vector, vec_r).size / 2,
        get_val(gc,vector, vec_r).size);
  sorted(gc,temp_r + 2, temp_r);
  sorted(gc,temp_r + 3, temp_r + 1);
  merged(gc,sorted_r, temp_r + 2, temp_r + 3);
  // printf("slice ");
  // print_vec(temp_r);
  // printf("slice ");
  // print_vec(temp_r + 1);
  // printf("sorted half ");
  // print_vec(temp_r + 2);
  // printf("sorted half ");
  // print_vec(temp_r + 3);
  // printf("merged ");
  // print_vec(sorted_r);
  // printf("\n");
  remove_roots(gc,4);
}

int check_vec(garbage_collector_t* gc,size_t vec_r, size_t length) {
  if (get_val(gc,vector, vec_r).size != length) return 0;
  for (int i = 0; i < get_val(gc,vector, vec_r).size; i++) {
    if (get_val(gc,vector, vec_r).buffer[i] != i + 1) return 0;
  }
  return 1;
}

int main(int argc, char const* argv[]) {
  size_t length = 1000;
  garbage_collector_t garbage_collector;
  garbage_collector_t* gc = &garbage_collector;
  init(gc);
  /*
  size_t r = add_roots(2);
  init_vec(r, 2);
  push(r, 0);
  push(r, 1);
  slice(r + 1, r, 1, 2);
  print_vec(r + 1);
  */
  /*
  size_t r = add_roots(1);
  void* a = alloc(20, 1);
  get_ref(gc,r) = a;
  gc();
  void* b=  alloc(20, 1);
  get_val(gc,int*, r) = b;
  gc();
  get_ref(gc,r) = get_val(gc,int*, r);
  gc();
  remove_roots(1);
  gc();*/
  // remove_roots(1);
  // gc();

  size_t vec_r = add_roots(gc,3);
  init_vec(gc,vec_r, 0);
  init_vec(gc,vec_r + 1, 0);

  for (int i = 0; i < length; i++) {
    push(gc,vec_r, length - i);
  }
  // print_vec(vec_r);

  sorted(gc,vec_r + 1, vec_r);
  // merged(vec_r + 2, vec_r, vec_r + 1);
  // print_vec(vec_r + 1);
  printf("%d\n",check_vec(gc,vec_r + 1, length));
  collect(gc);
  set_roots_back(gc,0);
  collect(gc);

  /*gc();
  printf("sort\n");
*/
  //print_vec(vec_r + 1);
  
  return 0;
}
