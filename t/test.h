/*
  Copyright (c) 2012-2016 James Hunt

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software..

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
 */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "../core.h"

unsigned int FAILED_TESTS = 0;
unsigned int TOTAL_TESTS  = 0;

unsigned int NEXT_TEST = 1;
unsigned int PLANNED = 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static inline void
run_tests(unsigned int ntests)
{
	PLANNED = 1;
	fprintf(stdout, "1..%u\n", ntests);
}

static inline void
done_testing(void)
{
	if (!PLANNED) {
		fprintf(stdout, "1..%u\n", TOTAL_TESTS);
	}
}

static inline void
diag(const char *msg)
{
	fprintf(stdout, "# %s\n", msg);
}
static inline void
vdiag(char *vstr)
{
	diag(vstr);
	free(vstr);
}

static inline void
pass(const char *msg)
{
	fprintf(stdout, "ok %u - %s\n", ++TOTAL_TESTS, msg);
}

static inline void
fail(const char *msg)
{
	fprintf(stdout, "not ok %u - %s\n", ++TOTAL_TESTS, msg);
	FAILED_TESTS++;
}

static inline void
skip(const char *msg)
{
	fprintf(stdout, "ok %u # SKIP %s\n", ++TOTAL_TESTS, msg);
}

static inline void
bail(const char *msg)
{
	fprintf(stdout, "Bail out! %s\n", msg);
	exit(1);
}

static char*
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

static inline void
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

/* kids, don't ever abuse C like this.  it's wrong. */
#define __jmpvar__ __I_PROMISE_NEVER_TO_ABUSE_C_MACROS_LIKE_THIS_EVER_AGAIN__
#define WITH_ABORT_PROTECTION \
	jmp_buf __jmpvar__; \
	on_abort(&__jmpvar__); \
	if (setjmp(__jmpvar__) != 0) { bail("ABORTED ABNORMALLY"); } else

static void
sym_is(const char *a, const char *b, const char *msg)
{
	obj sym1 = intern(a);
	obj sym2 = intern(b);

	if (sym1 == sym2) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %s @ %p\n", b, sym2));
	vdiag(str("  expected: %s @ %p\n", a, sym1));
	     diag("");
}

static void
sym_isnt(const char *a, const char *b, const char *msg)
{
	obj sym1 = intern(a);
	obj sym2 = intern(b);

	if (sym1 != sym2) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %s @ %p\n", b, sym2));
	     diag("  expected: <anything else>");
	     diag("");
}

static void
fixnum_is(obj got, long expect, const char *msg)
{
	if (got->value.fixnum == expect) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %li\n", got->value.fixnum));
	vdiag(str("  expected: %li\n", expect));
	     diag("");
}

static void
fixnum_isnt(obj got, long expect, const char *msg)
{
	if (got->value.fixnum != expect) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %li\n", got->value.fixnum));
	     diag("  expected: <anything else>");
	     diag("");
}

static void
vstring_is(obj got, const char *expect, const char *msg)
{
	if (!IS_STRING(got)) {
		fail(msg);
		     diag("");
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("       got: a non-string"));
		     diag("");
		return;
	}

	if (strcmp(got->value.string.data, expect) == 0) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: '%s'", got->value.string.data));
	vdiag(str("  expected: '%s'", expect));
	     diag("");
}

static void
vstring_isnt(obj got, const char *expect, const char *msg)
{
	if (strcmp(got->value.string.data, expect) != 0) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: '%s'", got->value.string.data));
	     diag("  expected: anything else...");
	     diag("");
}

static void
obj_equal(obj got, obj exp, const char *msg)
{
	if (IS_T(equal(got, exp))) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(got)));
	vdiag(str("  expected: %s", cdump(exp)));
	     diag("");
}

static void
obj_eql(obj got, obj exp, const char *msg)
{
	if (IS_T(eql(got, exp))) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(got)));
	vdiag(str("  expected: %s", cdump(exp)));
	     diag("");
}

static void
obj_eq(obj got, obj exp, const char *msg)
{
	if (IS_T(eq(got, exp))) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(got)));
	vdiag(str("  expected: %s", cdump(exp)));
	     diag("");
}

static void
is_defined(obj x, const char *msg)
{
	if (DEF(x)) {
		pass(msg);
		return;
	}

	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	     diag("       got: <undefined>");
	     diag("  expected: anything else...");
	     diag("");
}

static void
isnt_defined(obj x, const char *msg)
{
	if (!DEF(x)) {
		pass(msg);
		return;
	}

	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(x)));
	     diag("  expected: <undefined>");
	     diag("");
}

#pragma clang diagnostic pop

#endif
