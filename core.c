#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>

#include "core.h"

/* abort-specific implementation details */
static jmp_buf *ABORT_JMP = NULL;

int DEBUG_LEVEL = 0;

#define ABORT_OOM(subsys) abort(subsys ": out of memory")

static obj
OBJECT(unsigned short type, size_t varlen)
{
	debug3("allocating %s (%#04x) of +%i bytes\n", OBJ_TYPE_NAMES[type], type, varlen);
	obj o = calloc(1,sizeof(bigobj)+varlen);
	if (!o) ABORT_OOM("OBJECT"); /* LCOV_EXCL_LINE */
	o->type = type;
	return o;
}

static char*
lc(const char *s)
{
	char *p, *new = strdup(s);
	if (!new) ABORT_OOM("lc"); /* LCOV_EXCL_LINE */
	for (p = new; *p; p++) {
		if (isupper(*p)) *p = tolower(*p);
	}
	debug3("lc('%s') => '%s'\n", s, new);
	return new;
}

/**************************************************/

#define SYMBOL_TABLE_SIZE 211
obj SYMBOL_TABLE[SYMBOL_TABLE_SIZE];

/**************************************************/

void
_abort(const char *file, unsigned int line, const char *msg)
{
	debug1("ABORT @%s:%u: %s\n", file, line, msg);
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

/**  Debugging  *************************************************/

int debugging(int new_level)
{
	int old = DEBUG_LEVEL;
	DEBUG_LEVEL = new_level;
	return old;
}

void _debug(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "debug%i> ", level);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

/**  Initialization  ********************************************/

void
INIT(void)
{
	unsigned int i;
	debug1("initializing runtime\n");
	debug2("creating initial symbol table of %i entries\n", SYMBOL_TABLE_SIZE);
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

#define OP_NAME(x)  (x)->value.builtin.name
#define OP_FN(x)    (x)->value.builtin.fn

#define FNUM(x)     (x)->value.fixnum

#define CAR(x)      (x)->value.cons.car
#define CDR(x)      (x)->value.cons.cdr

#define SYM(x) (x)->value.sym.name

#define STR_N(s)    (s)->value.string.n
#define STR_LEN(s)  (s)->value.string.len
#define STR_VAL(s)  (s)->value.string.data
#define STR_LEFT(s) (STR_N(s) - STR_LEN(s))

#define IO_FD(io)   (io)->value.io.fd
#define IO_IDX(io)  (io)->value.io.i
#define IO_LEN(io)  (io)->value.io.len
#define IO_STR(io)  (io)->value.io.data

#define L_PARAMS(l) (l)->value.lambda.params
#define L_BODY(l)   (l)->value.lambda.code


#define SET_BUILTIN(e,n,op) set((e), intern(n), builtin(n,(op)))
obj
globals(void)
{
	obj e =  env_init();
	SET_BUILTIN(e, "+", op_add);
	SET_BUILTIN(e, "-", op_sub);
	SET_BUILTIN(e, "*", op_mult);
	SET_BUILTIN(e, "/", op_div);

	SET_BUILTIN(e, "eq", op_eq);
	SET_BUILTIN(e, "eql", op_eql);
	SET_BUILTIN(e, "equal", op_equal);

	SET_BUILTIN(e, "cons", op_cons);
	SET_BUILTIN(e, "list", op_list);
	SET_BUILTIN(e, "car", op_car);
	SET_BUILTIN(e, "cdr", op_cdr);

	SET_BUILTIN(e, "prs", op_prs);

	SET_BUILTIN(e, "typeof", op_typeof);

	SET_BUILTIN(e, "call", op_call);
	SET_BUILTIN(e, "apply", op_apply);

	SET_BUILTIN(e, "exit", op_exit);

	/* transitional stuff */
	SET_BUILTIN(e, "load", opx_load);

	load("sys/core.n7", e);

	return e;
}
#undef SET_BUILTIN

obj
builtin(const char *name, op_fn fn)
{
	obj op = OBJECT(OBJ_BUILTIN, 0);
	strncpy(OP_NAME(op), name, 15);
	OP_FN(op) = fn;
	return op;
}

obj
eq(obj a, obj b)
{
	if (DEF(a) && DEF(b) && a == b) return T;
	return NIL;
}

obj
eql(obj a, obj b)
{
	if (IS_T(eq(a,b))) return T;
	if (TYPE(a) != TYPE(b)) return NIL;

	if (TYPE(a) == OBJ_FIXNUM) {
		return FNUM(a) == FNUM(b) ? T : NIL;
	}

	return NIL;
}

obj
equal(obj a, obj b)
{
	if (IS_T(eql(a,b))) return T;
	char *rep_a = cdump(a);
	char *rep_b = cdump(b);

	obj rc = (strcmp(rep_a, rep_b) == 0 ? T : NIL);
	free(rep_a);
	free(rep_b);
	return rc;
}

/**************************************************/

obj
get(obj env, obj sym)
{
	obj level, x;
	for_list(level, env) {
		for_list(x, car(level)) {
			if (IS_T(eq(car(car(x)), sym))) {
				return cdr(car(x));
			}
		}
	}
	return UNDEF;
}

obj
set(obj env, obj sym, obj val)
{
	obj e, x;
	/* FIXME: this is WRONG.  we shouldn't recurse
	   up through the call stack until we find a sym
	   we like, and set that.  we need the concept of
	   symbol/variable visibility; i.e. global/local
	   always visible, closures visible, but the
	   internals of a caller, not so much... */

	/* FIXME: I think the real problem lies in the
	   misuse of env as a call-stack.  We need to
	   split out stack handling from environment. */

	for_list(e, env) {
		for_list(x, car(e)) {
			if (IS_T(eq(car(car(x)), sym))) {
				debug1("set existing %s to %s\n",
						cdump(sym), cdump(val));
				CDR(car(x)) = val;
				return env;
			}
		}
	}

	debug1("set new %s to %s\n",
			cdump(sym), cdump(val));
	CAR(env) = cons(
		cons(sym, val),
		car(env)
	);
	return env;
}

/*

   (
    car: ( (a . 1) (b . 2) (c . 3) )
    cdr: (
          car: ( (a . 2) (b . 4) (c . 6) )
          cdr: (
                car: ( (a . 42) )
                cdr: nil )))

 */
obj
env_init(void)
{
	return cons(NIL, NIL);
}

obj
env_new(obj env)
{
	obj oldtop = cons(car(env), cdr(env));
	CDR(env) = oldtop;
	CAR(env) = NIL;
	return env;
}

obj
env_del(obj env)
{
	CAR(env) = car(cdr(env));
	CDR(env) = cdr(cdr(env));
	return env;
}

/**************************************************/

char*
next_token(obj io)
{

	obj token = str_dupc("");
	char c;
next_char:
	switch (c = io_getc(io)) {
		case EOF:
			break;

		case ' ':
		case '\t':
		case '\r':
		case '\n':
			if (STR_LEN(token) > 0) break;
			goto next_char;

		case ';': /* Lisp-style comments */
			for (c = io_getc(io); c != EOF && c != '\n'; c = io_getc(io))
				;
			if (STR_LEN(token) > 0) break;
			goto next_char;

		case '(':
		case ')':
		case '`':
		case '\'':
			if (STR_LEN(token) > 0) {
				io_ungetc(io, c);
				break;
			}
			strc(token, c);
			break;

		default:
			strc(token, c);
			goto next_char;
	}

	if (STR_LEN(token) == 0) return NULL;
	return strdup(STR_VAL(token));
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
read_string(obj io)
{
	obj s = str_dupc("");
	char c;
	int esc = 0;

again:
	c = io_getc(io);
	if (c == EOF || (c == '"' && !esc)) return s;

	if (esc) {
		switch (c) {
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			// default: fall-through (\", \', \\)
		}

		esc = 0;
	} else if (c == '\\') {
		esc = 1;
		goto again;
	}

	strc(s,c);
	goto again;
}

static obj
read_list(obj io)
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

static int QQ = 0;
static int QQ_ESC = 0;

obj
readx(obj io)
{
	char *token, c;
	obj val;

	token = NULL;

again:
	c = io_getc(io);
	debug3("readx: io_getc returned %c\n", c);
	switch (c) {
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
			io_ungetc(io, c);
			token = next_token(io);
			val = read_number(token);
			break;

		case '(':
			val = read_list(io);
			if (QQ && !QQ_ESC) {
				val = cons(intern("list"), val);
			}
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

		case '\'':
			val = cons(intern("quote"), cons(readx(io), NIL));
			break;

		case '`': // `(a ,x b ,y (c ,z)) -> ('a x 'b y ('c z))
		          // (list 'a x 'b y (list 'c z))
			QQ = 1; val = readx(io); QQ = 0;
			break;

		case ',':
			QQ_ESC = 1; val = readx(io); QQ_ESC = 0;
			break;

		case ';':
			for (c = io_getc(io); c != EOF && c != '\n'; c = io_getc(io))
				;
			goto again;

		default:
			io_ungetc(io, c);
			token = next_token(io);
			val = intern(token);
			if (QQ && !QQ_ESC) {
				val = cons(intern("quote"), cons(val, NIL));
			}
			break;
	}

	free(token);
	return val;
}

static void vdump_x(obj str, obj what);
static void vdump_l(obj str, obj rest);

static void
vdump_l(obj str, obj rest)
{
	vdump_x(str, car(rest));
	if (!IS_NIL(cdr(rest))) {
		strc(str, ' ');
		if (IS_CONS(cdr(rest))) {
			vdump_l(str, cdr(rest));
		} else {
			strc(str, '.');
			strc(str, ' ');
			vdump_x(str, cdr(rest));
			strc(str, ')');
		}
	} else {
		strc(str, ')');
	}
}

static void
vdump_x(obj str, obj what)
{
	if (IS_T(what)) {
		strc(str, 't');
	} else if (IS_NIL(what)) {
		strx(str, "nil");
	} else {
		switch (TYPE(what)) {
			case OBJ_FIXNUM:
				strf(str, "%li", FNUM(what));
				break;

			case OBJ_CONS:
				strc(str, '(');
				vdump_l(str, what);
				break;

			case OBJ_SYMBOL:
				strf(str, "%s", SYM(what));
				break;

			case OBJ_STRING:
				/* FIXME: need to handle escaped characters... */
				strf(str, "\"%s\"", STR_VAL(what));
				break;

			case OBJ_BUILTIN:
				strf(str, "<#:op:%s:%p:#>", OP_NAME(what), what);
				break;

			case OBJ_LAMBDA:
				strf(str, "<#:lambda:%p:#>", what);
				break;

			default:
				strf(str, "<#:UNKNOWN:%x:%p:#>", TYPE(what), what);
				break;
		}
	}
}

obj
vdump(obj what)
{
	obj str = str_dupc("");
	vdump_x(str, what);
	return str;
}

char*
cdump(obj what)
{
	return STR_VAL(vdump(what));
}

/**************************************************/

obj
cons(obj car, obj cdr)
{
	obj c = OBJECT(OBJ_CONS, 0);
	CAR(c) = car;
	CDR(c) = cdr;
	return c;
}

obj
car(obj cons)
{
	if (!cons) abort("car() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("car() called with non-cons arg");

	return CAR(cons);
}

obj
cdr(obj cons)
{
	if (!cons) abort("cdr() called with NULL cons");
	if (IS_NIL(cons)) return NIL;
	if (!IS_CONS(cons)) abort("cdr() called with non-cons arg");

	return CDR(cons);
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
	FNUM(fixn) = n;
	return fixn;
}

/**  Symbols  ***************************************************/

#define symname(cons) (const char*)untag((car(cons)))

static obj
findsym(unsigned int key, const char *name)
{
	obj rest;
	for_list(rest, SYMBOL_TABLE[key]) {
		if (strcasecmp(name, SYM(car(rest))) == 0) {
			return car(rest);
		}
	}
	return NIL;
}

static obj
mksym(unsigned int key, const char *name)
{
	size_t len = strlen(name)+1;
	obj sym = OBJECT(OBJ_SYMBOL, len);
	strncpy(SYM(sym), name, len);

	push(SYMBOL_TABLE[key], sym);
	return sym;
}

obj
intern(const char *name)
{
	if (!name) return NIL;

	char *symname = lc(name);
	if (strcasecmp(symname, "nil") == 0) return NIL;
	if (strcasecmp(symname, "t") == 0)   return T;

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
eval(obj args, obj env)
{
	debug2("+%s\n", cdump(args));
	/* self-evaluating stuff */
	if (IS_NIL(args)) return NIL;
	if (IS_T(args)) return T;

	if (IS_FIXNUM(args)) return args;
	if (IS_STRING(args)) return args;
	//if (IS_IO(args))     return args;

	/* symbol lookup */
	if (IS_SYMBOL(args)) {
		obj val = get(env, args);
		val = DEF(val) ? val : NIL;
		debug2("lookup %s == %s\n", cdump(args), cdump(val));
		return val;
	}

	if (IS_CONS(args)) {
		obj head = car(args);
		args = cdr(args);

		/* handle special forms like QUOTE and DO */

		/* (quote x) -> x */
		if (head == intern("quote")) {
			debug2("eval::quote\n");
			/* FIXME check args to quote */
			return car(args);

		/* (eval '(+ 1 2 3)) -> call eval again! */
		} else if (head == intern("eval")) {
			debug2("eval::eval (that's so meta)\n");
			/* FIXME check that args is a list of 1 */
			debug1("calling eval again, args = %s\n", cdump(car(args)));
			return eval(car(args), env);

		/* (set x value) */
		} else if (head == intern("set")) {
			debug2("Eval::set\n");
			/* FIXME: what if arg 1 isn't a sym? */
			obj v = eval(car(cdr(args)), env);
			set(env, car(args), v);
			return v;

		/* (let (x y) (...)) */
		} else if (head == intern("let")) {
			debug2("eval::let\n");
			return NIL; /* FIXME: TODO */

		/* (do x y z) -> eval all, return last */
		} else if (head == intern("do")) {
			debug2("eval::do\n");
			obj x, result = NIL;
			for_list(x, args) {
				result = eval(car(x), env);
			}
			return result;

		/* (if cond (then) (else)) -> do the right thing */
		} else if (head == intern("if")) {
			debug2("eval::if\n");
			/* check arity, should be 3 (if (test) (then-do) (else)) */
			obj test = car(args);
			args = cdr(args);

			if (IS_T(eval(test, env))) {
				return eval(car(args), env);
			}

			args = cdr(args);
			return eval(car(args), env);

		/* (and cond1 cond2 ...) -> t if all cond are not NIL */
		} else if (head == intern("and")) {
			debug2("eval::and\n");
			obj cond;
			for_list(cond, args) {
				if (IS_NIL(eval(car(cond), env))) {
					return NIL;
				}
			}
			return T;

		/* (or cond1 cond2 ...) -> t if at least one cond is not NIL */
		} else if (head == intern("or")) {
			debug2("eval::or\n");
			obj cond;
			for_list(cond, args) {
				if (!IS_NIL(eval(car(cond), env))) {
					return T;
				}
			}
			return NIL;

		/* (lambda (arglist) body) -> #lambda */
		} else if (head == intern("lambda")) {
			debug2("eval::lambda\n");
			obj l = OBJECT(OBJ_LAMBDA, 0);
			L_PARAMS(l) = car(args);
			L_BODY(l)   = car(cdr(args));
			return l;

		/* (macro name (args) replacement) -> ... */
		} else if (head == intern("macro")) {
			debug2("eval::macro\n");
			return NIL; /* FIXME: TODO */

		/* normal function application */
		} else {
			debug2("eval::[%s]\n", cdump(head));
			obj new = NIL, x;
			for_list(x, args) {
				new = cons(eval(car(x), env), new);
			}
			new = revl(new);
			return op_call(cons(eval(head, env), new), env);
		}
	}

	fprintf(stderr, "  can't eval %s\n", cdump(args));
	abort("eval not finished");
}

/**  String Handling  *******************************************/

#define STR_BLK(n) ((n) + STR_SEGMENT_SIZE - ((n) % STR_SEGMENT_SIZE))

static void
str_segm(obj s, size_t plus)
{
	if (STR_LEFT(s) > plus) return;

	if (STR_VAL(s)) {
		debug3("STR_SEGM:grow %p from %lu/%lu to %lu/%lu\n",
				STR_VAL(s),
				STR_LEN(s), STR_N(s),
				STR_LEN(s)+plus,STR_N(s)+STR_BLK(plus));
	} else {
		debug3("STR_SEGM:init %p to %lu/%lu\n",
				STR_VAL(s),
				STR_LEN(s)+plus,STR_N(s)+STR_BLK(plus));
	}

	size_t l = STR_N(s) + STR_BLK(plus);
	char *re = realloc(STR_VAL(s), l+1);
	if (!re) ABORT_OOM("str_segm"); /* LCOV_EXCL_LINE */

	STR_N(s) = l;
	STR_VAL(s) = re;

	/* put a null-terminator at len+plus */
	STR_VAL(s)[STR_LEN(s)+plus] = '\0';
}

obj
str_dup(obj s)
{
	return str_dupc(cstr(s));
}

obj
str_dupc(const char *c)
{
	if (!c) return NIL;
	return str_dupb(c, strlen(c));
}

obj
str_dupb(const char *buf, size_t len)
{
	if (!buf) return NIL;
	obj s = OBJECT(OBJ_STRING, 0);

	str_segm(s, len);
	memcpy(STR_VAL(s), buf, len);
	STR_LEN(s) = len;
	return s;
}

obj
str_dupf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	obj s = OBJECT(OBJ_STRING, 0);
	str_segm(s, len);

	va_start(ap, fmt);
	vsnprintf(STR_VAL(s), len+1, fmt, ap);
	STR_LEN(s) = len;
	va_end(ap);

	return s;
}

obj
str_cat(obj dst, obj src)
{
	if (!IS_STRING(dst) || !IS_STRING(src)) return NIL;
	return str_dupf("%s%s", STR_VAL(dst), STR_VAL(src));
}

obj
str_catc(obj dst, const char *src)
{
	if (!IS_STRING(dst)) return NIL;
	if (!src) return str_dup(dst);
	return str_dupf("%s%s", STR_VAL(dst), src);
}

obj
str_catb(obj dst, const char *buf, size_t len)
{
	return str_cat(dst, str_dupb(buf, len));
}

obj
str_catf(obj dst, const char *fmt, ...)
{
	if (!IS_STRING(dst)) return NIL;

	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	char *buf = calloc(len+1, sizeof(char));
	if (!buf) ABORT_OOM("str_catf"); /* LCOV_EXCL_LINE */

	va_start(ap, fmt);
	vsnprintf(buf, len+1, fmt, ap);
	va_end(ap);

	obj s = str_catc(dst, buf);
	free(buf);
	return s;
}

char*
cstr(obj s)
{
	if (!IS_STRING(s)) return NULL;
	return STR_VAL(s);
}

void
strx(obj dst, const char *src)
{
	if (!IS_STRING(dst)) return;

	size_t d_len = STR_LEN(dst);
	size_t s_len = strlen(src);

	str_segm(dst, s_len);
	strncpy(STR_VAL(dst)+d_len, src, s_len+1);
	STR_LEN(dst) += s_len;
}

void
strc(obj dst, char c)
{
	if (!IS_STRING(dst)) return;

	str_segm(dst, 1);
	STR_VAL(dst)[STR_LEN(dst)++] = c;
	STR_VAL(dst)[STR_LEN(dst)] = '\0';
}

void
strf(obj dst, const char *fmt, ...)
{
	if (!IS_STRING(dst)) return;

	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	char *buf = calloc(len+1, sizeof(char));
	if (!buf) ABORT_OOM("strf"); /* LCOV_EXCL_LINE */

	va_start(ap, fmt);
	vsnprintf(buf, len+1, fmt, ap);
	va_end(ap);

	strx(dst, buf);
	free(buf);
}

/**  IO  ********************************************************/

obj
io_fdopen(FILE *file)
{
	obj io = OBJECT(OBJ_IO, 0);
	IO_STR(io) = NULL;
	IO_LEN(io) = IO_IDX(io) = 0;

	IO_FD(io) = file;
	return io;
}

obj
io_fopen(const char *path, const char *mode)
{
	FILE *fd = fopen(path, mode);
	return fd ? io_fdopen(fd) : NIL;
}

obj
io_string(const char *str)
{
	obj io = OBJECT(OBJ_IO, 0);
	IO_FD(io) = NULL;

	IO_STR(io) = strdup(str);
	IO_LEN(io) = strlen(str);
	IO_IDX(io) = 0;
	return io;
}

obj
io_eof(obj io)
{
	if (IO_FD(io)) {
		return feof(IO_FD(io)) ? T : NIL;
	} else {
		return IO_IDX(io) >= IO_LEN(io) ? T : NIL;
	}
}

void
io_rewind(obj io)
{
	/* FIXME: check type of io and str! */
	if (IO_FD(io)) {
		fseek(IO_FD(io), 0, SEEK_SET);
	} else {
		IO_IDX(io) = 0;
	}
}

obj
io_close(obj io)
{
	/* FIXME: check type of io and str! */
	if (IO_FD(io)) {
		fclose(IO_FD(io));
		IO_FD(io) = NULL;
	}

	return T;
}

char
io_getc(obj io)
{
	/* FIXME: check type of io and str! */
	if (IO_FD(io)) {
		return fgetc(IO_FD(io));
	} else {
		if (IO_IDX(io) >= IO_LEN(io)) return EOF;
		return IO_STR(io)[IO_IDX(io)++];
	}

	return EOF;
}

char
io_ungetc(obj io, char c)
{
	/* FIXME: check type of io and str! */
	if (IO_FD(io)) {
		return ungetc(c, IO_FD(io));
	} else {
		if (IO_IDX(io) > 0) {
			IO_STR(io)[--IO_IDX(io)] = c;
			return c;
		}
	}

	return EOF;
}

obj
io_write_str(obj io, obj str)
{
	/* FIXME: check type of io and str! */
	if (IO_FD(io)) {
		int nc = fprintf(IO_FD(io), "%s", STR_VAL(str));
		if (nc < 0) return NIL;
		return T;
	} else {
		/* FIXME: io_write_str needs some work */
		free(IO_STR(io));
		IO_STR(io) = strdup(STR_VAL(str));
		IO_LEN(io) = strlen(IO_STR(io));
		IO_IDX(io) = IO_LEN(io);
		return T;
	}
	return NIL;
}

obj
io_read_buf(obj io, size_t n)
{
	/* FIXME: check type of io and str! */
	obj res = NIL;

	char *buf = calloc(n+1, sizeof(char));
	if (!buf) ABORT_OOM("io_read_buf"); /* LCOV_EXCL_LINE */

	if (IO_FD(io)) {
		if (fread(buf, 1, n, IO_FD(io)) > 0) {
			res = str_dupc(buf);
		}
		free(buf);

	} else {
		strncpy(buf, IO_STR(io)+IO_IDX(io), n);
		IO_LEN(io) += strlen(buf);

		res = str_dupc(buf);
		free(buf);
	}

	return res;
}

/**  Primitive Operators  ***************************************/

/* FIXME: math operations DON'T handle overflow well */

obj
op_add(obj args, obj env)
{
	debug2("+op_add\n");
	long acc = 0;
	obj term;
	for_list(term, args) {
		/* FIXME: op_add: check types! */
		acc += FNUM(car(term));
	}
	return fixnum(acc);
}

obj
op_sub(obj args, obj env)
{
	debug2("+op_sub\n");
	long acc = 0;
	obj term;

	/* FIXME: op_sub: check types! */
	if (IS_NIL(args)) return fixnum(0);
	if (IS_NIL(cdr(args))) {
		acc = 0; /* (- 45) === (- 0 45) */
	} else {
		acc = FNUM(car(args));
		args = cdr(args);
	}

	for_list(term, args) {
		acc -= FNUM(car(term));
	}
	return fixnum(acc);
}

obj
op_mult(obj args, obj env)
{
	debug2("+op_mult\n");
	long acc = 1;
	obj term;
	for_list(term, args) {
		acc *= FNUM(car(term));
	}
	return fixnum(acc);
}

obj
op_div(obj args, obj env)
{
	debug2("+op_div\n");
	long acc;
	obj term;

	/* FIXME: op_div: check types and arity (i.e. (/) -> ABORT) */
	if (IS_NIL(args)) abort("/: wrong number of arguments");
	if (IS_NIL(cdr(args))) {
		acc = 1; /* (/ 4) == 1/4 */
		abort("/: no ratio support; ergo, no (/ x) support -- go bug jrh");
	} else {
		acc = FNUM(car(args));
		args = cdr(args);
	}

	for_list(term, args) {
		acc /= FNUM(car(term));
	}
	return fixnum(acc);
}

obj
op_call(obj args, obj env)
{
	debug2("+op_call\n");
	return op_apply(nlist(2, car(args), cdr(args)), env);
}

obj
op_apply(obj args, obj env)
{
	debug2("+op_apply\n");
	obj fn = car(args);
	args = car(cdr(args));
	debug1("applying %s with args %s\n", cdump(fn), cdump(args));

	if (!fn) abort("fn cannot be NULL");
	if (IS_BUILTIN(fn)) {
		return (*(OP_FN(fn)))(args, env);

	} else if (IS_LAMBDA(fn)) {
		debug1("apply:lambda code is... %s\n", cdump(L_BODY(fn)));
		env_new(env);
		obj a, p;
		for (a = args, p = L_PARAMS(fn);
		     !IS_NIL(a) && !IS_NIL(p);
		     a = cdr(a), p = cdr(p)) {

			set(env, car(p), car(a));
		}
		debug1("apply:calling lambda with e: %s\n", cdump(env));
		debug1("apply:lambda code is... %s\n", cdump(L_BODY(fn)));
		obj v = eval(L_BODY(fn), env);
		env_del(env);
		debug1("RETURN %s\n", cdump(v));
		return v;

	} else {
		debug1("applying function %s\n", cdump(fn));
		abort("called apply on non-function");
	}
}

obj
op_eq(obj args, obj env)
{
	debug2("+op_eq\n");
	/* FIXME: check arity */
	return eq(
		car(args),
		car(cdr(args)));
}

obj
op_eql(obj args, obj env)
{
	debug2("+op_eql\n");
	/* FIXME: check arity */
	return eql(
		car(args),
		car(cdr(args)));
}

obj
op_equal(obj args, obj env)
{
	debug2("+op_equal\n");
	/* FIXME: check arity */
	return equal(
		car(args),
		car(cdr(args)));
}

obj
op_cons(obj args, obj env)
{
	debug2("+op_cons\n");
	/* FIXME: check arity */
	return cons(car(args), car(cdr(args)));
}

obj
op_list(obj args, obj env)
{
	debug2("+op_list\n");
	return args; /* FIXME: eval each arg */
}

obj
op_car(obj args, obj env)
{
	debug2("+op_car\n");
	/* FIXME: check arity */
	return car(car(args));
}

obj
op_cdr(obj args, obj env)
{
	debug2("+op_cdr\n");
	/* FIXME: check arity */
	return cdr(car(args));
}

obj
op_prs(obj args, obj env)
{
	debug2("+op_prs\n");
	obj STDOUT = io_fdopen(stdout);
	obj x, s;
	for_list(x, args) {
		/* FIXME: this really should move to in-lang */
		if (IS_STRING(car(x))) {
			s = car(x);
		} else if (IS_FIXNUM(car(x))) {
			s = str_dupf("%i", FNUM(car(x)));
		} else {
			/* extra quotes to make gcc stop bitching about ??? trigraphs */
			s = str_dupc("(?""?""?)");
		}
		io_write_str(STDOUT, s);
	}
	return T;
}

obj
op_typeof(obj args, obj env)
{
	debug2("+op_typeof\n");
	/* FIXME: check arity of args */
	obj x = car(args);
	if (IS_T(x))       return intern("BOOLEAN");
	if (IS_CONS(x))    return intern("CONS");
	if (IS_SYMBOL(x))  return intern("SYMBOL");
	if (IS_FIXNUM(x))  return intern("FIXNUM");

	if (IS_BUILTIN(x)) return intern("FUNCTION");
	if (IS_LAMBDA(x))  return intern("FUNCTION");

	if (IS_STRING(x))  return intern("STRING");
	if (IS_IO(x))      return intern("IO");

	return intern("NULL");
}

obj
op_exit(obj args, obj env)
{
	debug2("+op_exit\n");
	obj x = car(args);
	if (IS_NIL(x))     exit(0);
	if (IS_FIXNUM(x))  exit(FNUM(x));
	exit(255);
}

obj
opx_load(obj args, obj env)
{
	debug2("+opx_load\n");
	if (IS_STRING(car(args))) {
		return load(STR_VAL(car(args)), env);
	}
	abort("called load without a valid path argument");
}

obj
load(const char *path, obj env)
{
	obj f = io_fopen(path, "r");
	debug1("loadsys... %s\n", path);
	if (IS_NIL(f)) {
		debug1("%s - failed to load\n", path);
		return NIL;
	}

	while (IS_NIL(io_eof(f))) {
		debug1(" ...\n");
		eval(readx(f), env);
	}

	io_close(f);
	debug1("done... %s\n", path);
	return T;
}
