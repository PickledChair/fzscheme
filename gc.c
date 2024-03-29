#include "fzscheme.h"

#define INITIAL_EXTENT_SIZE (sizeof(Object) * 100000)

static size_t extent = INITIAL_EXTENT_SIZE;
static void *to_space, *from_space;
static void *free_ptr;

#define TOP_PTR (to_space + extent)

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

void reset_gc_state(void) {
  gc_occured_before_reset_gc_state = false;
}

#define FORWARD(from_ref)                                                      \
  memcpy(free_ptr, (from_ref), sizeof(Object));                                \
  (from_ref)->tag = OBJ_MOVED;                                                 \
  (from_ref)->fields_of.moved.address = free_ptr;

#define TOSPACE_CONTAINS(ref) (to_space <= (void *)(ref) && (void *)(ref) < to_space + extent)

static void flip(void) {
  void *to_space_tmp = to_space;
  to_space = from_space;
  from_space = to_space_tmp;
}

static bool obj_is_in_gc_heap(Object *obj) {
  return (obj != NULL && obj->tag != OBJ_BOOLEAN && obj->tag != OBJ_SYMBOL && obj != NIL
          && obj->tag != OBJ_PRIMITIVE && obj != UNDEF);
}

static void forward_parser_roots(void) {
  ParserRootNode *cur_root = get_parser_roots();

  while (cur_root != NULL) {
    if (obj_is_in_gc_heap(*cur_root->value)) {
      size_t obj_size = sizeof(**cur_root->value);
      if (debug_flag) {
        print_obj(*cur_root->value);
        printf(" object size: 0x%zx, address: %p\n", obj_size, *cur_root->value);
      }
      if (free_ptr + obj_size > TOP_PTR) {
        printf("memory error: shortage of memory\n");
        exit(1);
      }

      if (!TOSPACE_CONTAINS(*cur_root->value)) {
        if ((*cur_root->value)->tag != OBJ_MOVED) {
          FORWARD(*cur_root->value);

          *cur_root->value = free_ptr;
          free_ptr += obj_size;
        } else {
          if ((*cur_root->value)->fields_of.moved.address->tag == OBJ_MOVED) {
            printf("!!! 2 times moved !!! ");
            print_obj(*cur_root->value);
            putchar('\n');
            exit(1);
          }
          *cur_root->value = (*cur_root->value)->fields_of.moved.address;
        }
      } else {
        if ((*cur_root->value)->tag == OBJ_MOVED) {
          printf("to_space contains moved object: %p, ", *cur_root->value);
          print_obj(*cur_root->value);
          putchar('\n');
          exit(1);
        }
      }
    }
    cur_root = cur_root->next;
  }
}

