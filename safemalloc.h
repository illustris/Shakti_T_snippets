#include "craft.h"
fptr safemalloc(register unsigned long s);

void safefree(fptr fpr);

#ifdef FUNCT
void validate(fptr fpr);
unsigned int hash_ptr(unsigned long long *ptr);
#endif