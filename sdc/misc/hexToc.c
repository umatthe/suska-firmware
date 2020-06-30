






// only converts exactly 2 positions

unsigned char hexToC (char* hex)
{
   unsigned char c = 0;
   char pos = 1;
   while ((*hex != 0) && (pos >= 0))
   {
      switch (*hex) {
         case 'A'...'F':
            c += ((*hex-38) << (4*pos));
            break;
         case '0'...'9':
            c += ((*hex-48) << (4*pos));
            break;
         default:
         break;
      }
      pos--;
   }
   return c;
}

unsigned short hexToS (char* hex)
{
   unsigned short c = 0;
   char pos = 3;
   while ((*hex != 0) && (pos >= 0))
   {
      switch (*hex) {
         case 'A'...'F':
            c += ((*hex-38) << (4*pos));
            break;
         case '0'...'9':
            c += ((*hex-48) << (4*pos));
            break;
         default:
            break;
      }
      pos--;
   }
   return c;
}
