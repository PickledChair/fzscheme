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

static Object *prim_eq(Object *obj) {
  if (get_list_length(obj) != 2) {
    return new_error_obj("eq?: wrong number of arguments, eq? requires 2");
  }
  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  return (fst == snd) ? TRUE : FALSE;
}
DEFINE_PRIM_OBJ(prim_eq)

static bool eqv(Object *fst, Object *snd) {
  if (fst->tag == OBJ_INTEGER && snd->tag == OBJ_INTEGER) {
    return fst->fields_of.integer.value == snd->fields_of.integer.value;
  }
  return fst == snd;
}

static Object *prim_eqv(Object *obj) {
  if (get_list_length(obj) != 2) {
    return new_error_obj("eqv?: wrong number of arguments, eqv? requires 2");
  }
  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  return eqv(fst, snd) ? TRUE : FALSE;
}
DEFINE_PRIM_OBJ(prim_eqv)

static bool equal(Object *fst, Object *snd) {
  if (eqv(fst, snd)) return true;

  if (fst->tag == OBJ_CELL && snd->tag == OBJ_CELL) {
    if (get_list_length(fst) != get_list_length(snd)) return false;

    for (Object *l = fst, *r = snd; l != NIL; l = CDR(l), r = CDR(r)) {
      if (!equal(CAR(l), CAR(r))) return false;
    }
    return true;
  }

  if (fst->tag == OBJ_STRING && snd->tag == OBJ_STRING) {
    return strcmp(fst->fields_of.string.str_node->value,
                  snd->fields_of.string.str_node->value) == 0;
  }

  return false;
}

static Object *prim_equal(Object *obj) {
  if (get_list_length(obj) != 2) {
    return new_error_obj("equal?: wrong number of arguments, equal? requires 2");
  }
  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  return equal(fst, snd) ? TRUE : FALSE;
}
DEFINE_PRIM_OBJ(prim_equal)
