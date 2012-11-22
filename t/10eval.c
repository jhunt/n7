#include "assert.h"

static void
test_eval(void)
{
	WITH_ABORT_PROTECTION {

		obj env = NIL;

		ok(IS_NIL(eval(NIL, env)), "(eval nil) self-evaluates");
		ok(IS_T(eval(T, env)), "(eval t) self-evaluates");

		obj num = fixnum(42);
		ok(eval(num, env) == num, "(eval 42) self-evaluates");

		obj str = str_dupc("test");
		ok(eval(str, env) == str, "(eval \"test\") self-evaluates");

		obj sym = intern("x");
		env = set(env, sym, fixnum(42));
		ok(IS_T(eql(eval(sym, env), fixnum(42))), "(eval x) causes symbol lookup");
	}
}

static void
test_apply(void)
{
	obj PLUS;
	obj args = nlist(4, fixnum(1), fixnum(10), fixnum(100), fixnum(1000));
	obj env = globals();

	WITH_ABORT_PROTECTION {
		fixnum_is(op_add(args, env), 1111, "op_add (direct call)");

		PLUS = calloc(1, sizeof(bigobj));
		PLUS->type = OBJ_BUILTIN;
		PLUS->value.builtin = op_add;

		fixnum_is(op_call(cons(PLUS, args), env), 1111, "op_add via op_call");
	}

	jmp_buf comeback;
	on_abort(&comeback);

	if (setjmp(comeback) == 0) {
		op_call(NULL, env);
		fail("op_call(NULL) should abort");
		diag("op_call(NULL) did not abort");
	} else {
		pass("op_call(NULL) should abort");
	}

	if (setjmp(comeback) == 0) {
		op_call(args, env);
		fail("op_call( ... not an fn ... ) should abort");
		diag("op_call() did not abort when first arg was not a fn");
	} else {
		pass("op_call( ... not an fn ... ) should abort");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_eval();
	test_apply();

	done_testing();
	return 0;
}
