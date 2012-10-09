#include <stdlib.h>
#include <stdio.h>

#define S(x) printf("sizeof(" #x ") = %lu (%lu bits)\n", sizeof(x), sizeof(x)*8)

int main(int argc, char **argv)
{
	S(char);
	S(short);
	S(int);
	S(long);
	S(long long);
	S(void*);

	void *x = malloc(8192);
	unsigned int n, mask = 0;
	for (n = 0; n < sizeof(x)*8; n++) {
		mask += (1 << n);
		if (((unsigned long)(x) & ~mask) != (unsigned long)x) {
			printf("pointer alignment: %u bit available; (%p & 0x%X)\n", n, x,
					mask - (1 << n));
			break;
		}
	}

	return 0;
}
