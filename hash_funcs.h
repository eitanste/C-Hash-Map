#ifndef HASH_H_
#define HASH_H_

#include <stdlib.h>

/**
 * Integers simple hash func.
 */
size_t hash_int(const void *elem){
  size_t hash = *((int *) elem);
  return hash;
}

/**
 * Chars simple hash func.
 */
size_t hash_char(const void *elem){
  size_t hash = *((char *) elem);
  return hash;
}

/**
 * Doubles simple hash func.
 */
size_t hash_double(const void *elem){
  size_t hash = *((double *) elem);
  return hash;
}

#endif // HASH_H_
