#include "assert.h"

static obj ENV = NIL;

static void
ok_eval(const char *code, obj expect, const char *msg)
{
	if (!msg) {
		msg = str("%s -> %s", code, cdump(expect));;
	}

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
test_eval(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();

		ok_eval("T", T, "self-eval T");
		ok_eval("NIL", NIL, "self-eval NIL");
		ok_eval("1", fixnum(1), "self-eval literals");

		ok_eval("(eval 1)", fixnum(1), "eval can eval an eval!");
	}
}

static void
test_logic(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();

		ok_eval("(and)", T, NULL);
		ok_eval("(and T)", T, NULL);
		ok_eval("(and 1)", T, NULL);
		ok_eval("(and 0)", T, NULL);
		ok_eval("(and 0 1 2)", T, NULL);
		ok_eval("(and NIL)", NIL, NULL);
		ok_eval("(and T NIL)", NIL, NULL);
		ok_eval("(and 1 (and 2 3) (and 4 (and)))", T, "nested (and)s");

		ok_eval("(or)", NIL, NULL);
		ok_eval("(or T T)", T, NULL);
		ok_eval("(or T NIL)", T, NULL);
		ok_eval("(or NIL T)", T, NULL);
		ok_eval("(or (or 1 2) (or NIL NIL NIL (or NIL T)))", T, "nested (or)s");

		ok_eval("(and (or NIL T) (and T NIL))", NIL, "nested (and)/(or)s");
	}
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
		ok_eval("(eql 3 (do (set z 3) z))", T,
				"set/get works inside of language");

		ok_eval("(eql 8 (do (set x 1) (set y 6) (+ x y 1)))", T,
				"set/get works with ops");
	}
}

static void
test_combinators(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();
		ok_eval("(K t)", T, NULL);
		ok_eval("(K nil)", NIL, NULL);
		ok_eval("(K 42)", fixnum(42), NULL);
	}
}

static void
test_predicates(void)
{
	WITH_ABORT_PROTECTION {
		ENV = globals();
		ok_eval("(ne 'x 'y)", T, NULL);
		ok_eval("(ne 'x 'x)", NIL, NULL);
		ok_eval("(not (eq 'x 'x))", NIL, NULL);
		ok_eval("(eq 'x 'x)", T, NULL);
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_eval();
	test_logic();

	test_math();
	test_equality();
	test_special_ops();
	test_core_ops();
	test_lambda();
	test_get_set();

	test_combinators();
	test_predicates();

	done_testing();
	return 0;
}
