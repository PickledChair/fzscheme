#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doubly_linked_list.h"

extern bool debug_flag;

//
// string_list.c
//

// typedef struct StringNode StringNode;
// struct StringNode {
//   char *value;
//   StringNode *prev;
//   StringNode *next;
// };
DEFINE_NODE_TYPE(StringNode, char *)

StringNode *NODE_TYPE_NEW_FUNC_NAME(StringNode)(char *value);
void mark_string_node(StringNode *node);
void string_list_gc(void);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(StringNode)(void);

//
// env.c
//

typedef struct Object Object;

typedef struct Env Env;
DEFINE_NODE_TYPE(EnvNode, Env *)

struct Env {
  Object *vars;
  EnvNode *next;
};

Env *new_env(void);
void env_collect_roots(EnvNode *env);
int location(EnvNode *env, Object *symbol, int *i, int *j);
Object *get_lvar(EnvNode *env, int i, int j);
int set_lvar(EnvNode *env, int i, int j, Object *val);

EnvNode *NODE_TYPE_NEW_FUNC_NAME(EnvNode)(Env * env);
void mark_env_node(EnvNode *node);
void env_list_gc(void);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(EnvNode)(void);

//
// vector_list.c
//

DEFINE_NODE_TYPE(ObjectVectorNode, Object **)

ObjectVectorNode *NODE_TYPE_NEW_FUNC_NAME(ObjectVectorNode)(Object **objs);
void mark_vector_node(ObjectVectorNode *node);
void vector_list_gc(void);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(ObjectVectorNode)(void);


//
// hash.c
//

uint32_t str_hash(char *str);

//
// object.c
//

typedef struct Inst Inst;
DEFINE_NODE_TYPE(CodeNode, Inst *)

typedef enum ObjectTag {
  OBJ_UNDEF,
  OBJ_BOOLEAN,
  OBJ_CELL,
  OBJ_CLOSURE,
  OBJ_ERROR,
  OBJ_INTEGER,
  OBJ_PRIMITIVE,
  OBJ_STRING,
  OBJ_SYMBOL,
  OBJ_MOVED,
  OBJ_VECTOR,
} ObjectTag;

typedef Object *(*PrimFunc)(Object *obj);
struct Object {
  ObjectTag tag;
  union {
    struct {
      bool value;
    } boolean;

    struct {
      Object *car;
      Object *cdr;
    } cell;

    struct {
      CodeNode *code_node;
      EnvNode *env_node;
    } closure;

    struct {
      StringNode *message;
    } error;

    struct {
      long value;
    } integer;

    struct {
      Object *symbol;
      PrimFunc fn;
    } primitive;

    struct {
      StringNode *str_node;
    } string;

    struct {
      char *name;
    } symbol;

    struct {
      Object *address;
    } moved;

    struct {
      ObjectVectorNode *vec_node;
      size_t size;
    } vector;
  } fields_of;
};

#define CAR(obj) (obj)->fields_of.cell.car
#define CDR(obj) (obj)->fields_of.cell.cdr

#define CHECK_OBJ_MOVING(obj)               \
  if ((obj)->tag == OBJ_MOVED) {            \
    (obj) = (obj)->fields_of.moved.address; \
  }

Object *new_cell_obj(Object *car, Object *cdr);
Object *new_closure_obj(CodeNode *code, EnvNode *env);
Object *new_error_obj(char *message);
Object *new_integer_obj(long value);
Object *new_string_obj(char *value);
Object *new_symbol_obj(char *name);
void free_symbol_obj(Object *obj);
Object *new_vector_obj(size_t size, Object *fill);
// void free_obj(Object *obj);
void print_obj(Object *obj);
// Object *process_moved_obj(Object *obj);
int get_list_length(Object *obj);

extern Object *UNDEF;
extern Object *NIL;
extern Object *TRUE;
extern Object *FALSE;

//
// symbol_table.c
//

