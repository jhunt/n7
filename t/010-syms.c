#include "test.h"
#include "../core.h"

int main(int argc, char **argv)
{
	INIT();

	diag("Basic Intern Tests");
	obj a, b;
	a = intern("SYM");
	b = intern("sym");
	if (a == b) {
		pass("SYM == sym (case-insensitive intern)");
	} else {
		fail("SYM != sym (case-insensitive intern)");
	}

	done_testing();
	return 0;
}
