/*! @brief conversion from unsigned char to hex
    @param c : character to be converted
    @return pointer to \0 terminated static buffer holding the 2 digits
*/

#include "ctoHex.h"

// Attention: no check on c > 0xF !!!!!!
noinline unsigned char convertNibbleToHex(unsigned char c)
{
   if (c > 9) {
      c -= 10;
      return( 'A' + c);
   } else {
      return( '0' + c);
   }
}


unsigned char* uctoHex(unsigned char c)
{
   static char buf[3];

   buf[0] = convertNibbleToHex((c & 0xF0) >> 4);
   buf[1] = convertNibbleToHex(c & 0x0F);
   buf[2] = 0;
   
   return buf;
}

unsigned char* ustoHex(unsigned short c)
{
   static unsigned char buf[5];

   buf[0] = convertNibbleToHex((c & 0xF000) >> 12);
   buf[1] = convertNibbleToHex((c & 0x0F00) >> 8);
   buf[2] = convertNibbleToHex((c & 0x00F0) >> 4);
   buf[3] = convertNibbleToHex(c & 0x000F);
   buf[4] = 0;

   return buf;
}
