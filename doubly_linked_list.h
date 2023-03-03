#define DEFINE_NODE_TYPE(NODE_TYPE_NAME, VALUE_TYPE_NAME) \
  typedef struct NODE_TYPE_NAME NODE_TYPE_NAME;           \
  struct NODE_TYPE_NAME {                                 \
    VALUE_TYPE_NAME value;                                \
    NODE_TYPE_NAME *prev;                                 \
    NODE_TYPE_NAME *next;                                 \
  };

#define DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(NODE_TYPE_NAME) NODE_TYPE_NAME##_push
#define DEFINE_DOUBLY_LINKED_LIST_PUSH_FUNC(NODE_TYPE_NAME)      \
  static void DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(NODE_TYPE_NAME)( \
      NODE_TYPE_NAME * node, NODE_TYPE_NAME * *head) {           \
    if ((*head)->next != NULL) {                                 \
      (*head)->next->prev = node;                                \
    }                                                            \
    node->next = (*head)->next;                                  \
                                                                 \
    (*head)->next = node;                                        \
    node->prev = *head;                                          \
  }

#define DOUBLY_LINKED_LIST_HEAD_NAME(NODE_TYPE_NAME) NODE_TYPE_NAME##_head
#define DEFINE_DOUBLY_LINKED_LIST_HEAD(NODE_TYPE_NAME)                  \
  static NODE_TYPE_NAME *DOUBLY_LINKED_LIST_HEAD_NAME(NODE_TYPE_NAME) = \
      &(NODE_TYPE_NAME){};

#define NODE_TYPE_NEW_FUNC_NAME(NODE_TYPE_NAME) NODE_TYPE_NAME##_new
#define DEFINE_NODE_TYPE_NEW_FUNC(NODE_TYPE_NAME, VALUE_TYPE_NAME)          \
  NODE_TYPE_NAME *NODE_TYPE_NEW_FUNC_NAME(NODE_TYPE_NAME)(VALUE_TYPE_NAME * \
                                                          value) {          \
    NODE_TYPE_NAME *node = calloc(1, sizeof(NODE_TYPE_NAME));               \
    node->value = value;                                                    \
    DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(NODE_TYPE_NAME)                       \
    (node, &DOUBLY_LINKED_LIST_HEAD_NAME(NODE_TYPE_NAME));                  \
    return node;                                                            \
  }

#define DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(NODE_TYPE_NAME) \
  NODE_TYPE_NAME##_remove
#define DEFINE_DOUBLY_LINKED_LIST_REMOVE_FUNC(NODE_TYPE_NAME)               \
  void DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(NODE_TYPE_NAME)(NODE_TYPE_NAME * \
                                                           node) {          \
    node->prev->next = node->next;                                          \
    if (node->next != NULL) {                                               \
      node->next->prev = node->prev;                                        \
    }                                                                       \
  }

#define DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(NODE_TYPE_NAME) \
  NODE_TYPE_NAME##_clear
#define DEFINE_DOUBLY_LINKED_LIST_CLEAR_FUNC(NODE_TYPE_NAME,                        \
                                             SHOULD_FREE_VALUE)                     \
  void DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(NODE_TYPE_NAME)(void) {                   \
    NODE_TYPE_NAME *cur = DOUBLY_LINKED_LIST_HEAD_NAME(NODE_TYPE_NAME)->next,       \
                   *next;                                                           \
    DOUBLY_LINKED_LIST_HEAD_NAME(NODE_TYPE_NAME)->next = NULL;                      \
                                                                                    \
    while (cur != NULL) {                                                           \
      next = cur->next;                                                             \
      if (SHOULD_FREE_VALUE && strcmp(#NODE_TYPE_NAME, "StringNode") == 0) {        \
        if (debug_flag) {                                                           \
          printf("free string: %s\n", (char *)cur->value);                          \
        }                                                                           \
        free(cur->value);                                                           \
      }                                                                             \
      if (SHOULD_FREE_VALUE && strcmp(#NODE_TYPE_NAME, "ObjectVectorNode") == 0) {  \
        free(cur->value);                                                           \
      }                                                                             \
      free(cur);                                                                    \
      cur = next;                                                                   \
    }                                                                               \
  }

#define DEFINE_DOUBLY_LINKED_LIST_FUNCS(NODE_TYPE_NAME, VALUE_TYPE_NAME, \
                                        SHOULD_FREE_VALUE)               \
  DEFINE_DOUBLY_LINKED_LIST_HEAD(NODE_TYPE_NAME)                         \
  DEFINE_DOUBLY_LINKED_LIST_PUSH_FUNC(NODE_TYPE_NAME)                    \
  DEFINE_NODE_TYPE_NEW_FUNC(NODE_TYPE_NAME, VALUE_TYPE_NAME)             \
  DEFINE_DOUBLY_LINKED_LIST_REMOVE_FUNC(NODE_TYPE_NAME)                  \
  DEFINE_DOUBLY_LINKED_LIST_CLEAR_FUNC(NODE_TYPE_NAME, SHOULD_FREE_VALUE)
