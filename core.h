#ifndef CORE_H
#define CORE_H

#include "q.h"

#define VTYPE_NIL      0
#define VTYPE_CONS     1
#define VTYPE_FIXNUM   2
#define VTYPE_CHAR     3

static const char* VAL_TYPES[] = {
	"NIL",
	"CONS",
	"FIXNUM",
	"CHAR"
};

struct cons;

typedef struct val {
	Q l_gc;
	int type;
	union {
		struct cons *cons;
		signed long fixnum;
		unsigned char c;
	} value;
	unsigned int refs;
} VAL;

typedef struct cons {
	VAL *car;
	VAL *cdr;
} CONS;

/* bail out, with an error */
void ERR(const char *msg);

/* helpers for managing ref counts */
VAL* ref(VAL *val);
VAL* deref(VAL *val);

/* helpers for creating typed values */
VAL* vnil(void);
VAL* vcons(CONS *cons);
VAL* vfixnum(signed long n);
VAL* vchar(unsigned char c);

/* garbage collection */
void gc(void);
size_t gc_free(VAL *val);

/* functions for dealing with conses */
VAL* cons(VAL *car, VAL *cdr);
VAL* car(VAL *cons);
VAL* cdr(VAL *cons);

/* debugging functions */
void dump_gc(void);

#endif
