CC := gcc

CFLAGS := -Wall -lc
CFLAGS := -g
CFLAGS += -fprofile-arcs -ftest-coverage
CFLAGS += -lgcov

LDFLAGS := -fprofile-arcs

LCOV := lcov --directory . --base-directory .
GENHTML := genhtml --prefix $(shell dirname `pwd`)

TEST_FILES := $(shell find t -name '*.c' | grep -v 'assert.c' | sed -e 's/.c$$/.t/')
BINARIES := repl syms sizes


all: test $(BINARIES)

test: $(TEST_FILES)
	prove -o $+

coverage:
	$(LCOV) --capture -o $@.tmp
	$(LCOV) --remove $@.tmp > lcov.info
	rm -f $@.tmp
	rm -rf doc/coverage
	mkdir -p doc
	$(GENHTML) -o doc/coverage lcov.info
	@echo
	@echo file://$(PWD)/doc/coverage/index.html
	@echo

repl: repl.o core.o
syms: syms.o core.o

clean:
	find . -name '*.o' | xargs -r rm -f
	find . -name '*.gc??' | xargs -r rm -f
	rm -f t/*.t
	rm -f repl sizes

t/%.t: t/%.o core.o t/assert.o t/test.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< core.o t/assert.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $+
