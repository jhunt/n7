#include "assert.h"

static void
ok_eval(const char *code, obj expect, obj env, const char *msg)
{
	obj io = io_string(code);
	obj form = readx(io);

	obj result = eval(form, env);
	if (IS_T(equal(result, expect))) {
		pass(msg);
		return;
	}

	fail(msg);
	     diag("");
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("     %s", cdump(form)));
	vdiag(str("       -> %s", cdump(result)));
	vdiag(str("  expected: %s", cdump(expect)));
	     diag("");
}

static void
test_math(void)
{
	WITH_ABORT_PROTECTION {
		obj env = globals();

		ok_eval("(+ 4 5)", fixnum(9), env, "(+ 4 5) -> 9");

		ok_eval("(+ 4 5 (* 9 2) (- 18) 1)", fixnum(10), env,
				"(+ 4 5 (* 9 2) (- 18) 1) -> 10");
	}
}

static void
test_special_ops(void)
{
	WITH_ABORT_PROTECTION {
		obj env = globals();

		ok_eval("(quote 1)", fixnum(1), env, "'1 -> 1");
		ok_eval("(quote (1 2 3))",
				cons(fixnum(1), cons(fixnum(2), cons(fixnum(3), NIL))),
				env,
				"'(1 2 3) -> (1 2 3)");


		ok_eval("(do 1)", fixnum(1), env, "(do 1) -> 1");
		ok_eval("(do 1 2 3)", fixnum(3), env, "(do 1 2 3) -> 3");
		ok_eval("(do t nil (+ 1 1))", fixnum(2), env, "(do t nil (+ 1 1)) -> 2");
	};
}

static void
test_equality(void)
{
	WITH_ABORT_PROTECTION {
		obj env = globals();

		ok_eval("(eq 'A 'A)", T,   env, "basic symbol equality");
		ok_eval("(eq 'A 'B)", NIL, env, "basic symbol inequality");

		ok_eval("(eql 1 1)", T,   env, "basic fixnum equality");
		ok_eval("(eql 2 4)", NIL, env, "basic fixnum inequality");

		ok_eval("(equal \"s1\" \"s1\")", T,   env, "basic string equality");
		ok_eval("(equal \"s1\" \"s2\")", NIL, env, "basic string inequality");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_math();
	test_equality();
	test_special_ops();

	done_testing();
	return 0;
}
