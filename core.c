#include <stdlib.h>

#include <stdio.h>

#include "core.h"
#include "q.h"

#define MAX_BITS 32
#define FULL_MASK 0xffffffff

typedef struct {
	Q       heap;

	size_t  size; /* size of each object */

	unsigned long map; /* bit field of which slots are used */

	unsigned char *max;    /* pointer to end of slots */
	unsigned char slots[]; /* raw data, variable-length */
} POOL;

/*************************************************************/

#define HEAP(name) static Q name = { &(name), &(name) }

HEAP(GC_HEAP);
HEAP(VAL_HEAP);
HEAP(CONS_HEAP);

/*************************************************************/

static POOL* alloc_pool(Q *heap, size_t len, size_t objsize)
{
	POOL *pool = malloc(sizeof(POOL) + objsize * len);
	if (!pool) {
		ERR("new_pool: out of memory");
	}
	pool->max = pool->slots + (len * objsize);
	pool->size = objsize;
	pool->map = 0;
	q_append(heap, &pool->heap);
}

static inline int pool_empty(POOL *pool)
{
	return pool->map == 0;
}

static inline int pool_full(POOL *pool)
{
	return pool->map == FULL_MASK;
}

static inline int pool_has(POOL *pool, void *ptr)
{
	return (unsigned char*)ptr >= pool->slots
	    && (unsigned char*)ptr <= pool->max;
}

static inline void pool_claim(POOL *pool, signed char i)
{
	pool->map = pool->map | (1 << i);
}

static inline void pool_forfeit(POOL *pool, signed char i)
{
	pool->map = pool->map & ~(1 << i);
}

static void* pool_slot(POOL *pool, signed char i)
{
	return (void*)(pool->slots+(i*pool->size));
}

static signed char pool_ptr(POOL *pool, void *ptr)
{
	return ((unsigned char*)ptr - pool->slots) / pool->size;
}

static void* pool_next(POOL *pool)
{
	unsigned char i;
	for (i = 0; i < MAX_BITS; i++) {
		if ((pool->map & (1 << i)) != (1 << i)) { /* FIXME: refactor */
			pool_claim(pool, i);
			return pool_slot(pool, i);
		}
	}
	return NULL;
}

static void dealloc_pool(POOL *pool)
{
	if (!pool_empty(pool)) {
		return;
	}
	q_remove(&pool->heap);
	free(pool);
}

static void* halloc(Q *heap, size_t objsize)
{
	POOL *pool;
	for_each(heap, pool, heap) {
		if (!pool_full(pool)) {
			return pool_next(pool);
		}
	}

	pool = alloc_pool(heap, MAX_BITS, objsize);
	return pool_next(pool);
}

static void hfree(Q *heap, void *ptr)
{
	POOL *pool;
	for_each(heap, pool, heap) {
		if (!pool_has(pool, ptr)) {
			continue;
		}

		pool_forfeit(pool, pool_ptr(pool, ptr));
		return;
	}

	ERR("heap corruption in hfree");
}

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
	VAL *val = (VAL*)halloc(&VAL_HEAP, sizeof(VAL));
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

	POOL *pool, *ptmp;
	for_each_safe(&CONS_HEAP, pool, ptmp, heap) {
		dealloc_pool(pool);
	}
	for_each_safe(&VAL_HEAP, pool, ptmp, heap) {
		dealloc_pool(pool);
	}
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
			hfree(&CONS_HEAP, val->value.cons);
			break;

		default:
			ERR("unknown value type");
			return 0;
	}

	q_remove(&val->l_gc);
	hfree(&VAL_HEAP, val);
	return bytes;
}

/*************************************************************/

VAL* cons(VAL *car, VAL *cdr)
{
	CONS *cons = (CONS*)halloc(&CONS_HEAP, sizeof(CONS));
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

