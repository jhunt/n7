CC := gcc

CFLAGS := -Wall -lc
CFLAGS := -g
CFLAGS += -fprofile-arcs -ftest-coverage
CFLAGS += -lgcov

LDFLAGS := -fprofile-arcs

LCOV := lcov --directory . --base-directory .
GENHTML := genhtml --prefix $(shell dirname `pwd`)

TEST_FILES := $(shell find t -name '*.c' | grep -v 'assert.c' | sort | sed -e 's/.c$$/.t/')
BINARIES := n7i

all: test $(BINARIES)

test: test0 test1
test0: $(TEST_FILES)
	prove -o -e '' $+

test1: t/*.n7
	prove -o -e '' $+

t/%.n7: n7i

coverage:
	make clean test
	$(LCOV) --capture -o $@.tmp
	$(LCOV) --remove $@.tmp t/* > lcov.info
	rm -f $@.tmp
	rm -rf doc/coverage
	mkdir -p doc
	$(GENHTML) -o doc/coverage lcov.info
	@echo
	@echo file://$(PWD)/doc/coverage/index.html
	@echo

fixme:
	find . -name '*.[ch]' | xargs grep -in --color FIXME:

clean:
	find . -name '*.o' | xargs -r rm -f
	find . -name '*.gc??' | xargs -r rm -f
	rm -f t/*.t
	rm -rf doc/coverage
	rm -f $(BINARIES)

n7i: n7i.o core.o

t/%.t: t/%.o core.o t/assert.o t/test.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< core.o t/assert.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $+
