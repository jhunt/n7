#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>

unsigned int FAILED_TESTS = 0;
unsigned int TOTAL_TESTS  = 0;

unsigned int NEXT_TEST = 1;
unsigned int PLANNED = 0;

inline void
run_tests(unsigned int ntests)
{
	PLANNED = 1;
	fprintf(stdout, "1..%u\n", ntests);
}

inline void
done_testing(void)
{
	if (!PLANNED) {
		fprintf(stdout, "1..%u\n", TOTAL_TESTS);
	}
	if (FAILED_TESTS) {
		fprintf(stdout, "# Looks like you failed %u out of %u tests\n",
				FAILED_TESTS, TOTAL_TESTS);
	}
}

inline void
diag(const char *msg)
{
	fprintf(stdout, "# %s\n", msg);
}

inline void
pass(const char *msg)
{
	fprintf(stdout, "ok %u - %s\n", ++TOTAL_TESTS, msg);
}

inline void
fail(const char *msg)
{
	fprintf(stdout, "not ok %u - %s\n", ++TOTAL_TESTS, msg);
	FAILED_TESTS++;
}

inline void
skip(const char *msg)
{
	fprintf(stdout, "ok %u # SKIP %s\n", ++TOTAL_TESTS, msg);
}

inline void
bail(const char *msg)
{
	fprintf(stdout, "bail out! %s\n", msg);
	exit(1);
}

#endif
