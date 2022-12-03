#include <stdio.h>
#include "fzscheme.h"

// リストの長さを返す。渡されたオブジェクトがペアだったら -1 を返す
static int get_list_length(Object *obj) {
  int count = 0;
  while (obj != NIL) {
    count++;
    if (CDR(obj)->tag != OBJ_CELL) {
      count = -1;
      break;
    }
    obj = CDR(obj);
  }
  return count;
}

#define DEFINE_PRIM_OBJ(FN_NAME)      \
  Object *FN_NAME##_obj =             \
      &(Object){.tag = OBJ_PRIMITIVE, \
                .fields_of = {.primitive = {.symbol = NULL, .fn = FN_NAME}}};

static Object *prim_car(Object *obj) {
  Object *fst = CAR(obj);
  if (fst->tag != OBJ_CELL) {
    return new_error_obj("car: pair required");
  } else if (fst == NIL) {
    return new_error_obj("car: pair required, but got ()");
  }
  return CAR(fst);
}
DEFINE_PRIM_OBJ(prim_car)

static Object *prim_cdr(Object *obj) {
  Object *fst = CAR(obj);
  if (fst->tag != OBJ_CELL) {
    return new_error_obj("cdr: pair required");
  } else if (fst == NIL) {
    return new_error_obj("cdr: pair required, but got ()");
  }
  return CDR(fst);
}
DEFINE_PRIM_OBJ(prim_cdr)

static Object *prim_cons(Object *obj) {
  if (get_list_length(obj) != 2) {
    return new_error_obj("cons: wrong number of arguments, cons requires 2");
  }
  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  return new_cell_obj(fst, snd);
}
DEFINE_PRIM_OBJ(prim_cons)

static Object *prim_display(Object *obj) {
  Object *fst = CAR(obj);
  print_obj(fst);
  return UNDEF;
}
DEFINE_PRIM_OBJ(prim_display)

static Object *prim_newline(Object *obj) {
  putchar('\n');
  return UNDEF;
}
DEFINE_PRIM_OBJ(prim_newline)
