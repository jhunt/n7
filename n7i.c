#include <stdio.h>
#include "core.h"
#include <unistd.h>

#define print(out,x) io_print(out,vdump(x))
#define PROMPT() printf("\n> ")

int main(int argc, char **argv)
{
	int dbg = 0;
	char opt;
	while ((opt = getopt(argc, argv, "d")) != -1) {
		switch (opt) {
		case 'd':
			dbg++;
			if (dbg > 3) { dbg = 3; }
			break;
		default:
			fprintf(stderr, "Usage: %s [-d[+]] < script\n", argv[0]);
			exit(1);
		}
	}

	debugging(dbg);
	INIT();

	obj in  = io_fdopen(stdin);
	obj out = io_fdopen(stdout);
	obj env = globals();

	fprintf(stdout, "\n   ..::' n7i '::..\n");

	jmp_buf err;
	if (setjmp(err) != 0) {
		printf("aborted.  retrying...\n");
	} else {
		on_abort(&err);
	}

	PROMPT();
	while (!feof(stdin)) {
		print(
			out,
			eval(
				readx(in),
				env));
		PROMPT();
	}
	return 0;
}
