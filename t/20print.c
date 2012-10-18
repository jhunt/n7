#include <unistd.h>
#include "assert.h"

static void
print_ok(obj x, const char *expect, const char *msg)
{
	char *s, file[] = "/tmp/n7.test.XXXXXXXX";
	int tmpfd = mkstemp(file);

	if (tmpfd < 0) {
		fail(msg);
		vdiag(str("%s: mkstemp call failed"));
		return;
	}

	FILE *fd = fdopen(tmpfd, "w+");

	s = str("%s: printed to temp file", msg);
	ok(IS_T(printx(fd, x)), s);
	free(s);

	fseek(fd, 0, SEEK_SET);

	char buf[8192];
	fgets(buf, 8191, fd);

	if (strcmp(buf, expect) == 0) {
		pass(msg);
	} else {
		fail(msg);
		vdiag(str("  Failed test '%s'", msg));
		vdiag(str("        got: '%s'",  buf));
		vdiag(str("   expected: '%s'",  expect));
	}

	fclose(fd);
}

static void
test_print(void)
{
	WITH_ABORT_PROTECTION {
		print_ok(T, "t", "T prints ok");
		print_ok(NIL, "nil", "NIL prints ok");
		print_ok(fixnum(42), "42", "fixnum prints ok");

		obj X = intern("X");
		obj Y = intern("Y");
		print_ok(cons(X,Y),   "(x . y)", "CONS prints ok");
		print_ok(cons(X,NIL), "(x)", "list prints ok");
		print_ok(cons(X,cons(Y,NIL)), "(x y)", "list of 2 prints ok");
		print_ok(cons(X,cons(Y,cons(X,NIL))), "(x y x)", "list of 3 prints ok");

		obj str = str_dupc("kill troll with axe");
		print_ok(str, "\"kill troll with axe\"", "string prints ok");

		obj A = intern("A");
		obj B = intern("B");
		obj tree = cons(
			cons(A, B),
			cons(X, Y));
		/* odd, but correct... */
		print_ok(tree, "((a . b) x . y)", "simple tree prints ok");
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_print();

	done_testing();
	return 0;
}
