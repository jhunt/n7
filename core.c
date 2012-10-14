#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>

#include "core.h"

/* abort-specific implementation details */
static jmp_buf *ABORT_JMP = NULL;

static obj
OBJECT(unsigned short type, size_t varlen)
{
	obj o = calloc(1,sizeof(bigobj)+varlen);
	if (!o) abort("malloc failed");
	o->type = type;
	return o;
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

/**************************************************/

void
_abort(const char *file, unsigned int line, const char *msg)
{
	fprintf(stderr, "ABORT @%s:%u: %s\n", file, line, msg);

	/* LCOV_EXCL_START */
	if (ABORT_JMP) {
		longjmp(*ABORT_JMP, 42);
	} else {
		fprintf(stderr, "So long, and thanks for all the fish!\n");
		exit(42);
	}
	/* LCOV_EXCL_STOP */
}

void on_abort(jmp_buf *jmp)
{
	ABORT_JMP = jmp;
}

/**  Initialization  ********************************************/

void
INIT(void)
{
	unsigned int i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		SYMBOL_TABLE[i] = NIL;
	}
}

unsigned int
hash(const char *str, unsigned int lim)
{
	/* the positively stupidest hashing function you ever did meet */
	if (!str) return 0;
	return *str % lim;
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
	if (!cons) abort("car() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("car() called with non-cons arg");

	return cons->value.cons.car;
}

obj
cdr(obj cons)
{
	if (!cons) abort("cdr() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("cdr() called with non-cons arg");

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
	if (!name) return NIL;

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

/* LCOV_EXCL_START */
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
/* LCOV_EXCL_STOP */
