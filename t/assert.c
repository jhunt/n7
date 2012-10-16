#include <stdarg.h>

#include "../core.h"

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
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %s @ %p\n", b, sym2));
	vdiag(str("  expected: %s @ %p\n", a, sym1));
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
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %s @ %p\n", b, sym2));
	     diag("  expected: <anything else>");
}

void
fixnum_is(obj got, long expect, const char *msg)
{
	if (got->value.fixnum == expect) {
		pass(msg);
		return;
	}

	fail(msg);
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %li\n", got->value.fixnum));
	vdiag(str("  expected: %li\n", expect));
}

void
fixnum_isnt(obj got, long expect, const char *msg)
{
	if (got->value.fixnum != expect) {
		pass(msg);
		return;
	}

	fail(msg);
	vdiag(str("  Failed test '%s'\n", msg));
	vdiag(str("       got: %li\n", got->value.fixnum));
	     diag("  expected: <anything else>");
}

