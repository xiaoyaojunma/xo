#include "pch.h"
#include "nuPlatform.h"

void* nuMallocOrDie( size_t bytes )
{
	void* b = malloc( bytes );
	NUASSERT(b);
	return b;
}
