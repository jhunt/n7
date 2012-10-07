CFLAGS := -g

repl: repl.o core.o

clean:
	rm -f *.o
	rm -f repl
	rm -f t/*test

test: t/qtest
	echo $+ | xargs -n 1 /bin/sh -c
