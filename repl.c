#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUF_SIZE 256

int repl(void) {
  char buffer[INPUT_BUF_SIZE];
  size_t length;

  for (;;) {
    printf(">>> ");

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

    Token *tok_tmp = tok;
    Object *obj = parse_obj(&tok);
    if (obj != NULL) {
      printf("==> ");
      print_obj(obj);
      putchar('\n');
      free_obj(obj);
    }
    tok = tok_tmp;

    free_token(tok);
  }

  return 0;
}
