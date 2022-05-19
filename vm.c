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

struct VM {
  StackNode *s;
  Inst *c;
} VM;

VMPtr new_vm(Inst *code) {
  VMPtr vm = calloc(1, sizeof(VM));
  vm->c = code;
  return vm;
}

Object *vm_run(VMPtr vm) {
  Inst *cur_c = vm->c;
  for (;;) {
    switch (cur_c->tag) {
    case INST_LDC: {
      StackNode *s_node = new_stack_node(cur_c->args_of.ldc.constant);
      s_node->next = vm->s;
      vm->s = s_node;
      break;
    }
    case INST_STOP:
      return vm->s->item;
    }

    cur_c = cur_c->next;
  }
}

void free_vm(VMPtr vm) {
  free(vm->s);
  free_code(vm->c);
  free(vm);
}
