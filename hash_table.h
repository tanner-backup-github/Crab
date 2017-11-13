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
	array keys;
	array values;
	size_t buckets;
} hash_table;

// @TODO: resizing

uint32_t hash(const char *str) {
	uint32_t hash = 5381;
	int c;
	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
        return hash;
}

void init_hash_table(hash_table *table, size_t buckets, size_t value_size, void (* free_value)(void *)) {
	table->buckets = buckets;
	INIT_ARRAY(&table->keys, buckets, sizeof(array));
	INIT_ARRAY(&table->values, buckets, sizeof(array));
	
	// @TODO: defer this until later?
	for (size_t i = 0; i < buckets; ++i) {
		GET_ARRAY(&table->values, i, array *) = malloc(sizeof(array));
		GET_ARRAY(&table->keys, i, array *) = malloc(sizeof(array));
		init_array_f(GET_ARRAY(&table->values, i, array *), buckets, sizeof(char *), (void *) free_value); //
		init_array_f(GET_ARRAY(&table->keys, i, array *), buckets, value_size, free); //
	}
}

void free_hash_table(hash_table *table) {
	for (size_t i = 0; i < table->buckets; ++i) {
		free_array(GET_ARRAY(&table->values, i, array *));
		free(GET_ARRAY(&table->values, i, array *));
		
		free_array(GET_ARRAY(&table->keys, i, array *));
		free(GET_ARRAY(&table->keys, i, array *));
	}
	free_array(&table->keys);
	free_array(&table->values);
}

void add_hash_table(hash_table *table, char *key, void *value) {
	size_t i = hash(key) % table->buckets;
	array *values = GET_ARRAY(&table->values, i, array *);
	array *keys = GET_ARRAY(&table->keys, i, array *);
        add_array(keys, strdup(key));
	add_array(values, value);
}

void *get_hash_table(hash_table *table, char *key) {
	size_t i = hash(key) % table->buckets;
	array *keys = GET_ARRAY(&table->keys, i, array *);
	bool f = false;
	size_t j = 0;
	while (j < keys->size) {
		if (strcmp(key, GET_ARRAY(keys, j, char *)) == 0) {
			f = true;
			break;
		}
		++j;
	}
	assert(f);
	
	array *values = GET_ARRAY(&table->values, i, array *);
	return GET_ARRAY(values, j, void *);
}

#endif
