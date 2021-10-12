//
// OnScreenDisplay Driver 
// Derived from Mist/Minimig Project
// 
// 20200321 Udo Matthe: Initial Version
// 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "charrom.h"
#include "osd.h"
#include "suskaspi.h"

static unsigned char titlebuffer[64];
static uint8_t osdchar[8];

static void rotatechar(unsigned char *in,unsigned char *out)
{
        int a;
        int b;
        int c;
        for(b=0;b<8;++b)
        {
                a=0;
                for(c=0;c<8;++c)
                {
                        a<<=1;
                        a|=(in[c]>>b)&1;
                }
                out[b]=a;
        }
}
void OsdSetTitle(char *s)
{
        // Compose the title, condensing character gaps
        char zeros=0;
        char i=0,j=0;
        char outp=0;
        while(1)
        {
                int c=s[i++];
                if(c && (outp<64))
                {
                memcpy_P(&osdchar,&charfont[c][0],sizeof(osdchar));
                unsigned char *p = (unsigned char *)&osdchar;
                        for(j=0;j<8;++j)
                        {
                                unsigned char nc=*p++;
                                if(nc)
                                {
                                        zeros=0;
                                        titlebuffer[outp++]=nc;
                                }
                                else if(zeros==0)
                                {
                                        titlebuffer[outp++]=0;
                                        zeros=1;
                                }
                                if(outp>63)
                                        break;
                        }
                }
                else
                        break;
        }
        for(i=outp;i<64;++i)
        {
                titlebuffer[i]=0;
        }

        // Now centre it:
        int c=(63-outp)/2;
        for(i=(63-c);i>=0;--i)
        {
                titlebuffer[i+c]=titlebuffer[i];
        }
        for(i=0;i<c;++i)
                titlebuffer[i]=0;

        // Finally rotate it.
        for(i=0;i<64;i+=8)
        {
                unsigned char tmp[8];
                rotatechar(&titlebuffer[i],tmp);
                for(c=0;c<8;++c)
                {
                        titlebuffer[i+c]=tmp[c];
                }
        }
}


void OsdWrite(unsigned char n, char *s, unsigned char invert)
{
        OsdWriteOffset(n,s,invert,0);
}

// write a null-terminated string <s> to the OSD buffer starting at line <n>
void OsdWriteOffset(unsigned char n, char *s, unsigned char invert,char offset)
{
  unsigned short i;
  unsigned char b;
  const unsigned char *p;
  int linelimit=OSDLINELEN;

  // select buffer and line to write to
    spi_osd_cmd_cont(MM1_OSDCMDWRITE | n);

  if(invert)
  {
    invert=255;
  }
  i = 0;
  // send all characters in string to OSD
  while (1) 
  {
    if(i==0) 
    {  // Render sidestripe
      unsigned char j;

      p = &titlebuffer[(7-n)*8];

      spi16(0xffff);  // left white border

      for(j=0;j<8;j++)
      {
        spi_n(255^*p++, 2);
      }

      spi16(0xffff);  // right white border
      spi16(0x0000);  // blue gap
      i += 22;
    } 
    else 
    {
      b = *s++;
      if (b == 0) // end of string
      {
        break;
      }
      else if (b == 0x0d || b == 0x0a) 
      { // cariage return / linefeed, go to next line
        // increment line counter
        if (++n >= linelimit)
          n = 0;

        // send new line number to OSD
        DisableOsd();

          spi_osd_cmd_cont(MM1_OSDCMDWRITE | n);
      }
      else if(i<(linelimit-8)) 
      { // normal character
        unsigned char c;
        memcpy_P(&osdchar,&charfont[b][0],sizeof(osdchar));
        p = (unsigned char *)&osdchar;
        for(c=0;c<8;c++) 
        {
          spi8(((*p++<<offset))^invert);
        }
        i += 8;
      }
    }
  }

  for (; i < linelimit; i++) // clear end of line
  {
    spi8(invert);
  }
  // deselect OSD SPI device
  DisableOsd();
}
