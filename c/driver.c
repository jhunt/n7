#include <stdio.h>
#include "pipeline.h"

int main(int argc, char **argv)
{
	reader_t r;
	codepoint c;

	r = reader_new(fileno(stdin));
	fprintf(stdout, "READER_EOF: %x\n", READER_EOF);
	while ( (c = reader_next(r)) != READER_EOF ) {
		fprintf(stderr, "read %x\n", c);
	}
	fprintf(stderr, "EOF\n");
	return 0;
}
