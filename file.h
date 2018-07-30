#pragma once

#include <stdlib.h>
#include <stdio.h>

char *read_entire_file(const char *path) {
	FILE *f = fopen(path, "r");
	assert(f);
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);
	char *buf = calloc(fsize + 1, sizeof(char));
	assert(buf);
        long res = fread(buf, sizeof(char), fsize, f);
	(void) res;
	fclose(f);
	return buf;
}
