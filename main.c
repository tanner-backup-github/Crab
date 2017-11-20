#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"
#include "dumb_string.h"
#include "file.h"
#include "hash_table.h"
#include "util.h"

// @TODO: lists & quote
// @TODO: (print "\n")
// @TODO: char[] array
// @TODO: safe_free safe_malloc
// @TODO: Line numbers on errors
// @TODO: comments are messed up
// @TODO: negative numbers tokenizer
// @TODO: pool allocator

typedef struct {
	union {
		double number;
		bool boolean;
		struct {
			union {
				parse_node *root;
				void (* native)(array *);
			};
			char **arg_names;
			size_t num_args;
			bool is_native;
		} function;
	};
	enum {
		VALUE_NUMBER,
		VALUE_BOOLEAN,
		VALUE_FUNCTION,
	} tag;
} crab_value;

#define CRAB_OP(name, op) void crab_ ##name(array *stack) {		\
		crab_value *o1 = GET_ARRAY(stack, stack->size - 1, crab_value *); \
		crab_value *o2 = GET_ARRAY(stack, stack->size - 2, crab_value *); \
	        ASSERT_LOG(o1->tag == VALUE_NUMBER && o2->tag == VALUE_NUMBER, "Type mismatch!\n"); \
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

#define CRAB_CMP(name, op) void crab_ ##name(array *stack) {		\
		crab_value *o1 = GET_ARRAY(stack, stack->size - 1, crab_value *); \
		crab_value *o2 = GET_ARRAY(stack, stack->size - 2, crab_value *); \
	        ASSERT_LOG(o1->tag == VALUE_NUMBER && o2->tag == VALUE_NUMBER, "Type mismatch!\n"); \
		crab_value *r = malloc(sizeof(*r));			\
		r->boolean = (o2->number op o1->number) ? true : false;	\
		r->tag = VALUE_BOOLEAN;					\
		remove_array(stack, stack->size - 1);			\
		remove_array(stack, stack->size - 1);			\
		add_array(stack, r);					\
	}

CRAB_CMP(gt, >);
CRAB_CMP(lt, <);
CRAB_CMP(gte, <=);
CRAB_CMP(lte, >=);
CRAB_CMP(eq, ==);

void print_crab_value(crab_value *v) {
	switch (v->tag) {
	case VALUE_NUMBER: {
		printf("%f ", v->number);
		break;
	}
	case VALUE_FUNCTION: {
		printf("Functions do not print right now!");
		break;
	}
	case VALUE_BOOLEAN: {
		printf("%s ", v->boolean ? "true" : "false");
	}
	}
}

void print_stack(array *stack) {
	for (int64_t i = stack->size - 1; i >= 0; --i) {
		print_crab_value(GET_ARRAY(stack, (size_t) i, crab_value *));
	}
}

void crab_print(array *stack) {
	crab_value *v = GET_ARRAY(stack, stack->size - 1, crab_value *);
	print_crab_value(v);
	remove_array(stack, stack->size - 1);
}

void free_crab_value(crab_value *v) {
	switch (v->tag) {
	case VALUE_FUNCTION: {
		if (v->function.arg_names) {
			for (size_t i = 0; i < v->function.num_args; ++i) {
				free(v->function.arg_names[i]);
			}
			free(v->function.arg_names);
		}
		break;
	}
	case VALUE_NUMBER: {
		break;
	}
	case VALUE_BOOLEAN: {
		break;
	}
	}
	free(v);
}

void print_parse_nodes(parse_node *node) {
	if (!node->children) {
		printf("%s\n", node->token.buf);
	} else {
		printf("root: %s\n", node->token.buf);
		for (size_t i = 0; i < node->children->size; ++i) {
			print_parse_nodes(GET_ARRAY(node->children, i, parse_node *));
		}
	}
}

crab_value *make_native_function(void (* native)(array *), size_t num_args) {
	crab_value *r = malloc(sizeof(*r));
	r->function.arg_names = NULL;
	r->function.num_args = num_args;
	r->function.root = NULL;
	r->function.is_native = true;
	r->function.native = native;
	r->tag = VALUE_FUNCTION;
	return r;
}

crab_value *make_lambda_function(parse_node *root, parse_node *arg_nodes) {
	crab_value *r = malloc(sizeof(*r));
	assert(r);
	
	if (str_equals(arg_nodes->token.buf, "()")) {
		r->function.num_args = 0;
		r->function.arg_names = NULL;
	} else {		
		r->function.num_args = 1 + arg_nodes->children->size;
		r->function.arg_names = malloc(sizeof(char *) * (r->function.num_args));
		assert(r->function.arg_names);
		
		r->function.arg_names[0] = strdup(arg_nodes->token.buf);
		for (size_t i = 0; i < r->function.num_args - 1; ++i) {
			r->function.arg_names[1 + i] =
				strdup(GET_ARRAY(arg_nodes->children, i, parse_node *)->token.buf);
		}
	}
	
	r->function.is_native = false;
	r->function.root = root;
	r->tag = VALUE_FUNCTION;
	return r;
}

