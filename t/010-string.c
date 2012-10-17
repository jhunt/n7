#include "assert.h"

int main(int argc, char **argv)
{
	obj s = vstring("test string");
	vstring_is(s, "test string", "Basic string creation");

	obj hello = vstring("Hello, ");
	obj world = vstring("World!");
	obj new = vstrcat(hello, world);

	ok(new != hello, "vstrcat does not reuse first arg");
	ok(new != world, "vstrcat does not reuse second arg");

	vstring_is(hello, "Hello, ", "vstrcat does not modify first arg");
	vstring_is(world, "World!",  "vstrcat does not modify second arg");
	vstring_is(new, "Hello, World!", "vstrcat cats strs yay!");

	new = vextend(hello, "Test!  this is ignored", 5);
	ok(new == hello, "vextend reuses first arg");
	vstring_is(hello, "Hello, Test!", "vextend strcats in-place");


	done_testing();
	return 0;
}
