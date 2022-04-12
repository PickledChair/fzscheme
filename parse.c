#include "fzscheme.h"
#include <string.h>

Object *parse_objs(Token **tok) {
  Object *head = parse_obj(tok);
  if (head == NULL)
    return NIL;
  else
    head = new_cell_obj(head, NIL);

  Object *cur = head;
  for (;;) {
    Object *obj = parse_obj(tok);
    if (obj == NULL)
      return head;
    else
      cur = CDR(cur) = new_cell_obj(obj, NIL);
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
