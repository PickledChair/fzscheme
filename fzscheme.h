#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern bool debug_flag;

//
// string_list.c
//

typedef struct StringNode StringNode;
struct StringNode {
  char *value;
  StringNode *prev;
  StringNode *next;
};

StringNode *new_string_node(char *value);
void mark_string_node(StringNode *node);
void string_list_gc(void);
void clear_string_list(void);

//
// hash.c
//

uint32_t str_hash(char *str);

//
// object.c
//

typedef enum ObjectTag {
  OBJ_BOOLEAN,
  OBJ_CELL,
  OBJ_ERROR,
  OBJ_INTEGER,
  OBJ_STRING,
  OBJ_SYMBOL,
  OBJ_MOVED,
} ObjectTag;

typedef struct Object Object;
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
      StringNode *message;
    } error;

    struct {
      long value;
    } integer;

    struct {
      StringNode *str_node;
    } string;

    struct {
      char *name;
    } symbol;

    struct {
      Object *address;
    } moved;
  } fields_of;
};

#define CAR(obj) (obj)->fields_of.cell.car
#define CDR(obj) (obj)->fields_of.cell.cdr

#define CHECK_OBJ_MOVING(obj)\
  if ((obj)->tag == OBJ_MOVED) {\
    (obj) = (obj)->fields_of.moved.address;\
  }

Object *new_cell_obj(Object *car, Object *cdr);
Object *new_error_obj(char *message);
Object *new_integer_obj(long value);
Object *new_string_obj(char *value);
Object *new_symbol_obj(char *name);
void free_symbol_obj(Object *obj);
// void free_obj(Object *obj);
void print_obj(Object *obj);
// Object *process_moved_obj(Object *obj);

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
void clear_symbol_table(void);

//
// gc.c
//

void fzscm_memspace_init(size_t semispace_size);
void fzscm_memspace_fin(void);
void reset_gc_state(void);
void fzscm_gc(void);
void *fzscm_alloc(size_t size);

typedef struct RootNode RootNode;
struct RootNode {
  Object **obj;
  RootNode *next;
};

RootNode *get_roots(void);
void add_root(Object **obj);
void clear_roots(void);

//
// tokenize.c
//

typedef enum {
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

//
// compiler.c
//

typedef enum {
  INST_DEF,
  INST_LDC,
  INST_LDG,
  INST_STOP,
} InstTag;

typedef struct Inst Inst;
struct Inst {
  InstTag tag;
  Inst *next;

  union {
    struct {
      Object *symbol;
    } def;

    struct {
      Object *constant;
    } ldc;

    struct {
      Object *symbol;
    } ldg;
  } args_of;
};

void free_code(Inst *code);
void print_code(Inst *code, int level);
Inst *compile(Object *ast);

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
// repl.c
//

int repl(void);
