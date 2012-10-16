#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>

#include "core.h"

/* abort-specific implementation details */
static jmp_buf *ABORT_JMP = NULL;

static obj
OBJECT(unsigned short type, size_t varlen)
{
	obj o = calloc(1,sizeof(bigobj)+varlen);
	if (!o) abort("malloc failed"); /* LCOV_EXCL_LINE */
	o->type = type;
	return o;
}

static char*
lc(const char *s)
{
	char *p, *new = strdup(s);
	if (!new) abort("strdup failed"); /* LCOV_EXCL_LINE */
	for (p = new; *p; p++) {
		if (isupper(*p)) *p = tolower(*p);
	}
	return new;
}

/**************************************************/

#define SYMBOL_TABLE_SIZE 211
obj SYMBOL_TABLE[SYMBOL_TABLE_SIZE];

/**************************************************/

void
_abort(const char *file, unsigned int line, const char *msg)
{
	/* LCOV_EXCL_START */
	if (ABORT_JMP) {
		longjmp(*ABORT_JMP, 42);
	} else {
		fprintf(stderr, "ABORT @%s:%u: %s\n", file, line, msg);
		fprintf(stderr, "So long, and thanks for all the fish!\n");
		exit(42);
	}
	/* LCOV_EXCL_STOP */
}

void on_abort(jmp_buf *jmp)
{
	ABORT_JMP = jmp;
}

/**  Initialization  ********************************************/

void
INIT(void)
{
	unsigned int i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		SYMBOL_TABLE[i] = NIL;
	}
}

unsigned int
hash(const char *str, unsigned int lim)
{
	/* the positively stupidest hashing function you ever did meet */
	if (!str) return 0;
	return *str % lim;
}

obj
eq(obj a, obj b)
{
	if (a == b) return T;
	return NIL;
}

#define TVAL(test) ((test) ? T : NIL)

obj
eql(obj a, obj b)
{
	if (IS_T(eq(a,b))) return T;
	if (TYPE(a) != TYPE(b)) return NIL;

	switch (TYPE(a)) {
		case OBJ_FIXNUM:
			return TVAL(a->value.fixnum == b->value.fixnum);

		case OBJ_CONS:
			return TVAL(
				eql(car(a), car(b)) &&
				eql(cdr(a), cdr(b)));

		case OBJ_STRING:
			return TVAL(
				strcmp(a->value.string.data,
				       b->value.string.data) == 0);
	}

	return NIL;
}

obj
equal(obj a, obj b)
{
	if (IS_T(eql(a,b))) return T;
	return NIL;
}

/**************************************************/

#define TOKEN_MAX 100
static struct {
	unsigned short idx;
	char buf[TOKEN_MAX];
} TOKEN = { 0, {0} };

static inline void
token_put(char c)
{
	/* FIXME: in token_put, we should expand the buffer if need be */
	if (TOKEN.idx < TOKEN_MAX - 1) {
		TOKEN.buf[TOKEN.idx++] = c;
	}
}
static inline char*
token_val(void)
{
	TOKEN.buf[TOKEN.idx++] ='\0';
	TOKEN.idx = 0;
	return strdup(TOKEN.buf);
}
static inline int
token_ok(void)
{
	return TOKEN.idx > 0;
}

char*
next_token(FILE *io)
{
	char c;
next_char:
	switch (c = fgetc(io)) {
		case EOF:
			break;

		case ' ':
		case '\t':
		case '\r':
		case '\n':
			if (token_ok()) break;
			goto next_char;

		case ';': /* Lisp-style comments */
			for (c = fgetc(io); c != EOF && c != '\n'; c = fgetc(io))
				;
			if (token_ok()) break;
			goto next_char;

		case '(':
		case ')':
		case '`':
		case '\'':
			if (token_ok()) {
				ungetc(c, io);
				break;
			}
			token_put(c);
			break;

		default:
			token_put(c);
			goto next_char;
	}

	if (!token_ok()) return NULL;
	return token_val();
}

static obj
read_number(const char *token)
{
	/* FIXME: read_number should support negatives, imaginaries, ratios, decimals, arbitrary base, etc. */
	long fixn = 0;
	size_t i, l;
	for (i = 0, l = strlen(token); i < l; i++) {
		fixn = fixn*10 + (token[i]-'0');
	}
	return fixnum(fixn);
}

#define READ_STRING_MAX 100

static obj
read_string(FILE *io)
{
	obj s = vstring("");
	char c, buf[READ_STRING_MAX];
	size_t idx = 0;

again:
	switch (c = fgetc(io)) {
		case '"':
			break;

		/* FIXME: handle escape chars in string literals */

		default:
			if (idx >= READ_STRING_MAX) {
				vextend(s, buf, idx);
				idx = 0;
			}
			buf[idx++] = c;

			goto again;
	}

	vextend(s, buf, idx);
	return s;
}

