#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>

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

struct VM {
  StackNode *s;
  Inst *c;
} VM;

VMPtr current_working_vm = NULL;

VMPtr new_vm(Inst *code) {
  VMPtr vm = calloc(1, sizeof(VM));
  vm->c = code;
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
        return ret_obj;
      } else {
        s_push(&vm->s, ret_obj);
      }
      break;
    }
    case INST_ARGS: {
      Object *args_list = NIL;
      RootNode *args_list_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&args_list);
      size_t args_num = vm->c->args_of.args.args_num;
      while (args_num--) {
        args_list = new_cell_obj(s_pop(&vm->s), NIL);
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

    Inst *next_inst = vm->c->next;
    free(vm->c);
    vm->c = next_inst;
  }
}

void vm_collect_roots(VMPtr vm) {
  for (StackNode *cur = vm->s; cur != NULL; cur = cur->next) {
    NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->item);
  }

  for (Inst *cur = vm->c; cur->tag != INST_STOP; cur = cur->next) {
    switch (cur->tag) {
    case INST_LDC:
      NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->args_of.ldc.constant);
      break;
    default:
      break;
    }
  }
}

void free_vm(VMPtr vm) {
  free_s(vm->s);
  free_code(vm->c);
  free(vm);
}

Object *apply(Object *proc, Object *args) {
  return proc->fields_of.primitive.fn(args);
}
