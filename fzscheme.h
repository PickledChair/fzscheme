#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern bool debug_flag;

//
// object.c
//

typedef enum ObjectTag {
  OBJ_CELL,
  OBJ_INTEGER,
  OBJ_STRING,
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
      char *value;
    } string;

    struct {
      Object *address;
    } moved;
  } fields_of;
};

#define CAR(obj) (obj)->fields_of.cell.car
#define CDR(obj) (obj)->fields_of.cell.cdr

Object *new_cell_obj(Object *car, Object *cdr);
Object *new_integer_obj(long value);
Object *new_string_obj(char *value);
// void free_obj(Object *obj);
void print_obj(Object *obj);
Object *process_moved_obj(Object *obj);

extern Object *NIL;

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
