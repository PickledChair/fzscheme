#include "fzscheme.h"

static Inst *new_inst(InstTag tag) {
  Inst *inst = calloc(1, sizeof(Inst));
  inst->tag = tag;
  return inst;
}

static Inst *inst_stop(void) { return new_inst(INST_STOP); }

static Inst *inst_def(Object *symbol) {
  Inst *inst = new_inst(INST_DEF);
  inst->args_of.def.symbol = symbol;
  return inst;
}

static Inst *inst_ldc(Object *constant) {
  Inst *inst = new_inst(INST_LDC);
  inst->args_of.ldc.constant = constant;
  return inst;
}

static Inst *inst_ldg(Object *symbol) {
  Inst *inst = new_inst(INST_LDG);
  inst ->args_of.ldg.symbol = symbol;
  return inst;
}

static Inst *inst_args(size_t args_num) {
  Inst *inst = new_inst(INST_ARGS);
  inst->args_of.args.args_num = args_num;
  return inst;
}

static Inst *inst_sel(Inst *t_clause, Inst *f_clause) {
  Inst *inst = new_inst(INST_SEL);
  inst->args_of.sel.t_clause = t_clause;
  inst->args_of.sel.f_clause = f_clause;
  return inst;
}

static Inst *inst_join(void) { return new_inst(INST_JOIN); }

static Inst *inst_app(void) { return new_inst(INST_APP); }

void free_code(Inst *code) {
  Inst *next;
  for (Inst *cur = code; cur != NULL; cur = next) {
    if (cur->tag == INST_SEL) {
      free_code(cur->args_of.sel.t_clause);
      free_code(cur->args_of.sel.f_clause);
    }
    next = cur->next;
    free(cur);
  }
}

void print_code(Inst *code, int level) {
  for (Inst *cur = code; cur != NULL; cur = cur->next) {
    for (int i = 0; i < level; i++) {
      putchar(' ');
    }
    switch (cur->tag) {
    case INST_DEF:
      printf("def ");
      print_obj(cur->args_of.def.symbol);
      putchar('\n');
      break;
    case INST_LDC:
      printf("ldc ");
      print_obj(cur->args_of.ldc.constant);
      putchar('\n');
      break;
    case INST_LDG:
      printf("ldg ");
      print_obj(cur->args_of.ldg.symbol);
      putchar('\n');
      break;
    case INST_ARGS:
      printf("args %zu\n", cur->args_of.args.args_num);
      break;
    case INST_APP:
      printf("app\n");
      break;
    case INST_SEL:
      printf("sel\n");
      for (int i = 0; i < level; i++) {
        putchar(' ');
      }
      printf("then clause:\n");
      print_code(cur->args_of.sel.t_clause, level + 1);
      for (int i = 0; i < level; i++) {
        putchar(' ');
      }
      printf("else clause:\n");
      print_code(cur->args_of.sel.f_clause, level + 1);
      break;
    case INST_JOIN:
      printf("join\n");
      break;
    case INST_STOP:
      printf("stop\n");
      break;
    }
  }
}

void code_collect_roots(Inst *code) {
  for (Inst *cur = code; cur->tag != INST_STOP; cur = cur->next) {
    switch (cur->tag) {
    case INST_LDC:
      NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->args_of.ldc.constant);
      break;
    default:
      break;
    }
  }
}

static size_t get_args_len(Object *obj) {
  size_t len = 0;
  Object *cur = obj;
  while (cur != NIL) {
    len++;
    if (CDR(cur)->tag != OBJ_CELL) {
      len++;
      break;
    }
    cur = CDR(cur);
  }
  // 先頭の要素の分を引く
  return len - 1;
}

static Inst *compile_list(Object *list, Inst *code);

static Inst *compile_expr(Object *obj, Inst *code) {
  switch (obj->tag) {
  case OBJ_BOOLEAN:
  case OBJ_INTEGER:
  case OBJ_STRING: {
    Inst *ldc_code = inst_ldc(obj);
    ldc_code->next = code;
    return ldc_code;
  }
  case OBJ_SYMBOL: {
    Inst *ldg_code = inst_ldg(obj);
    ldg_code->next = code;
    return ldg_code;
  }
  case OBJ_CELL:
    if (obj != NIL) {
      if (CAR(obj) == intern_name("quote")) {
        if (CDR(obj)->tag == OBJ_CELL && CDR(obj) != NIL) {
          Inst *ldc_code = inst_ldc(CAR(CDR(obj)));
          ldc_code->next = code;
          return ldc_code;
        } else {
          printf("compile error: shortage of the args of `quote`\n");
          free_code(code);
          return NULL;
        }
      }

      if (CAR(obj) == intern_name("if")) {
        if (CDR(obj) != NIL && CDR(CDR(obj)) != NIL) {
          Object *second = CAR(CDR(obj));
          Object *third = CAR(CDR(CDR(obj)));
          Object *forth = CDR(CDR(CDR(obj))) == NIL ? NULL : CAR(CDR(CDR(CDR(obj))));

          Inst *t_clause = compile_expr(third, inst_join());
          Inst *f_clause = NULL;
          if (forth) {
            f_clause = compile_expr(forth, inst_join());
          } else {
            f_clause = inst_ldc(UNDEF);
            f_clause->next = inst_join();
          }
          Inst *sel_code = inst_sel(t_clause, f_clause);
          sel_code->next = code;
          return compile_expr(second, sel_code);
        } else {
          printf("compile error: shortage of the args of `if`\n");
          free_code(code);
          return NULL;
        }
      }

      if (CAR(obj) == intern_name("define")) {
        if (CDR(obj) != NIL && CDR(CDR(obj)) != NIL) {
          Object *second = CAR(CDR(obj)), *third = CAR(CDR(CDR(obj)));
          if (second->tag != OBJ_SYMBOL) {
            printf("compile error: only syntax (define name value) is supported\n");
            free_code(code);
            return NULL;
          }
          Inst *def_code = inst_def(second);
          def_code->next = code;
          return compile_expr(third, def_code);
        }
      }

      Inst *args_code = inst_args(get_args_len(obj));
      Inst *app_code = inst_app();
      app_code->next = code;
      args_code->next = compile_expr(CAR(obj), app_code);

      return compile_list(CDR(obj), args_code);
    } else {
      printf("compile error: attempt to evaluate nil\n");
      free_code(code);
      return NULL;
    }
  default:
    printf("compile error: not yet implemented instructions\n");
    free_code(code);
    return NULL;
  }
}

static Inst *compile_list(Object *list, Inst *code) {
  if (list == NIL) {
    return code;
  } else {
    if (list->tag == OBJ_CELL) {
      Inst *compiled_list = compile_list(CDR(list), code);
      return compile_expr(CAR(list), compiled_list);
    } else {
      // for pair
      return compile_expr(list, code);
    }
  }
}

Inst *compile(Object *ast) {
  Inst *stop_code = inst_stop();
  Inst *result_code = compile_expr(ast, stop_code);
  return result_code;
}
