CFLAGS := -g

TEST_FILES := $(shell find t -name '*.c' | grep -v 'assert.c' | sed -e 's/.c$$/.t/')
BINARIES := repl syms sizes

all: test $(BINARIES)

test: $(TEST_FILES)
	prove -o $+

repl: repl.o core.o
syms: syms.o core.o

clean:
	rm -f *.o
	rm -f repl
	rm -f syms
	rm -f sizes
	rm -f t/*.t t/*.o

t/%.t: t/%.o core.o t/assert.o t/test.h
	$(CC) -o $@ $< core.o t/assert.o