static obj
read_list(FILE *io)
{
	size_t nread = 1;
	obj first, next, rest;
	first = next = readx(io);

	if (next == CLOSE_PAREN) return NIL;
	if (next == CONS_DOT) abort("invalid form starting with (.");

	obj lst = NIL;
	for (; next != CLOSE_PAREN; next = readx(io)) {
		if (next == CONS_DOT) {
			if (nread == 1) {
				rest = readx(io);
				if (readx(io) == CLOSE_PAREN)
					return cons(first, rest);
			}
			abort("abuse of dotted notation!");
		}

		push(lst, next);
	}

	return revl(lst);
}

obj
readx(FILE *io)
{
	char *token, c;
	obj val;

	token = NULL;

again:
	switch (c = fgetc(io)) {
		case EOF:
			return NIL;

		case ' ':
		case '\t':
		case '\r':
		case '\n':
			goto again;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ungetc(c, io);
			token = next_token(io);
			val = read_number(token);
			break;

		case '(':
			val = read_list(io);
			break;

		case ')':
			val = CLOSE_PAREN;
			break;

		case '.':
			val = CONS_DOT;
			break;

		case '"':
			val = read_string(io);
			break;

		default:
			ungetc(c, io);
			token = next_token(io);
			val = intern(token);
			break;
	}

	free(token);
	return val;
}

static void
print_list(FILE *io, obj rest)
{
	printx(io, car(rest));
	if (!IS_NIL(cdr(rest))) {
		fprintf(io, " ");
		if (IS_CONS(cdr(rest))) {
			print_list(io, cdr(rest));
		} else {
			fprintf(io, ". ");
			printx(io, cdr(rest));
			fprintf(io, ")");
		}
	} else {
		fprintf(io, ")");
	}
}

obj
printx(FILE *io, obj what)
{
	if (IS_T(what)) {
		fprintf(io, "t");
	} else if (IS_NIL(what)) {
		fprintf(io, "nil");
	} else {
		switch (TYPE(what)) {
			case OBJ_FIXNUM:
				fprintf(io, "%li", what->value.fixnum);
				break;

			case OBJ_CONS:
				fprintf(io, "(");
				print_list(io, what);
				break;

			case OBJ_SYMBOL:
				fprintf(io, "%s", what->value.sym.name);
				break;

			case OBJ_STRING:
				/* FIXME: need to handle escaped characters... */
				fprintf(io, "\"%s\"", what->value.string.data);
				break;

			default:
				abort("unprintable object!");
		}
	}
	return T;
}

/**************************************************/

obj
cons(obj car, obj cdr)
{
	obj c = OBJECT(OBJ_CONS, 0);
	c->value.cons.car = car;
	c->value.cons.cdr = cdr;
	return c;
}

