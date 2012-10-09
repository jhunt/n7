#include "core.h"

int main(int argc, char **argv)
{
	INIT();
	dump_syms();

	intern("A");
	intern("B");

	dump_syms();

	intern("a");
	dump_syms();
	return 0;
}
