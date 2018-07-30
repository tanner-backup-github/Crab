#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "array.h"
#include "dumb_string.h"
#include "util.h"

typedef enum {
	IDENTIFIER,
	LITERAL,
	PUNCTUATION,
} token_type;

typedef struct {
	char *buf;
        token_type type;
	size_t line;
} token;

// @TODO: escape char validation
bool is_str(const char *s, size_t *len) {
	size_t llen = strlen(s);
	if (s[0] != '"') { return false; }
	if (s[llen - 1] != '"') { return false; }
	if (len) {
		*len = llen;
	}
	return true;
}

bool is_num(const char *s) {
	size_t i = 0;
	if (*s == '-') { ++i; }

        bool dot = false;
	while (s[i] != '\0') {
		char c = s[i];
		if (c == '.' && !dot) {
			dot = true;
		} else if (!isdigit(c) || (c == '.' && dot)) {
			return false;
		}
		++i;
	}
	return true;
}

bool is_boolean(const char *s) {
	return str_equals(s, "true") || str_equals(s, "false");
}

static token *make_token(char *buf, token_type type, size_t line) {
	token *t = malloc(sizeof(*t));
	assert(t);
	t->buf = buf;
	t->type = type;
	t->line = line;
	return t;
}

void free_token(token *t) {
	free(t->buf);
	t->buf = NULL;
	free(t);
	t = NULL;
}


#define MODIFY_TOKEN_TYPE(cb, t) if (is_str((cb), NULL) || is_num((cb)) || is_boolean((cb))) { \
		(t) = LITERAL;						\
	}

array *tokenize(const char *src, size_t src_len) {
        array *tokens = malloc(sizeof(*tokens));
        init_array_f(tokens, 32, sizeof(token), (void *) free_token);

	dumb_string buf;
	init_dumb_string(&buf, "", 32);
	bool in_str = false;
	bool in_comment = false;
	int parens = 0;
	size_t line = 1;
	for (size_t i = 0; i < src_len; ++i) {
		char c = src[i];
		bool c_space = isspace(c);

		if (c == '"') { // @TODO: escape chars
			in_str = !in_str;
		} else if (c == '\n' && in_comment) {
			in_comment = false;
			continue;
		} else if (c == '#' && !in_str) {
			in_comment = true;
			continue;
		} else if (c == '\n') {
			++line;
		}

		if (c == '(' && !in_str) {
			parens++;
			token *t = make_token(strdup("("), PUNCTUATION, line);
			add_array(tokens, t);
		} else if (c == ')' && !in_str) {
			parens--;
			if (buf.len) {
				char *copybuf = buf.data;
				size_t copy_len = buf.len;
				token_type type = IDENTIFIER;
				MODIFY_TOKEN_TYPE(copybuf, type);

				token *t = make_token(strndup(copybuf, copy_len), type, line);

				add_array(tokens, t);
				clear_dumb_string(&buf);
			}

		        token *t = make_token(strdup(")"), PUNCTUATION, line);
			add_array(tokens, t);
		} else if (c_space && !in_str && buf.len) {
			token_type type = IDENTIFIER;
			char *copybuf = buf.data;
			size_t copy_len = buf.len;

			MODIFY_TOKEN_TYPE(copybuf, type);

			token *t = make_token(strndup(copybuf, copy_len), type, line);
			add_array(tokens, t);

		        clear_dumb_string(&buf);
		}

		if ((!c_space || in_str) && c != '(' && c != ')' && !in_comment) {
			append_dumb_string_char(&buf, c);
		}
	}
	free_dumb_string(&buf);

	assert(parens == 0);

	return tokens;
}