Object *intern_name(char *name);
Object *insert_to_global_env(Object *symbol, Object *value);
Object *get_from_global_env(Object *symbol);
void global_env_collect_roots(void);
void init_symbol_table(void);
void clear_symbol_table(void);

//
// gc.c
//

void fzscm_memspace_init(size_t semispace_size);
void fzscm_memspace_fin(void);
void reset_gc_state(void);
void fzscm_gc(void);
void *fzscm_alloc(size_t size);

// typedef struct RootNode RootNode;
// struct RootNode {
//   Object **value;
//   RootNode *prev;
//   RootNode *next;
// };
DEFINE_NODE_TYPE(RootNode, Object **)

RootNode *get_roots(void);
bool roots_is_empty(void);
RootNode *NODE_TYPE_NEW_FUNC_NAME(RootNode)(Object **value);
void DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(RootNode *node);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(RootNode)(void);

//
// tokenize.c
//

typedef enum {
  TK_DOT,
  TK_FALSE,
  TK_IDENT,
  TK_INT,
  TK_LPAREN,
  TK_QUOTE,
  TK_RPAREN,
  TK_STR,
  TK_TRUE,
} TokenTag;

typedef struct Token Token;
struct Token {
  TokenTag tag;
  Token *next;
  char *loc;
  int len;
  int64_t val;
  char *str;
};

Token *tokenize(char *input);
void print_token(Token *tok);
void free_token(Token *tok);

//
// parse.c
//

Object *parse_obj(Token **tok);
Object *parse_objs(Token **tok);

DEFINE_NODE_TYPE(ParserRootNode, Object **)

ParserRootNode *get_parser_roots(void);
bool parser_roots_is_empty(void);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(ParserRootNode)(void);

//
// compiler.c
//

typedef enum {
  INST_DEF,
  INST_LD,
  INST_LDC,
  INST_LDG,
  INST_LDF,
  INST_ARGS,
  INST_APP,
  INST_RTN,
  INST_SEL,
  INST_JOIN,
  INST_POP,
  INST_STOP,
} InstTag;

struct Inst {
  InstTag tag;
  Inst *next;

  union {
    struct {
      Object *symbol;
    } def;

    struct {
      int i;
      int j;
    } ld;

    struct {
      Object *constant;
    } ldc;

    struct {
      Object *symbol;
    } ldg;

    struct {
      CodeNode *code;
    } ldf;

    struct {
      size_t args_num;
    } args;

    struct {
      CodeNode *t_clause;
      CodeNode *f_clause;
    } sel;
  } args_of;
};

void free_code(Inst *code);
void print_code(Inst *code, int level);
void code_collect_roots(CodeNode *code);
Inst *compile(Object *ast);

CodeNode *NODE_TYPE_NEW_FUNC_NAME(CodeNode)(Inst *code);
void mark_code_node(CodeNode *node);
void code_list_gc(void);
void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(CodeNode)(void);

//
// vm.c
//

typedef struct VM *VMPtr;
extern VMPtr current_working_vm;

VMPtr new_vm(Inst *code);
Object *vm_run(VMPtr vm);
void vm_collect_roots(VMPtr vm);
void free_vm(VMPtr vm);

//
// primitive.c
//

extern Object *prim_car_obj;
extern Object *prim_cdr_obj;
extern Object *prim_cons_obj;
extern Object *prim_display_obj;
extern Object *prim_newline_obj;
extern Object *prim_eq_obj;
extern Object *prim_eqv_obj;
extern Object *prim_equal_obj;
extern Object *prim_plus_obj;
extern Object *prim_times_obj;
extern Object *prim_minus_obj;
extern Object *prim_div_obj;
extern Object *prim_modulo_obj;
extern Object *prim_exit_obj;
extern Object *prim_make_vector_obj;
extern Object *prim_vector_ref_obj;
extern Object *prim_vector_set_obj;
extern Object *prim_vector_length_obj;

//
// repl.c
//

int repl(void);

//
// fzscheme.c
//

void fzscm_deinit(void);
