#include "assert.h"

static obj
read_from(const char *path)
{
	char *test = str("open %s for reading", path);

	FILE *fd = fopen(path, "r");
	ok(fd != NULL, test);

	free(test);
	if (!fd) bail("Cannot continue with read tests...");

	obj result = readio(fd);
	fclose(fd);
	return result;
}

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {
		obj x;
		x = read_from("t/read/number");
		if (IS_FIXNUM(x)) {
			fixnum_is(x, 20, "read 20 as a fixnum");
		} else {
			diag("failed to read fixnum from t/read/number");
			fail("read 20 as a fixnum");
		}

		x = read_from("t/read/symbol");
		if (IS_SYM(x)) {
			ok(x == intern("symbolix"), "read 'symbolix from t/read/symbol");
		} else {
			diag("failed to read symbol from t/read/symbol");
			fail("read 'symbolix as a symbol");
		}

		x = read_from("t/read/nil");
		ok(IS_NIL(x), "read nil as NIL value");

		x = read_from("t/read/t");
		ok(IS_T(x), "read t as T value");

		x = read_from("t/read/comments");
		ok(IS_T(x), "read through the comments");

		x = read_from("t/read/whitespace");
		ok(IS_T(x), "read through the whitespace");

		x = read_from("t/read/eof");
		ok(IS_NIL(x), "got nil when reading to EOF (end comments)");
	}

	done_testing();
	return 0;
}
