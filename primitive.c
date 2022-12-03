#include <stdio.h>
#include "fzscheme.h"

static Object *prim_car(Object *obj) {
  Object *fst = CAR(obj);
  if (fst->tag != OBJ_CELL) {
    return new_error_obj("list expected for the argument of `car`");
  } else if (fst == NIL) {
    return new_error_obj("pair required, but got ()");
  }
  return CAR(fst);
}
Object *prim_car_obj = &(Object){
  .tag = OBJ_PRIMITIVE,
  .fields_of = {
    .primitive = { .symbol = NULL, .fn = prim_car }
  }
};

static Object *prim_display(Object *obj) {
  Object *fst = CAR(obj);
  print_obj(fst);
  return UNDEF;
}
Object *prim_display_obj = &(Object){
  .tag = OBJ_PRIMITIVE,
  .fields_of = {
    .primitive = { .symbol = NULL, .fn = prim_display }
  }
};

static Object *prim_newline(Object *obj) {
  putchar('\n');
  return UNDEF;
}
Object *prim_newline_obj = &(Object){
  .tag = OBJ_PRIMITIVE,
  .fields_of = {
    .primitive = { .symbol = NULL, .fn = prim_newline }
  }
};
