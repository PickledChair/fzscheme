#include "fzscheme.h"
#include <stdio.h>
#include <string.h>

bool debug_flag = false;

int main(int argc, char *argv[]) {
  if (argc == 2) {
    debug_flag = strcmp(argv[1], "--debug") == 0;
  }
  fzscm_memspace_init(0);
  repl();
  fzscm_memspace_fin();

  return 0;
}
