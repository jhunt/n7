#include <stdio.h>
#include "core.h"

int main(int argc, char **argv)
{
	INIT();
	obj lst =
		cons(
			intern("A"),
			cons(
				intern("B"),
				cons(
					intern("C"),
					cons(
						intern("D"),
						NIL))));
	lst = NIL; /* for valgrind */
	return 0;
}
