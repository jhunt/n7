#include "assert.h"

static void
ok_eval(const char *code, obj result, obj env, const char *msg)
{
	obj io = io_string(code);
	obj form = readx(io);

	obj_equal(eval(form, env), result, msg);
}

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {

		obj env = globals();

		ok_eval("(+ 4 5)", fixnum(9), env, "(+ 4 5) -> 9");

		ok_eval("(+ 4 5 (* 9 2) (- 18) 1)", fixnum(10), env,
				"(+ 4 5 (* 9 2) (- 18) 1) -> 10");
	}

	done_testing();
	return 0;
}
