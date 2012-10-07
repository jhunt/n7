#include <stdio.h>
#include "core.h"

int main(int argc, char **argv)
{
	VAL *lst =
		ref(cons(
			vfixnum(1),
			cons(
				vfixnum(2),
				cons(
					vfixnum(3),
					cons(
						vfixnum(4),
						vnil())))));
	printf("===========================\n"); dump_gc();

	deref(lst);
	printf("===========================\n"); dump_gc();

	gc();
	printf("===========================\n"); dump_gc();

	lst = NULL; /* for valgrind */
	return 0;
}
