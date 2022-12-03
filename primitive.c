#include <stdio.h>
#include "fzscheme.h"

#define DEFINE_PRIM_OBJ(FN_NAME)      \
  Object *FN_NAME##_obj =             \
      &(Object){.tag = OBJ_PRIMITIVE, \
                .fields_of = {.primitive = {.symbol = NULL, .fn = FN_NAME}}};

static Object *prim_car(Object *obj) {
  Object *fst = CAR(obj);
  if (fst->tag != OBJ_CELL) {
    return new_error_obj("list expected for the argument of `car`");
  } else if (fst == NIL) {
    return new_error_obj("pair required, but got ()");
  }
  return CAR(fst);
}
DEFINE_PRIM_OBJ(prim_car)

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
