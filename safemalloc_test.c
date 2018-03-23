#include "safemalloc.h"
#include <stdio.h>

int main()
{
	asm("lui zero, 0x1336");
	fptr test;
	asm("lui zero, 0x1337");
	test = safemalloc(10);
	asm("lui zero, 0x1338");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (test >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) test);
	asm("lui zero, 0x1339");
	safefree(test);
	asm("lui zero, 0x133a");
}