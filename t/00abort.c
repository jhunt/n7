#include "test.h"

int main(int argc, char **argv)
{
	INIT();

	jmp_buf comeback;
	if (setjmp(comeback) == 0) {
		on_abort(&comeback);
		abort("just testing");
		diag("abort fell through");
		fail("abort/jmp");
	} else {
		pass("abort/jmp");
	}

	done_testing();
	return 0;
}
