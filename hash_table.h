#pragma once

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "util.h"
#include "array.h"

typedef struct {
    array **keys;
    array **buckets;
    size_t num_buckets;
} hash_table;

#define GET_HASH_TABLE(t, k, ty) ((ty) get_raw_hash_table((t), (k)))

uint32_t hash(char *str) {
	uint32_t hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}

hash_table *init_hash_table(hash_table *table, size_t num_buckets, void (*cleanup)(void *)) {
	table->num_buckets = num_buckets;
	table->buckets = malloc(sizeof(*table->buckets) * num_buckets);
	assert(table->buckets);
	for (size_t i = 0; i < num_buckets; i++) {
		array *bucket = malloc(sizeof(*bucket));
		assert(bucket);
		init_array_f(bucket, 4, sizeof(array *), cleanup);

		table->buckets[i] = bucket;
	}
	table->keys = malloc(sizeof(*table->keys) * num_buckets);
	assert(table->keys);
	for (size_t i = 0; i < num_buckets; i++) {
		array *keys = malloc(sizeof(*keys));
		assert(keys);
		init_array_f(keys, 4, sizeof(array *), NULL);

		table->keys[i] = keys;
	}
	return table;
}

bool key_exists_hash_table(hash_table *h, char *key, size_t *where_in_bucket) {
	array *keys = h->keys[hash(key) % h->num_buckets];
	for (size_t i = 0; i < keys->size; i++) {
		if (str_equals(GET_ARRAY(keys, i, char *), key)) {
			if (where_in_bucket) {
				*where_in_bucket = i;
			}
			return true;
		}
	}
	return false;
}

void add_hash_table(hash_table *h, char *key, void *data) {
	size_t bucket_idx = hash(key) % h->num_buckets;
	size_t replace;
	bool should_replace = key_exists_hash_table(h, key, &replace);
	if (should_replace) {
		// @TODO: slower than replacing it
		remove_array(h->buckets[bucket_idx], replace);
		remove_array(h->keys[bucket_idx], replace);

		add_array(h->keys[bucket_idx], key);
		add_array(h->buckets[bucket_idx], data);
	} else {
		add_array(h->buckets[bucket_idx], data);
		add_array(h->keys[bucket_idx], key);
	}
}

void *get_raw_hash_table(hash_table *h, char *key) {
	assert(key_exists_hash_table(h, key, NULL));

	size_t idx = hash(key) % h->num_buckets;
	array *bucket = h->buckets[idx];
	array *keys = h->keys[idx];
	if (bucket->size > 1) {
		for (size_t i = 0; i < keys->size; i++) {
			if (str_equals(GET_ARRAY(keys, i, char *), key)) {
				return GET_ARRAY(bucket, i, void *);
			}
		}
	}
	return GET_ARRAY(bucket, 0, void *);
}

void remove_hash_table(hash_table *h, char *key) {
	assert(key_exists_hash_table(h, key, NULL));

	size_t idx = hash(key) % h->num_buckets;
	array *keys = h->keys[idx];
	array *bucket = h->buckets[idx];
	for (size_t i = 0; i < keys->size; i++) {
		if (str_equals(GET_ARRAY(keys, i, char *), key)) {
			remove_array(keys, i);
			remove_array(bucket, i);
		}
	}
}

void free_hash_table(hash_table *h) {
	for (size_t i = 0; i < h->num_buckets; i++) {
	        free_array(h->keys[i]);
		free_array(h->buckets[i]);
		free(h->buckets[i]);
		free(h->keys[i]);
	}
	free(h->keys);
	free(h->buckets);
}
