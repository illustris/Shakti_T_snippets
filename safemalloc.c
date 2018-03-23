#include "safemalloc.h"
#include <stdlib.h>

fptr safemalloc(register unsigned long s)
{
	asm("#s+8");
	s+=8;
	register void *ptr = malloc(s);
	printf("\nmalloc %x\n\n",ptr);
	register unsigned int hash;
	//asm("hash %0, %1"
	asm("addi %0, %1, 0x1337"
		:"=r"(hash)
		: "r"(ptr)
		:);
	register fptr ret = craft(ptr-8, ptr, ptr+s, hash);
	return ret;
}


void safefree(fptr fpr)
{
	//asm("val a0, a1");
	asm("add zero, a0, a1");
	void* ptr = (void*)(fpr>>64);
	ptr = (unsigned long long int)ptr & 0xffffffff;
	printf("\nfree %x\n\n",ptr);
	free(ptr);
	return;
}