#include "core.h"

int main(int argc, char **argv)
{
	VAL *lst = cons(
			vfixnum(1),
			cons(
				vfixnum(2),
				cons(
					vfixnum(3),
					cons(
						vfixnum(4),
						vnil()))));

	return 0;
}
