#include <stdlib.h>
#include "fzscheme.h"

Object *parse_objs(Token **tok) {
  Object *head = parse_obj(tok);
  RootNode *head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
  if (head == NULL) {
    DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(head_node);
    free(head_node);
    return NIL;
  } else {
    head = new_cell_obj(head, NIL);
    if (roots_is_empty()) {
      head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
    }
  }

  Object *cur = head;
  RootNode *cur_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur);
  Object *obj = NULL;
  RootNode *obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&obj);
  for (;;) {
    obj = parse_obj(tok);
    if (roots_is_empty()) {
      head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
      cur_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur);
      obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&obj);
    }
    if (obj == NULL) {
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(obj_node);
      free(obj_node);
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(cur_node);
      free(cur_node);
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(head_node);
      free(head_node);
      return head;
    } else {
      Object *cdr_obj = new_cell_obj(obj, NIL);
      if (roots_is_empty()) {
        head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
        cur_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur);
        obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&obj);
      }
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
    Object *obj = new_string_obj((*tok)->str);
    *tok = (*tok)->next;
    return obj;
  }
  case TK_TRUE:
    *tok = (*tok)->next;
    return TRUE;
  }
}
