/*
    This code has beem cut of the uClibc subdir misc.
    The avr-lib provides a method as well, but with a
    less nice interface.
*/


#define __MAX_INT_CHARS 7

char *
itoa(i)
int   i;
{
   static char a[__MAX_INT_CHARS];
   char *b = a + sizeof(a) - 1;
   int   sign = (i < 0);

   if(i==0x8000)
   {
	   return "-32768";
   }
   if (sign)
      i = -i;
   *b = 0;
   do
   {
      *--b = '0' + (i % 10);
      i /= 10;
   }
   while (i);
   if (sign)
      *--b = '-';
   return b;
}
