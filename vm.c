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
  Inst *code;
  DumpNode *next;
};

static DumpNode *new_dump_node(StackNode *stack, Inst *code) {
  DumpNode *dump = calloc(1, sizeof(DumpNode));
  dump->stack = stack;
  dump->code = code;
  return dump;
}

static void d_push(DumpNode **d, StackNode *stack, Inst *code) {
  DumpNode *node = new_dump_node(stack, code);
  node->next = *d;
  *d = node;
}

static int d_pop(DumpNode **d, StackNode **stack, Inst **code) {
  if (*d == NULL) {
    *stack = NULL;
    *code = NULL;
    return -1;
  }
  if (stack) {
    *stack = (*d)->stack;
  }
  if (code) {
    *code = (*d)->code;
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
    if (cur->code) {
      code_collect_roots(cur->code);
    }
  }
}

struct VM {
  StackNode *s;
  Inst *c;
  DumpNode *d;
  Inst *code_top;
} VM;

VMPtr current_working_vm = NULL;

VMPtr new_vm(Inst *code) {
  VMPtr vm = calloc(1, sizeof(VM));
  vm->c = vm->code_top = code;
  return vm;
}

#define ERROR_MESSAGE_LEN 512

Object *apply(Object *prim, Object *args);

Object *vm_run(VMPtr vm) {
  current_working_vm = vm;
  char error_message[ERROR_MESSAGE_LEN] = {};
  for (;;) {
    switch (vm->c->tag) {
    case INST_DEF: {
      Object *symbol = vm->c->args_of.def.symbol;
      insert_to_global_env(symbol, s_pop(&vm->s));
      s_push(&vm->s, symbol);
      break;
    }
    case INST_LDC: {
      s_push(&vm->s, vm->c->args_of.ldc.constant);
      break;
    }
    case INST_LDG: {
      Object *value = get_from_global_env(vm->c->args_of.ldg.symbol);
      if (value != NULL) {
        s_push(&vm->s, value);
      } else {
        sprintf(error_message,
                "symbol `%s` is not found in the global environment",
                vm->c->args_of.ldg.symbol->fields_of.symbol.name);
        current_working_vm = NULL;
        return new_error_obj(error_message);
      }
      break;
    }
    case INST_APP: {
      Object *proc_obj = s_pop(&vm->s);
      RootNode *proc_obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&proc_obj);
      if (!(proc_obj->tag == OBJ_PRIMITIVE)) {
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
      break;
    }
    case INST_SEL: {
      d_push(&vm->d, NULL, vm->c->next);
      Object *cond_obj = s_pop(&vm->s);
      if (cond_obj->tag != OBJ_BOOLEAN
          || (cond_obj->tag == OBJ_BOOLEAN && cond_obj == TRUE)) {
        vm->c = vm->c->args_of.sel.t_clause;
      } else {
        vm->c = vm->c->args_of.sel.f_clause;
      }
      continue;
    }
    case INST_JOIN: {
      Inst *inst_node;
      d_pop(&vm->d, NULL, &inst_node);
      vm->c = inst_node;
      continue;
    }
    case INST_ARGS: {
      Object *args_list = NIL;
      RootNode *args_list_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&args_list);
      size_t args_num = vm->c->args_of.args.args_num;
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

    vm->c = vm->c->next;
  }
}

void vm_collect_roots(VMPtr vm) {
  stack_collect_roots(vm->s);

  code_collect_roots(vm->c);
  dump_collect_roots(vm->d);
}

void free_vm(VMPtr vm, bool also_free_code) {
  free_s(vm->s);
  if (also_free_code) free_code(vm->code_top);
  free_d(vm->d);
  free(vm);
}

Object *apply(Object *proc, Object *args) {
  return proc->fields_of.primitive.fn(args);
}
