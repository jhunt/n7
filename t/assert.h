#ifndef ASSERT_H
#define ASSERT_H

#include "test.h"
#include "../core.h"

/* kids, don't ever abuse C like this.  it's wrong. */
#define __jmpvar__ __I_PROMISE_NEVER_TO_ABUSE_C_MACROS_LIKE_THIS_EVER_AGAIN__
#define WITH_ABORT_PROTECTION \
	jmp_buf __jmpvar__; \
	on_abort(&__jmpvar__); \
	if (setjmp(__jmpvar__) != 0) { bail("ABORTED ABNORMALLY"); } else

char* str(const char *fmt, ...);

void sym_is(const char *a, const char *b, const char *msg);
void sym_isnt(const char *a, const char *b, const char *msg);

void fixnum_is(obj got, long expect, const char *msg);
void fixnum_isnt(obj got, long expect, const char *msg);

void vstring_is(obj got, const char *expect, const char *msg);
void vstring_isnt(obj got, const char *expect, const char *msg);

void obj_equal(obj got, obj exp, const char *msg);
void obj_eql(obj got, obj exp, const char *msg);
void obj_eq(obj got, obj exp, const char *msg);

void is_defined(obj x, const char *msg);
void isnt_defined(obj x, const char *msg);

#endif
