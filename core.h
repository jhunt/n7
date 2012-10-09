#ifndef CORE_H
#define CORE_H

void INIT(void);

/* tagged pointers */
typedef unsigned long obj;
/* FIXME: mark-sweep gc - use 0x8 as 'marked' bit */
#define TAGMASK 0x0FL
#define lotag(p)  ((obj)(p) &  TAGMASK)
#define hitag(p)  ((obj)(p) & ~TAGMASK)
#define tag(p,t)  (hitag(p)|(t))
#define untag(p)  (void*)(hitag(p))

/* TAG values can range from 0x0 (000) to 0x7 (111) */
#define TAG_FIXNUM 0x0
#define TAG_SYMBOL 0x1
#define TAG_CONS   0x2
#define TAG_OP     0x3

/* nihilistic tendencies */
extern obj nil;
#define nilp(x) ((x) == nil)

/* consing to a better tomorrow */
typedef struct {
	obj car;
	obj cdr;
} CONS;

obj cons (obj car, obj cdr);
#define car(cons) (((CONS*)(untag(cons)))->car)
#define cdr(cons) (((CONS*)(untag(cons)))->cdr)
#define for_list(obj, list) \
	for (obj = (list); !nilp(obj); obj = cdr(obj))

/* symbol manipulation */
obj intern(const char *name);

/* fixed-sized numbers */
#define mkfixnum(n) ((n) << TAGMASK)   /* implementation detail: */
#define fixnum(v)   ((v) >> TAGMASK)   /*   fixnum tag is 0x0    */

/* garbage collection */
#define GC_POOLSZ 32
struct pool;
typedef struct pool {
	struct pool *next;
	unsigned long map;
	size_t ptrsz;
	char ptrs[];
} POOL;

void* halloc(POOL *p);
void  hfree(POOL *p, void *ptr);
void gc(void);

/* debugging; it happens to the best of us */
void dump_obj(obj o);
void dump_sym(obj sym);
void dump_syms(void);

#endif
