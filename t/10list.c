#include "assert.h"

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
test_abort(void)
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

int main(int argc, char **argv)
{
	INIT();

	test_car_cdr();
	test_abort();
	test_nlist_helpers();

	done_testing();
	return 0;
}
