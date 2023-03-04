#include "fzscheme.h"

typedef struct StackNode StackNode;
struct StackNode {
  Object *item;
  StackNode *next;
};

static StackNode *new_stack_node(Object *item) {
  StackNode *stack = calloc(1, sizeof(StackNode));
  stack->item = item;
  return stack;
}

static void s_push(StackNode **s, Object *item) {
  StackNode *node = new_stack_node(item);
  node->next = *s;
  *s = node;
}

static Object *s_pop(StackNode **s) {
  if (*s == NULL) return NULL;
  Object *item = (*s)->item;
  StackNode *next = (*s)->next;
  free(*s);
  *s = next;
  return item;
}

static void free_s(StackNode *s) {
  StackNode *next;
  for (StackNode *cur = s; cur != NULL; cur = next) {
    next = cur->next;
    free(cur);
  }
}

static void stack_collect_roots(StackNode *node) {
  for (StackNode *cur = node; cur != NULL; cur = cur->next) {
    NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->item);
  }
}

typedef struct DumpNode DumpNode;
struct DumpNode {
  StackNode *stack;
  EnvNode *env;
  CodeNode *code;
  Inst *pc;
  DumpNode *next;
};

static DumpNode *new_dump_node(StackNode *stack, EnvNode *env, CodeNode *code, Inst *pc) {
  DumpNode *dump = calloc(1, sizeof(DumpNode));
  dump->stack = stack;
  dump->env = env;
  dump->code = code;
  dump->pc = pc;
  return dump;
}

static void d_push(DumpNode **d, StackNode *stack, EnvNode *env, CodeNode *code, Inst *pc) {
  DumpNode *node = new_dump_node(stack, env, code, pc);
  node->next = *d;
  *d = node;
}

static int d_pop(DumpNode **d, StackNode **stack, EnvNode **env, CodeNode **code, Inst **pc) {
  if (*d == NULL) {
    *stack = NULL;
    *env = NULL;
    *code = NULL;
    return -1;
  }
  if (stack) {
    *stack = (*d)->stack;
  }
  if (env) {
    *env = (*d)->env;
  }
  if (code) {
    *code = (*d)->code;
  }
  if (pc) {
    *pc = (*d)->pc;
  }
  DumpNode *next = (*d)->next;
  free(*d);
  *d = next;
  return 0;
}

static void free_d(DumpNode *d) {
  DumpNode *next;
  for (DumpNode *cur = d; cur != NULL; cur = cur->next) {
    next = cur->next;
    if (cur->stack) {
      free_s(cur->stack);
    }
    free(cur);
  }
}

static void dump_collect_roots(DumpNode *node) {
  for (DumpNode *cur = node; cur != NULL; cur = cur->next) {
    if (cur->stack) {
      stack_collect_roots(cur->stack);
    }
    if (cur->env) {
      env_collect_roots(cur->env);
    }
    if (cur->code) {
      code_collect_roots(cur->code);
    }
  }
}

struct VM {
  StackNode *s;
  EnvNode *e;
  CodeNode *c;
  DumpNode *d;
  Inst *pc;
} VM;

VMPtr current_working_vm = NULL;

VMPtr new_vm(Inst *code) {
  VMPtr vm = calloc(1, sizeof(VM));
  vm->e = NODE_TYPE_NEW_FUNC_NAME(EnvNode)(new_env());
  vm->c = NODE_TYPE_NEW_FUNC_NAME(CodeNode)(code);
  vm->pc = vm->c->value;
  return vm;
}

#define ERROR_MESSAGE_LEN 512

Object *apply(Object *proc, Object *args);

