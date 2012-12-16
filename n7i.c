#include <stdio.h>
#include "core.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>

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

	int INTERACTIVE = 0;
	obj in, out, env;
	if (optind < argc) {
		debug1("attempting to load %s\n", argv[optind]);
		in = io_fopen(argv[optind], "r");
		if (IS_NIL(in)) {
			fprintf(stderr, "%s: %s\n", argv[optind], strerror(errno));
			exit(1);
		}
		INTERACTIVE = 0;

	} else {
		INTERACTIVE = 1;
		in  = io_fdopen(stdin);
		fprintf(stdout, "\n   ..::' n7i '::..\n");
	}
	out = io_fdopen(stdout);
	env = globals();

	jmp_buf err;
	obj result;

	if (INTERACTIVE) {
		if (setjmp(err) != 0) {
			printf("aborted.  retrying...\n");
		} else {
			on_abort(&err);
		}

		PROMPT();
	}
	while (IS_NIL(io_eof(in))) {
		result = eval(readx(in, env), env);
		if (INTERACTIVE) {
			print(out, result);
			PROMPT();
		}
	}
	return 0;
}
