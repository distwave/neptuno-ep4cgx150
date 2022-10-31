//
// Multicore 2
//
// Copyright (c) 2017-2020 - Victor Trucco
//
// Additional code, debug and fixes: Diogo Patrão
//
// All rights reserved
//
// Redistribution and use in source and synthezised forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// Redistributions in synthesized form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// Neither the name of the author nor the names of other contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// You are responsible for any legal issues arising from your use of this code.
//

#include "charrom.h"

#define MM1_OSDCMDWRITE    0x20      // OSD write video data command
#define MM1_OSDCMDENABLE   0x41      // OSD enable command
#define MM1_OSDCMDDISABLE  0x40      // OSD disable command

#define OSDNLINE         8           // number of lines of OSD
#define OSDLINELEN       256         // single line length in bytes

#define OSD_ARROW_LEFT 1
#define OSD_ARROW_RIGHT 2


void spi_osd_cmd_cont(unsigned char cmd) 
{
  EnableOsdSPI();
  SPI.transfer(cmd);
}

void spi_n(unsigned char val, unsigned short cnt) 
{
  while(cnt--) 
    SPI.transfer(val);
}

void spi8(unsigned char parm) 
{
  SPI.transfer(parm);
}

void spi16(unsigned short parm) 
{
  SPI.transfer(parm >> 8);
  SPI.transfer(parm >> 0);
}

void spi24(unsigned long parm) 
{
  SPI.transfer(parm >> 16);
  SPI.transfer(parm >> 8);
  SPI.transfer(parm >> 0);
}

void spi32(unsigned long parm) 
{
  SPI.transfer(parm >> 24);
  SPI.transfer(parm >> 16);
  SPI.transfer(parm >> 8);
  SPI.transfer(parm >> 0);
}

void EnableOsdSPI()
{
     SPI.setModule(2); // select the second SPI (connected on FPGA) 
     SPI_SELECTED();   // slave active
}

void DisableOsdSPI()
{
    SPI_DESELECTED(); // slave deselected
    SPI.setModule(1); // select the SD Card SPI
}

void OSDVisible (bool visible)
{
        if (visible)
        {
          spi_osd_cmd_cont(0x41);
          menu_opened = millis();
        }
        else
        {
          spi_osd_cmd_cont(0x40);
        }

        DisableOsdSPI();
}



void OsdClear(void)
{

     spi_osd_cmd_cont(0x20);


    // clear buffer
    spi_n(0x00, OSDLINELEN * OSDNLINE);

  DisableOsdSPI();

}

void OsdWrite(unsigned char n, char *s, unsigned char invert, unsigned char stipple, unsigned char arrow)
{
    OsdWriteOffset(n,s,invert,stipple,0, arrow);
}

// write a null-terminated string <s> to the OSD buffer starting at line <n>
void OsdWriteOffset(unsigned char n, char *s, unsigned char invert, unsigned char stipple,char offset, unsigned char arrow)
{
      unsigned short i;
      unsigned char b;
      const unsigned char *p;
      unsigned char stipplemask=0xff;
      int linelimit=OSDLINELEN-1;
      int arrowmask=arrow;

      // Serial.print("OsdWriteOffset ");
      //Serial.println(s);
      
      if(n==7 && (arrow & OSD_ARROW_RIGHT))
      {
          linelimit-=22;
      }
      
      if(stipple) 
      {
            stipplemask=0x55;
            stipple=0xff;
      } 
      else
      {
            stipple=0;
      }
      
      // select buffer and line to write to
      spi_osd_cmd_cont(MM1_OSDCMDWRITE | n);

      
      if(invert) invert=255;
      
      i = 0;
      
      // send all characters in string to OSD
      while (1) 
      {
        
         /* if(i==0) 
          {     
                // Render sidestripe
                unsigned char j;
          
                p = &titlebuffer[(7-n)*8];
          
                spi16(0xffff);  // left white border
          
                for(j=0;j<8;j++) spi_n(255^*p++, 2);
          
                spi16(0xffff);  // right white border
                spi16(0x0000);  // blue gap
                i += 22;
          } 
          else*/ if(n==7 && (arrowmask & OSD_ARROW_LEFT)) // Draw initial arrow
          { 
                
                unsigned char b;
          
                spi24(0x00);
                
                p = &charfont[0x10][0];
                
                for(b=0;b<8;b++) spi8(*p++<<offset);
                
                p = &charfont[0x14][0];
                
                for(b=0;b<8;b++) spi8(*p++<<offset);
                
                spi24(0x00);
                spi_n(invert, 2);
                i+=24;
                arrowmask &= ~OSD_ARROW_LEFT;
                
                if(*s++ == 0) break;  // Skip 3 characters, to keep alignent the same.
                if(*s++ == 0) break;
                if(*s++ == 0) break;
          } 
          else 
          {
            
                b = *s++;
                
                if (b == 0)  break; // end of string
           
                
                else if (b == 0x0d || b == 0x0a) // cariage return / linefeed, go to next line
                { 
                    // increment line counter
                    if (++n >= linelimit) n = 0;
                  
                    // send new line number to OSD
                    DisableOsdSPI();
                    
                    spi_osd_cmd_cont(MM1_OSDCMDWRITE | n);
                }
                else if(i<(linelimit-8)) // normal character
                { 
                      unsigned char c;
                      p = &charfont[b][0];
                      
                     // if (b==0x41) Serial.println("----");
                     
                      for( c=0; c < 8; c++ ) //character width
                      {
                        //  if (b==0x41) Serial.println(*p);
                        
                            spi8(((*p<<offset)&stipplemask)^invert);
                            p++;
                            stipplemask^=stipple;
                      }
                      
                      i += 8;
                      
                 }
          }
      }
    
      for (; i < linelimit; i++)  spi8(invert); // clear end of line
       
    
      if(n==7 && (arrowmask & OSD_ARROW_RIGHT)) // Draw final arrow if needed
      { 
            unsigned char c;
            
            spi24(0x00);
            p = &charfont[0x15][0];
            
            for(c=0;c<8;c++) spi8(*p++<<offset);
            
            p = &charfont[0x11][0];
            
            for(c=0;c<8;c++) spi8(*p++<<offset);
            
            spi24(0x00);
            i+=22;
      }
      
      // deselect OSD SPI device
      DisableOsdSPI();
}


// write a null-terminated string <s> to the OSD buffer starting at line <n>
void OSD_progressBar(unsigned char line, char *text, unsigned char percent )
{
    // line : OSD line number (0-7)
    // text : pointer to null-terminated string
    // start : start position (in pixels)
    // width : printed text length in pixels
    // offset : scroll offset in pixels counting from the start of the string (0-7)
    // invert : invertion flag
  
    unsigned long start = 215;
    unsigned long width = 40;
    unsigned long offset = 0;
    unsigned char invert = 0;

    unsigned char count = 0;
    
    const unsigned char *p;
    int i,j;
    
    // select buffer and line to write to
    spi_osd_cmd_cont(MM1_OSDCMDWRITE | line);

    while ( count < start )
    {
        count++;
        
        if (percent < (count*100/214))
        { 
            spi8( 0x00 );
        }
        else
        {
            spi8( 0xff );
        }
    }

    while ( width > 8 ) 
    {
        unsigned char b;
        p = &charfont[*text++][0];
        for(b=0;b<8;b++) spi8(*p++);
        width -= 8;
    }
    
    if (width) 
    {
        p = &charfont[*text++][0];
        while (width--)
          spi8(*p++);
    }
  
    DisableOsdSPI();
}
