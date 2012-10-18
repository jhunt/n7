#ifndef CORE_H
#define CORE_H

/**


       n7: a Lisp dialect for system programming


 **/

/*******************************************************

  TODO:

  LISP ESSENTIALS
  - io stream support (std*, file, string, etc.)
  - support lambda
  - full reader
  - funcall stack
  - map out core operators

  DATA TYPES
  - vectors
  - complex numbers
  - floating point numbers
  - ratios
  - bignums

  FFI
  - syscall interface

  COMPILING
  - tail-call optimization / CPS?
  - intermediate language (3-addr?)
  - code optimizer
  - machine code translation

  FUN STUFF TO ADD
  - something like Perl's topic var $_
    or Arc's 'it' pronoun.

********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#define TAGGED_BITS 8
#define TAGGED_MASK 0xf

#define TAG_POINTER   0x0   /* 0000 */
#define TAG_FIXNUM    0x1   /* 0001 */
#define TAG_CONSTANT  0x3   /* 0011 */

#define hitag(obj) ((unsigned long)(obj)&~TAGGED_MASK)
#define lotag(obj) ((unsigned long)(obj)& TAGGED_MASK)

#define OBJ_SPECIAL      0x00
#define OBJ_SYMBOL       0x01
#define OBJ_CONS         0x02
#define OBJ_FIXNUM       0x03
#define OBJ_BUILTIN      0x04
#define OBJ_STRING       0x05
#define OBJ_IO           0x06

typedef struct big_object  bigobj;
typedef struct big_object* obj;

typedef obj (*op_fn)(obj);

struct big_object {
	unsigned short type;
	union {
		long fixnum;
		op_fn builtin;

		struct {
			size_t len;
			char name[];
		} sym;

		struct {
			obj car;
			obj cdr;
		} cons;

		struct {
			size_t len;
			char *data;
		}  string;

		struct {
			/* string io */
			size_t len;
			size_t i;
			char *data;

			/* standard file io */
			FILE *fd;
		} io;

	} value;
};

#define TYPE(x) (lotag(x) == 0x00 ? x->type : OBJ_SPECIAL)

#define IS_A(x,tag) (TYPE(x) == OBJ_ ## tag)
#define IS_SPECIAL(x)    IS_A(x, SPECIAL)
#define IS_CONS(x)       IS_A(x, CONS)
#define IS_SYMBOL(x)     IS_A(x, SYMBOL)
#define IS_FIXNUM(x)     IS_A(x, FIXNUM)
#define IS_BUILTIN(x)    IS_A(x, BUILTIN)
#define IS_STRING(x)     IS_A(x, STRING)
#define IS_IO(x)         IS_A(x, IO)

#define MAKE_CONSTANT(n) (obj)((n<<TAGGED_BITS)|TAG_CONSTANT)
#define NIL            MAKE_CONSTANT(0)
#define T              MAKE_CONSTANT(1)
/* these are used internally by the parser */
#define CLOSE_PAREN    MAKE_CONSTANT(33)
#define CONS_DOT       MAKE_CONSTANT(34)
#define UNDEF          MAKE_CONSTANT(35) /* more nil than NIL */

#define IS_T(x)   ((x) == T)
#define IS_NIL(x) ((x) == NIL)
#define DEF(x)    ((x) != UNDEF)

/**************************************************/

void on_abort(jmp_buf *jmp);
void _abort(const char *file, unsigned int line, const char *msg);
#define abort(m) _abort(__FILE__, __LINE__, (m))

/**************************************************/

void INIT(void);
unsigned int hash(const char *str, unsigned int lim);
int type(obj o);

obj globals(void);

#if 0
obj same(obj a, obj b);
obj is(obj a, obj b);
obj eq(obj a, obj b);
#endif
obj eq(obj a, obj b);
obj eql(obj a, obj b);
obj equal(obj a, obj b);

/* environmentalism */
obj get(obj env, obj sym);
obj set(obj env, obj sym, obj val);

/* read... */
obj readx(FILE *io);
obj printx(FILE *io, obj what);
obj vdump(obj what);
char* cdump(obj what);

/* consing to a better tomorrow */
obj cons(obj car, obj cdr);
obj car(obj cons);
obj cdr(obj cons);
obj revl(obj lst);
obj nlist(size_t n, ...);
#define push(ls,v) (ls) = cons((v),(ls))
#define for_list(obj, list) \
	for (obj = (list); !IS_NIL(obj); obj = cdr(obj))

obj fixnum(long n);

/* symbol manipulation */
obj intern(const char *name);

/* evaluation */
obj eval(obj args, obj env);

/* string */
obj vstring(const char *s);
obj vextend(obj s, const char *cstr, size_t n);
obj vextendc(obj s, char c);
obj vappend(obj s, const char *cstr);
obj vformat(obj s, const char *fmt, ...);
obj vstrcat(obj root, obj add);
char vchar(obj s, size_t idx);

/* io */
obj iofd(FILE *file);
obj iofile(const char *path, const char *mode);
obj iostring(const char *str, const char *mode);
obj ioclose(obj io);
obj iowriteb(obj io, obj str);
obj ioreadb(obj io, obj n);
obj iowrite(obj io, obj form);
obj ioread(obj io);

/* primitive ops */
obj op_add(obj args);
obj op_sub(obj args);
obj op_mult(obj args);
obj op_div(obj args);
obj op_apply(obj fn, obj args);

/* debugging; superceded by vdump / printx and friends */
#if 0
void dump_obj(const char *tag, obj o);
void dump_sym(obj sym);
void dump_syms(void);
#endif

#endif
