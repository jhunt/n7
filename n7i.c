#include <stdio.h>
#include "core.h"

int main(int argc, char **argv)
{
	INIT();

	obj in  = io_fdopen(stdin);
	obj out = io_fdopen(stdout);

	obj env = globals();

	fprintf(stdout, "\n   ..::' n7i '::..\n\n> ");

	while (!feof(stdin)) {
		printx(
			out,
			eval(
				readx(in),
				env));
		fprintf(stdout, "\n> ");
	}
	return 0;
}
