#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "../pipeline.h"

struct reader_opaque {
	int source;    /* file descriptor to read from   */
	size_t line;   /* line number of read head       */
	size_t column; /* column number of read head     */
	int eof;       /* set to non-zero on end-of-file */

	utf8 buffer[BUFSIZ]; /* buffer for i/o operations          */
	size_t read;         /* where to start reading from buffer */
	size_t fill;         /* where to start writing to buffer   */
};

reader_t reader_new(int fd) {
	reader_t r = calloc(1, sizeof(struct reader_opaque));
	if (!r) {
		return NULL;
	}

	r->source = fd;
	r->line = 1;
	return r;
}

size_t reader_line(reader_t r)
{
	assert((reader_t)r != NULL);
	return r->line;
}

size_t reader_column(reader_t r)
{
	assert((reader_t)r != NULL);
	return r->column;
}

int reader_eof(reader_t r)
{
	assert((reader_t)r != NULL);
	return r->eof;
}

#define unread(r)   ((r)->fill - (r)->read)
#define unfilled(r) (BUFSIZ - (r)->fill - 1)

static int s_fill(reader_t r, size_t atleast)
{
	ssize_t n;

	if (atleast < unread(r)) {
		return 1;
	}

	if (atleast > unfilled(r)) {
		if (r->read == 0) {
			/* can't shift the buffer.
			   caller asks too much of us. */
			return 0;
		}

		memmove(r->buffer, r->buffer + r->read, unread(r));
		r->fill -= r->read;
		r->read = 0;
	}

	n = read(r->source, r->buffer, unfilled(r));
	if (n < 0) {
		/* propagate errors to caller */
		return 0;
	}
	if (!n) {
		r->eof = 1;
	}
	r->fill += n;
	return 1;
}

codepoint reader_next(reader_t r)
{
	codepoint c;
	utf8 u;

	if (r->eof) {
		return READER_EOF;
	}

	if (!s_fill(r, 1)) {
		fprintf(stderr, "failed to read: %s (errno %i)\n", strerror(errno), errno);
		exit(1);
	}

	if (r->eof) {
		return READER_EOF;
	}

	u = r->buffer[r->read++];
	if (u <= 0x7f) {
		return (codepoint)u;
	}
	if (0xc0 == (u & 0xe0)) { /* 110x xxxx  10xx xxxx*/
		if (r->fill <= r->read + 1) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill, r->read);
			exit(1);
		}

		c = (u & ~0xe0) << 6;
		c += (r->buffer[r->read++] & ~0xc0);
		return c;
	}
	if (0xe0 == (u & 0xf0)) { /* 1110 xxxx  10xx xxxx  10xx xxxx */
		if (r->fill <= r->read + 2) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill, r->read);
			exit(1);
		}
		c = (u & ~0xf0) << 12;
		c += (r->buffer[r->read++] & ~0xc0) << 6;
		c += (r->buffer[r->read++] & ~0xc0);
		return c;
	}
	if (0xf0 == (u & 0xf8)) { /* 1111 0xxx  10xx xxxx  10xx xxxx 10xx xxxx*/
		if (r->fill <= r->read + 3) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill, r->read);
			exit(1);
		}
		c = (u & ~0xf8) << 18;
		c += (r->buffer[r->read++] & ~0xc0) << 12;
		c += (r->buffer[r->read++] & ~0xc0) << 6;
		c += (r->buffer[r->read++] & ~0xc0);
		return c;
	}

	return READER_ERR;
}
