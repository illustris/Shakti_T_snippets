#include "safemalloc.h"
#include <stdlib.h>

#ifdef FUNCT
unsigned int hash_ptr(unsigned long long *ptr)
{
	unsigned int* a = ptr;
	a[0]=rand();a[1]=rand();
	
	#ifdef DEBUG
	printf("hash: Generated random %x%x at %x\n\n",a[0],a[1],ptr);
	#endif
	
	return a[0]^a[1];
}

void validate(fptr fpr)
{
	unsigned int ptr,hash,base,bound;
	unsigned int* base_ptr;
	ptr = fpr;
	hash = fpr >> 32;
	base = fpr >> 64;
	base_ptr = (unsigned long long) base;
	bound = fpr >> 96;

	#ifdef DEBUG
	printf("\nValidating:\nptr = \t%x\nhash = \t%x\nbase = \t%x\nbound =\t%x\n",ptr,hash,base,bound);
	#endif

	if(ptr > base)
	{
		#ifdef DEBUG
		printf("ptr > base\n");
		#endif
		if(ptr < bound)
		{
			#ifdef DEBUG
			printf("ptr < bound\n");
			#endif
			if(hash == base_ptr[0]^base_ptr[1])
			{
				#ifdef DEBUG
				printf("hash match\n** POINTER VALID **\n\n");
				#endif
				return;
			}
		}
	}
	#ifdef DEBUG
	printf("** INVALID POITNER **")	;
	#endif
	exit(0);
}
#endif

fptr safemalloc(unsigned long s)
{
	register unsigned int hash;
	s+=8;
	register void *ptr = malloc(s);

	#ifdef DEBUG
	printf("\nmalloc %x\n\n",ptr);
	#endif

	#ifdef FUNCT
	hash = hash_ptr(ptr);
	#endif

	#ifdef ASMGEN
	asm("addi %0, %1, 0x1337"
		:"=r"(hash)
		: "r"(ptr)
		:);
	#endif

	#ifdef REL
	asm("hash %0, %1"
		:"=r"(hash)
		: "r"(ptr)
		:);
	#endif

	fptr ret = craft(ptr+8, ptr, ptr+s, hash);
	return ret;
}


void safefree(fptr fpr)
{
	#ifdef FUNCT
	validate(fpr);
	#endif

	#ifdef ASMGEN
	asm("add zero, a0, a1");
	#endif

	#ifdef REL
	asm("val a0, a1");
	#endif
	
	void* ptr = (void*)(fpr>>64);
	ptr = (unsigned long long int)ptr & 0xffffffff;
	//asm("hash %0, %1"
	asm("addi zero, %0, 0x1337"
		:
		: "r"(ptr)
		:);
	#ifdef DEBUG
	printf("\nfree %x \n\n",ptr);
	#endif
	free(ptr);
	return;
}

fptr saferealloc(volatile register fptr fpr, unsigned long s)
{
	#ifdef FUNCT
	validate(fpr);
	#endif

	#ifdef ASMGEN
	asm("add zero, a0, a1");
	#endif

	#ifdef REL
	asm("val a0, a1");
	#endif
}