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
  OBJ_CELL,
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
      Object *car;
      Object *cdr;
    } cell;

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

#define CHECK_OBJ_MOVING(obj) if ((obj)->tag == OBJ_MOVED) {\
    (obj) = (obj)->fields_of.moved.address; \
  }

Object *new_cell_obj(Object *car, Object *cdr);
Object *new_integer_obj(long value);
Object *new_string_obj(char *value);
Object *new_symbol_obj(char *name);
void free_symbol_obj(Object *obj);
// void free_obj(Object *obj);
void print_obj(Object *obj);
// Object *process_moved_obj(Object *obj);

extern Object *NIL;

//
// symbol_table.c
//

Object *intern_name(char *name);
void clear_symbol_table(void);

//
// gc.c
//

void fzscm_memspace_init(size_t semispace_size);
void fzscm_memspace_fin(void);
void reset_fresh_obj_count(void);
void fzscm_gc(void);
void *fzscm_alloc(size_t size);

typedef struct RootNode RootNode;
struct RootNode {
  Object *obj;
  RootNode *next;
};

RootNode *get_roots(void);
void add_root(Object *obj);
void clear_roots(void);

//
// tokenize.c
//

typedef enum {
  TK_IDENT,
  TK_INT,
  TK_LPAREN,
  TK_RPAREN,
  TK_STR,
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
// repl.c
//

int repl(void);
