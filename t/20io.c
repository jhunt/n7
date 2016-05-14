#include "test.h"

static void
print_ok(obj x, const char *expect, const char *msg)
{

	obj exp = str_dupc(expect);
	obj got = vdump(x);
	obj_equal(got, exp, msg);
}

/*****************************************************/

static void
test_readfile(void)
{
	obj xyzzy = str_dupc("something to write");
	obj io = io_fopen("t/io/writefile1", "w");

	ok(IS_IO(io), "io_fopen returns an io object");
	ok(IS_T(io_write_str(io, xyzzy)), "wrote something to t/io/writefile1");
	io_close(io);

	io = io_fopen("t/io/writefile1", "r");
	ok(IS_IO(io), "io_fopen returns an io object (for reading)");

	obj tmp = io_read_buf(io, 1024); /* arbitrary n */
	obj_equal(tmp, xyzzy, "read back what we wrote out");
	io_rewind(io);

	char c;
	c = io_getc(io);
	ok(c == 's', "io_getc returned first character");
	c = io_getc(io);
	ok(c == 'o', "io_getc returned second character");

	io_rewind(io);
	c = io_getc(io);
	ok(c == 's', "io_getc (after rewind) returns first character");

	io_close(io);
}

static void
test_read_badfile(void)
{
	ok(IS_NIL(io_fopen("/E/NO/ENT", "r")), "io_fopen enoent file -> NIL");
}

static void
test_readstring(void)
{
	obj xyzzy = str_dupc("this is a test string");
	obj io = io_string(xyzzy->value.string.data);

	ok(IS_IO(io), "io_string returns an io object");

	char c;
	ok((c = io_getc(io)) == 't', "io_getc(ios) returns 1st character");
	ok((c = io_getc(io)) == 'h', "io_getc(ios) returns 2nd character");
	io_rewind(io);
	ok((c = io_getc(io)) == 't', "io_getc(ios) returns 1st character again (after rewind)");
	io_ungetc(io, 't');
	ok((c = io_getc(io)) == 't', "io_getc(ios) returns 1st character again (after ungetc(t))");

	/* io_read_buf */
	io_rewind(io);
	obj tmp = io_read_buf(io, 1024); /* arbitrary n */
	obj_equal(tmp, xyzzy, "read io string properly");

	/* ungetc(Q) */
	io_rewind(io);
	io_getc(io);
	io_ungetc(io, 'Q');
	ok((c = io_getc(io)) == 'Q', "io_getc(ios) returns Q (after ungetc(Q))");

	obj overwrite = str_dupc("OVERWRITE!");
	io_write_str(io, overwrite);
	io_rewind(io);
	tmp = io_read_buf(io, 1024);
	obj_equal(tmp, overwrite, "read overwritten io string properly");

	io_close(io);
}


static void
test_print(void)
{
	WITH_ABORT_PROTECTION {
		print_ok(T, "t", "T prints ok");
		print_ok(NIL, "nil", "NIL prints ok");
		print_ok(fixnum(42), "42", "fixnum prints ok");

		obj X = intern("X");
		obj Y = intern("Y");
		print_ok(cons(X,Y),   "(x . y)", "CONS prints ok");
		print_ok(cons(X,NIL), "(x)", "list prints ok");
		print_ok(cons(X,cons(Y,NIL)), "(x y)", "list of 2 prints ok");
		print_ok(cons(X,cons(Y,cons(X,NIL))), "(x y x)", "list of 3 prints ok");

		obj str = str_dupc("kill troll with axe");
		print_ok(str, "\"kill troll with axe\"", "string prints ok");

		obj A = intern("A");
		obj B = intern("B");
		obj tree = cons(
			cons(A, B),
			cons(X, Y));
		/* odd, but correct... */
		print_ok(tree, "((a . b) x . y)", "simple tree prints ok");

		/* check to make sure we don't abort */
		ok(!IS_NIL(vdump(builtin("+", op_add))), "builtin ops can be printed");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_readfile();
	test_read_badfile();
	test_readstring();

	test_print();

	done_testing();
	return 0;
}
