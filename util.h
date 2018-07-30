#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool str_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}
