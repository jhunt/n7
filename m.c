#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "m.h"

#define push(ls,v) (ls) = cons((v),(ls))
#define for_list(obj, list) \
	for (obj = (list); (obj)->type != TYPE_NIL; obj = cdr(obj))

static void
abort1(const char *msg)
{
	fprintf(stderr, "ABORT: %s\n", msg);
	exit(42);
}

static obj
OBJECT(unsigned short type, size_t varlen)
{
	obj o = calloc(1,sizeof(struct tvalue)+varlen);
	if (!o) abort1("oom: object");
	o->type = type;
	return o;
}

static char*
lc(const char *s)
{
	char *p, *new = strdup(s);
	if (!new) abort1("oom: lc");
	for (p = new; *p; p++) {
		if (isupper(*p)) *p = tolower(*p);
	}
	return new;
}

static obj
revl(obj lst)
{
	/* FIXME: type checking in revl! */
	obj t, new = NIL;
	for_list(t, lst) {
		push(new, car(t));
	}
	return new;
}

static void dumpx(FILE *io, obj what);
static void dumpl(FILE *io, obj rest);

static void
dumpl(FILE *io, obj rest)
{
	dumpx(io, car(rest));
	if ((cdr(rest))->type != TYPE_NIL) {
		fprintf(io, " ");
		if ((cdr(rest))->type == TYPE_CONS) {
			dumpl(io, cdr(rest));
		} else {
			fprintf(io, ". ");
			dumpx(io, cdr(rest));
			fprintf(io, ")");
		}
	} else {
		fprintf(io, ")");
	}
}

static void
dumpx(FILE *io, obj what)
{
	switch (what->type) {
		case TYPE_NIL:
			fprintf(io, "NIL");
			break;

		case TYPE_T:
			fprintf(io, "T");
			break;

		case TYPE_FIXNUM:
			fprintf(io, "%li", what->val.fixnum);
			break;

		case TYPE_CONS:
			fprintf(io, "(");
			dumpl(io, what);
			break;

		case TYPE_SYMBOL:
			fprintf(io, "%s", what->val.symbol.name);
			break;

		case TYPE_STRING:
			/* FIXME: need to handle escaped characters... */
			fprintf(io, "\"%s\"", what->val.string.data);
			break;

		case TYPE_LAMBDA:
			fprintf(io, "<#:lambda:%p:#>", what);
			break;

		default:
			fprintf(io, "<#:lambda:%x:%p:#>", what->type, what);
			break;
	}
}

/**************************************************/

#define SYMBOL_TABLE_SIZE 211
obj SYMBOL_TABLE[SYMBOL_TABLE_SIZE];

/**  Initialization  ********************************************/

static obj
globals(void)
{
	unsigned int i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		SYMBOL_TABLE[i] = NIL;
	}

	obj env = cons(cons(intern("t"), T), NIL);
	set(env, intern("nil"), NIL);

	return env;
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
	if (a != NIL && a == b) return T;
	return NIL;
}

/**************************************************/

#define BUF_MAX 8192
static int lineno = 1;

static char*
next_token(FILE *io)
{
	char buf[BUF_MAX] = {0};
	size_t n = 0;
	char c;
next_char:
	switch (c = fgetc(io)) {
		case EOF:
			break;

		case '\n':
			lineno++;
		case ' ':
		case '\t':
		case '\r':
			if (n > 0) break;
			goto next_char;

		case ';': /* Lisp-style comments */
			for (c = fgetc(io); c != EOF && c != '\n'; c = fgetc(io))
				;
			if (n > 0) break;
			goto next_char;

		case '(':
		case ')':
		case '`':
		case '\'':
			if (n > 0) {
				ungetc(c, io);
				break;
			}
			// explicit fall-through

		default:
			if (n == BUF_MAX - 1) abort1("buffer overrun");
			buf[n++] = c;
			goto next_char;
	}

	if (n == 0) return NULL;
	return strdup(buf);
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

	obj fixnum = OBJECT(TYPE_FIXNUM, 0);
	fixnum->val.fixnum = fixn;
	return fixnum;
}

