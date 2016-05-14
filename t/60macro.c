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

static obj ENV = NIL;

static void
ok_eval(const char *code, obj expect, const char *msg)
{
	obj io = io_string(code);
	obj form = readx(io, ENV);

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
