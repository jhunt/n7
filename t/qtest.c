#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../q.h"

static int q_dump(const char *tag, Q *q)
{
	printf("%s: %p <-- [%p] --> %p\n", tag, q->prev, q, q->next);
}

struct num
{
	Q list;
	int n;
};

static void line(const char *msg)
{
	char line[80];
	memset(line, '-', 80);
	size_t end;
	end = 80 - strlen(msg) - 4;
	if (end > 79) { end = 79; }

	line[end] = '\0';
	printf("-- %s %s\n", msg, line);
}

int main_primitives(int argc, char **argv)
{
	Q *a = q_new();
	Q *b = q_new();
	Q *c = q_new();

	line ("initial values");
	q_init(a); q_init(b); q_init(c);
	q_dump("a", a); q_dump("b", b); q_dump("c", c);


	line("(a + b + c)");
	q_init(a); q_init(b); q_init(c);
	q_append(a, b);
	q_append(a, c);
	q_dump("a", a); q_dump("b", b); q_dump("c", c);

	line("prepend");
	q_init(a); q_init(b); q_init(c);
	q_prepend(b, c);
	q_prepend(a, b);
	q_dump("a", a); q_dump("b", b); q_dump("c", c);

	line("a + b + c - b");
	q_init(a); q_init(b); q_init(c);
	q_append(a, b);
	q_append(a, c);
	q_remove(b);
	q_dump("a", a); q_dump("b", b); q_dump("c", c);

	line("a - a");
	q_init(a); q_init(b); q_init(c);
	q_remove(a);
	q_dump("a", a); q_dump("b", b); q_dump("c", c);

	return 0;
}

static void dump_list(Q *l)
{
	struct num *n;
	printf("(");
	for_each(l, n, list) {
		printf("%u -> ", n->n);
	}
	printf("nil)\n");
}

static void dump_list_r(Q *l)
{
	struct num *n;
	printf("(");
	for_each_r(l, n, list) {
		printf("%u -> ", n->n);
	}
	printf("nil)\n");
}

static void dump_list_free(Q *l)
{
	struct num *n, *tmp;
	printf("(");
	for_each_safe(l, n, tmp, list) {
		printf("%u -> ", n->n);
		q_remove(&n->list);
		free(n);
	}
	printf("nil)\n");
}

int main(int argc, char **argv)
{
	struct num *a = malloc(sizeof(struct num));
	a->n = 1;
	q_init(&a->list);

	struct num *b = malloc(sizeof(struct num));
	b->n = 2;
	q_init(&b->list);

	struct num *c = malloc(sizeof(struct num));
	c->n = 3;
	q_init(&c->list);

	Q *l = q_new();
	dump_list(l); dump_list_r(l);

	q_append(l, &a->list);
	dump_list(l); dump_list_r(l);

	q_append(l, &b->list);
	dump_list(l); dump_list_r(l);

	q_append(l, &c->list);
	dump_list(l); dump_list_r(l);

	dump_list_free(l);
	return 0;
}

