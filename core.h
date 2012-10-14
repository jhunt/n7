#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <setjmp.h>

#define TAGGED_BITS 8
#define TAGGED_MASK 0xf

#define TAG_POINTER   0x0   /* 0000 */
#define TAG_FIXNUM    0x1   /* 0001 */
#define TAG_CONSTANT  0x3   /* 0011 */

#define OBJ_SYMBOL       0x01
#define OBJ_CONS         0x02
#define OBJ_FIXNUM       0x03

#define hitag(obj) ((unsigned long)(obj)&~TAGGED_MASK)
#define lotag(obj) ((unsigned long)(obj)& TAGGED_MASK)

typedef struct big_object  bigobj;
typedef struct big_object* obj;
struct big_object {
	unsigned short type;
	union {
		struct {
			size_t len;
			char name[];
		} sym;

		struct {
			obj car;
			obj cdr;
		} cons;
	} value;
};

#define IS_A(obj,tag) ((obj)->type == OBJ_ ## tag)
#define IS_CONS(obj) IS_A(obj,CONS)

#define MAKE_CONSTANT(n) (struct big_object*)((n<<TAGGED_BITS)|TAG_CONSTANT)
#define NIL MAKE_CONSTANT(0)
#define T   MAKE_CONSTANT(1)

#define IS_T(obj)   ((obj) == T)
#define IS_NIL(obj) ((obj) == NIL)

/**************************************************/

void on_abort(jmp_buf *jmp);
void _abort(const char *file, unsigned int line, const char *msg);
#define abort(m) _abort(__FILE__, __LINE__, (m))

/**************************************************/

void INIT(void);

unsigned int hash(const char *str, unsigned int lim);

/* consing to a better tomorrow */
obj cons(obj car, obj cdr);
obj car(obj cons);
obj cdr(obj cons);
#define push(ls,v) (ls) = cons((v),(ls))
#define for_list(obj, list) \
	for (obj = (list); !IS_NIL(obj); obj = cdr(obj))

/* symbol manipulation */
obj intern(const char *name);

#if 0
/* fixed-sized numbers */
#define mkfixnum(n) ((n) << TAGMASK)   /* implementation detail: */
#define fixnum(v)   ((v) >> TAGMASK)   /*   fixnum tag is 0x0    */
#endif

/* debugging; it happens to the best of us */
void dump_obj(obj o);
void dump_sym(obj sym);
void dump_syms(void);

#endif
