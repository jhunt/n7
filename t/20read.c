#include <string.h>

#include "assert.h"

extern char *next_token(obj io);

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

static obj
open_file(const char *path)
{
	char *test = str("open %s for reading", path);

	FILE *fd = fopen(path, "r");
	ok(fd != NULL, test);

	free(test);
	if (!fd) bail("Cannot continue with read tests...");

	return io_fdopen(fd);
}

static obj
read_from(const char *path)
{
	obj io = open_file(path);
	obj result = readx(io);
	io_close(io);
	return result;
}

static void
test_tokenizer(void)
{
	WITH_ABORT_PROTECTION {
		char *token;
		obj io;

		io = io_string(
			"; tokenizer baseline test; ensures that next_token\n"
			"; tokenizer baseline test; ensures that next_token\n"
			"; can split up lisp code into discrete chunks\n"
			"\n"
			"(a b -42.5\n"
			"\n"
			"	(\n"
			"	   * ; this is multiplication\n"
			"	   delta 2))\n"
			"\n"
			"; vim:ft=lisp\n");

		token = next_token(io);
		string_is(token, "(", "first token is '('");

		token = next_token(io);
		string_is(token, "a", "second token is 'a'");

		token = next_token(io);
		string_is(token, "b", "third token is 'b'");

		token = next_token(io);
		string_is(token, "-42.5", "4th token is '-42.5'");

		token = next_token(io);
		string_is(token, "(", "5th token is '('");

		token = next_token(io);
		string_is(token, "*", "6th token is '*'");

		token = next_token(io);
		string_is(token, "delta", "7th token is 'delta'");

		token = next_token(io);
		string_is(token, "2", "8th token is '2'");

		token = next_token(io);
		string_is(token, ")", "9th token is ')'");

		token = next_token(io);
		string_is(token, ")", "10th token is ')'");

		token = next_token(io);
		ok(token == NULL, "final token is NULL (EOF)");
	}
}

#define LONG_TOKEN_NAME "this-is-a-long-symbol-probably-the-longest-symbol-that-we-will-ever-see-in-n7-because--well--it-is-ridiculous-at-almost-140-characters-long"
static void
test_token_limits(void)
{
	WITH_ABORT_PROTECTION {
		obj io = io_string(LONG_TOKEN_NAME);
		obj sym = readx(io);
		ok(strcmp(sym->value.sym.name, LONG_TOKEN_NAME) == 0,
			"symbol names not truncated during read");
	}
}

static void
test_reader(void)
{
	WITH_ABORT_PROTECTION {
		obj io, x;

		io = io_string("20\n"); x = readx(io);
		if (IS_FIXNUM(x)) {
			fixnum_is(x, 20, "read 20 as a fixnum");
		} else {
			fail("read 20 as a fixnum");
		}

		io = io_string("symbolix\n"); x = readx(io);
		if (IS_SYMBOL(x)) {
			ok(x == intern("symbolix"), "read 'symbolix");
		} else {
			diag("failed to read symbol");
			fail("read 'symbolix as a symbol");
		}

		io = io_string("nil\n"); x = readx(io);
		ok(IS_NIL(x), "read nil as NIL value");

		io = io_string("t\n"); x = readx(io);
		ok(IS_T(x), "read t as T value");

		io = io_string("; these are comments\n\t; ending in a T value\n  t\n");
		x = readx(io);
		ok(IS_T(x), "read through the comments");

		io = io_string("          \n\t\r\n\n\nt\n\n   \n");
		x = readx(io);
		ok(IS_T(x), "read through the whitespace");

		io = io_string("; nothing but comments, until EOF\n");
		x = readx(io);
		ok(IS_NIL(x), "got nil when reading to EOF (end comments)");
	}
}

static void
test_reader_lists(void)
{
	WITH_ABORT_PROTECTION {

		obj io;

		obj x = intern("x");
		obj y = intern("y");
		obj z = intern("z");

		obj lst, expect;
		obj str;

		io = io_string("()"); lst = readx(io);
		ok(IS_NIL(lst), "() == nil");

		io = io_string("(x)"); lst = readx(io);
		expect = cons(x, NIL);
		obj_equal(lst, expect, "read (x)");

		io = io_string("(x y)"); lst = readx(io);
		expect = cons(x, cons(y, NIL));
		obj_equal(lst, expect, "read (x y)");

		io = io_string("(x y z)"); lst = readx(io);
		expect = cons(x, cons(y, cons(z, NIL)));
		obj_equal(lst, expect, "read (x y z)");

		io = io_string("(x (y) z)"); lst = readx(io);
		expect = cons(x, cons( cons(y, NIL), cons(z, NIL)));
		obj_equal(lst, expect, "read (x (y) z)");

		io = io_string("(x . NIL)"); lst = readx(io);
		expect = cons(x, NIL);
		obj_equal(lst, expect, "read (x . NIL)");

		io = io_string("(x . y)"); lst = readx(io);
		expect = cons(x, y);
		obj_equal(lst, expect, "read (x . y)");

		io = io_string("(x . (y . (z . NIL)))"); lst = readx(io);
		expect = cons(x, cons(y, cons(z, NIL)));
		obj_equal(lst, expect, "read (x . (y . (z . NIL)))");

		io = io_string("(x . ( y . z))"); lst = readx(io);
		expect = cons(x, cons(y, z));
		obj_equal(lst, expect, "read (x . (y . z))");

		io = io_string("\"test string\""); str = readx(io);
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
