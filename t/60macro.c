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
test_backquote(void)
{
	//WITH_ABORT_PROTECTION {
	if (1) {
		ENV = globals();

		ok_eval("(equal `(x 2 e) '(x 2 e))", T, "backquote works");

		ok_eval("(do (set x 1) (set y 2) "
				"(equal `(x ,y) '(x 2)))", T,
				"backquote unquotes at commas");

		ok_eval("(do (set x 1) (set y 2) "
				"(equal `(x ,(+ x y) (+ x y)) "
					"(list 'x 3 (list '+ 'x 'y))))", T,
				"backquote+comma works on sublists");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_backquote();

	done_testing();
	return 0;
}
