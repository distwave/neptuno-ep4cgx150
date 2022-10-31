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

#define MaxIR_ChainLength 100

int IRlen = 0;
int nDevices = 0;

struct codestr  
{    
  unsigned char onebit:1;     
  unsigned int manuf:11; 
  unsigned int size:9;  
  unsigned char family:7; 
  unsigned char rev:4; 
};

union 
{
    unsigned long code = 0;
    codestr b;
} idcode;

/*
void JTAG_clock()  
{
//    digitalWrite(PIN_TCK, HIGH);  
//    digitalWrite(PIN_TCK, LOW);

      GPIOB->regs->ODR |= 1;
      GPIOB->regs->ODR &= ~(1);
}
*/

void JTAG_reset()   
{   
    int i;   
    
    digitalWrite(PIN_TMS, HIGH);
          
    // go to reset state   
    for(i=0; i<10; i++) 
    {
        //JTAG_clock();
        GPIOB->regs->ODR |= 1;
        GPIOB->regs->ODR &= ~(1);
    }
}

void JTAG_EnterSelectDR()   
{ 
    // go to select DR   
    digitalWrite(PIN_TMS, LOW); JTAG_clock();   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();  
}  

void JTAG_EnterShiftIR()   
{   
  digitalWrite(PIN_TMS, HIGH); JTAG_clock();
  digitalWrite(PIN_TMS, LOW); JTAG_clock();
  digitalWrite(PIN_TMS, LOW); JTAG_clock();
     
} 

void JTAG_EnterShiftDR()   
{   
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    digitalWrite(PIN_TMS, LOW); JTAG_clock();   

   // digitalWrite(PIN_TMS, LOW); JTAG_clock(); //extra ?
} 

void JTAG_ExitShift()   
{   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();   
}   

void JTAG_ReadDR(int bitlength)   
{   
    JTAG_EnterShiftDR();   
    JTAG_ReadData(bitlength);   
}  


void JTAG_ReadData(int bitlength)   
// note: call this function only when in shift-IR or shift-DR state   
{   
    int bitofs = 0;

    unsigned long temp;
  
     
    bitlength--;   
    while(bitlength--)   
    {   
        digitalWrite(PIN_TCK, HIGH);

        temp = digitalRead(PIN_TDO);


       // Serial.println(temp, HEX); 
        
        temp = temp << bitofs ;
        idcode.code |= temp;
        
        digitalWrite(PIN_TCK, LOW);  
        bitofs++;   

        
    }  

    digitalWrite(PIN_TMS, HIGH);
    digitalWrite(PIN_TCK, HIGH);
 
    temp = digitalRead(PIN_TDO);

   // Serial.println(temp, HEX); 
    
    temp = temp << bitofs ;
    idcode.code |= temp;
        
    digitalWrite(PIN_TCK, LOW); 
     
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();  // go back to select-DR   
}

int JTAG_DetermineChainLength(char* s)   
{   
    int i; 
  // Serial.println("JTAG_DetermineChainLength"); 

    
   
    // empty the chain (fill it with 0's)
    digitalWrite(PIN_TDI, LOW);   
    for(i=0; i<MaxIR_ChainLength; i++) { digitalWrite(PIN_TMS, LOW); JTAG_clock();   }
    
    digitalWrite(PIN_TCK, LOW);
         
    // feed the chain with 1's   
    digitalWrite(PIN_TDI, HIGH);
    for(i=0; i<MaxIR_ChainLength; i++) 
    { 
 
  
      digitalWrite(PIN_TCK, HIGH);

       if(digitalRead(PIN_TDO) == HIGH) break;  
        
           digitalWrite(PIN_TCK, LOW);
    }
   
      digitalWrite(PIN_TCK, LOW);

   
         Serial.print(s); 
         Serial.print(" = ");  
         Serial.println(i); 
      
      
    JTAG_ExitShift();   
    return i;   
} 

