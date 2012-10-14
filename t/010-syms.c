#include "assert.h"
#include "../core.h"

int main(int argc, char **argv)
{
	INIT();

	//printf("1..N # basic intern tests\n");
	sym_is("sym", "sym", "Symbols only get interned once");
	sym_is("SYM", "sym", "Symbol names are case-insensitive");

	done_testing();
	return 0;
}
