#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>

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