#define READ_STRING_MAX 100

static obj
read_string(FILE *io)
{
	char buf[BUF_MAX] = {0};
	size_t n = 0;
	char c;
	int esc = 0;

again:
	c = fgetc(io);
	if (c == EOF || (c == '"' && !esc)) {
		obj s = OBJECT(TYPE_STRING, 0);
		s->val.string.len = n;
		s->val.string.data = strdup(buf);
		return s;
	}

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

	if (n > BUF_MAX - 1) abort1("read string buffer overrun");
	buf[n++] = c;
	goto again;
}

#define CLOSE_PAREN NULL

static obj
read_list(FILE *io, obj env)
{
	size_t nread = 1;
	obj first, next, rest;
	first = next = readf(io, env);

	if (next == CLOSE_PAREN) return NIL;

	obj lst = NIL;
	for (; next != CLOSE_PAREN; next = readf(io, env)) {
		push(lst, next);
	}

	return revl(lst);
}

obj
readf(FILE *io, obj env)
{
	char *token, c;
	obj val;

	token = NULL;

again:
	c = fgetc(io);
	switch (c) {
		case EOF:
			return NIL;

		case '\n':
			lineno++;
		case ' ':
		case '\t':
		case '\r':
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
			val = read_list(io, env);
			break;

		case ')':
			val = CLOSE_PAREN;
			break;

		case '"':
			val = read_string(io);
			break;

		case ';':
			for (c = fgetc(io); c != EOF && c != '\n'; c = fgetc(io))
				;
			goto again;

		default:
			ungetc(c, io);
			token = next_token(io);
			val = intern(token);
			break;
	}

	free(token);
	return val;
}

/**************************************************/

obj
cons(obj car, obj cdr)
{
	obj c = OBJECT(TYPE_CONS, 0);
	c->val.cons.car = car;
	c->val.cons.cdr = cdr;
	return c;
}

obj
car(obj cons)
{
	return cons->val.cons.car;
}

obj
cdr(obj cons)
{
	return cons->val.cons.cdr;
}

/**  Symbols  ***************************************************/

static obj
findsym(unsigned int key, const char *name)
{
	obj rest;
	for_list(rest, SYMBOL_TABLE[key]) {
		if (strcasecmp(name, (car(rest))->val.symbol.name) == 0) {
			return car(rest);
		}
	}
	return NIL;
}

static obj
mksym(unsigned int key, const char *name)
{
	size_t len = strlen(name)+1;
	obj sym = OBJECT(TYPE_SYMBOL, len);
	strncpy(sym->val.symbol.name, name, len);

	push(SYMBOL_TABLE[key], sym);
	return sym;
}

obj
intern(const char *name)
{
	if (!name) return NIL;

	char *symname = lc(name);
	unsigned int key = hash(symname, SYMBOL_TABLE_SIZE);
	obj sym = findsym(key, symname);

	if (sym->type == TYPE_NIL) {
		sym = mksym(key, symname);
	}

	free(symname);
	return sym;
}

/**  Evaluation  ************************************************/

obj
get(obj env, obj sym)
{
	obj x;
	for_list(x, env) {
		if ((eq(car(car(x)), sym))->type == TYPE_T) {
			return cdr(car(x));
		}
	}
	fprintf(stderr, "DBG: %s sym not found\n", sym->val.symbol.name);
	abort1("var access error");
}

obj
set(obj env, obj sym, obj val)
{
	obj new = cons(sym, val);
	obj tmp = cons(car(env), cdr(env));
	env->val.cons.cdr = tmp;
	env->val.cons.car = new;

	return env;
}

static obj
evlis(obj args, obj env)
{
	obj x, new = NIL;
	for_list(x, args) { // evlis!
		new = cons(eval(car(x), env), new);
	}
	return revl(new);
}

