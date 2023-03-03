#include "fzscheme.h"

DEFINE_DOUBLY_LINKED_LIST_FUNCS(ObjectVectorNode, Object *, true)
static ObjectVectorNode *marked_vector_list_head = &(ObjectVectorNode){};

void mark_vector_node(ObjectVectorNode *node) {
  DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(ObjectVectorNode)(node);
  DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(ObjectVectorNode)(node, &marked_vector_list_head);
}

void vector_list_gc(void) {
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(ObjectVectorNode)();
  if (marked_vector_list_head->next != NULL) {
    DOUBLY_LINKED_LIST_HEAD_NAME(ObjectVectorNode)->next = marked_vector_list_head->next;
    marked_vector_list_head->next->prev = DOUBLY_LINKED_LIST_HEAD_NAME(ObjectVectorNode);

    marked_vector_list_head->next = NULL;
  }
}