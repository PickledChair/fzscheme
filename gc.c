#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_EXTENT_SIZE (sizeof(Object) * 100000)

static size_t extent = INITIAL_EXTENT_SIZE;
static void *to_space, *from_space;
static void *free_ptr;

#define TOP_PTR (to_space + extent)

static void *fresh_obj_root_start, *fresh_obj_root_end;
static size_t fresh_obj_count = 0;
static bool gc_occured_before_reset_gc_state = false;

void fzscm_memspace_init(size_t semispace_size) {
  if (semispace_size >= sizeof(Object)) {
    extent = (semispace_size / sizeof(Object)) * sizeof(Object);
  }
  to_space = calloc(1, extent);
  from_space = calloc(1, extent);
  free_ptr = to_space;
}

void fzscm_memspace_fin(void) {
  free(to_space);
  free(from_space);
}

static void inc_fresh_obj_count(void) {
  if (fresh_obj_count == 0) {
    fresh_obj_root_start = free_ptr;
  }
  fresh_obj_count++;
}

static void reset_fresh_obj_count(void) {
  fresh_obj_count = 0;
  fresh_obj_root_start = free_ptr;
}

void reset_gc_state(void) {
  reset_fresh_obj_count();
  gc_occured_before_reset_gc_state = false;
}

#define FORWARD(from_ref)                                                      \
  memcpy(free_ptr, (from_ref), sizeof(Object));                                \
  (from_ref)->tag = OBJ_MOVED;                                                 \
  (from_ref)->fields_of.moved.address = free_ptr;

static void flip(void) {
  void *to_space_tmp = to_space;
  to_space = from_space;
  from_space = to_space_tmp;
}

static bool obj_is_in_gc_heap(Object *obj) {
  return (obj->tag != OBJ_BOOLEAN && obj->tag != OBJ_SYMBOL && obj != NIL);
}

static void forward_roots(void) {
  RootNode *cur_root = get_roots();

  while (cur_root != NULL) {
    if (obj_is_in_gc_heap(*cur_root->obj)) {
      size_t obj_size = sizeof(**cur_root->obj);
      if (debug_flag) {
        print_obj(*cur_root->obj);
        printf(" object size: 0x%zx\n", obj_size);
      }
      if (free_ptr + obj_size > TOP_PTR) {
        printf("memory error: shortage of memory\n");
        exit(1);
      }

      FORWARD(*cur_root->obj);

      *cur_root->obj = free_ptr;
      free_ptr += obj_size;
    }
    cur_root = cur_root->next;
  }
}

static void forward_fresh_obj_roots(void) {
  void *cur_root = fresh_obj_root_start;
  if (debug_flag) {
    // ????????????????????????????????????????????????????????????????????????
    printf("fresh obj count: %zu\n", fresh_obj_count - 1);
  }
  for (size_t root_cnt = 1; root_cnt < fresh_obj_count;
       root_cnt++, cur_root += sizeof(Object)) {
    if (free_ptr + sizeof(Object) > TOP_PTR) {
      printf("memory error: shortage of memory\n");
      exit(1);
    }
    Object *cur_obj = (Object *)cur_root;
    if (debug_flag) {
      printf("fresh obj (%p): ", cur_obj);
      print_obj(cur_obj);
      printf("\n");
    }

    FORWARD(cur_obj);

    if (fresh_obj_root_start == cur_root) {
      fresh_obj_root_start = free_ptr;
    }
    free_ptr += sizeof(Object);
  }
}

static void forward_non_roots(void) {
  for (void *scan_ptr = to_space;
      scan_ptr < free_ptr;
      scan_ptr += sizeof(Object)) {
    if (debug_flag) {
      printf("scan_ptr: %p, free_ptr: %p\n", scan_ptr, free_ptr);
    }
    if (free_ptr + sizeof(Object) > TOP_PTR) {
      printf("memory error: shortage of memory\n");
      exit(1);
    }
    Object *cur_obj = (Object *)scan_ptr;
    if (debug_flag) {
      print_obj(cur_obj);
      printf(": being scanned\n");
    }
    switch (cur_obj->tag) {
    case OBJ_CELL:
      if (CAR(cur_obj)->tag != OBJ_MOVED) {
        if (obj_is_in_gc_heap(CAR(cur_obj))) {
          FORWARD(CAR(cur_obj));

          CAR(cur_obj) = free_ptr;
          free_ptr += sizeof(Object);
        }
      } else {
        CAR(cur_obj) = CAR(cur_obj)->fields_of.moved.address;
      }
      if (CDR(cur_obj) != NIL) {
        if (CDR(cur_obj)->tag != OBJ_MOVED) {

          FORWARD(CDR(cur_obj));

          CDR(cur_obj) = free_ptr;
          free_ptr += sizeof(Object);
        } else {
          CDR(cur_obj) = CDR(cur_obj)->fields_of.moved.address;
        }
      }
      break;
    case OBJ_STRING:
      mark_string_node(cur_obj->fields_of.string.str_node);
      break;
    default:
      break;
    }
  }
  if (debug_flag) {
    printf("finish scanning\n");
  }
}

void fzscm_gc(void) {
  if (gc_occured_before_reset_gc_state) {
    printf("gc error: there are some new objects those are not in root set\n");
    exit(1);
  }
  memset(from_space, 0, extent);

  // flip ??????
  flip();

  // GC ?????????????????????????????????????????????????????????
  fresh_obj_root_end = free_ptr;

  // free_ptr ????????????????????? to_space ?????????????????????????????????
  free_ptr = to_space;

  // ???????????????????????????????????????????????????
  global_env_collect_roots();

  // vm ??????????????????????????????
  if (current_working_vm != NULL) {
    vm_collect_roots(current_working_vm);
  }

  // root ??? forward ??????
  forward_roots();
  forward_fresh_obj_roots();

  // root ????????? forward ??????
  forward_non_roots();

  // ???????????? Scheme ?????????????????????????????? GC ?????????
  string_list_gc();

  // GC ?????????????????????????????????????????????????????????
  clear_roots();

  // GC ?????????????????????????????????????????????????????????????????????
  reset_fresh_obj_count();

  gc_occured_before_reset_gc_state = true;
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
  inc_fresh_obj_count();
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

static RootNode *roots = &(RootNode){};
static RootNode *top_root = NULL;

void add_root(Object **obj) {
  if (debug_flag) {
    printf("add_root: ");
    print_obj(*obj);
    printf(" (%p)\n", *obj);
  }
  RootNode *node = calloc(1, sizeof(RootNode));
  node->obj = obj;
  if (top_root == NULL) {
    roots->next = top_root = node;
  } else {
    top_root = top_root->next = node;
  }
}

RootNode *get_roots(void) { return roots->next; }

void clear_roots(void) {
  RootNode *cur = roots->next;
  while (cur != NULL) {
    RootNode *next = cur->next;
    free(cur);
    cur = next;
  }
  top_root = NULL;
  roots->next = NULL;
}
