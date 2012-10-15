#include "assert.h"

#define N(n) fixnum(n)

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {
		fixnum_is(fixnum(5), 5, "fixnum value");

		fixnum_is(op_add(nlist(0)), 0, "(+) == 0");
		fixnum_is(op_add(nlist(1, N(0))), 0, "(+ 0) == 0");
		fixnum_is(op_add(nlist(5, N(0), N(0), N(0), N(0), N(0))), 0, "(+ 0 0 0 0 0) == 0");
		fixnum_is(op_add(nlist(1, N(45))), 45, "(+ 45) == 45");
		fixnum_is(op_add(nlist(2, N(1), N(1))), 2, "(+ 1 1) == 2");
		fixnum_is(op_add(nlist(3, N(1), N(2), N(3))), 6, "(+ 1 2 3) == 6");

		fixnum_is(op_sub(nlist(0)), 0, "(-) == 0");
		fixnum_is(op_sub(nlist(1, N(0))), 0, "(- 0) == 0");
		fixnum_is(op_sub(nlist(5, N(0), N(0), N(0), N(0), N(0))), 0, "(- 0 0 0 0 0) == 0");
		fixnum_is(op_sub(nlist(1, N(23))), -23, "(- 23) == -23");
		fixnum_is(op_sub(nlist(2, N(6), N(4))), 2, "(- 6 4) == 2");
		fixnum_is(op_sub(nlist(3, N(42), N(10), N(2))), 30, "(- 42 10 2) == 30");

		fixnum_is(op_mult(nlist(0)), 1, "(*) == 1");
		fixnum_is(op_mult(nlist(1, N(1))), 1, "(* 1) == 1");
		fixnum_is(op_mult(nlist(5, N(1), N(1), N(1), N(1), N(1))), 1, "(* 1 1 1 1 1) == 1");
		fixnum_is(op_mult(nlist(1, N(4))), 4, "(* 4) == 4");
		fixnum_is(op_mult(nlist(2, N(4), N(3))), 12, "(* 4 3) == 12");
		fixnum_is(op_mult(nlist(3, N(4), N(3), N(2))), 24, "(* 4 3 2) == 24");
		fixnum_is(op_mult(nlist(2, N(1), N(-2))), -2, "(* 1 (- 2)) == -2");
		fixnum_is(op_mult(nlist(1, N(0))), 0, "(* 0) == 0");
		fixnum_is(op_mult(nlist(3, N(4), N(0), N(2))), 0, "(* 4 0 2) == 0");

		/* FIXME: need ratios to do the '/' operator any testing justice... */
		fixnum_is(op_div(nlist(2, N(8), N(4))), 2, "(/ 8 4) == 2");
		fixnum_is(op_div(nlist(2, N(16), N(1))), 16, "(/ 16 1) == 16");
		fixnum_is(op_div(nlist(3, N(16), N(2), N(2))), 4, "(/ 16 2 2) == 4");
	}

	jmp_buf comeback;
	on_abort(&comeback);

	if (setjmp(comeback) == 0) {
		op_div(nlist(0));
		diag("(/) did not abort");
		fail("(/) should abort");
	} else {
		pass("(/) should abort");
	}

	if(setjmp(comeback) == 0) {
		op_div(nlist(1, N(4)));
		diag("(/ 4) should abort # TODO");
		pass("(/ 4) should abort # TODO");
	} else {
		fail("(/ 4) should abort # TODO should not abort");
	}

	done_testing();
	return 0;
}
