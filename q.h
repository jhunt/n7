#ifndef Q_H
#define Q_H

#include <stdlib.h>

/* structure-agnostic lists, for house-keeping */

struct q_head;
typedef struct q_head {
	struct q_head *next;
	struct q_head *prev;
} Q;

#define q_node(q, type, attr) \
	((type *)((char *)(q)-(unsigned long)(&((type *)0)->attr)))

#define for_each_loop(q, i, attr, dir) \
	for (i = q_node((q)->dir, typeof(*i), attr); \
	     &i->attr != (q); \
	     i = q_node(i->attr.dir, typeof(*i), attr))

#define for_each_loop_safe(q, i, tmp, attr, dir) \
	for (i = q_node((q)->dir, typeof(*i), attr), \
		 tmp = q_node(i->attr.dir, typeof(*i), attr); \
		 &i->attr != (q); \
		 i = tmp, \
		 tmp = q_node(i->attr.dir, typeof(*i), attr))

#define for_each(q, i, attr) \
	for_each_loop(q, i, attr, next)

#define for_each_r(q, i, attr) \
	for_each_loop(q, i, attr, prev)

#define for_each_safe(q, i, tmp, attr) \
	for_each_loop_safe(q, i, tmp, attr, next)

#define for_each_safe_r(q, i, tmp, attr) \
	for_each_loop_safe(q, i, tmp, attr, prev)

static inline void q_init(Q *q)
{
	q->next = q->prev = q;
}

static inline Q* q_new(void)
{
	Q *q = malloc(sizeof(Q));
	if (!q) {
		return NULL;
	}
	q_init(q);
	return q;
}

static inline int q_empty(Q *q)
{
	return q->next == q->prev;
}

static inline int q_splice(Q *a, Q *b)
{
	b->prev = a;
	a->next = b;
}

static inline void q_remove(Q *q)
{
	q_splice(q->prev, q->next);
}

static inline void q_insert(Q *prev, Q *n, Q *next)
{
	q_splice(prev, n);
	q_splice(n, next);
}

static inline void q_append(Q *l, Q *n)
{
	q_insert(l->prev, n, l);
}

static inline void q_prepend(Q *l, Q *n)
{
	q_insert(l, n, l->next);
}

#endif
