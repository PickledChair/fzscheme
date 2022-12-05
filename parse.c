#include "fzscheme.h"

static Object *RPAREN_FOUND = &(Object){OBJ_UNDEF};

Object *parse_objs(Token **tok) {
  Object *head = parse_obj(tok);
  RootNode *head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
  if (head == NULL) {
    // error
    DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(head_node);
    free(head_node);
    return NULL;
  } else if (head == RPAREN_FOUND) {
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
  int dot_pos = 0;
  for (;;) {
    if ((*tok)->tag == TK_DOT) {
      dot_pos = -1;
      *tok = (*tok)->next;
    } else if (dot_pos < 0) {
      dot_pos--;
    }
    obj = parse_obj(tok);
    if (roots_is_empty()) {
      head_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&head);
      cur_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur);
      obj_node = NODE_TYPE_NEW_FUNC_NAME(RootNode)(&obj);
    }
    if (obj == NULL || obj == RPAREN_FOUND) {
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(obj_node);
      free(obj_node);
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(cur_node);
      free(cur_node);
      DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(RootNode)(head_node);
      free(head_node);
      if (obj == NULL) {
        // error
        return NULL;
      } else if (obj == RPAREN_FOUND) {
        if (dot_pos == 0 || dot_pos == -2) {
          return head;
        } else {
          printf("parse error: bad dot syntax\n");
          return NULL;
        }
      }
    } else {
      if (dot_pos == -1) {
        CDR(cur) = obj;
        cur = obj;
        continue;
      } else if (dot_pos < -1) {
        printf("parse error: bad dot syntax\n");
        return NULL;
      }
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
  case TK_DOT:
    *tok = (*tok)->next;
    printf("parse error: bad dot syntax\n");
    return NULL;
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
    Object *next_obj = parse_obj(tok);
    if (next_obj == NULL) return NULL;
    return new_cell_obj(obj, new_cell_obj(next_obj, NIL));
  }
  case TK_RPAREN:
    *tok = (*tok)->next;
    return RPAREN_FOUND;
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
