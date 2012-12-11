#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
}

inline void
diag(const char *msg)
{
	fprintf(stdout, "# %s\n", msg);
}
inline void
vdiag(char *vstr)
{
	diag(vstr);
	free(vstr);
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
	fprintf(stdout, "Bail out! %s\n", msg);
	exit(1);
}

char*
str(const char *fmt, ...)
{
	size_t len;

	va_list ap;
	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap)+1; /* +1 for \0 */
	va_end(ap);

	char *buf = calloc(len, sizeof(char));
	if (!buf) bail("memory exhausted (string:malloc failed)");

	va_start(ap, fmt);
	vsnprintf(buf, len, fmt, ap);
	va_end(ap);

	return buf;
}

inline void
ok(int expr, const char *msg)
{
	if (expr) {
		pass(msg);
	} else {
		fail(msg);
	     diag("");
		vdiag(str("  Failed test '%s'", msg));
	     diag("");
	}
}

#endif
