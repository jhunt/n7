#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>

#define READER_EOF ~(codepoint)(0)
#define READER_ERR ~(codepoint)(1)

typedef uint8_t utf8;
typedef uint32_t codepoint;
typedef struct reader_opaque* reader_t;

reader_t reader_new(int);
size_t reader_line(reader_t);
size_t reader_column(reader_t);
int reader_eof(reader_t);
codepoint reader_next(reader_t);

#endif
