#include "fzscheme.h"

bool debug_flag = false;
static size_t semispace_size = 0;

static void fzscm_init(void) {
  fzscm_memspace_init(semispace_size);
}

void fzscm_deinit(void) {
  if (current_working_vm) {
    free_vm(current_working_vm);
  }
  fzscm_memspace_fin();
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(StringNode)();
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(ObjectVectorNode)();
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(EnvNode)();
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(CodeNode)();
  clear_symbol_table();
}

static void parse_args(int argc, char *argv[]) {
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

  fzscm_init();

  repl();

  fzscm_deinit();

  return 0;
}
