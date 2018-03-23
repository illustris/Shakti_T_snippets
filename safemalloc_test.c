#include "safemalloc.h"
#include <stdio.h>

int main()
{
	printf("Malloc/frealloc/free test:\n");

	fptr t1,t2,t3;

	printf("\n----------------------\nt1 = safemalloc(10);\n----------------------\n");
	t1 = safemalloc(10);
	printf("t1:\n");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (t1 >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) t1);
	
	printf("\n----------------------\nt2 = safemalloc(99);\n----------------------\n");
	t2 = safemalloc(99);
	printf("t2:\n");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (t2 >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) t2);

	printf("\n----------------------\nt1 = saferealloc(55);\n----------------------\n");
	t1 = saferealloc(t1,55);
	printf("t1:\n");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (t1 >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) t1);	

	printf("\n----------------------\nsafefree(t2);\n----------------------\n");
	safefree(t2);

	printf("\n----------------------\nt3 = safemalloc(13);\n----------------------\n");
	t3 = safemalloc(13);
	printf("t3:\n");
	printf("BOUND:BASE  %016llx\n", (unsigned long long int) (t3 >> 64));
	printf("IDHASH:PTR  %016llx\n", (unsigned long long int) t3);

	printf("\n----------------------\nsafefree(t1);\n----------------------\n");
	safefree(t1);

	printf("\n----------------------\nsafefree(t2);\n----------------------\n");
	safefree(t3);
}