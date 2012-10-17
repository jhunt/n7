#include "assert.h"

#define N(n) fixnum(n)

int main(int argc, char **argv)
{
	INIT();

	obj PLUS;
	obj args = nlist(4, N(1), N(10), N(100), N(1000));

	WITH_ABORT_PROTECTION {
		fixnum_is(op_add(args), 1111, "op_add (direct call)");

		PLUS = calloc(1, sizeof(bigobj));
		PLUS->type = OBJ_BUILTIN;
		PLUS->value.builtin = op_add;

		fixnum_is(op_apply(PLUS, args), 1111, "op_add via op_apply");
	}

	jmp_buf comeback;
	on_abort(&comeback);

	if (setjmp(comeback) == 0) {
		op_apply(NULL, args);
		fail("op_apply(NULL) should abort");
		diag("op_apply(NULL) did not abort");
	} else {
		pass("op_apply(NULL) should abort");
	}

	if (setjmp(comeback) == 0) {
		op_apply(args, NIL);
		fail("op_apply( ... not an fn ... ) should abort");
		diag("op_apply() did not abort when first arg was not a fn");
	} else {
		pass("op_apply( ... not an fn ... ) should abort");
	}

	done_testing();
	return 0;
};
