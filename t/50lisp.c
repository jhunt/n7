#include "assert.h"

static void
ok_eval(const char *code, obj result, const char *msg)
{
	/* need a function for reading expressions from a string */
}

int main(int argc, char **argv)
{
	WITH_ABORT_PROTECTION {
		ok_eval("(+ 4 5)", fixnum(9), "(+ 4 5) -> 9");
	}

	done_testing();
	return 0;
}
