#include "fzscheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool debug_flag = false;
static size_t semispace_size = 0;

void parse_args(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) {
      debug_flag = true;
    }
    if (strcmp(argv[i], "--heap_size") == 0) {
      i++;
      if (i >= argc) {
        exit(1);
      }
      semispace_size = atoi(argv[i]);
    }
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  fzscm_memspace_init(semispace_size);
  repl();
  fzscm_memspace_fin();

  return 0;
}
