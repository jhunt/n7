#include <stdlib.h>
#include <stdio.h>

#include "core.h"
#include "q.h"

static Q GC_HEAP = { &GC_HEAP, &GC_HEAP };

/*************************************************************/

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
		if (val->type == VTYPE_CONS) {
			ref(val->value.cons->car);
			ref(val->value.cons->cdr);
		}
	}
	return val;
}

VAL* deref(VAL *val)
{
	if (val && val->refs != 0) {
		val->refs--;
		if (val->type == VTYPE_CONS) {
			deref(val->value.cons->car);
			deref(val->value.cons->cdr);
		}
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
	q_append(&GC_HEAP, &val->l_gc);
	val->refs = 0;
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

void gc(void)
{
	VAL *v, *tmp;
	size_t bytes_freed = 0;
	for_each_safe(&GC_HEAP, v, tmp, l_gc) {
		if (v->refs == 0) {
			bytes_freed += gc_free(v);
		}
	}

	if (bytes_freed > 0) { gc(); }
}

size_t gc_free(VAL *val)
{
	size_t bytes = sizeof(VAL);
	switch (val->type) {
		case VTYPE_NIL:
		case VTYPE_FIXNUM:
		case VTYPE_CHAR:
			break; /* direct-storage, nothing to free */

		case VTYPE_CONS:
			deref(val->value.cons->car);
			deref(val->value.cons->cdr);
			bytes += sizeof(CONS);
			free(val->value.cons);
			break;

		default:
			ERR("unknown value type");
			return 0;
	}

	q_remove(&val->l_gc);
	free(val);
	return bytes;
}

/*************************************************************/

VAL* cons(VAL *car, VAL *cdr)
{
	CONS *cons = malloc(sizeof(CONS));
	if (!cons) {
		ERR("cons: out of memory");
	}
	cons->car = ref(car);
	cons->cdr = ref(cdr);

	return vcons(cons);
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

void dump_gc(void)
{
	VAL *v;
	int i = 0;
	for_each(&GC_HEAP, v, l_gc) {
		printf("%i: %p (%s (refs %u))\n",
			i++, v, VAL_TYPES[v->type], v->refs);
	}
}