int JTAG_scan()
{
 int i=0;

  JTAG_reset();  
  JTAG_EnterSelectDR();
  JTAG_EnterShiftIR() ;  

  IRlen = JTAG_DetermineChainLength("tamanho do IR");   

  JTAG_EnterShiftDR();   
  nDevices = JTAG_DetermineChainLength("Qtd devices");  

  if (IRlen == MaxIR_ChainLength || nDevices == MaxIR_ChainLength )
  {
    error();
    return 1;
  }

  // read the IDCODEs (assume all devices support IDCODE, so read 32 bits per device)   
    JTAG_reset();   
    JTAG_EnterSelectDR();
    JTAG_ReadDR(32*nDevices); 

    for(i=0; i<nDevices; i++)   
    {   
       // assert(idcode[i].onebit);  // if the bit is zero, that means IDCODE is not supported for this device   
       // printf("Device %d IDCODE: %08X (Manuf %03X, Part size %03X, Family code %02X, Rev %X)\n", i+1, idcode[i], idcode[i].manuf, idcode[i].size, idcode[i].family, idcode[i].rev);   

      
          Serial.print("Device IDCODE: ");
          
          Serial.println(idcode.code, HEX); 
          
          Serial.print(" rev: ");
          Serial.print(idcode.b.rev, HEX);
          
          Serial.print(" family: ");
          Serial.print(idcode.b.family, HEX); 
          
          Serial.print(" size: ");
          Serial.print(idcode.b.size, HEX); 
          
          Serial.print(" manuf: ");
          Serial.print(idcode.b.manuf, HEX); 
          
          Serial.print(" onebit: ");
          Serial.println(idcode.b.onebit, HEX); 
       
    }  

    return 0;

}

void JTAG_PREprogram()
{
    int n;
    
    JTAG_reset();  
    JTAG_EnterSelectDR();
    JTAG_EnterShiftIR() ;  

      //  digitalWrite(PIN_TMS, LOW); JTAG_clock(); //extra ?

    // aqui o TMS jÃ¡ esta baixo, nao precisa de outro comando pra abaixar.

    // IR = PROGRAM =   00 0000 0010    // IR = CONFIG_IO = 00 0000 1101
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, HIGH); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();

    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
                        
    digitalWrite(PIN_TDI, LOW);   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();

    // aqui o modo Ã© exit IR 
    
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
     // aqui o modo Ã© update IR
        
     // Drive TDI HIGH while moving to SHIFTDR */
    digitalWrite(PIN_TDI, HIGH);  

    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
    // aqui o modo Ã© select dr scan
    
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    
    // aqui o modo estÃ¡ em shift dr

 //digitalWrite(PIN_TMS, LOW); JTAG_clock(); //extra ?


    /* Issue MAX_JTAG_INIT_CLOCK clocks in SHIFTDR state */
   digitalWrite(PIN_TDI, HIGH);
   for(n=0;n<300;n++)
    {
        JTAG_clock();
    }

     digitalWrite(PIN_TDI, LOW);  
}

void JTAG_POSprogram()
{
    int n;
    
 //aqui esta no exit DR
    
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
    // aqui esta no update DR
    
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    
    //Aqui esta no RUN/IDLE
    
    JTAG_EnterSelectDR();   
    JTAG_EnterShiftIR(); 

    // aqui em shift ir


    // IR = CHECK STATUS = 00 0000 0100
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, HIGH); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW);   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
    
    //aqui esta no exit IR
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
    //aqui esta no select dr scan



    
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    
    //   aqui esta no shift IR
    
    
    // IR = START = 00 0000 0011
    digitalWrite(PIN_TDI, HIGH); JTAG_clock();
    digitalWrite(PIN_TDI, HIGH); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW); JTAG_clock();
    
    digitalWrite(PIN_TDI, LOW);   
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    
    
    //aqui esta no exit IR
    
    digitalWrite(PIN_TMS, HIGH); JTAG_clock();
    digitalWrite(PIN_TMS, LOW); JTAG_clock();
    
    //aqui esta no IDLE
    
    //espera 
    for(n=0; n<200; n++) 
    {
        //JTAG_clock();
        GPIOB->regs->ODR |= 1;
        GPIOB->regs->ODR &= ~(1);
    }

    JTAG_reset();
 
}

