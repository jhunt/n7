#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "core.h"

static void
_abort(const char *m, const char *f, unsigned long lineno)
{
	fprintf(stderr, "ABORT @%s:%lu: %s\n", f, lineno, m);
	exit(42);
}
#define ABORT(m) _abort(m,__FILE__,__LINE__)

static obj
OBJECT(unsigned short type, size_t varlen)
{
	obj o = calloc(1,sizeof(bigobj)+varlen);
	if (!o) ABORT("malloc failed");
	o->type = type;
	return o;
}

static unsigned int
hash(const char *str, unsigned int lim)
{
	/* the positively stupidest hashing function you ever did meet */
	if (!str) return 0;
	return *str % lim;
}

static char*
lc(const char *s)
{
	char *new = strdup(s);
	char *p;
	for (p = new; *p; p++) {
		if (isupper(*p)) *p = tolower(*p);
	}
	return new;
}

/**************************************************/

#define SYMBOL_TABLE_SIZE 211
obj SYMBOL_TABLE[SYMBOL_TABLE_SIZE];

/**  Initialization  ********************************************/

void
INIT(void)
{
	unsigned int i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		SYMBOL_TABLE[i] = NIL;
	}
}

/**************************************************/

obj
cons(obj car, obj cdr)
{
	obj c = OBJECT(OBJ_CONS, 0);
	c->value.cons.car = car;
	c->value.cons.cdr = cdr;
	return c;
}

obj
car(obj cons)
{
	if (!cons) ABORT("car() called with NULL cons");
	if (!IS_CONS(cons)) ABORT("car() called with non-cons arg");

	return cons->value.cons.car;
}

obj
cdr(obj cons)
{
	if (!cons) ABORT("cdr() called with NULL cons");
	if (!IS_CONS(cons)) ABORT("cdr() called with non-cons arg");

	return cons->value.cons.cdr;
}

/**  Symbols  ***************************************************/

#define symname(cons) (const char*)untag((car(cons)))

static obj
findsym(unsigned int key, const char *name)
{
	obj rest;
	for_list(rest, SYMBOL_TABLE[key]) {
		if (strcasecmp(name, car(rest)->value.sym.name) == 0) {
			return car(rest);
		}
	}
	return NIL;
}

static obj
mksym(unsigned int key, const char *name)
{
	size_t len = strlen(name)+1;
	obj S = OBJECT(OBJ_SYMBOL, len);
	S->value.sym.len = len;
	strncpy(S->value.sym.name, name, len);

	push(SYMBOL_TABLE[key], S);
	return S;
}

obj
intern(const char *name)
{
	char *symname = lc(name);
	unsigned int key = hash(symname, SYMBOL_TABLE_SIZE);
	obj sym = findsym(key, symname);

	if (IS_NIL(sym)) {
		sym = mksym(key, symname);
	}

	free(symname);
	return sym;
}

/**  Debugging Functions  ***************************************/

void
dump_obj(obj o)
{
	fprintf(stderr, "obj:[%#08lx:%#01lx]\n",
			hitag(o), lotag(o));
}

void
dump_sym(obj sym)
{
	fprintf(stderr, "sym:[%#08lx:%#01lx >> %s]\n",
			hitag(sym), lotag(sym), sym->value.sym.name);
}

void
dump_syms(void)
{
	obj rest;
	size_t i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		if (!IS_NIL(SYMBOL_TABLE[i])) {
			fprintf(stderr, "SYMBOL_TABLE[%lu] = \n", (unsigned long)i);
			for_list(rest, SYMBOL_TABLE[i]) {
				dump_obj(car(rest));
				dump_sym(car(rest));
			}
		}
	}
	fprintf(stderr, "-----------------\n");
}
