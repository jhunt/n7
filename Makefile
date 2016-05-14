CFLAGS := -Wall -lc
CFLAGS := -g
CFLAGS += -fprofile-arcs -ftest-coverage
CFLAGS += -lgcov

LDFLAGS := -fprofile-arcs

LCOV := lcov --directory . --base-directory .
GENHTML := genhtml --prefix $(shell dirname `pwd`)

TEST0_FILES := $(shell find t -name '*.c' | sort | sed -e 's/.c$$/.t/')
TEST1_FILES := $(shell find t -name '*.n7')
BINARIES := n7i

all: test $(BINARIES)

test: test0 test1
test0: $(TEST0_FILES)
	prove -o -e '' $(TEST0_FILES)

test1: n7i $(TEST1_FILES)
	prove -o -e '' $(TEST1_FILES)

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
	find . -name '*.o' -exec rm -f \{} \;
	find . -name '*.gc??' -exec rm -f \{} \;
	rm -f t/*.t
	rm -rf doc/coverage
	rm -f $(BINARIES)

n7i: n7i.o core.o

t/%.t: t/%.o core.o t/test.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< core.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $+
