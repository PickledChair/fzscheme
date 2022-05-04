#include <stdint.h>

//
// object.c
//

typedef enum ObjectTag {
  OBJ_CELL,
  OBJ_INTEGER,
  OBJ_STRING,
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
  } fields_of;
};

#define CAR(obj) (obj)->fields_of.cell.car
#define CDR(obj) (obj)->fields_of.cell.cdr

Object *new_cell_obj(Object *car, Object *cdr);
Object *new_integer_obj(long value);
Object *new_string_obj(char *value);
void free_obj(Object *obj);
void print_obj(Object *obj);

extern Object *NIL;

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
