#ifndef CORE_H
#define CORE_H

#define VTYPE_NIL      0
#define VTYPE_CONS     1
#define VTYPE_FIXNUM   2
#define VTYPE_CHAR     3

struct cons;

typedef struct val {
	int type;
	union {
		struct cons *cons;
		signed long fixnum;
		unsigned char c;
	} value;
	int refs;
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

/* functions for dealing with conses */
VAL* cons(VAL *car, VAL *cdr);
VAL* car(VAL *cons);
VAL* cdr(VAL *cons);

#endif
