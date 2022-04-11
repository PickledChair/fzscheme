//
// object.c
//

typedef enum ObjectTag {
  OBJ_TAG_CELL,
  OBJ_TAG_INTEGER,
  OBJ_TAG_STRING,
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
