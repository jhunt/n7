#include "assert.h"

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {

		ok(IS_NIL(eval(NIL)), "(eval nil) self-evaluates");
		ok(IS_T(eval(T)), "(eval t) self-evaluates");

		obj num = fixnum(42);
		ok(eval(num) == num, "(eval 42) self-evaluates");
	}

	done_testing();
	return 0;
}
