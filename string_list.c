#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>

static StringNode *string_list_head = &(StringNode){};
static StringNode *string_list_top = NULL;

static StringNode *marked_string_list_head = &(StringNode){};
static StringNode *marked_string_list_top = NULL;

static void push_string_node(StringNode *node, StringNode **head, StringNode **top) {
  if (debug_flag) {
    if (top == &marked_string_list_top) {
      printf("marked ");
    }
    printf("string list top: %p\n", *top);
  }
  if (*top == NULL) {
    (*head)->next = *top = node;
    node->prev = *head;
  } else {
    (*top)->next = node;
    node->prev = *top;
    *top = node;
  }
  node->next = NULL;
}

StringNode *new_string_node(char *value) {
  StringNode *node = calloc(1, sizeof(StringNode));
  node->value = value;
  push_string_node(node, &string_list_head, &string_list_top);
  return node;
}

static void remove_string_node(StringNode *node) {
  node->prev->next = node->next;
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }
  if (string_list_top == node) {
    if (node->prev == string_list_head) {
      string_list_top = NULL;
    } else {
      string_list_top = node->prev;
    }
  }
}

void mark_string_node(StringNode *node) {
  remove_string_node(node);
  push_string_node(node, &marked_string_list_head, &marked_string_list_top);
}

void string_list_gc(void) {
  clear_string_list();
  if (marked_string_list_head->next != NULL) {
    string_list_head->next = string_list_top = marked_string_list_head->next;
    marked_string_list_head->next->prev = string_list_head;
    marked_string_list_head->next = NULL;
    marked_string_list_top = NULL;
  }
}

void clear_string_list(void) {
  StringNode *cur = string_list_head->next, *next;
  string_list_head->next = NULL;
  string_list_top = NULL;
  while (cur != NULL) {
    next = cur->next;
    if (debug_flag) {
      printf("free string: %s\n", cur->value);
    }
    free(cur->value);
    free(cur);
    cur = next;
  }
}