obj
car(obj cons)
{
	if (!cons) abort("car() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("car() called with non-cons arg");

	return cons->value.cons.car;
}

obj
cdr(obj cons)
{
	if (!cons) abort("cdr() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("cdr() called with non-cons arg");

	return cons->value.cons.cdr;
}

obj
revl(obj lst)
{
	/* FIXME: type checking in revl! */
	obj t, new = NIL;
	for_list(t, lst) {
		push(new, car(t));
	}
	return new;
}

obj
nlist(size_t n, ...)
{
	va_list ap;
	obj lst = NIL;
	va_start(ap, n);

	for (; n > 0; n--) {
		/* this line is lcov-excluded b/c va_arg
		   has weird branching... */
		push(lst, va_arg(ap, obj)); /* LCOV_EXCL_LINE */
	}

	va_end(ap);
	return revl(lst);
}

obj
fixnum(long n)
{
	obj fixn = OBJECT(OBJ_FIXNUM, 0);
	fixn->value.fixnum = n;
	return fixn;
}

/**  Symbols  ***************************************************/

#define symname(cons) (const char*)untag((car(cons)))

static obj
findsym(unsigned int key, const char *name)
{
	obj rest;
	for_list(rest, SYMBOL_TABLE[key]) {
		if (strcasecmp(name, car(rest)->value.sym.name) == 0) {
			return car(rest);
		}
	}
	return NIL;
}

static obj
mksym(unsigned int key, const char *name)
{
	size_t len = strlen(name)+1;
	obj S = OBJECT(OBJ_SYMBOL, len);
	S->value.sym.len = len;
	strncpy(S->value.sym.name, name, len);

	push(SYMBOL_TABLE[key], S);
	return S;
}

obj
intern(const char *name)
{
	if (!name) return NIL;

	char *symname = lc(name);
	if (strcmp(symname, "nil") == 0) return NIL;
	if (strcmp(symname, "t") == 0)   return T;

	unsigned int key = hash(symname, SYMBOL_TABLE_SIZE);
	obj sym = findsym(key, symname);

	if (IS_NIL(sym)) {
		sym = mksym(key, symname);
	}

	free(symname);
	return sym;
}

/**  Evaluation  ************************************************/

obj
eval(obj args)
{
	if (IS_NIL(args)) return NIL;
	if (IS_T(args)) return T;
	if (IS_FIXNUM(args)) return args;

	abort("eval not finished");
}

/**  String Handling  *******************************************/

obj
vstring(const char *s)
{
	obj str = OBJECT(OBJ_STRING, 0);
	str->value.string.len = strlen(s);
	str->value.string.data = strdup(s);
	return str;
}

obj
vextend(obj s, const char *cstr, size_t n)
{
	size_t len = s->value.string.len + n;
	char *data = realloc(s->value.string.data, len+1);
	if (!data) abort("vextend out of memory");

	strncat(data, cstr, n);
	data[len] = '\0';
	s->value.string.len = len;
	s->value.string.data = data;
	return s;
}

obj
vstrcat(obj root, obj add)
{
	obj str = OBJECT(OBJ_STRING, 0);
	size_t len = root->value.string.len + add->value.string.len;
	str->value.string.len = len;

	char *raw = calloc(len+1, sizeof(char));
	strcat(raw, root->value.string.data);
	strcat(raw, add->value.string.data);
	str->value.string.data = raw;

	return str;
}

/**  Primitive Operators  ***************************************/

/* FIXME: math operations DON'T handle overflow well */

obj
op_add(obj args)
{
	long acc = 0;
	obj term;
	for_list(term, args) {
		/* FIXME: op_add: check types! */
		acc += car(term)->value.fixnum;
	}
	return fixnum(acc);
}

obj
op_sub(obj args)
{
	long acc = 0;
	obj term;

	/* FIXME: op_sub: check types! */
	if (IS_NIL(args)) return fixnum(0);
	if (IS_NIL(cdr(args))) {
		acc = 0; /* (- 45) === (- 0 45) */
	} else {
		acc = car(args)->value.fixnum;
		args = cdr(args);
	}

	for_list(term, args) {
		acc -= car(term)->value.fixnum;
	}
	return fixnum(acc);
}

obj
op_mult(obj args)
{
	long acc = 1;
	obj term;
	for_list(term, args) {
		acc *= car(term)->value.fixnum;
	}
	return fixnum(acc);
}

obj
op_div(obj args)
{
	long acc;
	obj term;

	/* FIXME: op_div: check types and arity (i.e. (/) -> ABORT) */
	if (IS_NIL(args)) abort("/: wrong number of arguments");
	if (IS_NIL(cdr(args))) {
		acc = 1; /* (/ 4) == 1/4 */
		abort("/: no ratio support; ergo, no (/ x) support -- go bug jrh");
	} else {
		acc = car(args)->value.fixnum;
		args = cdr(args);
	}

	for_list(term, args) {
		acc /= car(term)->value.fixnum;
	}
	return fixnum(acc);
}

obj
op_apply(obj fn, obj args)
{
	if (!fn) abort("fn cannot be NULL");
	if (!IS_BUILTIN(fn)) abort("fn is not a builtin");
	return (*(fn->value.builtin))(args);
}

/**  Debugging Functions  ***************************************/

/* LCOV_EXCL_START */

static const char *DUMPER_TYPE_NAMES[] = {
	"UNKNOWN",
	"symbol",
	"cons",
	"fixnum",
	"builtin-op"
};

static void
_dump(FILE* io, unsigned int indent, obj var)
{
	unsigned int i;
	for (i = 0; i < indent; i++) { fprintf(io, " "); }

	if (IS_T(var)) {
		fprintf(io, "T");
	} else if (IS_NIL(var)) {
		fprintf(io, "NIL");
	} else {
		/* <# [TYPE] [PTR] [VALUE] #> */
		fprintf(io, "<# %s %p", DUMPER_TYPE_NAMES[var->type], var);
		if (IS_CONS(var)) {
			fprintf(io, " (\n");
			_dump(io, indent+2, car(var));
			fprintf(io, "\n");
			_dump(io, indent+2, cdr(var));
			fprintf(io, " )");
		} else if (IS_SYMBOL(var)) {
			fprintf(io, " '%s", var->value.sym.name);
		} else if (IS_FIXNUM(var)) {
			fprintf(io, " %lu", var->value.fixnum);
		}
		fprintf(io, " #>");
	}
}

void
dump_obj(const char *tag, obj o)
{
	fprintf(stderr, "%s", tag);
	_dump(stderr, 0, o);
	fprintf(stderr, "\n");
}

void
dump_sym(obj sym)
{
	fprintf(stderr, "sym:[%#08lx:%#01lx >> %s]\n",
			hitag(sym), lotag(sym), sym->value.sym.name);
}

void
dump_syms(void)
{
	obj rest;
	size_t i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		if (!IS_NIL(SYMBOL_TABLE[i])) {
			fprintf(stderr, "SYMBOL_TABLE[%lu] = \n", (unsigned long)i);
			for_list(rest, SYMBOL_TABLE[i]) {
				dump_obj("", car(rest));
				dump_sym(car(rest));
			}
		}
	}
	fprintf(stderr, "-----------------\n");
}
/* LCOV_EXCL_STOP */
