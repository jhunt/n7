#include "gc.h"

#define GC_BLOCK_USED 0x01

typedef struct {
	size_t len;  /* in # conses */
	char data[];
} HEAP;

/* semispaces */
static HEAP *FROM, *TO;

/* Add 8 header bits to the CONS */
#define BLOCK_SIZE (sizeof(CONS)+1)

static inline HEAP*
mkheap(size_t len)
{
	HEAP *heap = calloc(1, sizeof(HEAP) + (BLOCK_SIZE * len));
	if (!heap) {
		return NULL;
	}
	heap->len = len;
	return heap;
}

#define heap_start(h) ((h)->data)
#define heap_end(h)   (heap_start(h) + ((h)->len * BLOCK_SIZE))

#define heap_ptr(p) ((char*)(p)-1)
#define heap_val(p) (CONS*)((char*)(p)+1)

#define in_heap(p,h) ((char*)(p) > (heap_start(h)) && (char*)(p) < (heap_end(h)))

#define test(x,f) (((x) & 0xff) & ~(f) == (f))
#define flag(x,f) ((x) = ((x) | (f)))

#define is_free(blk)  (!test((*(blk)),GC_BLOCK_USED))
#define mark_used(blk) (flag((*(blk)),GC_BLOCK_USED))

void gc_init(size_t len)
{
	TO   = mkheap(len);
	FROM = mkheap(len);

	if (!TO || !FROM) {
		exit(43);
	}
}

CONS* xalloc(void)
{
	char *ptr = heap_start(FROM);
	char *end = heap_end(FROM);

	for (; ptr < end; ptr += BLOCK_SIZE) {
		if (is_free(ptr)) {
			mark_used(ptr);
			return heap_val(ptr);
		}
	}
	exit(37);
}

/* I don't think we even need this... */
void xfree(CONS *cons)
{
	if (!in_heap(cons, FROM)) {
		return;
	}
	char *ptr = heap_ptr(cons);
	*ptr = 0x00; /* clear header bits */
}

void gc(void)
{
	HEAP *t = TO;
	TO = FROM;
	FROM = t;

	/* FIXME: need roots! */
}
