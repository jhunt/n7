#include "assert.h"

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


	char c;
	c = io_getc(io);
	ok(c == 's', "io_getc returned first character");
	c = io_getc(io);
	ok(c == 'o', "io_getc returned second character");

	io_rewind(io);
	c = io_getc(io);
	ok(c == 's', "io_getc (after rewind) returns first character");

	io_rewind(io);

	obj tmp = io_read_buf(io, 1024); /* arbitrary n */
	obj_equal(tmp, xyzzy, "read back what we wrote out");
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

	io_rewind(io);

	obj tmp = io_read_buf(io, 1024); /* arbitrary n */
	obj_equal(tmp, xyzzy, "read io string properly");

	io_rewind(io);

	obj overwrite = str_dupc("OVERWRITE!");
	io_write_str(io, overwrite);
	io_rewind(io);
	tmp = io_read_buf(io, 1024);
	obj_equal(tmp, overwrite, "read overwritten io string properly");
}

int main(int argc, char **argv)
{
	INIT();

	test_readfile();
	test_readstring();

	done_testing();
	return 0;
}
