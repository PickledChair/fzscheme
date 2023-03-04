#include "fzscheme.h"

DEFINE_DOUBLY_LINKED_LIST_FUNCS(CodeNode, Inst, true)
static CodeNode *marked_code_list_head = &(CodeNode){};

void mark_code_node(CodeNode *node) {
  DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(CodeNode)(node);
  DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(CodeNode)(node, &marked_code_list_head);
}

void code_list_gc(void) {
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(CodeNode)();
  if (marked_code_list_head->next != NULL) {
    DOUBLY_LINKED_LIST_HEAD_NAME(CodeNode)->next = marked_code_list_head->next;
    marked_code_list_head->next->prev = DOUBLY_LINKED_LIST_HEAD_NAME(CodeNode);

    marked_code_list_head->next = NULL;
  }
}

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

static Inst *inst_ld(int i, int j) {
  Inst *inst = new_inst(INST_LD);
  inst->args_of.ld.i = i;
  inst->args_of.ld.j = j;
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

static Inst *inst_ldf(Inst *code) {
  Inst *inst = new_inst(INST_LDF);
  inst->args_of.ldf.code = NODE_TYPE_NEW_FUNC_NAME(CodeNode)(code);
  return inst;
}

static Inst *inst_args(size_t args_num) {
  Inst *inst = new_inst(INST_ARGS);
  inst->args_of.args.args_num = args_num;
  return inst;
}

static Inst *inst_sel(Inst *t_clause, Inst *f_clause) {
  Inst *inst = new_inst(INST_SEL);
  inst->args_of.sel.t_clause = NODE_TYPE_NEW_FUNC_NAME(CodeNode)(t_clause);
  inst->args_of.sel.f_clause = NODE_TYPE_NEW_FUNC_NAME(CodeNode)(f_clause);
  return inst;
}

static Inst *inst_pop(void) { return new_inst(INST_POP); }

static Inst *inst_join(void) { return new_inst(INST_JOIN); }

static Inst *inst_app(void) { return new_inst(INST_APP); }

static Inst *inst_rtn(void) { return new_inst(INST_RTN); }

void free_code(Inst *code) {
  Inst *next;
  for (Inst *cur = code; cur != NULL; cur = next) {
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
    case INST_LD:
      printf("ld (%d . %d)\n", cur->args_of.ld.i, cur->args_of.ld.j);
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
    case INST_LDF:
      printf("ldf\n");
      for (int i = 0; i < level; i++) {
        putchar(' ');
      }
      printf("code:\n");
      print_code(cur->args_of.ldf.code->value, level + 1);
      break;
    case INST_ARGS:
      printf("args %zu\n", cur->args_of.args.args_num);
      break;
    case INST_APP:
      printf("app\n");
      break;
    case INST_RTN:
      printf("rtn\n");
      break;
    case INST_SEL:
      printf("sel\n");
      for (int i = 0; i < level; i++) {
        putchar(' ');
      }
      printf("then clause:\n");
      print_code(cur->args_of.sel.t_clause->value, level + 1);
      for (int i = 0; i < level; i++) {
        putchar(' ');
      }
      printf("else clause:\n");
      print_code(cur->args_of.sel.f_clause->value, level + 1);
      break;
    case INST_JOIN:
      printf("join\n");
      break;
    case INST_POP:
      printf("pop\n");
      break;
    case INST_STOP:
      printf("stop\n");
      break;
    }
  }
}

