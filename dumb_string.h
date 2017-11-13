#ifndef __DUMB_STRING__
#define __DUMB_STRING__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
	char *data;
	size_t len;
	size_t cap;
} dumb_string;

void init_dumb_string(dumb_string *ds, char *data, size_t cap) {
	size_t len = strlen(data);
	#ifdef DEBUG
	if (cap) {
		assert(len <= cap);
	}
	#endif
	ds->cap = cap ? cap : len * 2;
	ds->data = malloc((ds->cap + 1) * sizeof(char));
        assert(ds->data);
	memcpy(ds->data, data, len);
	ds->data[len] = '\0';
	ds->len = len;
}

#define INIT_DUMB_STRING_DEF(ds, data) init_dumb_string((ds), (data), 0);

void append_dumb_string(dumb_string *ds, char *a) {
	size_t a_len = strlen(a);
	if (a_len + ds->len >= ds->cap) {
		ds->cap += a_len * 2;
		void *m = realloc(ds->data, (ds->cap + 1) * sizeof(char));
	        assert(m);
		ds->data = m;
	}
	memcpy(ds->data + ds->len * sizeof(char), a, a_len);
	ds->len += a_len;
	ds->data[ds->len] = '\0';
}

void append_dumb_string_char(dumb_string *ds, char c) {
	if (1 + ds->len >= ds->cap) {
		ds->cap += 8;
		void *m = realloc(ds->data, (ds->cap + 1) * sizeof(char));
	        assert(m);
		ds->data = m;
	}
	ds->data[ds->len] = c;
	++ds->len;
	ds->data[ds->len] = '\0';
}

extern inline void free_dumb_string(dumb_string *ds) {
	free(ds->data);
	ds->data = NULL;
	ds->len = 0;
	ds->cap = 0;
}

extern inline void clear_dumb_string(dumb_string *ds) {
	memset(ds->data, 0, ds->len);
	ds->len = 0;
}

#endif
