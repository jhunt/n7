#include "assert.h"

static void
test_symbols(void)
{
	WITH_ABORT_PROTECTION {
		/* FIXME: we need to find symbols that hash the same... */
		ok(IS_NIL(intern(NULL)), "intern(NULL) is nil");

		sym_isnt("symbol-a", "symbol-b", "Symbols with different names are not equivalent");

		sym_is("sym", "sym", "Symbols only get interned once");
		sym_is("SYM", "sym", "Symbol names are case-insensitive");

		ok(IS_NIL(intern("nil")), "nil == nil");
		ok(IS_T(intern("t")), "t == t");
	}
}

static void
obj_eql(obj got, obj exp, const char *msg)
{
	if (IS_T(eql(got, exp))) {
		pass(msg);
		return;
	}

	fail(msg);
	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(got)));
	vdiag(str("  expected: %s", cdump(exp)));
}

static void
is_defined(obj x, const char *msg)
{
	if (DEF(x)) {
		pass(msg);
		return;
	}

	vdiag(str("  Failed test '%s'", msg));
	     diag("       got: <undefined>");
	     diag("  expected: anything else...");
}

static void
isnt_defined(obj x, const char *msg)
{
	if (!DEF(x)) {
		pass(msg);
		return;
	}

	vdiag(str("  Failed test '%s'", msg));
	vdiag(str("       got: %s", cdump(x)));
	 diag("  expected: <undefined>");
}

static void
test_get(void)
{
	WITH_ABORT_PROTECTION {
		obj x = intern("x");
		obj y = intern("y");

		obj e = NIL;
		isnt_defined(get(e,x), "e[x] is undefined");
		isnt_defined(get(e,y), "e[y] is undefined");

		e = set(e,x,fixnum(42));
		is_defined(get(e,x), "e[x] is defined");
		isnt_defined(get(e,y), "e[y] is not defined");
		obj_eql(get(e,x), fixnum(42), "x is set to 42");

		e = set(e,y,get(e,x)); /* (set y x) */
		is_defined(get(e,x), "x persists");
		is_defined(get(e,y), "y is set now");
		obj_eql(get(e,y), fixnum(42), "y is also set to 42");

		e = set(e,x,vstring("hi"));
		obj_eql(get(e,x), vstring("hi"), "x is now set to the string 'hi'");
		obj_eql(get(e,y), fixnum(42), "y is still set to 42, unaffected by (set x \"hi\")");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_symbols();
	test_get();

	done_testing();
	return 0;
}
