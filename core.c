#include <stdlib.h>
#include <stdio.h>

#include "core.h"

void ERR(const char *msg)
{
	/* FIXME: phase 0 - all errors get printed to stderr */
	fprintf(stderr, "ERROR: %s\n", msg);
	exit(42);
}

/*************************************************************/

VAL* ref(VAL *val)
{
	if (val) {
		val->refs++;
	}
	return val;
}

VAL* deref(VAL *val)
{
	if (val) {
		val->refs--;
	}
	return val;
}

/*************************************************************/

VAL* vnil(void)
{
	VAL *val = malloc(sizeof(VAL));
	if (!val) {
		ERR("new value: out of memory");
	}
	val->type = VTYPE_NIL;
	return val;
}

VAL* vcons(CONS *cons)
{
	VAL *val = vnil();
	val->type = VTYPE_CONS;
	val->value.cons = cons;
	return val;
}

VAL* vfixnum(signed long n)
{
	VAL *val = vnil();
	val->type = VTYPE_FIXNUM;
	val->value.fixnum = n;
	return val;
}

VAL* vchar(unsigned char c)
{
	VAL *val = vnil();
	val->type = VTYPE_CHAR;
	val->value.c = c;
	return val;
}

/*************************************************************/

VAL* cons(VAL *car, VAL *cdr)
{
	CONS *cons = malloc(sizeof(CONS));
	if (!cons) {
		ERR("cons: out of memory");
	}
	cons->car = car;
	cons->cdr = cdr;

	return ref(vcons(cons));
}

VAL* car(VAL *cons)
{
	if (cons->type == VTYPE_NIL) { return ref(vnil()); }
	if (cons->type != VTYPE_CONS) { ERR("not a list"); }
	return cons->value.cons->car;
}

VAL* cdr(VAL *cons)
{
	if (cons->type == VTYPE_NIL) { return ref(vnil()); }
	if (cons->type != VTYPE_CONS) { ERR("not a list"); }
	return cons->value.cons->cdr;
}

/*************************************************************/
