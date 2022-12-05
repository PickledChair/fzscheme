#include <ctype.h>

#include "fzscheme.h"

static Token *new_token(TokenTag tag, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->tag = tag;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

static bool is_special_initial_char(char ch) {
  // NOTE: strchr 関数の検索対象文字列は末尾に必ずヌル文字を含んでいるので除外する必要がある
  return (ch != '\0' && strchr("!$%&*/:<=>?^_~", ch) != NULL);
}

static bool is_special_subsequent_char(char ch) {
  return (ch != '\0' && strchr("+-.@", ch) != NULL);
}

static int read_escaped_char(char **new_pos, char *p) {
  *new_pos = p + 1;

  switch (*p) {
  case '"': return '"';
  case '\\': return '\\';
  case 'n': return '\n';
  case 'r': return '\r';
  case 'f': return '\f';
  case 't': return '\t';
  case 'a': return '\a';
  case 'b': return '\b';
  case '0': return '\0';
  default:
    return *p;
  }
}

static char *string_literal_end(char *p) {
  char *start = p;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0') {
      printf("tokenize error: string literal is not closed by \"\n");
      return NULL;
    }
    if (*p == '\\') {
      p++;
    }
  }
  return p;
}

static Token *read_string_literal(char *start) {
  char *end = string_literal_end(start + 1);
  if (end == NULL) {
    return NULL;
  }
  char *buf = calloc(1, end - start);
  int len = 0;

  for (char *p = start + 1; p < end;) {
    if (*p == '\\') {
      buf[len++] = read_escaped_char(&p, p + 1);
    } else {
      buf[len++] = *p++;
    }
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->str = buf;
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
      Token *str_tok = read_string_literal(input);
      if (str_tok == NULL) {
        free_token(head.next);
        return NULL;
      }
      cur = cur->next = str_tok;
      input += cur->len;
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

    // ドット
    if (*input == '.') {
      cur = cur->next = new_token(TK_DOT, input, input+1);
      input++;
      continue;
    }

    if (*input == '#') {
      char *start = input;
      input++;
      if (*input == 't') {
        cur = cur->next = new_token(TK_TRUE, start, input+1);
        input++;
        continue;
      } else if (*input == 'f') {
        cur = cur->next = new_token(TK_FALSE, start, input+1);
        input++;
        continue;
      }
      printf("tokenize error: unimplemented #... literal\n");
      free_token(head.next);
      return NULL;
    }

    // 識別子
    if (isalpha(*input) || is_special_initial_char(*input)
        || *input == '+' || *input == '-') {
      char *start = input;
      while (isalnum(*input) || is_special_subsequent_char(*input)) {
        input++;
      }
      char *end = input;
      cur = cur->next = new_token(TK_IDENT, start, end);
      int name_len = end - start;
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
  case TK_DOT:
    printf("DOT\t.\n");
    break;
  case TK_FALSE:
    printf("FALSE\t#f\n");
    break;
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
  case TK_TRUE:
    printf("TRUE\t#t\n");
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
