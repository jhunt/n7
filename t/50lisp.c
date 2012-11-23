#include "assert.h"

static obj ENV = NIL;

static void
ok_eval(const char *code, obj expect, const char *msg)
{
	obj io = io_string(code);
	obj form = readx(io);

	obj result = eval(form, ENV);
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
		ENV = globals();

		ok_eval("(+ 4 5)", fixnum(9), "(+ 4 5) -> 9");

		ok_eval("(+ 4 5 (* 9 2) (- 18) 1)", fixnum(10),
				"(+ 4 5 (* 9 2) (- 18) 1) -> 10");
	}
}

static void
test_special_ops(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();

		ok_eval("(quote nil)", NIL, "'nil -> nil");
		ok_eval("(quote)", NIL, "(quote) -> nil");
		ok_eval("(quote 1)", fixnum(1), "'1 -> 1");
		ok_eval("(quote (1 2 3))",
				nlist(3, fixnum(1), fixnum(2), fixnum(3)),
				"'(1 2 3) -> (1 2 3)");


		ok_eval("(do)", NIL, "(do) -> nil");
		ok_eval("(do 1)", fixnum(1), "(do 1) -> 1");
		ok_eval("(do 1 2 3)", fixnum(3), "(do 1 2 3) -> 3");
		ok_eval("(do t nil (+ 1 1))", fixnum(2), "(do t nil (+ 1 1)) -> 2");


		ok_eval("(if t 'then 'else)", intern("then"), "(if t 'then 'else) -> 'then");
		ok_eval("(if nil 'then 'else)", intern("else"), "(if nil 'then 'else) -> 'else");
		ok_eval("(if t 'then)", intern("then"), "(if t 'then) -> 'then");
		ok_eval("(if nil 'then)", NIL, "(if nil 'then) -> nil");
		ok_eval("(if t)", NIL, "(if t) -> nil");
		ok_eval("(if nil)", NIL, "(if nil) -> nil");
	}
}

static void
test_core_ops(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();

		obj x = intern("X");
		obj y = intern("Y");

		ok_eval("(cons 'x 'y)", cons(x,y), "(cons x y) works");
		ok_eval("(car (cons 'x 'y))", x, "(car (cons 'x 'y)) -> x");
		ok_eval("(cdr (cons 'x 'y))", y, "(cdr (cons 'x 'y)) -> y");
		ok_eval("(car nil)", NIL, "(car nil) -> nil");
		ok_eval("(cdr nil)", NIL, "(cdr nil) -> nil");

		ok_eval("(apply + '(1 2 3 4))", fixnum(10), "(apply '+ '(1 2 3 4)) -> 10");
		ok_eval("(call + 1 2 3 4)", fixnum(10), "(call '+ 1 2 3 4) -> 10");
	}
}

static void
test_equality(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();

		ok_eval("(eq 'A 'A)", T,   "basic symbol equality");
		ok_eval("(eq 'A 'B)", NIL, "basic symbol inequality");

		ok_eval("(eql 1 1)", T,   "basic fixnum equality");
		ok_eval("(eql 2 4)", NIL, "basic fixnum inequality");

		ok_eval("(equal \"s1\" \"s1\")", T,   "basic string equality");
		ok_eval("(equal \"s1\" \"s2\")", NIL, "basic string inequality");

		ok_eval("(equal '(1 2 3) '(1 2 3))", T,   "basic list equality");
		ok_eval("(equal '(1 2 3) '(3 2 1))", NIL, "basic list inequality");
		ok_eval("(equal '(1 2 3) '(1))", NIL, "short list inequality");
	}
}

static void
test_lambda(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();
		ok_eval("(eql 42 (call (lambda (x y z) (+ x y z)) 30 9 3))", T, "lambda def/call");
	}
}

static void
test_get_set(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();
		ok_eval("(eql 3 (do (set 'z 3) z))", T,
				"set/get works inside of language");

		ok_eval("(eql 8 (do (set 'x 1) (set 'y 6) (+ x y 1)))", T,
				"set/get works with ops");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_math();
	test_equality();
	test_special_ops();
	test_core_ops();
	test_lambda();
	test_get_set();

	done_testing();
	return 0;
}
