#include "fzscheme.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_SIZE (1024 + 9)

typedef struct SymTableEntry SymTableEntry;
struct SymTableEntry {
  uint32_t hash;
  Object *symbol;
  Object *value;
  SymTableEntry *next;
};

static SymTableEntry *new_sym_table_entry(uint32_t hash, Object *symbol) {
  SymTableEntry *ent = calloc(1, sizeof(SymTableEntry));
  ent->hash = hash;
  ent->symbol = symbol;
  return ent;
}

static SymTableEntry *symbol_table[SYMBOL_TABLE_SIZE] = {};

static Object *get_symbol_obj(char *name) {
  uint32_t hash = str_hash(name);
  SymTableEntry *ent = symbol_table[hash % SYMBOL_TABLE_SIZE];

  while (ent) {
    if (hash == ent->hash) {
      if (strcmp(name, ent->symbol->fields_of.symbol.name) == 0) {
        return ent->symbol;
      }
    }
    ent = ent->next;
  }

  return NULL;
}

Object *intern_name(char *name) {
  Object *symbol = get_symbol_obj(name);
  if (symbol != NULL) {
    return symbol;
  } else {
    uint32_t hash = str_hash(name);
    symbol = new_symbol_obj(strdup(name));
    SymTableEntry *ent = new_sym_table_entry(hash, symbol);

    size_t idx = hash % SYMBOL_TABLE_SIZE;
    if (symbol_table[idx] == NULL) {
      symbol_table[idx] = ent;
    } else {
      SymTableEntry *cur_ent = symbol_table[idx];

      for (;;) {
        if (cur_ent->next == NULL) {
          cur_ent->next = ent;
          break;
        }
        cur_ent = cur_ent->next;
      }
    }

    return symbol;
  }
}

Object *insert_to_global_env(Object *symbol, Object *value) {
  uint32_t hash = str_hash(symbol->fields_of.symbol.name);
  size_t idx = hash % SYMBOL_TABLE_SIZE;

  SymTableEntry *cur_ent = symbol_table[idx];
  for (;;) {
    if (cur_ent->hash == hash) {
      if (strcmp(cur_ent->symbol->fields_of.symbol.name,
                 symbol->fields_of.symbol.name) == 0) {
        Object *res_value = cur_ent->value;
        cur_ent->value = value;
        return res_value;
      }
    }
    if (cur_ent->next == NULL) {
      break;
    }
    cur_ent = cur_ent->next;
  }

  return NULL;
}

Object *get_from_global_env(Object *symbol) {
  uint32_t hash = str_hash(symbol->fields_of.symbol.name);
  size_t idx = hash % SYMBOL_TABLE_SIZE;

  SymTableEntry *cur_ent = symbol_table[idx];
  for (;;) {
    if (cur_ent->hash == hash) {
      if (strcmp(cur_ent->symbol->fields_of.symbol.name,
                 symbol->fields_of.symbol.name) == 0) {
        return cur_ent->value;
      }
    }
    if (cur_ent->next == NULL) {
      break;
    }
    cur_ent = cur_ent->next;
  }

  return NULL;
}

void global_env_collect_roots(void) {
  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
    if (symbol_table[i] != NULL) {
      SymTableEntry *ent = symbol_table[i];
      while (ent) {
        if (ent->value != NULL) {
          if (debug_flag) {
            printf("root in global environment: %s\n",
                  ent->symbol->fields_of.symbol.name);
          }
          add_root(&ent->value);
        }
        ent = ent->next;
      }
    }
  }
}

void clear_symbol_table(void) {
  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
    if (symbol_table[i] != NULL) {
      SymTableEntry *ent = symbol_table[i], *next_ent;
      while (ent) {
        next_ent = ent->next;
        if (debug_flag) {
          printf("free symbol: %s\n", ent->symbol->fields_of.symbol.name);
        }
        free_symbol_obj(ent->symbol);
        free(ent);
        ent = next_ent;
      }
    }
  }
}
