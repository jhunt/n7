#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"
#include "gc.h"

static obj SYMBOLS = 0;
obj nil;

static POOL CONS_HEAP = {
#include "gc.h"
	.next = NULL,
	.map  = ~0UL,
	.ptrsz = sizeof(CONS)
};

static CONS*
mkcons(obj car, obj cdr)
{
	CONS *cons = xalloc();
	//CONS *cons = halloc(&CONS_HEAP);
	cons->car = car;
	cons->cdr = cdr;
	return cons;
}

obj
cons(obj car, obj cdr)
{
	CONS* cons = mkcons(car, cdr);
	return tag(cons, TAG_CONS);
}

/**  Symbols  ***************************************************/

#define symname(cons) (const char*)untag((car(cons)))

static obj
findsym(const char *name)
{
	obj rest;
	for_list(rest, SYMBOLS) {
		fprintf(stderr, "checking %p (%s) against %s\n",
				untag(rest), symname(rest), name);
		if (strcasecmp(name, symname(rest)) == 0) {
			return rest;
		}
	}
	return nil;
}

static char*
mkname(const char *orig)
{
	char *new = strdup(orig);
	char *p;
	for (p = new; *p; p++) {
		if (isupper(*p)) {
			*p = tolower(*p);
		}
	}
	return new;
}
#define mksym(name) tag(mkname(name), TAG_SYMBOL)

obj
intern(const char *name)
{
	obj sym;
	sym = findsym(name);
	if (!nilp(sym)) {
		return sym;
	}
	SYMBOLS = cons(mksym(name), SYMBOLS);
	return SYMBOLS;
}

/**  Garbage Collection  ****************************************/

#define nmask(i) (1 << (i))

#define gc_slot(p,slot)    ((p)->ptrs + ((slot)*((p)->ptrsz)))
#define gc_ptr(p,ptr)      (((char*)(ptr) - (char*)((p)->ptrs)) / (p)->ptrsz)

#define gc_avail(p,slot)   (!((p)->map & nmask(slot)))
#define gc_claim(p,slot)   ((p)->map = (p)->map | nmask(slot))
#define gc_forfeit(p,slot) ((p)->map = (p)->map & ~nmask(slot))

#define between(n,x,y)     ((n) >= (x) && (n) <= (y))
#define gc_pool_has(p,ptr) between((char*)(ptr), (p)->ptrs, (char*)((p)+sizeof(*(p))))
#define gc_pool_empty(p)   ((p)->map == ~0)

static void*
gc_next(POOL *p)
{
	int i;
	for (i = 0; i < GC_POOLSZ; i++) {
		if (gc_avail(p,i)) {
			gc_claim(p,i);
			return (void*)(gc_slot(p,i));
		}
	}
}

static POOL*
gc_pool(size_t ptrsz)
{
	POOL *p = malloc(sizeof(POOL) + (GC_POOLSZ * ptrsz));
	p->ptrsz = ptrsz;
	p->map = 0;
	p->next = NULL;

	return p;
}

void*
halloc(POOL *p)
{
	POOL *tail;
	for (tail = p, p = p->next; p; tail = p, p = p->next) {
		if (gc_pool_empty(p)) {
			continue;
		}
		return gc_next(p);
	}

	tail->next = gc_pool(tail->ptrsz);
	return gc_next(tail->next);
}

void
hfree(POOL *p, void *ptr)
{
	for (p = p->next; p; p = p->next) {
		if (!gc_pool_has(p, ptr)) {
			continue;
		}
		gc_forfeit(p, gc_ptr(p, ptr));
	}
}

static void
gc_walk_pool (POOL *p)
{
	POOL *tail;
	for (tail = p, p = p->next; p; tail = p, p = p->next) {
		/* FIXME: not ref-counting objects... */
		if (gc_pool_empty(p)) {
			tail->next = p->next;
			free(p);
			p = tail;
		}
	}
}

void gc(void)
{
	gc_walk_pool(&CONS_HEAP);
	/* FIXME: how do we reclaim interned symbol memory? */
}

/**  Initialization  ********************************************/

void
INIT(void)
{
	nil = mksym("nil");
	SYMBOLS = nil;
	gc_init(64);
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
			hitag(sym), lotag(sym), (char*)untag(sym));
}

void
dump_syms(void)
{
	obj rest;
	for_list(rest, SYMBOLS) {
		dump_obj(car(rest));
		dump_sym(car(rest));
	}
	fprintf(stderr, "-----------------\n");
}
