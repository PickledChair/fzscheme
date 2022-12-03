#include "fzscheme.h"

DEFINE_DOUBLY_LINKED_LIST_FUNCS(StringNode, char, true)
static StringNode *marked_string_list_head = &(StringNode){};

void mark_string_node(StringNode *node) {
  DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(StringNode)(node);
  DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(StringNode)(node, &marked_string_list_head);
}

void string_list_gc(void) {
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(StringNode)();
  if (marked_string_list_head->next != NULL) {
    DOUBLY_LINKED_LIST_HEAD_NAME(StringNode)->next = marked_string_list_head->next;
    marked_string_list_head->next->prev = DOUBLY_LINKED_LIST_HEAD_NAME(StringNode);

    marked_string_list_head->next = NULL;
  }
}
