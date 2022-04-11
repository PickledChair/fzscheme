#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Object *NIL = &(Object){OBJ_TAG_CELL};

static Object *new_obj(ObjectTag tag) {
  Object *obj = (Object *)calloc(1, sizeof(Object));
  obj->tag = tag;
  return obj;
}

Object *new_cell_obj(Object *car, Object *cdr) {
  Object *obj = new_obj(OBJ_TAG_CELL);
  CAR(obj) = car;
  CDR(obj) = cdr;
  return obj;
}

static void free_cell_obj(Object *obj) {
  if (obj == NIL) {
    return;
  } else {
    if (CAR(obj)->tag == OBJ_TAG_CELL) {
      free_cell_obj(CAR(obj));
    } else {
      free_obj(CAR(obj));
    }
    free_cell_obj(CDR(obj));
  }
  free(obj);
}

Object *new_integer_obj(long value) {
  Object *obj = new_obj(OBJ_TAG_INTEGER);
  obj->fields_of.integer.value = value;
  return obj;
}

Object *new_string_obj(char *value) {
  Object *obj = new_obj(OBJ_TAG_STRING);
  obj->fields_of.string.value = value;
  return obj;
}

static void free_string_obj(Object *obj) {
  free(obj->fields_of.string.value);
  free(obj);
}

void free_obj(Object *obj) {
  switch (obj->tag) {
  case OBJ_TAG_CELL:
    free_cell_obj(obj);
    break;
  case OBJ_TAG_INTEGER:
    free(obj);
    break;
  case OBJ_TAG_STRING:
    free_string_obj(obj);
    break;
  }
}

void print_obj(Object *obj) {
  switch (obj->tag) {
  case OBJ_TAG_CELL:
    putchar('(');
    for (Object *cur = obj; cur != NIL; cur = CDR(cur)) {
      print_obj(CAR(cur));
      if (CDR(cur) != NIL)
        putchar(' ');
    }
    putchar(')');
    break;
  case OBJ_TAG_INTEGER:
    printf("%ld", obj->fields_of.integer.value);
    break;
  case OBJ_TAG_STRING:
    printf("%s", obj->fields_of.string.value);
    break;
  }
}
