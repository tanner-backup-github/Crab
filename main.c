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
		struct {
			parse_node *root;
			array *args;
		} function;
	};
	enum {
	        VALUE_STRING,
		VALUE_NUMBER,
		VALUE_FUNCTION
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
	crab_value *v = (crab_value *) pop_array(stack);
	print_crab_value(v);
}

void eval(parse_node *node, array *stack, hash_table *env) {
	(void) node, (void) stack, (void) env;
}

int main(void) {

	char *src = read_entire_file("test");
	
	array *tokens = tokenize(src, strlen(src));
	free(src);
	parse_node *root = parse(tokens);
	free_array(tokens);
	free(tokens);
	
	hash_table env;
	/* init_hash_table(&env, 32, sizeof(crab_value *), NULL); */

	crab_value PI = { 0 };
        PI.number = M_PI;
        PI.tag = VALUE_NUMBER;
	
	(void) env, (void) PI;
	
	/* add_hash_table(&env, "pi", &PI); */
	/* crab_value *x = get_hash_table(&env, "pi"); */
	
	array stack;
	INIT_ARRAY(&stack, 32, sizeof(crab_value *));
	eval(root, &stack, &env);
	free_array(&stack);
	/* free_hash_table(&env); */

	free_parse_nodes(root);

	printf("\n");
	
	return 0;
}
