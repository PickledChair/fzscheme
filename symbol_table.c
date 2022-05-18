#include "fzscheme.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_SIZE (1024 + 9)

typedef struct SymTableEntry SymTableEntry;
struct SymTableEntry {
  uint32_t hash;
  Object *value;
  SymTableEntry *next;
};

static SymTableEntry *new_sym_table_entry(uint32_t hash, Object *value) {
  SymTableEntry *ent = calloc(1, sizeof(SymTableEntry));
  ent->hash = hash;
  ent->value = value;
  return ent;
}

static SymTableEntry *symbol_table[SYMBOL_TABLE_SIZE] = {};

static Object *get_symbol_obj(char *name) {
  uint32_t hash = str_hash(name);
  SymTableEntry *ent = symbol_table[hash % SYMBOL_TABLE_SIZE];

  while (ent) {
    if (hash == ent->hash) {
      if (strcmp(name, ent->value->fields_of.symbol.name) == 0) {
        return ent->value;
      }
    }
    ent = ent->next;
  }

  return NULL;
}

Object *intern_name(char *name) {
  Object *symbol = get_symbol_obj(name);
  if (symbol != NULL) {
    if (debug_flag) {
      printf("symbol `%s` already exists, so return it\n", name);
    }
    return symbol;
  } else {
    uint32_t hash = str_hash(name);
    symbol = new_symbol_obj(name);
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
      }
    }

    return symbol;
  }
}

void clear_symbol_table(void) {
  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
    if (symbol_table[i] != NULL) {
      SymTableEntry *ent = symbol_table[i], *next_ent;
      while (ent) {
        next_ent = ent->next;
        if (debug_flag) {
          printf("free symbol: %s\n", ent->value->fields_of.symbol.name);
        }
        free_symbol_obj(ent->value);
        free(ent);
        ent = next_ent;
      }
    }
  }
}
