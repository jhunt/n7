#include "assert.h"

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {
		const char *str1 = "this is a string";
		ok(hash(str1, 4242) == hash(str1, 4242), "hash identity: hash of identical strings are identical");

		ok(hash(NULL, 4242) == 0, "hash(NULL) == 0");
	}

	done_testing();
	return 0;
}
