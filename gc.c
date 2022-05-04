#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_EXTENT_SIZE (sizeof(Object) * 4)

static size_t extent = INITIAL_EXTENT_SIZE;
static void *to_space, *from_space;
static void *free_ptr;

#define TOP_PTR (to_space + extent)

void fzscm_memspace_init(size_t semispace_size) {
  if (semispace_size > 0) {
    extent = semispace_size;
  }
  to_space = calloc(1, extent);
  from_space = calloc(1, extent);
  free_ptr = to_space;
}

void fzscm_memspace_fin(void) {
  free(to_space);
  free(from_space);
}

void fzscm_gc(void) {
  memset(from_space, 0, extent);

  // flip 処理
  {
    void *to_space_tmp = to_space;
    to_space = from_space;
    from_space = to_space_tmp;
  }
  free_ptr = to_space;

  // root の forward 処理
  RootNode *root = get_roots();
  while (root != NULL) {
    size_t obj_size = sizeof(*root->obj);
    if (debug_flag) {
      print_obj(root->obj);
      printf(" object size: %zu\n", obj_size);
    }
    if (free_ptr + obj_size > TOP_PTR) {
      printf("memory error: shortage of memory\n");
      exit(1);
    }
    memcpy(free_ptr, root->obj, obj_size);
    root->obj->tag = OBJ_MOVED;
    root->obj->fields_of.moved.address = free_ptr;
    root->obj = free_ptr;
    root = root->next;
    free_ptr += obj_size;
  }

  if (root != NULL) {
    // root 以外の forward 処理
    for (void *scan_ptr = to_space;
         scan_ptr != NULL;
         scan_ptr += sizeof(Object)) {
      if (scan_ptr + sizeof(Object) > TOP_PTR) {
        printf("memory error: shortage of memory\n");
        exit(1);
      }
      Object *cur_obj = (Object *)scan_ptr;
      switch (cur_obj->tag) {
      case OBJ_CELL:
        if (CAR(cur_obj)->tag != OBJ_MOVED) {
          memcpy(free_ptr, CAR(cur_obj), sizeof(Object));
          CAR(cur_obj)->tag = OBJ_MOVED;
          CAR(cur_obj)->fields_of.moved.address = free_ptr;
          free_ptr += sizeof(Object);
        }
        if (CDR(cur_obj)->tag != OBJ_MOVED) {
          memcpy(free_ptr, CDR(cur_obj), sizeof(Object));
          CDR(cur_obj)->tag = OBJ_MOVED;
          CDR(cur_obj)->fields_of.moved.address = free_ptr;
          free_ptr += sizeof(Object);
        }
        break;
      default:
        break;
      }
    }
  }
}

static void print_memory_status(void) {
  printf("memory status:\n");
  printf("\tto_space: %p, from_space: %p\n", to_space, from_space);
  printf("\tfree_ptr: %p, semispace_size: 0x%zx\n", free_ptr, extent);
}

void *fzscm_alloc(size_t size) {
  if (debug_flag) {
    print_memory_status();
  }
  if (free_ptr + size > TOP_PTR) {
    fzscm_gc();
    if (free_ptr + size > TOP_PTR) {
      printf("memory error: shortage of memory\n");
      exit(1);
    }
    if (debug_flag) {
      printf("after gc ... ");
      print_memory_status();
    }
  }

  void *ptr = free_ptr;
  free_ptr += size;

  return ptr;
}
