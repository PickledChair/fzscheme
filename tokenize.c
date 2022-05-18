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

static bool is_extended_identifier_char(char ch) {
  // NOTE: strchr 関数の検索対象文字列は末尾に必ずヌル文字を含んでいるので除外する必要がある
  if (ch != '\0' && strchr("!$%&*+-./:<=>?@^_~", ch) != NULL) {
    return true;
  } else {
    return false;
  }
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
      while (*(++input) != '"') {
        if (*input == '\n' || *input == '\0') {
          printf("tokenize error: string literal is not closed by \"\n");
          free_token(head.next);
          return NULL;
        }
      }
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

    // クォート
    if (*input == '\'') {
      cur = cur->next = new_token(TK_QUOTE, input, input+1);
      input++;
      continue;
    }

    // 識別子
    if (isalpha(*input) || is_extended_identifier_char(*input)) {
      char *start = input;
      while (isalnum(*input) || is_extended_identifier_char(*input)) {
        input++;
      }
      char *end = input - 1;
      cur = cur->next = new_token(TK_IDENT, start, end);
      int name_len = end - start + 1;
      char *name_buf = calloc(1, name_len + 1);
      strncpy(name_buf, start, name_len);
      name_buf[name_len] = '\0';
      cur->str = name_buf;
      continue;
    }

    printf("tokenize error: invalid character %c\n", *input);
    free_token(head.next);
    return NULL;
  }

  return head.next;
}

void print_token(Token *tok) {
  switch (tok->tag) {
  case TK_IDENT:
    printf("IDENT\t%s\n", tok->str);
    break;
  case TK_INT:
    printf("INT\t%ld\n", (long)(tok->val));
    break;
  case TK_LPAREN:
    printf("LPAREN\t(\n");
    break;
  case TK_QUOTE:
    printf("QUOTE\t'\n");
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
    if (cur->tag == TK_STR || cur->tag == TK_IDENT)
      free(cur->str);
    next = cur->next;
    free(cur);
  }
}
