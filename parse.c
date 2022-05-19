#include "fzscheme.h"
#include <string.h>

Object *parse_objs(Token **tok) {
  Object *head = parse_obj(tok);
  if (head == NULL) {
    return NIL;
  } else {
    head = new_cell_obj(head, NIL);
  }

  Object *cur = head;
  for (;;) {
    Object *obj = parse_obj(tok);
    if (obj == NULL) {
      return head;
    } else {
      Object *cdr_obj = new_cell_obj(obj, NIL);
      CHECK_OBJ_MOVING(head);
      CHECK_OBJ_MOVING(cur);
      cur = CDR(cur) = cdr_obj;
    }
  }
}

Object *parse_obj(Token **tok) {
  if (*tok == NULL) return NULL;

  switch ((*tok)->tag) {
  case TK_FALSE:
    *tok = (*tok)->next;
    return FALSE;
  case TK_IDENT: {
    Object *obj = intern_name((*tok)->str);
    *tok = (*tok)->next;
    return obj;
  }
  case TK_INT: {
    Object *obj = new_integer_obj((*tok)->val);
    *tok = (*tok)->next;
    return obj;
  }
  case TK_LPAREN:
    *tok = (*tok)->next;
    return parse_objs(tok);
  case TK_QUOTE: {
    Object *obj = intern_name("quote");
    *tok = (*tok)->next;
    return new_cell_obj(obj, new_cell_obj(parse_obj(tok), NIL));
  }
  case TK_RPAREN:
    *tok = (*tok)->next;
    return NULL;
  case TK_STR: {
    Object *obj = new_string_obj(strdup((*tok)->str));
    *tok = (*tok)->next;
    return obj;
  }
  case TK_TRUE:
    *tok = (*tok)->next;
    return TRUE;
  }
}
