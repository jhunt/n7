#include "assert.h"

/***  SYMBOLS  *********************************************/

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
test_get_set(void)
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
		obj_equal(get(e,x), fixnum(42), "x is set to 42");

		e = set(e,y,get(e,x)); /* (set y x) */
		is_defined(get(e,x), "x persists");
		is_defined(get(e,y), "y is set now");
		obj_equal(get(e,y), fixnum(42), "y is also set to 42");

		e = set(e,x,vstring("hi"));
		obj_equal(get(e,x), vstring("hi"), "x is now set to the string 'hi'");
		obj_equal(get(e,y), fixnum(42), "y is still set to 42, unaffected by (set x \"hi\")");
	}
}


/***  LISTS  ***********************************************/

static inline void
test_car_cdr(void)
{
	WITH_ABORT_PROTECTION {
		obj a = intern("a");
		obj b = intern("b");
		obj C = cons(a, b);

		ok(car(C) == a, "car(C) == a");
		ok(cdr(C) == b, "cdr(C) == b");

		ok(IS_NIL(car(NIL)), "car(NIL) is NIL");
		ok(IS_NIL(cdr(NIL)), "cdr(NIL) is NIL");
	}
}

static inline void
test_list_op_aborts(void)
{
	jmp_buf comeback;
	obj sym = intern("SYM");

	/* car(NULL) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		car(NULL);
		diag("car(NULL) did not abort");
		fail("car(NULL) should abort");
	} else {
		pass("car(NULL) should abort");
	}

	/* cdr(NULL) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		cdr(NULL);
		diag("cdr(NULL) did not abort");
		fail("cdr(NULL) should abort");
	} else {
		pass("cdr(NULL) should abort");
	}

	/* car('SYM) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		car(sym);
		diag("(car 'SYM) did not abort");
		fail("(car 'SYM) should abort");
	} else {
		pass("(car 'SYM) should abort");
	}

	/* cdr('SYM) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		cdr(sym);
		diag("(cdr 'SYM) did not abort");
		fail("(cdr 'SYM) should abort");
	} else {
		pass("(cdr 'SYM) should abort");
	}
}

static inline void
test_nlist_helpers(void)
{
	WITH_ABORT_PROTECTION {
		obj a = intern("a");
		obj b = intern("b");
		obj c = intern("c");

		obj l = nlist(3, a, b, c);
		ok(IS_CONS(l), "nlist returns a cons object");
		ok(car(l)                == a, "l[0] == a");
		ok(car(cdr(l))           == b, "l[1] == b");
		ok(car(cdr(cdr(l)))      == c, "l[2] == c");
		ok(IS_NIL(cdr(cdr(cdr(l)))), "l[3] is NIL");

		l = revl(l);
		ok(car(l)                == c, "l[0] == b (revl)");
		ok(car(cdr(l))           == b, "l[1] == b (revl)");
		ok(car(cdr(cdr(l)))      == a, "l[2] == a (revl)");
		ok(IS_NIL(cdr(cdr(cdr(l)))), "l[3] is NIL");
	}
}

/***  STRINGS  *********************************************/

static void
test_vstrings(void)
{
	obj s = vstring("test string");
	vstring_is(s, "test string", "Basic string creation");

	obj hello = vstring("Hello, ");
	obj world = vstring("World!");
	obj new = vstrcat(hello, world);

	ok(new != hello, "vstrcat does not reuse first arg");
	ok(new != world, "vstrcat does not reuse second arg");

	vstring_is(hello, "Hello, ", "vstrcat does not modify first arg");
	vstring_is(world, "World!",  "vstrcat does not modify second arg");
	vstring_is(new, "Hello, World!", "vstrcat cats strs yay!");

	new = vextend(hello, "Test!  this is ignored", 5);
	ok(new == hello, "vextend reuses first arg");
	vstring_is(hello, "Hello, Test!", "vextend strcats in-place");

	vextendc(hello, '!');
	vstring_is(hello, "Hello, Test!!", "vextendc works (albeit slowly)");
}

/***********************************************************/

int main(int argc, char **argv)
{
	INIT();

	test_symbols();
	test_get_set();

	test_car_cdr();
	test_list_op_aborts();
	test_nlist_helpers();

	test_vstrings();

	done_testing();
	return 0;
}
