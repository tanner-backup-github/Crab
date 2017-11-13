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
		VARIANT_STRING,
		VARIANT_NUMBER,
		VARIANT_IDENTIFIER,
	} tag;
} variant;

void crab_define(array *stack, hash_table *env) {
        variant *value = pop_array(stack);
        variant *name  = pop_array(stack);
	
	add_hash_table(env, name->string, &value->number);
}

void crab_print(array *stack, hash_table *env) {
	variant *thing = pop_array(stack);

	if (thing->tag == VARIANT_STRING) {
		printf("%s", thing->string);
	} else if (thing->tag == VARIANT_NUMBER) {
		printf("%f", thing->number);
	} else if (thing->tag == VARIANT_IDENTIFIER) {
		printf("%f", *(double *) get_hash_table(env, thing->string));
	}
}

void crab_add(array *stack, hash_table *env) {
	variant *operand1 = pop_array(stack);
	variant *operand2 = pop_array(stack);
	assert(operand1->tag == VARIANT_NUMBER);
	assert(operand2->tag == VARIANT_NUMBER);

	variant *r = malloc(sizeof(*r));
	r->number = operand1->number + operand2->number;
	r->tag = VARIANT_NUMBER;
	add_array(stack, r);
}

void crab_sub(array *stack, hash_table *env) {
	variant *operand2 = pop_array(stack);
	variant *operand1 = pop_array(stack);
	assert(operand1->tag == VARIANT_NUMBER);
	assert(operand2->tag == VARIANT_NUMBER);

	variant *r = malloc(sizeof(*r));
	r->number = operand1->number - operand2->number;
	r->tag = VARIANT_NUMBER;
	add_array(stack, r);
}

void crab_mul(array *stack, hash_table *env) {
	variant *operand2 = pop_array(stack);
	variant *operand1 = pop_array(stack);
	assert(operand1->tag == VARIANT_NUMBER);
	assert(operand2->tag == VARIANT_NUMBER);

	variant *r = malloc(sizeof(*r));
	r->number = operand1->number * operand2->number;
	r->tag = VARIANT_NUMBER;
	add_array(stack, r);
}

void crab_div(array *stack, hash_table *env) {
	variant *operand2 = pop_array(stack);
	variant *operand1 = pop_array(stack);
	assert(operand1->tag == VARIANT_NUMBER);
	assert(operand2->tag == VARIANT_NUMBER);

	variant *r = malloc(sizeof(*r));
	r->number = operand1->number / operand2->number;
	r->tag = VARIANT_NUMBER;
	add_array(stack, r);
}

void eval(parse_node *root, array *stack, hash_table *env) {
	if (!root->children) {
		variant v = { 0 };
		if (root->token.type == NUMBER) {
			v.number = strtof(root->token.buf, NULL);
			v.tag = VARIANT_NUMBER;
		} else if (root->token.type == STRING) {
			v.string = strdup(root->token.buf);
			v.tag = VARIANT_STRING;
		} else if (root->token.type == IDENTIFIER) {
			v.string = strdup(root->token.buf);
			v.tag = VARIANT_IDENTIFIER;
		}

		variant *vv = malloc(sizeof(*vv));
		*vv = v;
		add_array(stack, vv);
	} else {
		for (int i = 0; i < root->children->size; ++i) {
			eval(GET_ARRAY(root->children, i, parse_node *), stack, env);
		}
		if (str_equals(root->token.buf, "#ROOT#") && root->token.type == PUNCTUATION) {
			return;
		}

		if (str_equals(root->token.buf, "+")) {
			crab_add(stack, env);
		} else if (str_equals(root->token.buf, "-")) {
			crab_sub(stack, env);
		} else if (str_equals(root->token.buf, "*")) {
			crab_mul(stack, env);
		} else if (str_equals(root->token.buf, "/")) {
			crab_div(stack, env);
		} else if (str_equals(root->token.buf, "define")) {
			crab_define(stack, env);
		} else if (str_equals(root->token.buf, "print")) {
			crab_print(stack, env);
		} else if (str_equals(root->token.buf, "print-nl")) {
			printf("\n");
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
	init_hash_table(&env, 32, sizeof(double), NULL);

	double PI = M_PI;
	add_hash_table(&env, "pi", &PI);
	
	array stack;
	INIT_ARRAY(&stack, 32, sizeof(variant *));
	eval(root, &stack, &env);
	free_array(&stack);
	free_hash_table(&env);

	free_parse_nodes(root);
	
	return 0;
}
