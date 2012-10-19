#ifndef CORE_H
#define CORE_H

/**


       n7: a Lisp dialect for system programming


 **/

/*******************************************************

  TODO:

  LISP ESSENTIALS
  - support lambda
  - funcall stack
  - map out core operators
     QUOTE
     IF
     DO (PROGN)
     WITH (LET, LET* and friends)

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
			size_t n;   /* length of buffer */
			size_t len; /* length of string */
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

#define STR_SEGMENT_SIZE 32

/**************************************************/

void INIT(void);
unsigned int hash(const char *str, unsigned int lim);
int type(obj o);

obj globals(void);
obj builtin(op_fn op);

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
obj readx(obj io);
obj printx(obj io, obj what);
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

/* string : dup/copy/create */
obj str_dup(obj s);
obj str_dupc(const char *c);
obj str_dupb(const char *buf, size_t len);
obj str_dupf(const char *fmt, ...);
/* string : concatenate */
obj str_cat(obj dst, obj src);
obj str_catc(obj dst, const char *src);
obj str_catb(obj dst, const char *buf, size_t len);
obj str_catf(obj dst, const char *fmt, ...);
/* string : utilities */
char *cstr(obj s);
void strx(obj dst, const char *src);
void strc(obj dst, char c);
void strf(obj dst, const char *fmt, ...);

/* io */
obj io_fdopen(FILE *fd);
obj io_fopen(const char *path, const char *mode);
obj io_string(const char *str);
void io_rewind(obj io);
obj io_close(obj io);

/* io - read/write a single character */
char io_getc(obj io);
char io_ungetc(obj io, char c);
char io_putc(obj io, char c);

/* io - read delimited input */
obj io_read_delim(obj io, obj delims, int incl_delim);

/* io - write a single vstring */
obj io_write_str(obj io, obj str);

/* io - read/write binary segments */
obj io_read_buf(obj io, size_t n);
obj io_write_buf(obj io, const char *buf, size_t n);

/* io - read/write an S-expression */
obj io_read_form(obj io);
obj io_write_form(obj io, obj form);

/* primitive ops */
obj op_add(obj args);
obj op_sub(obj args);
obj op_mult(obj args);
obj op_div(obj args);
obj op_apply(obj fn, obj args);

obj op_eq(obj args);
obj op_eql(obj args);
obj op_equal(obj args);

/* debugging; superceded by vdump / printx and friends */
#if 0
void dump_obj(const char *tag, obj o);
void dump_sym(obj sym);
void dump_syms(void);
#endif

#endif
