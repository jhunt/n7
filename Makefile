CFLAGS := -g

repl: repl.o core.o gc.o
syms: syms.o core.o gc.o

clean:
	rm -f *.o
	rm -f repl
	rm -f syms
	rm -f sizes
	rm -f t/*test

test: t/qtest
	echo $+ | xargs -n 1 /bin/sh -c