Object *vm_run(VMPtr vm) {
  current_working_vm = vm;
  char error_message[ERROR_MESSAGE_LEN] = {};
  for (;;) {
    switch (vm->pc->tag) {
    case INST_DEF: {
      Object *symbol = vm->pc->args_of.def.symbol;
      insert_to_global_env(symbol, s_pop(&vm->s));
      s_push(&vm->s, symbol);
      break;
    }
    case INST_LD: {
      int i = vm->pc->args_of.ld.i, j = vm->pc->args_of.ld.j;
      Object *lvar = get_lvar(vm->e, i, j);
      s_push(&vm->s, lvar);
      break;
    }
    case INST_LDC: {
      s_push(&vm->s, vm->pc->args_of.ldc.constant);
      break;
    }
    case INST_LDG: {
      Object *value = get_from_global_env(vm->pc->args_of.ldg.symbol);
      if (value != NULL) {
        s_push(&vm->s, value);
      } else {
        sprintf(error_message,
                "symbol `%s` is not found in the global environment",
                vm->pc->args_of.ldg.symbol->fields_of.symbol.name);
        current_working_vm = NULL;
        return new_error_obj(error_message);
      }
      break;
    }
    case INST_LDF:
      s_push(&vm->s, new_closure_obj(vm->pc->args_of.ldf.code, vm->e));
      break;
    case INST_APP: {
      Object *proc_obj = s_pop(&vm->s);
      RootNode *proc_obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&proc_obj);
      if (!(proc_obj->tag == OBJ_CLOSURE || proc_obj->tag == OBJ_PRIMITIVE)) {
        Object *error = new_error_obj("apply only closure or primitive object");
        if (!roots_is_empty()) {
          DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(proc_obj_node);
          free(proc_obj_node);
        }
        current_working_vm = NULL;
        return error;
      }

      Object *args_list = s_pop(&vm->s);
      RootNode *args_list_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&args_list);

      if (proc_obj->tag == OBJ_PRIMITIVE) {
        Object *ret_obj = apply(proc_obj, args_list);

        if (!roots_is_empty()) {
          DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(proc_obj_node);
          free(proc_obj_node);
          DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(args_list_node);
          free(args_list_node);
        }

        if (ret_obj->tag == OBJ_ERROR) {
          current_working_vm = NULL;
          return ret_obj;
        } else {
          s_push(&vm->s, ret_obj);
        }
      } else {
        d_push(&vm->d, vm->s, vm->e, vm->c, vm->pc->next);
        vm->s = NULL;
        CodeNode *clo_code = proc_obj->fields_of.closure.code_node;
        EnvNode *clo_env = proc_obj->fields_of.closure.env_node;
        EnvNode *nenv = NODE_TYPE_NEW_FUNC_NAME(EnvNode)(new_env());
        nenv->value->next = clo_env;
        nenv->value->vars = args_list;
        vm->e = nenv;
        vm->c = clo_code;
        vm->pc = vm->c->value;
        continue;
      }
      break;
    }
    case INST_RTN: {
      StackNode *save_s;
      EnvNode *save_e;
      CodeNode *save_c;
      Inst *save_pc;
      d_pop(&vm->d, &save_s, &save_e, &save_c, &save_pc);

      Object *result = s_pop(&vm->s);
      if (result->tag == OBJ_ERROR) {
        return result;
      }

      free_s(vm->s);
      vm->s = save_s;
      s_push(&vm->s, result);
      vm->e = save_e;
      vm->c = save_c;
      vm->pc = save_pc;
      continue;
    }
    case INST_SEL: {
      d_push(&vm->d, NULL, NODE_TYPE_NEW_FUNC_NAME(EnvNode)(new_env()), vm->c, vm->pc->next);
      Object *cond_obj = s_pop(&vm->s);
      if (cond_obj->tag != OBJ_BOOLEAN
          || (cond_obj->tag == OBJ_BOOLEAN && cond_obj == TRUE)) {
        vm->c = vm->pc->args_of.sel.t_clause;
      } else {
        vm->c = vm->pc->args_of.sel.f_clause;
      }
      vm->pc = vm->c->value;
      continue;
    }
    case INST_JOIN: {
      CodeNode *save_c;
      Inst *save_pc;
      d_pop(&vm->d, NULL, NULL, &save_c, &save_pc);
      vm->c = save_c;
      vm->pc = save_pc;
      continue;
    }
    case INST_POP:
      s_pop(&vm->s);
      break;
    case INST_ARGS: {
      Object *args_list = NIL;
      RootNode *args_list_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&args_list);
      size_t args_num = vm->pc->args_of.args.args_num;
      while (args_num--) {
        args_list = new_cell_obj(s_pop(&vm->s), args_list);
        if (roots_is_empty()) {
          args_list_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&args_list);
        }
      }
      s_push(&vm->s, args_list);
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(args_list_node);
      free(args_list_node);
      break;
    }
    case INST_STOP:
      current_working_vm = NULL;
      return s_pop(&vm->s);
    }

    vm->pc = vm->pc->next;
  }
}

void vm_collect_roots(VMPtr vm) {
  stack_collect_roots(vm->s);
  env_collect_roots(vm->e);
  code_collect_roots(vm->c);
  dump_collect_roots(vm->d);
}

void free_vm(VMPtr vm) {
  free_s(vm->s);
  free_d(vm->d);
  free(vm);
}

Object *apply(Object *proc, Object *args) {
  return proc->fields_of.primitive.fn(args);
}
