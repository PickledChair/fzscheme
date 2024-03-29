#include "fzscheme.h"

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

static Object *prim_plus(Object *obj) {
  long result = 0;
  Object *args = obj;
  while (args != NIL) {
    if (CAR(args)->tag != OBJ_INTEGER) {
      return new_error_obj("cannot apply `+` for non-integer objects");
    }
    result += CAR(args)->fields_of.integer.value;
    args = CDR(args);
  }
  return new_integer_obj(result);
}
DEFINE_PRIM_OBJ(prim_plus)

static Object *prim_times(Object *obj) {
  long result = 1;
  Object *args = obj;
  while (args != NIL) {
    if (CAR(args)->tag != OBJ_INTEGER) {
      return new_error_obj("cannot apply `*` for non-integer objects");
    }
    result *= CAR(args)->fields_of.integer.value;
    args = CDR(args);
  }
  return new_integer_obj(result);
}
DEFINE_PRIM_OBJ(prim_times)

static Object *prim_minus(Object *obj) {
  switch (get_list_length(obj)) {
    case 0: return new_error_obj("`-`: arguments is empty");
    case 1:
      if (CAR(obj)->tag != OBJ_INTEGER) return new_error_obj("cannot apply `-` for non-integer object");
      return new_integer_obj(-CAR(obj)->fields_of.integer.value);
    default: { // 引数がペアの時は今のところ考慮しない
      if (CAR(obj)->tag != OBJ_INTEGER) return new_error_obj("cannot apply `-` for non-integer object");
      long result = CAR(obj)->fields_of.integer.value;
      Object *args = CDR(obj);
      while (args != NIL) {
        if (CAR(args)->tag != OBJ_INTEGER) return new_error_obj("cannot apply `-` for non-integer object");
        result -= CAR(args)->fields_of.integer.value;
        args = CDR(args);
      }
      return new_integer_obj(result);
    }
  }
}
DEFINE_PRIM_OBJ(prim_minus)

static Object *prim_div(Object *obj) {
  if (get_list_length(obj) != 2) return new_error_obj("div: wrong number of arguments, div requires 2");

  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  if (fst->tag != OBJ_INTEGER) return new_error_obj("div: first argument is not integer");
  if (snd->tag != OBJ_INTEGER) return new_error_obj("div: second argument is not integer");
  if (snd->fields_of.integer.value == 0) return new_error_obj("div: zero divisor is not allowed in integer division");

  return new_integer_obj(fst->fields_of.integer.value / snd->fields_of.integer.value);
}
DEFINE_PRIM_OBJ(prim_div)

static Object *prim_modulo(Object *obj) {
  if (get_list_length(obj) != 2) return new_error_obj("modulo: wrong number of arguments, modulo requires 2");

  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  if (fst->tag != OBJ_INTEGER) return new_error_obj("modulo: first argument is not integer");
  if (snd->tag != OBJ_INTEGER) return new_error_obj("modulo: second argument is not integer");
  if (snd->fields_of.integer.value == 0) return new_error_obj("modulo: zero divisor is not allowed in integer division");

  return new_integer_obj(fst->fields_of.integer.value % snd->fields_of.integer.value);
}
DEFINE_PRIM_OBJ(prim_modulo)

static Object *prim_exit(Object *obj) {
  int status_code = 0;
  if (get_list_length(obj) == 1) {
    if (CAR(obj)->tag == OBJ_INTEGER) {
      status_code = CAR(obj)->fields_of.integer.value;
    }
  }
  fzscm_deinit();
  exit(status_code);
}
DEFINE_PRIM_OBJ(prim_exit)

static Object *prim_make_vector(Object *obj) {
  int args_len = get_list_length(obj);
  if (args_len < 1) return new_error_obj("make-vector: arguments is empty");
  if (CAR(obj)->tag != OBJ_INTEGER) return new_error_obj("make-vector: first argument is not integer");

  size_t size = CAR(obj)->fields_of.integer.value;
  if (size < 0) return new_error_obj("make-vector: first argument is not positive integer");
  return new_vector_obj(size, (args_len > 1) ? CAR(CDR(obj)) : NULL);
}
DEFINE_PRIM_OBJ(prim_make_vector)

static Object *prim_vector_ref(Object *obj) {
  if (get_list_length(obj) != 2) return new_error_obj("vector-ref: wrong number of arguments, vector-ref requires 2");

  Object *fst = CAR(obj), *snd = CAR(CDR(obj));
  if (fst->tag != OBJ_VECTOR) return new_error_obj("vector-ref: first argument is not vector");
  if (snd->tag != OBJ_INTEGER) return new_error_obj("vector-ref: second argument is not integer");

  long index = snd->fields_of.integer.value;
  if (index < 0) return new_error_obj("vector-ref: negative index is not allowed");
  if (index >= fst->fields_of.vector.size) return new_error_obj("vector-ref: index is larger than the size of vector");

  return fst->fields_of.vector.vec_node->value[index];
}
DEFINE_PRIM_OBJ(prim_vector_ref)

static Object *prim_vector_set(Object *obj) {
  if (get_list_length(obj) != 3) return new_error_obj("vector-set!: wrong number of arguments, vector-set! requires 3");

  Object *fst = CAR(obj), *snd = CAR(CDR(obj)), *trd = CAR(CDR(CDR(obj)));
  if (fst->tag != OBJ_VECTOR) return new_error_obj("vector-set!: first argument is not vector");
  if (snd->tag != OBJ_INTEGER) return new_error_obj("vector-set!: second argument is not integer");

  long index = snd->fields_of.integer.value;
  if (index < 0) return new_error_obj("vector-set!: negative index is not allowed");
  if (index >= fst->fields_of.vector.size) return new_error_obj("vector-set!: index is larger than the size of vector");

  fst->fields_of.vector.vec_node->value[index] = trd;
  return UNDEF;
}
DEFINE_PRIM_OBJ(prim_vector_set)

static Object *prim_vector_length(Object *obj) {
  if (get_list_length(obj) != 1) return new_error_obj("vector-length: wrong number of arguments, vector-length requires 1");

  Object *fst = CAR(obj);
  if (fst->tag != OBJ_VECTOR) return new_error_obj("vector-set!: first argument is not vector");

  return new_integer_obj(fst->fields_of.vector.size);
}
DEFINE_PRIM_OBJ(prim_vector_length)
