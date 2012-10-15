#include "assert.h"

int main(int argc, char **argv)
{
	INIT();

	WITH_ABORT_PROTECTION {
		/* FIXME: we need to find symbols that hash the same... */
		ok(IS_NIL(intern(NULL)), "intern(NULL) is nil");

		sym_isnt("symbol-a", "symbol-b", "Symbols with different names are not equivalent");

		sym_is("sym", "sym", "Symbols only get interned once");
		sym_is("SYM", "sym", "Symbol names are case-insensitive");

		ok(IS_NIL(intern("nil")), "nil == nil");
		ok(IS_T(intern("t")), "t == t");
	}

	done_testing();
	return 0;
}
