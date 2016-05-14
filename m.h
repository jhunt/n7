/*
  Copyright (c) 2012-2016 James Hunt

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software..

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
 */
#ifndef M_H
#define M_H

/**


       m: a Lisp dialect for defining n7


 **/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define TYPE_NIL    0
#define TYPE_T      1
#define TYPE_CONS   2
#define TYPE_SYMBOL 3
#define TYPE_FIXNUM 4
#define TYPE_STRING 5
#define TYPE_LAMBDA 6

typedef struct tvalue* obj;

struct tvalue {
	uint8_t type;
	union {
		struct {
			void *reserved; /* for future expansion */
			char name[];
		} symbol;

		struct {
			obj car;
			obj cdr;
		} cons;

		struct {
			obj args;
			obj body;
		} lambda;

		long fixnum;

		struct {
			size_t len;
			char *data;
		} string;

	} val;
};

static struct tvalue v_NIL = { TYPE_NIL };
static obj NIL = &v_NIL;

static struct tvalue v_T   = { TYPE_T   };
static obj T = &v_T;

obj cons(obj a, obj d);
obj car(obj lst);
obj cdr(obj lst);
obj eq(obj a, obj b);

obj get(obj env, obj sym);
obj set(obj env, obj sym, obj val);

obj intern(const char *name);
obj readf(FILE *in, obj env);
obj eval(obj e, obj env);

#endif
