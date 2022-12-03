#include "fzscheme.h"

#define FNV_OFFSET_BASIS_32 2166136261U
#define FNV_PRIME_32 16777619U

static uint32_t fnv_1_hash_32(uint8_t *bytes, size_t length) {
  uint32_t hash;

  hash = FNV_OFFSET_BASIS_32;
  for (size_t i = 0; i < length; i++) {
    hash = (FNV_PRIME_32 * hash) ^ (bytes[i]);
  }

  return hash;
}

uint32_t str_hash(char *str) {
  return fnv_1_hash_32((uint8_t *)str, strlen(str));
}
