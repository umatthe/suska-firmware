/*
    This code has beem cut of the uClibc subdir misc.
    The avr-lib provides a method as well, but with a
    less nice interface.
*/

#include <avr/io.h>
#include <limits.h>

#if INT_MAX > 2147483647
#error need to increase size of buffer
#endif

/* 10 digits + 1 sign + 1 trailing nul */
static char buf[12];

char *ltoa(int32_t i)
{
	char *pos = buf + sizeof(buf) - 1;
	uint32_t u;
	int32_t negative = 0;

	if (i < 0) {
		negative = 1;
		u = ((uint32_t)(-(1+i))) + 1;
	} else {
		u = i;
	}

	*pos = 0;

	do {
		*--pos = '0' + (u % 10);
		u /= 10;
	} while (u);

	if (negative) {
		*--pos = '-';
	}

	return pos;
}
