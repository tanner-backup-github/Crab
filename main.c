#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"
#include "dumb_string.h"
#include "file.h"
#include "hash_table.h"

// @TODO: rewrite hash table ?
// @TODO: comments are messed up

bool str_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}

typedef struct {
	union {
		char *string;
		double number;
	};
	enum {
	        VALUE_STRING,
		VALUE_NUMBER,
	} tag;
} crab_value;

#define CRAB_OP(name, op) void crab_ ##name(array *stack) {		\
		crab_value *operand1 = pop_array(stack);		\
		crab_value *operand2 = pop_array(stack);		\
		assert(operand1->tag == VALUE_NUMBER);			\
		assert(operand2->tag == VALUE_NUMBER);			\
		crab_value *r = malloc(sizeof(*r));			\
		r->number = operand2->number op operand1->number;	\
		r->tag = VALUE_NUMBER;					\
		add_array(stack, r);					\
	}

CRAB_OP(add, +);
CRAB_OP(sub, -);
CRAB_OP(mul, *);
CRAB_OP(div, /);

void crab_define(char *name, array *stack, hash_table *env) {
	crab_value *v = pop_array(stack);
	add_hash_table(env, name, v);
}

void crab_print(array *stack, hash_table *env) {
	printf("%f\n", ((crab_value *) pop_array(stack))->number);
}

void eval(parse_node *root, array *stack, hash_table *env) {
	if (!root->children) {
		crab_value *v = malloc(sizeof(*v));
		switch (root->token.type) {
		case NUMBER: {
			v->number = strtof(root->token.buf, NULL);
			v->tag = VALUE_NUMBER;
			break;
		}
		case IDENTIFIER: {
			v = (crab_value *) get_hash_table(env, root->token.buf);
			break;
		}
		default: {
			assert(false);
		        break;
		}
		}
		add_array(stack, v);
	} else {
		size_t i = 0;
		if (str_equals(root->token.buf, "define")) {
			i = 1;
		}
		for (; i < root->children->size; ++i) {
			eval(GET_ARRAY(root->children, i, parse_node *), stack, env);
		}

		if (str_equals(root->token.buf, "+")) {
			crab_add(stack);
		} else if (str_equals(root->token.buf, "-")) {
			crab_sub(stack);
		} else if (str_equals(root->token.buf, "*")) {
			crab_mul(stack);
		} else if (str_equals(root->token.buf, "/")) {
			crab_div(stack);
		} else if (str_equals(root->token.buf, "define")) {
		        crab_define(GET_ARRAY(root->children, 0, parse_node *)->token.buf, stack, env);
		} else if (str_equals(root->token.buf, "print")) {
			crab_print(stack, env);
		}
	}
}

int main(void) {

	char *src = read_entire_file("test");
	
	array *tokens = tokenize(src, strlen(src));
	free(src);
	parse_node *root = parse(tokens);
	free_array(tokens);
	free(tokens);
	
	hash_table env;
	init_hash_table(&env, 32, sizeof(crab_value *), NULL);

	crab_value PI = { 0 };
        PI.number = M_PI;
        PI.tag = VALUE_NUMBER;
	add_hash_table(&env, "pi", &PI);
	
	array stack;
	INIT_ARRAY(&stack, 32, sizeof(crab_value *));
	eval(root, &stack, &env);
	free_array(&stack);
	free_hash_table(&env);

	free_parse_nodes(root);

	printf("\n");
	
	return 0;
}
