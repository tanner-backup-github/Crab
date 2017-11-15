#ifndef HASH_TABLE_H__
#define HASH_TABLE_H__

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "array.h"

// @TODO: rewrite entirely
// @NOTE: using seperate chaining
// @TODO: only strings?
// @TODO: occupied
typedef struct {
	array *keys;
	array *values;
	size_t buckets;
} hash_table;

// @TODO: resizing

uint32_t hash(const char *str) {
	uint32_t hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
        return hash;
}

#endif
