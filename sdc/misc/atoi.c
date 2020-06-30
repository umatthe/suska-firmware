#include "atoi.h"

int16_t atoi( uint8_t *a)
{
	int16_t i=0;
	int8_t v=1;

	if(*a=='-') { v=-1; a++; }
	else if (*a=='+') { a++; }

	while(*a!=0)
	{
		if(*a<'0' || *a>'9')
		{
			break;
		}
		i*=10;

		i+=(*a-'0');
		a++;
	}
	return v*i;
}

