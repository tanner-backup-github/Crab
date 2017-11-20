#ifndef UTIL_H__
#define UTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool str_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}

#define assert_LOG(b, s, fi, fu, li, ...) do {				\
		if (!(b)) {						\
			printf("\nAssertion failed [%s, %s, %d]: ", fi, fu, li); \
			printf(s, ##__VA_ARGS__);			\
			printf("\n");					\
			exit(-1);					\
		}							\
	} while (0);
#define ASSERT_LOG(b, s, ...) assert_LOG((b), (s), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
