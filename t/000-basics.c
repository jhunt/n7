#include "assert.h"

static void is_eq(obj a, obj b, const char *msg)
{
	ok(IS_T(eq(a, b)), msg);
}

static void isnt_eq(obj a, obj b, const char *msg)
{
	ok(IS_NIL(eq(a, b)), msg);
}

static void
test_hashing(void)
{
	WITH_ABORT_PROTECTION {
		const char *str1 = "this is a string";
		ok(hash(str1, 4242) == hash(str1, 4242), "hash identity: hash of identical strings are identical");
		ok(hash(NULL, 4242) == 0, "hash(NULL) == 0");
	}
}

#define TEST_EQ(op,test,res,msg) do {\
	msg = str("(%s %s %s) should be %s", #op, (test).name_a, (test).name_b, #res); \
	ok(IS_ ## res (op((test).a,(test).b)), msg); free(msg); \
} while (0)

static void
test_equality(void)
{
	WITH_ABORT_PROTECTION {

		obj A = intern("A");
		obj B = intern("B");

		obj n3 = fixnum(3);
		obj n3x = fixnum(3); /* different pointer... */
		obj n4 = fixnum(4);

		obj c1 = cons(A, B);
		obj c2 = cons(A, B);

		struct {
			obj a; const char *name_a;
			obj b; const char *name_b;
			int is_eq;
			int is_eql;
			int is_equal;
		} tests[] = {
			/* constant equality */
			{ NIL, "NIL",     NIL, "NIL",      1, 1, 1 },
			{ T,   "T",       T,   "T",        1, 1, 1 },
			/* symbolic equality */
			{ A,   "'A",      A,   "'A",       1, 1, 1 },
			{ A,   "'A",      B,   "'B",       0, 0, 0 },
			/* numeric equality */
			{ n3,  "3",       n3,  "3",        1, 1, 1 },
			{ n3,  "3",       n3x, "3'",       0, 1, 1 },
			{ n3,  "3",       n4,  "4",        0, 0, 0 },
			/* cons equality */
			{ c1,  "(A . B)", c1, "(A . B)",   1, 1, 1 },
			{ c1,  "(A . B)", c2, "(A . B)'",  0, 1, 1 },

			{ 0, 0, 0, 0, 0, 0, 0 }
		};

		char *msg;
		size_t i = 0;
		for (i = 0; tests[i].a; i++) {
			if (tests[i].is_eq == 1) {
				TEST_EQ(eq, tests[i], T, msg);
			} else {
				TEST_EQ(eq, tests[i], NIL, msg);
			}

			if (tests[i].is_eql == 1) {
				TEST_EQ(eql, tests[i], T, msg);
			} else {
				TEST_EQ(eql, tests[i], NIL, msg);
			}

			if (tests[i].is_equal == 1) {
				TEST_EQ(equal, tests[i], T, msg);
			} else {
				TEST_EQ(equal, tests[i], NIL, msg);
			}
		}
	}
}

static void
test_type_inequality(void)
{
	WITH_ABORT_PROTECTION {

		obj A = intern("A");
		obj B = intern("B");

		struct {
			obj value;
			const char *name;
		} zoo[] = {
			{ T,             "T"       },
			{ NIL,           "NIL"     },
			{ intern("sym"), "'sym"    },
			{ fixnum(42),    "42"      },
			{ cons(A, B),    "(A . B)" },
			{ 0, 0 }
		};

		char *msg;
		size_t i, j;
		for (i = 0; zoo[i].value; i++) {
			for (j = i+1; zoo[j].value; j++) {
				msg = str("(eq %s %s) should be NIL (cross-type)",
						zoo[i].name,
						zoo[j].name);
				ok(IS_NIL(eq(zoo[i].value, zoo[j].value)), msg);
				free(msg);
			}
		}
	}
}

int main(int argc, char **argv)
{
	INIT();

	test_hashing();
	test_equality();
	test_type_inequality();

	done_testing();
	return 0;
}