void eval(parse_node *node, array *stack, hash_table *env) {
	char *node_buf = node->token.buf;

	if (!node->children) {
		crab_value *v = malloc(sizeof(*v));
		assert(v);
		
		switch (node->token.type) {
		case LITERAL: {
			if (is_num(node_buf)) {
				v->number = strtof(node_buf, NULL);
				v->tag = VALUE_NUMBER;
			} else if (is_boolean(node_buf)) {
				v->boolean = str_equals(node_buf, "true") ? true : false;
				v->tag = VALUE_BOOLEAN;
			} else {
				assert(false);
			}
			break;
		}
		case IDENTIFIER: {
			*v = *GET_HASH_TABLE(env, node_buf, crab_value *);
			break;
		}
		case PUNCTUATION: {
			assert(false);
			break;
		}
		}

		add_array(stack, v);
	} else {
		if (str_equals(node_buf, "define")) {
			ASSERT_LOG(node->children->size == 2,
				   "Incorrect arity supplied to function %s\n", node_buf);
			
			for (size_t i = 1; i < node->children->size; ++i) {
				eval(GET_ARRAY(node->children, i, parse_node *), stack, env);
			}
			// @NOTE: deep copy at the expense of more memory? (edit: seems to be negligible?)

			crab_value *v = malloc(sizeof(*v));
			assert(v);
			
			crab_value *to_assign = GET_ARRAY(stack, stack->size - 1, crab_value *);
			*v = *to_assign;
			if (to_assign->tag == VALUE_FUNCTION) {
				v->function.arg_names = malloc(to_assign->function.num_args * sizeof(char *));

				for (size_t i = 0; i < to_assign->function.num_args; ++i) {
					v->function.arg_names[i] = strdup(to_assign->function.arg_names[i]);
				}
			}

			remove_array(stack, stack->size - 1);
			
			add_hash_table(env, GET_ARRAY(node->children, 0, parse_node *)->token.buf, v);
		} else if (str_equals(node_buf, "if")) {
			ASSERT_LOG(node->children->size == 3,
				   "Incorrect arity supplied to function %s\n", node_buf);
			
			eval(GET_ARRAY(node->children, 0, parse_node *), stack, env);

			crab_value *which = GET_ARRAY(stack, stack->size - 1, crab_value *);

			bool b = which->boolean;
			remove_array(stack, stack->size - 1);
			
			if (b) {
				eval(GET_ARRAY(node->children, 1, parse_node *), stack, env);
			} else {
				eval(GET_ARRAY(node->children, 2, parse_node *), stack, env);
			}			
		} else if (str_equals(node_buf, "lambda")) {
			ASSERT_LOG(node->children->size == 2,
				   "Incorrect arity supplied to function %s\n", node_buf);
			
			crab_value *lambda = make_lambda_function(GET_ARRAY(node->children, 1, parse_node *),
								  GET_ARRAY(node->children, 0, parse_node *));
			add_array(stack, lambda);
		} else if (str_equals(node_buf, "do")) {
			for (size_t i = 0; i < node->children->size; ++i) {
				eval(GET_ARRAY(node->children, i, parse_node *), stack, env);
			}
		} else if (str_equals(node_buf, "quote")) {
			
		} else {
			for (size_t i = 0; i < node->children->size; ++i) {
				eval(GET_ARRAY(node->children, i, parse_node *), stack, env);
			}
			if (str_equals(node_buf, "#ROOT#") && node->token.type == PUNCTUATION) {
				return;
			}

			crab_value *v = GET_HASH_TABLE(env, node_buf, crab_value *);
			size_t num_args = v->function.num_args;
			ASSERT_LOG(node->children->size == num_args,
				   "Incorrect arity supplied to function %s\n", node_buf);

			if (v->function.is_native) {
				v->function.native(stack);
			} else {
				while (num_args) {
					crab_value *local = malloc(sizeof(*local));
					assert(local);
					
					*local = *GET_ARRAY(stack, stack->size - 1, crab_value *);
					add_hash_table(env, v->function.arg_names[num_args - 1], local);
					remove_array(stack, stack->size - 1);
					
					--num_args;
				}
				
				eval(v->function.root, stack, env);

				// @TODO: fix recursion!!!!!!
				for (size_t i = 0; i < v->function.num_args; ++i) {
					remove_hash_table(env, v->function.arg_names[i]);
				}
			}
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Not enough arguments passed to evaluate a file!\n");
		return -1;
	}
	
	char *src = read_entire_file(argv[1]);

	array *tokens = tokenize(src, strlen(src));
	free(src);
	parse_node *root = parse(tokens);
	free_array(tokens);
	free(tokens);

	hash_table env;
	init_hash_table(&env, 64, (void *) free_crab_value);

	add_hash_table(&env, "+",  make_native_function(crab_add, 2));
	add_hash_table(&env, "-",  make_native_function(crab_sub, 2));
	add_hash_table(&env, "*",  make_native_function(crab_mul, 2));
	add_hash_table(&env, "/",  make_native_function(crab_div, 2));
	add_hash_table(&env, ">",  make_native_function(crab_gt, 2));
	add_hash_table(&env, "<",  make_native_function(crab_lt, 2));
	add_hash_table(&env, ">=", make_native_function(crab_lte, 2));
	add_hash_table(&env, "<=", make_native_function(crab_gte, 2));
	add_hash_table(&env, "=",  make_native_function(crab_eq, 2));
	add_hash_table(&env, "print", make_native_function(crab_print, 1));

	array stack;
	init_array_f(&stack, 32, sizeof(crab_value *), (void *) free_crab_value);

	eval(root, &stack, &env);

	free_array(&stack);
	free_hash_table(&env);

	free_parse_nodes(root);

	puts("");

	return 0;
}
