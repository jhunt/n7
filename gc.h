#ifndef GC_H
#define GC_H

#include <stdlib.h>
#include "core.h"

void gc_init(size_t heap_size);

CONS* xalloc(void);
void xfree(CONS *cons);

void gc(void);

#endif
