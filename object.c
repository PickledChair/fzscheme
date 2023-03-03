#include "fzscheme.h"

Object *UNDEF = &(Object){OBJ_UNDEF};
Object *NIL = &(Object){OBJ_CELL};
Object *TRUE = &(Object){OBJ_BOOLEAN, {true}};
Object *FALSE = &(Object){OBJ_BOOLEAN, {false}};

static Object *new_obj(ObjectTag tag) {
  // Object *obj = (Object *)calloc(1, sizeof(Object));
  Object *obj = (Object *)fzscm_alloc(sizeof(Object));
  obj->tag = tag;
  return obj;
}

Object *new_cell_obj(Object *car, Object *cdr) {
  Object *obj = new_obj(OBJ_CELL);
  CHECK_OBJ_MOVING(car);
  CHECK_OBJ_MOVING(cdr);
  CAR(obj) = car;
  CDR(obj) = cdr;
  return obj;
}

// static void free_cell_obj(Object *obj) {
//   if (obj == NIL) {
//     return;
//   } else {
//     if (CAR(obj)->tag == OBJ_CELL) {
//       free_cell_obj(CAR(obj));
//     } else {
//       free_obj(CAR(obj));
//     }
//     free_cell_obj(CDR(obj));
//   }
//   free(obj);
// }

Object *new_error_obj(char *message) {
  Object *obj = new_obj(OBJ_ERROR);
  obj->fields_of.error.message = NODE_TYPE_NEW_FUNC_NAME(StringNode)(strdup(message));
  return obj;
}

Object *new_integer_obj(long value) {
  Object *obj = new_obj(OBJ_INTEGER);
  obj->fields_of.integer.value = value;
  return obj;
}

Object *new_string_obj(char *value) {
  Object *obj = new_obj(OBJ_STRING);
  obj->fields_of.string.str_node = NODE_TYPE_NEW_FUNC_NAME(StringNode)(strdup(value));
  return obj;
}

Object *new_symbol_obj(char *name) {
  Object *obj = calloc(1, sizeof(Object));
  obj->tag = OBJ_SYMBOL;
  obj->fields_of.symbol.name = name;
  return obj;
}

void free_symbol_obj(Object *obj) {
  free(obj->fields_of.symbol.name);
  free(obj);
}

Object *new_vector_obj(size_t size, Object *fill) {
  Object *obj = new_obj(OBJ_VECTOR), *fill_obj = (fill == NULL) ? UNDEF : fill;
  obj->fields_of.vector.vec_node = NODE_TYPE_NEW_FUNC_NAME(ObjectVectorNode)(calloc(size, sizeof(Object *)));
  for (size_t i = 0; i < size; i++) {
    obj->fields_of.vector.vec_node->value[i] = fill_obj;
  }
  obj->fields_of.vector.size = size;
  return obj;
}

// static void free_string_obj(Object *obj) {
//   free(obj->fields_of.string.value);
//   free(obj);
// }

// void free_obj(Object *obj) {
//   switch (obj->tag) {
//   case OBJ_CELL:
//     free_cell_obj(obj);
//     break;
//   case OBJ_INTEGER:
//     free(obj);
//     break;
//   case OBJ_STRING:
//     free_string_obj(obj);
//     break;
//   }
// }

void print_obj(Object *obj) {
  switch (obj->tag) {
  case OBJ_UNDEF:
    printf("#<undef>");
    break;
  case OBJ_BOOLEAN: {
    printf("%s", obj->fields_of.boolean.value ? "#t" : "#f");
    break;
  }
  case OBJ_CELL:
    if (obj != NIL && CAR(obj)->tag == OBJ_SYMBOL
        && CAR(obj) == intern_name("quote")) {
      if (CDR(obj)->tag == OBJ_CELL) {
        putchar('\'');
        print_obj(CAR(CDR(obj)));
      }
    } else {
      putchar('(');
      for (Object *cur = obj; cur != NIL; cur = CDR(cur)) {
        print_obj(CAR(cur));
        if (CDR(cur) != NIL) {
          if (CDR(cur)->tag == OBJ_CELL) {
            putchar(' ');
          } else {
            printf(" . ");
            print_obj(CDR(cur));
            break;
          }
        }
      }
      putchar(')');
    }
    break;
  case OBJ_ERROR:
    printf("error: %s", obj->fields_of.error.message->value);
    break;
  case OBJ_INTEGER:
    printf("%ld", obj->fields_of.integer.value);
    break;
  case OBJ_PRIMITIVE:
    printf("#<primitive %s>", obj->fields_of.primitive.symbol->fields_of.symbol.name);
    break;
  case OBJ_STRING:
    printf("%s", obj->fields_of.string.str_node->value);
    break;
  case OBJ_SYMBOL:
    printf("%s", obj->fields_of.symbol.name);
    break;
  case OBJ_MOVED:
    printf("<moved>");
    print_obj(obj->fields_of.moved.address);
    break;
  case OBJ_VECTOR:
    printf("#(");
    for (size_t i = 0; i < obj->fields_of.vector.size; i++) {
      if (i != 0) putchar(' ');
      print_obj(obj->fields_of.vector.vec_node->value[i]);
    }
    putchar(')');
  }
}

// Object *process_moved_obj(Object *obj) {
//   if (obj == NULL) return NULL;
//   if (obj->tag == OBJ_CELL) {
//     CAR(obj) = process_moved_obj(CAR(obj));
//     CDR(obj) = process_moved_obj(CDR(obj));
//   } else if (obj->tag == OBJ_MOVED) {
//     obj = obj->fields_of.moved.address;
//     obj = process_moved_obj(obj);
//   }
//   return obj;
// }
