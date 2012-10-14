#ifndef ASSERT_H
#define ASSERT_H

#include "test.h"

char* str(const char *fmt, ...);

void sym_is(const char *a, const char *b, const char *msg);

#endif
