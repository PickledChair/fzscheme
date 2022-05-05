#include "fzscheme.h"
#include <string.h>

Object *parse_objs(Token **tok) {
  Object *head = parse_obj(tok);
  if (head == NULL) {
    return NIL;
  } else {
    Object *new_head = new_cell_obj(head, NIL);
    if (head->tag == OBJ_MOVED) {
      CAR(new_head) = process_moved_obj(head);
    }
    head = new_head;
  }

  Object *cur = head;
  for (;;) {
    Object *obj = parse_obj(tok);
    if (obj == NULL) {
      return head;
    } else {
      Object *cdr_obj = new_cell_obj(obj, NIL);
      if (head->tag == OBJ_MOVED) {
        head = process_moved_obj(head);
        if (cur->tag == OBJ_MOVED)
          cur = cur->fields_of.moved.address;
        cdr_obj = process_moved_obj(cdr_obj);
      }
      cur = CDR(cur) = cdr_obj;
    }
  }
}

Object *parse_obj(Token **tok) {
  if (*tok == NULL) return NULL;

  switch ((*tok)->tag) {
  case TK_INT: {
    Object *obj = new_integer_obj((*tok)->val);
    *tok = (*tok)->next;
    return obj;
  }
  case TK_LPAREN:
    *tok = (*tok)->next;
    return parse_objs(tok);
  case TK_RPAREN:
    *tok = (*tok)->next;
    return NULL;
  case TK_STR: {
    Object *obj = new_string_obj(strdup((*tok)->str));
    *tok = (*tok)->next;
    return obj;
  }
  }
}
