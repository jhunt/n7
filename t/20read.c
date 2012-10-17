#include <string.h>

#include "assert.h"

extern char *next_token(FILE *io);

static void
string_is(const char *got, const char *expect, const char *msg)
{
	if (got && expect && strcmp(got, expect) == 0) {
		pass(msg);
		return;
	}

	char *s;
	fail(msg);
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: '%s'", got));
	vdiag(str("  expected: '%s'", expect));
}

static int
obj_eq(obj got, obj expect, size_t n, const char *msg)
{
	char *s;

	if ((IS_NIL(got) && IS_NIL(expect)) || IS_T(got) && IS_T(expect)) {
		pass(msg);
		return 1;
	}

	if (!IS_NIL(got) && IS_NIL(expect)) {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("  list continued on past index %u", n));
		return 0;
	}

	if (IS_NIL(got) && !IS_NIL(expect)) {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("  list ended prematurely at index %u", n));
		return 0;
	}

	if (!IS_T(got) && IS_T(expect)) {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		/* FIXME: we are expecting something explicit... need princ support */
		vdiag(str("  item[%u] was: %s", n, "<not t>"));
		       diag("      expected: t");
	}

	if (IS_T(got) && !IS_T(expect)) {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("  item[%u] was: t", n));
		/* FIXME: we are expecting something explicit... need princ support */
		vdiag(str("      expected: %s", "<something else>"));
		return 0;
	}

	if (got->type != expect->type) {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("       got[%u] != expect[%u] - type mismatch", n, n));
		return 0;
	}


	switch (got->type) {
		case OBJ_CONS:
			return (obj_eq(car(got), car(expect), n, msg) &&
			        obj_eq(cdr(got), cdr(expect), n+1, msg));

		case OBJ_SYMBOL:
			if (got != expect) {
				fail(msg);
				vdiag(str("  Failed test '%s'", msg));
				vdiag(str("       got: %s @ %p\n", got->value.sym.name, got));
				vdiag(str("  expected: %s @ %p\n", expect->value.sym.name, got));
				return 0;
			}
			return 1;

		case OBJ_FIXNUM:
			if (got->value.fixnum != expect->value.fixnum) {
				fail(msg);
				vdiag(str("  Failed test '%s'", msg));
				vdiag(str("       got: %li", got->value.fixnum));
				vdiag(str("  expected: %li", expect->value.fixnum));
				return 0;
			}
			return 1;

		default:
			fail(msg);
			vdiag(str("  Failed test '%s'", msg));
			diag("unknown object type...");
			return 0;
	}
}

static void
list_is(obj got, obj expect, const char *msg)
{
	if (obj_eq(got, expect, 0, msg)) {
		pass(msg);
	}
}

static FILE*
open_file(const char *path)
{
	char *test = str("open %s for reading", path);

	FILE *fd = fopen(path, "r");
	ok(fd != NULL, test);

	free(test);
	if (!fd) bail("Cannot continue with read tests...");

	return fd;
}

static obj
read_from(const char *path)
{
	FILE *fd = open_file(path);
	obj result = readx(fd);
	fclose(fd);
	return result;
}

static void
test_tokenizer(void)
{
	WITH_ABORT_PROTECTION {
		char *token;
		FILE *fd;

		fd = open_file("t/read/tokens");

		token = next_token(fd);
		string_is(token, "(", "first token is '('");

		token = next_token(fd);
		string_is(token, "a", "second token is 'a'");

		token = next_token(fd);
		string_is(token, "b", "third token is 'b'");

		token = next_token(fd);
		string_is(token, "-42.5", "4th token is '-42.5'");

		token = next_token(fd);
		string_is(token, "(", "5th token is '('");

		token = next_token(fd);
		string_is(token, "*", "6th token is '*'");

		token = next_token(fd);
		string_is(token, "delta", "7th token is 'delta'");

		token = next_token(fd);
		string_is(token, "2", "8th token is '2'");

		token = next_token(fd);
		string_is(token, ")", "9th token is ')'");

		token = next_token(fd);
		string_is(token, ")", "10th token is ')'");

		token = next_token(fd);
		ok(token == NULL, "final token is NULL (EOF)");
	}
}

#define LONG_TOKEN_NAME "this-is-a-long-symbol-probably-the-longest-symbol-that-we-will-ever-see-in-n7-because--well--it-is-ridiculous-at-almost-140-characters-long"
static void
test_token_limits(void)
{
	WITH_ABORT_PROTECTION {
		obj sym = read_from("t/read/looooooong");
		ok(strcmp(sym->value.sym.name, LONG_TOKEN_NAME) == 0,
			"symbol names not truncated during read");
	}
}

static void
test_reader(void)
{
	WITH_ABORT_PROTECTION {
		obj x;
		x = read_from("t/read/number");
		if (IS_FIXNUM(x)) {
			fixnum_is(x, 20, "read 20 as a fixnum");
		} else {
			fail("read 20 as a fixnum");
		}

		x = read_from("t/read/symbol");
		if (IS_SYMBOL(x)) {
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
}

static void
test_reader_lists(void)
{
	WITH_ABORT_PROTECTION {

		obj x = intern("x");
		obj y = intern("y");
		obj z = intern("z");

		obj lst, expect;
		obj str;

		lst = read_from("t/read/list/empty");
		ok(IS_NIL(lst), "() == nil");

		expect = cons(x, NIL);
		lst = read_from("t/read/list/1");
		list_is(lst, expect, "read (x)");

		expect = cons(x, cons(y, NIL));
		lst = read_from("t/read/list/2");
		list_is(lst, expect, "read (x y)");

		expect = cons(x, cons(y, cons(z, NIL)));
		lst = read_from("t/read/list/3");
		list_is(lst, expect, "read (x y z)");

		expect = cons(x, cons( cons(y, NIL), cons(z, NIL)));
		lst = read_from("t/read/list/sub1");
		list_is(lst, expect, "read (x (y) z)");

		expect = cons(x, NIL);
		lst = read_from("t/read/list/dot1");
		list_is(lst, expect, "read (x . NIL)");

		expect = cons(x, y);
		lst = read_from("t/read/list/dot2");
		list_is(lst, expect, "read (x . y)");

		expect = cons(x, cons(y, cons(z, NIL)));
		lst = read_from("t/read/list/dot3");
		list_is(lst, expect, "read (x . (y . (z . NIL)))");

		expect = cons(x, cons(y, z));
		lst = read_from("t/read/list/dot-short");
		list_is(lst, expect, "read (x . (y . z))");

		str = read_from("t/read/string");
		vstring_is(str, "test string", "read \"test string\"");

		str = read_from("t/read/tldr");
		vstring_is(str, "Of Man's first disobedience, and the fruit "
				"Of that forbidden tree whose mortal taste "
				"Brought death into the World, and all our woe, "
				"With loss of Eden, till one greater Man "
				"Restore us, and regain the blissful seat, "
				"Sing, Heavenly Muse, that, on the secret top "
				"Of Oreb, or of Sinai, didst inspire "
				"That shepherd who first taught the chosen seed "
				"In the beginning how the heavens and earth "
				"Rose out of Chaos: or, if Sion hill "
				"Delight thee more, and Siloa's brook that flowed "
				"Fast by the oracle of God, I thence "
				"Invoke thy aid to my adventurous song, "
				"That with no middle flight intends to soar "
				"Above th' Aonian mount, while it pursues "
				"Things unattempted yet in prose or rhyme.",

				"long strings don't get truncated");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_tokenizer();
	test_token_limits();
	test_reader();
	test_reader_lists();

	done_testing();
	return 0;
}
