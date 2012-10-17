#include "assert.h"

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {

		obj env = NIL;

		ok(IS_NIL(eval(NIL, env)), "(eval nil) self-evaluates");
		ok(IS_T(eval(T, env)), "(eval t) self-evaluates");

		obj num = fixnum(42);
		ok(eval(num, env) == num, "(eval 42) self-evaluates");

		obj sym = intern("x");
		env = set(env, sym, fixnum(42));
		ok(IS_T(eql(eval(sym, env), fixnum(42))), "(eval x) causes symbol lookup");
	}

	done_testing();
	return 0;
}
