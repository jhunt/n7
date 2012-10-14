#ifndef TEST_H
#define TEST_H

#include <stdio.h>

unsigned int NEXT_TEST = 1;

inline void
run_tests(unsigned int ntests)
{
	fprintf(stdout, "1..%u\n", ntests);
}

inline void
done_testing(void)
{
	fprintf(stdout, "1..%u\n", NEXT_TEST-1);
}

inline void
diag(const char *msg)
{
	fprintf(stdout, "# %s\n", msg);
}

inline void
pass(const char *msg)
{
	fprintf(stdout, "ok %u - %s\n", NEXT_TEST++, msg);
}

inline void
fail(const char *msg)
{
	fprintf(stdout, "not ok %u - %s\n", NEXT_TEST++, msg);
}

inline void
skip(const char *msg)
{
	fprintf(stdout, "ok %u # SKIP %s\n", NEXT_TEST++, msg);
}

#endif
