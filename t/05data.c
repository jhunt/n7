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

		e = set(e,x,str_dupc("hi"));
		obj_equal(get(e,x), str_dupc("hi"), "x is now set to the string 'hi'");
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

	/* car(T) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		car(T);
		diag("(car T) did not abort");
		fail("(car T) should abort");
	} else {
		pass("(car T) should abort");
	}

	/* cdr(T) */
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);

		cdr(T);
		diag("(cdr T) did not abort");
		fail("(cdr T) should abort");
	} else {
		pass("(cdr T) should abort");
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

#define ASCII " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

static void
test_str_dups(void)
{
	ok(IS_NIL(str_dupc(NULL)),         "str_dupc(NULL) == NIL");
	ok(IS_NIL(str_dup(fixnum(8))),     "str_dup(8) == NIL");
	ok(IS_NIL(str_dup(intern("sym"))), "str_dup('sym) == NIL");

	ok(cstr(NIL)           == NULL, "cstr(NIL) == NULL");
	ok(cstr(fixnum(42))    == NULL, "cstr(42) == NULL");
	ok(cstr(intern("sym")) == NULL, "cstr('sym) == NULL");

	obj x = str_dupc(ASCII);
	obj y = str_dupc(ASCII);
	obj_equal(x,y, "str_dupc(cstr) == str_dupc(cstr)");
	ok(cstr(x) != NULL, "cstr returns non-NULL pointer");
	if (cstr(x) == NULL) bail("cstr(x) returned NULL! ALL BETS ARE OFF!!");
	ok(strcmp(cstr(x), ASCII) == 0, "cstr retrieves raw C-string");

	obj z = str_dup(x);
	obj_equal(x,z, "str_dup(obj) == str_dup(obj)");

	ok(IS_NIL(str_dupb(NULL, 0)), "str_dupb handles NULL buffer");

	const char *buf = "hello, world";
	x = str_dupb(buf, 5);
	y = str_dupc("hello");
	obj_equal(x,y, "str_dupb(...) handles substring properly");

	x = str_dupb(buf, 0);
	y = str_dupc("");
	obj_equal(x,y, "str_dupb handles empty string");

	x = str_dupf("%04x, %s", 48879, "it's what's for dinner");
	y = str_dupc("beef, it's what's for dinner");
	obj_equal(x,y, "str_dupf formats nicely");
}

static void
test_str_cats(void)
{
	obj hello = str_dupc("Hello, ");
	obj world = str_dupc("World!");
	obj combined = NIL;

	/** str_cat **/

	ok(IS_NIL(str_cat(NIL,       NIL)),           "str_cat(nil,nil) == NIL");
	ok(IS_NIL(str_cat(fixnum(4), fixnum(2))),     "str_cat(4,2) == NIL");
	ok(IS_NIL(str_cat(hello,     NIL)),           "str_cat(str,NIL) == NIL");
	ok(IS_NIL(str_cat(hello,     intern("sym"))), "str_cat(str,'sym) == NIL");

	combined = str_cat(hello, world);
	ok(combined != hello, "str_cat returns new pointer (not arg1)");
	ok(combined != world, "str_cat returns new pointer (not arg2)");
	obj_equal(combined, str_dupc("Hello, World!"), "str_cat works with string objects");

	/** str_catc **/

	ok(IS_NIL(str_catc(NIL, "...")), "str_cat(NIL, \"...\") == NIL");
	ok(IS_NIL(str_catc(intern("hello"), "...")), "str_cat('hello, \"...\") == NIL");

	combined = str_catc(hello, "Testing...");
	ok(combined != hello, "str_catc returns new pointer (not arg1)");
	obj_equal(combined, str_dupc("Hello, Testing..."), "str_catc works with C-strings");

	combined = str_catc(hello, "");
	obj_equal(combined, str_dupc("Hello, "), "str_catc works with an empty C-string");

	combined = str_catc(hello, NULL);
	obj_equal(combined, str_dupc("Hello, "), "str_catc treats NULL C-string as \"\"");

	/** str_catb **/

	const char *buf = "red blue green";
	obj start = str_dupc("the sky is ");

	ok(IS_NIL(str_catb(NIL, buf, 5)), "str_catb with non-string dst returns NIL");

	combined = str_catb(start, buf, 3);
	ok(combined != start, "str_catb returns new pointer (not arg1)");
	obj_equal(combined, str_dupc("the sky is red"), "str_catb can extract from a buffer");

	combined = str_catb(start, buf+3+1, 4);
	obj_equal(combined, str_dupc("the sky is blue"), "str_catb can extract from the middle of a buffer");

	combined = str_catb(start, buf, 0);
	obj_equal(combined, start, "str_catb with zero-length appends empty string");

	/** str_catf **/

	ok(IS_NIL(str_catf(fixnum(9), "%d", 99)), "str_catf with non-string dst 9 returns NIL");
	ok(IS_NIL(str_catf(T, "%s", "true")),     "str_catf with non-string dst T returns NIL");

	combined = str_catf(start, "%s, %d", buf+3+1+4+1, 42);
	ok(combined != start, "str_catf returns new pointer (not arg1)");
	obj_equal(combined, str_dupc("the sky is green, 42"), "str_catf works");
}

static void
test_str_utils(void)
{
	obj hello = str_dupc("Hello, ");

	strx(hello, "World");
	obj_equal(hello, str_dupc("Hello, World"), "strx concatenates in-place");

	strc(hello, '!');
	obj_equal(hello, str_dupc("Hello, World!"), "strc appends a single character");

	strf(hello, " -- %lu", 1337);
	obj_equal(hello, str_dupc("Hello, World! -- 1337"), "strf appends with style");

	strx(NIL, "test");
	pass("strx(NIL,str) does not crash");

	strx(fixnum(6), "test");
	pass("strx(6,str) does not crash");

	strc(NIL, '?');
	pass("strc(NIL,c) does not crash");

	strc(intern("SYM"), '%');
	pass("strc('SYM,c) does not crash");

	strf(NIL, "%s", "never fails");
	pass("strf(NIL, ...) does not crash");

	strf(fixnum(67), ".%d", 89);
	pass("strf(67, ...) does not crash");
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

	test_str_dups();
	test_str_cats();
	test_str_utils();

	done_testing();
	return 0;
}
