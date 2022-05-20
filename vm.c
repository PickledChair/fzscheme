#include "fzscheme.h"
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

Object *vm_run(VMPtr vm) {
  current_working_vm = vm;
  for (;;) {
    switch (vm->c->tag) {
    case INST_LDC: {
      s_push(&vm->s, vm->c->args_of.ldc.constant);
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
    add_root(&cur->item);
  }

  for (Inst *cur = vm->c; cur->tag != INST_STOP; cur = cur->next) {
    switch (cur->tag) {
    case INST_LDC:
      add_root(&cur->args_of.ldc.constant);
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
