#include <stdarg.h>

#include "../core.h"

char*
str(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, ap);
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

	char *s;
	fail(msg);
	s = str("  Failed test '%s'\n", msg);     diag(s); free(s);
	s = str("  expected: %s @ %p\n", a, sym1); diag(s); free(s);
	s = str("       got: %s @ %p\n", b, sym2); diag(s); free(s);
}

