#include <stdarg.h>

#include "../core.h"

void
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

void
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

void
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

void
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

void
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

void
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

