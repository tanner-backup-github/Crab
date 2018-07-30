#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "array.h"
#include "tokenizer.h"

typedef struct parse_node {
        token token;
        array *children;
	struct parse_node *parent;
} parse_node;

void free_parse_nodes(parse_node *root) {
	if (root->children) {
		free_array(root->children);
		free(root->children);
		root->children = NULL;
	}
	free(root->token.buf);
        root->token.buf = NULL;
	free(root);
	root = NULL;
}

parse_node *parse(array *tokens) {
	parse_node *root = malloc(sizeof(*root));
	assert(root);
	root->token = (token) { strdup("#ROOT#"), PUNCTUATION, 0 };
	root->children = malloc(sizeof(*root->children));
	assert(root->children);
        init_array_f(root->children, 4, sizeof(parse_node *), (void *) free_parse_nodes);
	root->parent = NULL;

	parse_node *parent = root;
	for (size_t i = 0; i < tokens->size; ++i) {
		token t = *GET_ARRAY(tokens, i, token *);

		if (strcmp(t.buf, "(") == 0) {
			++i;
			t = *GET_ARRAY(tokens, i, token *);

			token nt = t;
			if (strcmp(t.buf, ")") == 0) {
				nt.buf = strdup("()");
				nt.type = PUNCTUATION;

				parse_node *new_child = malloc(sizeof(*new_child));
				assert(new_child);
				new_child->token = nt;
				new_child->children = NULL;
				new_child->parent = parent;

				add_array(parent->children, new_child);
			} else {
				nt.buf = strdup(t.buf);
				assert(nt.buf);
				parse_node *new_parent = malloc(sizeof(*new_parent));
				assert(new_parent);
				new_parent->token = nt;
				new_parent->children = malloc(sizeof(*new_parent->children));
				assert(new_parent->children);
				init_array_f(new_parent->children, 4, sizeof(parse_node *), (void *) free_parse_nodes);

				new_parent->parent = parent;
				add_array(parent->children, new_parent);

				parent = new_parent;
			}
		} else if (strcmp(t.buf, ")") == 0) {
			parent = parent->parent;
		} else {
			token nt = t;
			nt.buf = strdup(t.buf);
			assert(nt.buf);

			parse_node *new_child = malloc(sizeof(*new_child));
			assert(new_child);
			new_child->token = nt;
			new_child->children = NULL;
			new_child->parent = parent;

			add_array(parent->children, new_child);
		}
	}

	return root;
}
