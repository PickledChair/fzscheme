#include "fzscheme.h"
#include <stdio.h>

int main(void) {
  char *srcs[] = {
    "42",
    "\"Hello, world!\"",
    "(1 2 3)",
    "()",
    "((1 2) (3 4) (5 6))",
  };

  for (int i = 0; i < sizeof(srcs) / sizeof(*srcs); i++) {
    Token *tok = tokenize(srcs[i]);

    for (Token *cur = tok; cur != NULL; cur = cur->next)
      print_token(cur);

    Token *tok_tmp = tok;
    Object *obj = parse_obj(&tok);
    if (obj != NULL) {
      print_obj(obj);
      putchar('\n');
      free_obj(obj);
    }
    tok = tok_tmp;

    free_token(tok);
  }

  return 0;
}
