/*
  Copyright (c) 2012-2016 James Hunt

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software..

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
 */
#include "test.h"
#include <string.h>

static void
test_eval(void)
{
	WITH_ABORT_PROTECTION {

		obj env = env_init();

		ok(IS_NIL(eval(NIL, env)), "(eval nil) self-evaluates");
		ok(IS_T(eval(T, env)), "(eval t) self-evaluates");

		obj num = fixnum(42);
		ok(eval(num, env) == num, "(eval 42) self-evaluates");

		obj str = str_dupc("test");
		ok(eval(str, env) == str, "(eval \"test\") self-evaluates");

		obj sym = intern("x");
		setv(env, sym, fixnum(42));
		ok(IS_T(eql(eval(sym, env), fixnum(42))), "(eval x) causes symbol lookup");
	}
	on_abort(NULL);
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
		strncpy(PLUS->value.builtin.name, "+", 15);
		PLUS->value.builtin.fn = op_add;

		fixnum_is(op_call(cons(PLUS, args), env), 1111, "op_add via op_call");
	}

	jmp_buf comeback;
	on_abort(&comeback);

	if (setjmp(comeback) == 0) {
		ok(1, "test...");
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