static void forward_roots(void) {
  RootNode *cur_root = get_roots();

  while (cur_root != NULL) {
    if (obj_is_in_gc_heap(*cur_root->value)) {
      size_t obj_size = sizeof(**cur_root->value);
      if (debug_flag) {
        print_obj(*cur_root->value);
        printf(" object size: 0x%zx, address: %p\n", obj_size, *cur_root->value);
      }
      if (free_ptr + obj_size > TOP_PTR) {
        printf("memory error: shortage of memory\n");
        exit(1);
      }

      if (!TOSPACE_CONTAINS(*cur_root->value)) {
        if ((*cur_root->value)->tag != OBJ_MOVED) {
          FORWARD(*cur_root->value);

          *cur_root->value = free_ptr;
          free_ptr += obj_size;
        } else {
          if ((*cur_root->value)->fields_of.moved.address->tag == OBJ_MOVED) {
            printf("!!! 2 times moved !!! ");
            print_obj(*cur_root->value);
            putchar('\n');
            exit(1);
          }
          *cur_root->value = (*cur_root->value)->fields_of.moved.address;
        }
      } else {
        if ((*cur_root->value)->tag == OBJ_MOVED) {
          printf("to_space contains moved object: %p, ", *cur_root->value);
          print_obj(*cur_root->value);
          putchar('\n');
          exit(1);
        }
      }
    }
    cur_root = cur_root->next;
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
      if (!TOSPACE_CONTAINS(CAR(cur_obj))) {
        if (CAR(cur_obj)->tag != OBJ_MOVED) {
          if (obj_is_in_gc_heap(CAR(cur_obj))) {
            FORWARD(CAR(cur_obj));

            CAR(cur_obj) = free_ptr;
            free_ptr += sizeof(Object);
          }
        } else {
          if (CAR(cur_obj)->fields_of.moved.address->tag == OBJ_MOVED) {
            printf("invalid car object (2 times moved): %p, moved from %p, ", CAR(cur_obj), CAR(cur_obj)->fields_of.moved.address);
            print_obj(CAR(cur_obj));
            putchar('\n');
            exit(1);
          } else {
            CAR(cur_obj) = CAR(cur_obj)->fields_of.moved.address;
          }
        }
      } else {
        if (CAR(cur_obj)->tag == OBJ_MOVED) {
          if (CAR(cur_obj)->fields_of.moved.address->tag == OBJ_MOVED) {
            printf("invalid car object: ");
            print_obj(CAR(cur_obj));
            putchar('\n');
            exit(1);
          }
        }
      }
      if (!TOSPACE_CONTAINS(CDR(cur_obj))) {
        if (CDR(cur_obj) != NIL) {
          if (CDR(cur_obj)->tag != OBJ_MOVED) {
            if (obj_is_in_gc_heap(CDR(cur_obj))) {
              FORWARD(CDR(cur_obj));

              CDR(cur_obj) = free_ptr;
              free_ptr += sizeof(Object);
            }
          } else {
            CDR(cur_obj) = CDR(cur_obj)->fields_of.moved.address;
          }
        }
      }
      break;
    case OBJ_CLOSURE: {
      Object *vars = cur_obj->fields_of.closure.env_node->value->vars;
      if (!TOSPACE_CONTAINS(vars)) {
        if (vars->tag != OBJ_MOVED) {
          if (obj_is_in_gc_heap(vars)) {
            FORWARD(vars);

            cur_obj->fields_of.closure.env_node->value->vars = free_ptr;
            free_ptr += sizeof(Object);
          }
        } else {
          cur_obj->fields_of.closure.env_node->value->vars = vars->fields_of.moved.address;
        }
      }
      env_collect_roots(cur_obj->fields_of.closure.env_node);
      code_collect_roots(cur_obj->fields_of.closure.code_node);
      forward_roots();
      break;
    }
    case OBJ_VECTOR:
      mark_vector_node(cur_obj->fields_of.vector.vec_node);
      for (size_t i = 0; i < cur_obj->fields_of.vector.size; i++) {
        Object *indexed_obj = cur_obj->fields_of.vector.vec_node->value[i];
        if (!TOSPACE_CONTAINS(indexed_obj)) {
          if (indexed_obj->tag != OBJ_MOVED) {
            if (obj_is_in_gc_heap(indexed_obj)) {
              FORWARD(indexed_obj);

              cur_obj->fields_of.vector.vec_node->value[i] = free_ptr;
              free_ptr += sizeof(Object);
            }
          } else {
            cur_obj->fields_of.vector.vec_node->value[i] = indexed_obj->fields_of.moved.address;
          }
        }
      }
      break;
    case OBJ_STRING:
      mark_string_node(cur_obj->fields_of.string.str_node);
      break;
    case OBJ_MOVED:
      printf("invalid moved object: ");
      print_obj(cur_obj);
      putchar('\n');
      exit(1);
    default:
      break;
    }
  }
  if (debug_flag) {
    for (void *p = to_space; p < free_ptr; p += sizeof(Object)) {
      if (((Object *)p)->tag == OBJ_MOVED) {
        printf("failed to move: ");
        print_obj((Object *)p);
        putchar('\n');
        exit(1);
      }
    }
    printf("finish scanning\n");
  }
}

void fzscm_gc(void) {
  // if (gc_occured_before_reset_gc_state) {
  //   printf("gc error: there are some new objects those are not in root set\n");
  //   exit(1);
  // }
  memset(from_space, 0, extent);

  // flip 処理
  flip();

  // free_ptr の更新を新しい to_space から始めるようにセット
  free_ptr = to_space;

  // グローバル環境からルート集合を得る
  global_env_collect_roots();

  // vm からルート集合を得る
  if (current_working_vm != NULL) {
    vm_collect_roots(current_working_vm);
  }

  // root の forward 処理
  forward_parser_roots();
  forward_roots();

  // root 以外の forward 処理
  forward_non_roots();

  // 文字列とベクタは Scheme オブジェクトとは別に GC を行う
  string_list_gc();
  vector_list_gc();
  env_list_gc();
  code_list_gc();

  // GC 開始時に集めたルート集合のリストを破棄
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(RootNode)();

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

DEFINE_DOUBLY_LINKED_LIST_FUNCS(RootNode, Object *, false)

RootNode *get_roots(void) {
  return DOUBLY_LINKED_LIST_HEAD_NAME(RootNode)->next;
}

bool roots_is_empty(void) {
  return DOUBLY_LINKED_LIST_HEAD_NAME(RootNode)->next == NULL;
}