void code_collect_roots(CodeNode *code) {
  mark_code_node(code);
  for (Inst *cur = code->value; cur != NULL; cur = cur->next) {
    switch (cur->tag) {
    case INST_LDC:
      NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->args_of.ldc.constant);
      break;
    case INST_LDF:
      mark_code_node(cur->args_of.ldf.code);
      code_collect_roots(cur->args_of.ldf.code);
      break;
    case INST_SEL:
      mark_code_node(cur->args_of.sel.t_clause);
      code_collect_roots(cur->args_of.sel.t_clause);

      mark_code_node(cur->args_of.sel.f_clause);
      code_collect_roots(cur->args_of.sel.f_clause);
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

static Inst *compile_list(Object *list, EnvNode *env, Inst *code);
static Inst *compile_body(Object *body, EnvNode *env, Inst *code);

static Inst *compile_expr(Object *obj, EnvNode *env, Inst *code) {
  switch (obj->tag) {
  case OBJ_BOOLEAN:
  case OBJ_INTEGER:
  case OBJ_STRING: {
    Inst *ldc_code = inst_ldc(obj);
    ldc_code->next = code;
    return ldc_code;
  }
  case OBJ_SYMBOL: {
    int i, j;
    if (location(env, obj, &i, &j) == 0) {
      Inst *ld_code = inst_ld(i, j);
      ld_code->next = code;
      return ld_code;
    } else {
      Inst *ldg_code = inst_ldg(obj);
      ldg_code->next = code;
      return ldg_code;
    }
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
          return NULL;
        }
      }

      if (CAR(obj) == intern_name("if")) {
        if (CDR(obj) != NIL && CDR(CDR(obj)) != NIL) {
          Object *second = CAR(CDR(obj));
          Object *third = CAR(CDR(CDR(obj)));
          Object *forth = CDR(CDR(CDR(obj))) == NIL ? NULL : CAR(CDR(CDR(CDR(obj))));

          Inst *join_code = inst_join();
          Inst *t_clause = compile_expr(third, env, join_code);
          if (t_clause == NULL) {
            return NULL;
          }
          Inst *f_clause = NULL;
          if (forth) {
            f_clause = compile_expr(forth, env, inst_join());
            if (f_clause == NULL) {
              return NULL;
            }
          } else {
            f_clause = inst_ldc(UNDEF);
            f_clause->next = inst_join();
          }
          Inst *sel_code = inst_sel(t_clause, f_clause);
          sel_code->next = code;
          return compile_expr(second, env, sel_code);
        } else {
          printf("compile error: shortage of the args of `if`\n");
          return NULL;
        }
      }

      if (CAR(obj) == intern_name("lambda")) {
        if (CDR(obj) != NIL && CDR(CDR(obj)) != NIL) {
          Object *args = CAR(CDR(obj)), *body = CDR(CDR(obj));

          EnvNode *nenv = NODE_TYPE_NEW_FUNC_NAME(EnvNode)(new_env());
          nenv->value->vars = args;
          nenv->value->next = env;

          Inst *rtn_code = inst_rtn();
          Inst *body_code = compile_body(body, nenv, rtn_code);
          if (body_code == NULL) {
            return NULL;
          }
          Inst *ldf_code = inst_ldf(body_code);
          ldf_code->next = code;
          return ldf_code;
        } else {
          printf("compile error: shortage of the args of `lambda`\n");
          return NULL;
        }
      }

      if (CAR(obj) == intern_name("define")) {
        if (CDR(obj) != NIL && CDR(CDR(obj)) != NIL) {
          Object *second = CAR(CDR(obj)), *third = CAR(CDR(CDR(obj)));
          if (second->tag != OBJ_SYMBOL) {
            printf("compile error: only syntax (define name value) is supported\n");
            return NULL;
          }
          Inst *def_code = inst_def(second);
          def_code->next = code;
          return compile_expr(third, env, def_code);
        }
      }

      Inst *args_code = inst_args(get_args_len(obj));
      Inst *app_code = inst_app();
      app_code->next = code;
      args_code->next = compile_expr(CAR(obj), env, app_code);

      return compile_list(CDR(obj), env, args_code);
    } else {
      printf("compile error: attempt to evaluate nil\n");
      return NULL;
    }
  default:
    printf("compile error: not yet implemented instructions\n");
    return NULL;
  }
}

static Inst *compile_list(Object *list, EnvNode *env, Inst *code) {
  if (list == NIL) {
    return code;
  } else {
    if (list->tag == OBJ_CELL) {
      Inst *compiled_list = compile_list(CDR(list), env, code);
      return compile_expr(CAR(list), env, compiled_list);
    } else {
      // for pair
      return compile_expr(list, env, code);
    }
  }
}

static Inst *compile_body(Object *body, EnvNode *env, Inst *code) {
  switch (get_list_length(body)) {
    case 0:
      printf("prevent body to be empty by following code\n");
      return NULL;
    case 1:
      return compile_expr(CAR(body), env, code);
    default: {
      Inst *pop_code = inst_pop();
      Inst *rest_code = compile_body(CDR(body), env, code);
      pop_code->next = rest_code;
      return compile_expr(CAR(body), env, pop_code);
    }
  }
}

Inst *compile(Object *ast) {
  Inst *stop_code = inst_stop();
  Inst *result_code = compile_expr(ast, NODE_TYPE_NEW_FUNC_NAME(EnvNode)(new_env()), stop_code);
  return result_code;
}
