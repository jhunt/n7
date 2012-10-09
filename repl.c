#include <stdio.h>
#include "core.h"

int main(int argc, char **argv)
{
	INIT();
	obj lst =
		cons(
			mkfixnum(1),
			cons(
				mkfixnum(2),
				cons(
					mkfixnum(3),
					cons(
						mkfixnum(4),
						nil))));
/*
	printf("===========================\n"); dump_gc();

	deref(lst);
	printf("===========================\n"); dump_gc();

	gc();
	printf("===========================\n"); dump_gc();
*/

	lst = nil; /* for valgrind */
	gc();
	return 0;
}
