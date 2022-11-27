#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>

DEFINE_DOUBLY_LINKED_LIST_FUNCS(StringNode, char, true)
// static StringNode *string_list_head = &(StringNode){};
static StringNode *marked_string_list_head = &(StringNode){};

// static void push_string_node(StringNode *node, StringNode **head) {
//   if ((*head)->next != NULL) {
//     (*head)->next->prev = node;
//   }
//   node->next = (*head)->next;

//   (*head)->next = node;
//   node->prev = *head;
// }

// StringNode *new_string_node(char *value) {
//   StringNode *node = calloc(1, sizeof(StringNode));
//   node->value = strdup(value);
//   push_string_node(node, &string_list_head);
//   return node;
// }

// static void remove_string_node(StringNode *node) {
//   node->prev->next = node->next;
//   if (node->next != NULL) {
//     node->next->prev = node->prev;
//   }
// }

void mark_string_node(StringNode *node) {
  // remove_string_node(node);
  DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(StringNode)(node);
  // push_string_node(node, &marked_string_list_head);
  DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(StringNode)(node, &marked_string_list_head);
}

void string_list_gc(void) {
  // clear_string_list();
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(StringNode)();
  if (marked_string_list_head->next != NULL) {
    DOUBLY_LINKED_LIST_HEAD_NAME(StringNode)->next = marked_string_list_head->next;
    marked_string_list_head->next->prev = DOUBLY_LINKED_LIST_HEAD_NAME(StringNode);

    marked_string_list_head->next = NULL;
  }
}

// void clear_string_list(void) {
//   StringNode *cur = string_list_head->next, *next;
//   string_list_head->next = NULL;

//   while (cur != NULL) {
//     next = cur->next;
//     if (debug_flag) {
//       printf("free string: %s\n", cur->value);
//     }
//     free(cur->value);
//     free(cur);
//     cur = next;
//   }
// }
