#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"
#include "dumb_string.h"
#include "file.h"

// @TODO: rewrite hash table
// @TODO: comments are messed up
// @TODO: assert array index within range

bool str_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}

typedef struct {
	char *id;
	// @TODO: ^ until hash table
	union {
		char *string;
		double number;
		struct {
			union {
				parse_node *root;
				void (* native)(array *);
			};
			array *args;
			bool is_native;
		} function;
	};
	enum {
	        VALUE_STRING,
		VALUE_NUMBER,
		VALUE_FUNCTION
	} tag;
} crab_value;

#define CRAB_OP(name, op) void crab_ ##name(array *stack) {		\
		crab_value *o1 = GET_ARRAY(stack, stack->size - 1, crab_value *); \
		crab_value *o2 = GET_ARRAY(stack, stack->size - 2, crab_value *); \
		assert(o1->tag == VALUE_NUMBER);			\
		assert(o2->tag == VALUE_NUMBER);			\
		crab_value *r = malloc(sizeof(*r));			\
		r->number = o2->number op o1->number;			\
		r->tag = VALUE_NUMBER;					\
	        remove_array(stack, stack->size - 1);			\
		remove_array(stack, stack->size - 1);			\
		add_array(stack, r);					\
	}

CRAB_OP(add, +);
CRAB_OP(sub, -);
CRAB_OP(mul, *);
CRAB_OP(div, /);
CRAB_OP(gt, >);
CRAB_OP(lt, <);
CRAB_OP(gte, >=);
CRAB_OP(lte, <=);

void print_crab_value(crab_value *v) {
	switch (v->tag) { // @TODO: this is a copy
	case VALUE_STRING: {
		printf("%s\n", v->string);
		break;
	}
	case VALUE_NUMBER: {
		printf("%f\n", v->number);
		break;
	}
	case VALUE_FUNCTION: {
		printf("HA YOU THOUGHT\n");
		break;
	}
	}
}

void print_stack(array *stack) {
	for (int i = stack->size - 1; i >= 0; --i) {
		print_crab_value(GET_ARRAY(stack, i, crab_value *));
	}
}

void crab_print(array *stack) {
	crab_value *cv = GET_ARRAY(stack, stack->size - 1, crab_value *);
	print_crab_value(cv);
	remove_array(stack, stack->size - 1);
}

void free_crab_value(crab_value *cv) {
	if (cv->tag == VALUE_STRING) {
		free(cv->string);
	} else if (cv->tag == VALUE_FUNCTION) {
		/* free_array(cv->function.args); */
	}
	free(cv);
}

void eval(parse_node *node, array *stack, array *env) {
	char *node_buf = node->token.buf;
	
	if (!node->children) {
		crab_value *cv = malloc(sizeof(*cv));
		cv->id = NULL;
		switch (node->token.type) {
		case STRING: {
			cv->string = strdup(node_buf);
			cv->tag = VALUE_STRING;
			break;
		}
		case NUMBER: {
			cv->number = strtof(node_buf, NULL);
			cv->tag = VALUE_NUMBER;
			break;
		}
		case IDENTIFIER: {
			bool found = false;
			for (size_t i = 0; i < env->size; ++i) {
				crab_value *cv_i = GET_ARRAY(env, i, crab_value *);
				if (str_equals(cv_i->id, node_buf)) {
					*cv = *cv_i;
					found = true;
					break;
				}
			}
			assert(found);
			break;
		}
		default: {
			assert(false);
			break;
		}
		}

		add_array(stack, cv);
	} else {
		if (str_equals(node_buf, "define")) {
			crab_value *cv = malloc(sizeof(*cv));
			// @TODO: asserty stuff
			for (size_t i = 1; i < node->children->size; ++i) {
				eval(GET_ARRAY(node->children, i, parse_node *), stack, env);
			}
			*cv = *GET_ARRAY(stack, stack->size - 1, crab_value *);
			cv->id = GET_ARRAY(node->children, 0, parse_node *)->token.buf;
			remove_array(stack, stack->size - 1);
			add_array(env, cv);
		} else {
			for (size_t i = 0; i < node->children->size; ++i) {
				eval(GET_ARRAY(node->children, i, parse_node *), stack, env);
			}
			if (str_equals(node_buf, "#ROOT#") && node->token.type == PUNCTUATION) {
				return;
			}

			bool found = false;
			for (size_t i = 0; i < env->size; ++i) {
				crab_value *cv = GET_ARRAY(env, i, crab_value *);
				if (str_equals(node_buf, cv->id)) {
					if (cv->function.is_native) {
						cv->function.native(stack);
					} else {
						eval(cv->function.root, stack, env);
					}
					found = true;
				}
			}
			assert(found);
		}
	}
}

crab_value *make_function(char *id, parse_node *root, void (* native)(array *)) {
	crab_value *r = malloc(sizeof(*r));
	if (root) {
		r->function.is_native = false;
		r->function.root = root;
	} else {
		r->function.is_native = true;
		r->function.native = native;
	}
	r->tag = VALUE_FUNCTION;
	r->id = id;
	return r;
}

int main(void) {

	char *src = read_entire_file("test");
	
	array *tokens = tokenize(src, strlen(src));
	free(src);
	parse_node *root = parse(tokens);
	free_array(tokens);
	free(tokens);

	array env;
	init_array_f(&env, 32, sizeof(crab_value *), (void *) free_crab_value);

	add_array(&env, make_function("+", NULL, crab_add));
	add_array(&env, make_function("-", NULL, crab_sub));
	add_array(&env, make_function("*", NULL, crab_mul));
	add_array(&env, make_function("/", NULL, crab_div));
	add_array(&env, make_function("print", NULL, crab_print));
	
	array stack;
	init_array_f(&stack, 32, sizeof(crab_value *), (void *) free_crab_value);
	
	eval(root, &stack, &env);
	
	free_array(&stack);
	free_array(&env);

	free_parse_nodes(root);
	
	return 0;
}
