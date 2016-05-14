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
	int handler = 1;
	char opt;
	while ((opt = getopt(argc, argv, "dx")) != -1) {
		switch (opt) {
		case 'd':
			dbg++;
			if (dbg > 3) { dbg = 3; }
			break;
		case 'x':
			handler = 0;
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
		if (handler) {
			if (setjmp(err) != 0) {
				printf("aborted.  retrying...\n");
			} else {
				on_abort(&err);
			}
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
