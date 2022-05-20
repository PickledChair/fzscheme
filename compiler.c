#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>

static Inst *new_inst(InstTag tag) {
  Inst *inst = calloc(1, sizeof(Inst));
  inst->tag = tag;
  return inst;
}

static Inst *inst_stop(void) {
  return new_inst(INST_STOP);
}

static Inst *inst_ldc(Object *constant) {
  Inst *inst = new_inst(INST_LDC);
  inst->args_of.ldc.constant = constant;
  return inst;
}

void free_code(Inst *code) {
  Inst *next;
  for (Inst *cur = code; cur != NULL; cur = next) {
    next = cur->next;
    free(cur);
  }
}

void print_code(Inst *code, int level) {
  for (int i = 0; i < level; i++) {
    putchar(' ');
  }
  for (Inst *cur = code; cur != NULL; cur = cur->next) {
    switch (cur->tag) {
    case INST_LDC:
      printf("ldc ");
      print_obj(cur->args_of.ldc.constant);
      putchar('\n');
      break;
    case INST_STOP:
      printf("stop\n");
      break;
    }
  }
}

static Inst *compile_expr(Object *obj, Inst *code) {
  switch (obj->tag) {
  case OBJ_BOOLEAN:
  case OBJ_INTEGER:
  case OBJ_STRING: {
    Inst *ldc_code = inst_ldc(obj);
    ldc_code->next = code;
    return ldc_code;
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
    }
    // break;
  default:
    printf("compile error: not yet implemented instructions\n");
    free_code(code);
    return NULL;
  }
}

Inst *compile(Object *ast) {
  Inst *stop_code = inst_stop();
  Inst *result_code = compile_expr(ast, stop_code);
  return result_code;
}
