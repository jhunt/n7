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
	size_t read_at;      /* where to start reading from buffer */
	size_t fill_at;      /* where to start writing to buffer   */
};

reader_t reader_new(int fd) {
	reader_t r = calloc(1, sizeof(reader_t));
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

codepoint reader_next(reader_t r)
{
	codepoint c;
	utf8 u;

	if (r->eof) {
		return READER_EOF;
	}

	if (r->read_at == r->fill_at) {
		ssize_t n;

		n = read(r->source, r->buffer + r->fill_at, BUFSIZ - r->fill_at);
		if (n < 0) {
			fprintf(stderr, "failed to read: %s (errno %i)\n", strerror(errno), errno);
			exit(1);
		}
		if (!n) {
			r->eof = 1;
			return READER_EOF;
		}
		r->fill_at += n;
	}

	u = r->buffer[r->read_at++];
	if (u <= 0x7f) {
		return (codepoint)u;
	}
	if (0xc0 == (u & 0xe0)) { /* 110x xxxx  10xx xxxx*/
		if (r->fill_at <= r->read_at + 1) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill_at, r->read_at);
			exit(1);
		}

		c = (u & ~0xe0) << 6;
		c += (r->buffer[r->read_at++] & ~0xc0);
		return c;
	}
	if (0xe0 == (u & 0xf0)) { /* 1110 xxxx  10xx xxxx  10xx xxxx */
		if (r->fill_at <= r->read_at + 2) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill_at, r->read_at);
			exit(1);
		}
		c = (u & ~0xf0) << 12;
		c += (r->buffer[r->read_at++] & ~0xc0) << 6;
		c += (r->buffer[r->read_at++] & ~0xc0);
		return c;
	}
	if (0xf0 == (u & 0xf8)) { /* 1111 0xxx  10xx xxxx  10xx xxxx 10xx xxxx*/
		if (r->fill_at <= r->read_at + 3) {
			fprintf(stderr, "FIXME: short read (fill %lu / read %lu)\n", r->fill_at, r->read_at);
			exit(1);
		}
		c = (u & ~0xf8) << 18;
		c += (r->buffer[r->read_at++] & ~0xc0) << 12;
		c += (r->buffer[r->read_at++] & ~0xc0) << 6;
		c += (r->buffer[r->read_at++] & ~0xc0);
		return c;
	}

	return READER_ERR;
}
