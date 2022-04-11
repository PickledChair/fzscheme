#include "fzscheme.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Token *new_token(TokenTag tag, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->tag = tag;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

Token *tokenize(char *input) {
  Token head = {};
  Token *cur = &head;

  while (*input) {
    // 空白文字をスキップ
    if (isspace(*input)) {
      input++;
      continue;
    }

    // 整数リテラル
    if (isdigit(*input)) {
      cur = cur->next = new_token(TK_INT, input, input);
      char *start = input;
      cur->val = strtoul(input, &input, 10);
      cur->len = input - start;
      continue;
    }

    // 文字列リテラル
    if (*input == '"') {
      char *start = input;
      while (*(++input) != '"');
      cur = cur->next = new_token(TK_STR, start, input);
      int str_len = input - (start+1);
      char *str_buf = calloc(1, str_len+1);
      strncpy(str_buf, start+1, str_len);
      str_buf[str_len] = '\0';
      cur->str = str_buf;
      input++;
      continue;
    }

    // 左括弧
    if (*input == '(') {
      cur = cur->next = new_token(TK_LPAREN, input, input+1);
      input++;
      continue;
    }

    // 右括弧
    if (*input == ')') {
      cur = cur->next = new_token(TK_RPAREN, input, input+1);
      input++;
      continue;
    }
  }

  return head.next;
}

void print_token(Token *tok) {
  switch (tok->tag) {
  case TK_INT:
    printf("INT\t%ld\n", (long)(tok->val));
    break;
  case TK_LPAREN:
    printf("LPAREN\t(\n");
    break;
  case TK_RPAREN:
    printf("RPAREN\t)\n");
    break;
  case TK_STR:
    printf("STRING\t%s\n", tok->str);
    break;
  }
}

void free_token(Token *tok) {
  Token *next;
  for (Token *cur = tok; cur != NULL; cur = next) {
    if (cur->tag == TK_STR)
      free(cur->str);
    next = cur->next;
    free(cur);
  }
}
