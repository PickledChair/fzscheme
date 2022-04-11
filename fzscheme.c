#include "fzscheme.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  char *source = "123 \"piyo\" \"hoge huga\" (3 2 1)";
  Token *tokens = tokenize(source);
  for (Token *cur = tokens; cur != NULL; cur = cur->next)
    print_token(cur);
  free_token(tokens);

  Object *int_obj = new_integer_obj(42);
  print_obj(int_obj);
  putchar('\n');
  free_obj(int_obj);

  Object *str_obj = new_string_obj(strdup("Hello, world!"));
  print_obj(str_obj);
  putchar('\n');
  free_obj(str_obj);

  Object *list_obj = new_cell_obj(new_integer_obj(1), new_cell_obj(new_integer_obj(2), new_cell_obj(new_integer_obj(3), NIL)));
  print_obj(list_obj);
  putchar('\n');
  free_obj(list_obj);

  print_obj(NIL);
  putchar('\n');

  return 0;
}
