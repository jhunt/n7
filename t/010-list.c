#include "assert.h"

static inline void test_car_cdr(void)
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

static inline void test_abort(void)
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

int main(int argc, char **argv)
{
	INIT();

	test_car_cdr();
	test_abort();

	done_testing();
	return 0;
}
