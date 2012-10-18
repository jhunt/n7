#include "assert.h"

static void
test_readfile(void)
{
	obj xyzzy = vstring("something to write");
	obj io = iofile("t/io/writefile1", "w");

	ok(IS_IO(io), "iofile returns an io object (for reading)");
	ok(IS_T(iowriteb(io, xyzzy)), "wrote something to t/io/writefile1");
	ioclose(io);

	io = iofile("t/io/writefile1", "r");
	ok(IS_IO(io), "iofile returns an io object (for reading)");
	obj tmp = ioreadb(io, NIL);

	obj_equal(tmp, xyzzy, "read back what we wrote out");
}

static void
test_readstring(void)
{
	ok(1, "dummy test");
}

int main(int argc, char **argv)
{
	INIT();

	test_readfile();
	test_readstring();

	done_testing();
	return 0;
}
