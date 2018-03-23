#include "craft.h"
#include <stdio.h>
int main()
{
	asm("lui zero, 0x1337");
	fptr test;
	test = craft(0x13371337, 0xb5b5b5b5, 0xbdbdbdbd, 0x1d1d1d1d);
	//test = craft(1, 3, 4, 2);
	asm("lui zero, 0x1338");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (test >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) test);
	asm("lui zero, 0x1339");
}