static obj
extend(obj env, obj vars, obj vals)
{
	if (vars->type == TYPE_CONS) {
		if (vals->type == TYPE_CONS) {
			return cons(cons(car(vars), car(vals)),
			            extend(env, cdr(vars), cdr(vals)));
		} else {
			abort1("Too few values");
		}
	} else {
		if (vars->type == TYPE_NIL) {
			if (vals->type == TYPE_NIL) {
				return env;
			} else {
				abort1("Too many values for (extend)");
			}
		} else {
			if (vars->type == TYPE_SYMBOL) {
				return cons(cons(vars, vals), env);
			} else {
				fprintf(stderr, "extend failed!\nvals: ");
				dumpx(stderr, vals); fprintf(stderr, "\n");
				fprintf(stderr, "vars: ");
				dumpx(stderr, vars); fprintf(stderr, "\n");
				abort1("No idea how to extend this");
			}
		}
	}
}

static obj
invoke(obj fn, obj args, obj env)
{
	obj env2 = extend(env, fn->val.lambda.args, args);
	obj exprs, v = NIL;
	for_list(exprs, fn->val.lambda.body) {
		v = eval(car(exprs), env2);
	}
	return v;
}

obj
eval(obj args, obj env)
{
	/* self-evaluating stuff */
	if (args->type == TYPE_CONS) {
		obj head = car(args);
		obj rest = cdr(args);

		/* (quote x) -> x */
		if (head == intern(".quote")) {
			return car(rest);

		/* (set x value) */
		} else if (head == intern(".set!")) {
			obj v = eval(car(cdr(rest)), env);
			set(env, car(rest), v);
			return v;

		/* (if cond (then) (else)) -> do the right thing */
		} else if (head == intern(".if")) {
			obj test = car(rest);
			obj then = car(cdr(rest));
			obj othe = car(cdr(cdr(rest)));

			if ((eval(test, env))->type == TYPE_T) {
				return eval(then, env);
			} else {
				return eval(othe, env);
			}

		/* (lambda (args) body) -> #lambda */
		} else if (head == intern(".lambda")) {
			obj l = OBJECT(TYPE_LAMBDA, 0);
			l->val.lambda.args = car(rest);
			l->val.lambda.body = cdr(rest);
			return l;

		/* not-so-special forms */
		} else if (head == intern("-cons")) {
			rest = evlis(rest, env);
			return cons(car(rest), car(cdr(rest)));

		} else if (head == intern("-car")) {
			rest = evlis(rest, env);
			return car(car(rest));

		} else if (head == intern("-cdr")) {
			rest = evlis(rest, env);
			return cdr(car(rest));

		} else if (head == intern("-eq?")) {
			rest = evlis(rest, env);
			return eq(car(rest), car(cdr(rest)));

		} else if (head == intern("-typeof")) {
			rest = evlis(rest, env);
			switch ((car(rest))->type) {
				case TYPE_NIL:    return intern("nil");
				case TYPE_T:      return intern("t");
				case TYPE_CONS:   return intern("cons");
				case TYPE_FIXNUM: return intern("number");
				case TYPE_SYMBOL: return intern("symbol");
				case TYPE_STRING: return intern("string");
				case TYPE_LAMBDA: return intern("function");
				default: return NIL;
			}

		} else if (head == intern("-set-cdr!")) {
			rest = evlis(rest, env);
			(car(rest))->val.cons.cdr = car(cdr(rest));
			return (car(rest))->val.cons.cdr;

		} else if (head == intern("-fail")) {
			abort1(car(rest)->val.string.data);

		/* normal function application */
		} else {
			obj fn = eval(head, env);
			rest = evlis(rest, env);
			return invoke(fn, rest, env);
		}

	} else {
		/* symbol lookup */
		if (args->type == TYPE_SYMBOL) {
			obj val = get(env, args);
			return val;
		}

		return args;
	}
	abort1("eval died");
}

int main(int argc, char **argv)
{
	obj env = globals();
	FILE *stdlib = fopen("m.n7", "r");
	while (!feof(stdlib)) {
		eval(readf(stdlib, env), env);
	}
	fclose(stdlib);

	while (!feof(stdin)) {
		printf("> ");
		dumpx(stdout, eval(readf(stdin, env), env));
		fprintf(stdout, "\n");
	}
	return 0;
}

/*

NEED:
 - vm architecture
 - p-code design
 - p-code interpreter
 - p-code compiler

  */
