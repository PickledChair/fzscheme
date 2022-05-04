#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_paren_level(int init_level, Token *start_tok, Token **end_tok_dest) {
  if (init_level < 0) return init_level;

  int level = init_level;
  Token *prev_tok = NULL;
  for (Token *cur = start_tok; cur != NULL; cur = cur->next) {
    switch (cur->tag) {
    case TK_LPAREN:
      level += 1;
      break;
    case TK_RPAREN:
      level -= 1;
      break;
    default:
      break;
    }
    *end_tok_dest = cur;
    if (level < 0) return level;
    if (level == 0 && cur->next != NULL) return -1;
  }

  return level;
}

#define INPUT_BUF_SIZE 256

int repl(void) {
  char buffer[INPUT_BUF_SIZE];
  size_t length;
  Token *top_tok = NULL, *cur_tok = NULL;
  int paren_level = 0;

  for (;;) {
    if (paren_level == 0) {
      printf(">>> ");
    } else {
      printf("... ");
    }

    if (fgets(buffer, INPUT_BUF_SIZE, stdin) == NULL) {
      printf("wrong input.\n");
      exit(1);
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
      buffer[--length] = '\0';
    }

    if (strcmp(buffer, "(exit)") == 0) {
      break;
    }

    if (strcmp(buffer, "") == 0) {
      continue;
    }

    Token *tok = tokenize(buffer);
    if (top_tok == NULL) {
      top_tok = tok;
    }

    {
      Token *end_tok;
      paren_level = get_paren_level(paren_level, tok, &end_tok);
      if (paren_level < 0) {
        printf("tokenize error: extra parenthesis or token\n");
        if (top_tok != NULL) {
          free_token(top_tok);
        }
        top_tok = NULL;
        cur_tok = NULL;
        paren_level = 0;
        continue;
      } else if (paren_level > 0) {
        if (cur_tok != NULL) {
          cur_tok->next = tok;
        }
        cur_tok = end_tok;
        continue;
      } else {
        if (cur_tok != NULL) {
          cur_tok->next = tok;
        }
      }
    }

    {
      Token *tok_tmp = top_tok;
      Object *obj = parse_obj(&top_tok);
      if (obj != NULL) {
        printf("==> ");
        print_obj(obj);
        putchar('\n');
        free_obj(obj);
      }
      top_tok = tok_tmp;
    }

    free_token(top_tok);
    top_tok = NULL;
    cur_tok = NULL;
  }

  return 0;
}
