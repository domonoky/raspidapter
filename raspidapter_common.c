//=============================================================================
//
//
// Raspidapter Common code
//
// This file is part of the Raspidapter library
//
//
// Copyright (C) Dominik Wenger 2015
// No rights reserved
// You may treat this program as if it was in the public domain
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//


#include "RaspberryPi-GPIO/include/rpiGpio.h"
#include "raspidapter_common.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>


// I/O chain buffers
int num_chained_io =0;
char* chained_io_buffer =0; 

#define CHAINED_IO_ENABLE 22
#define CHAINED_IO_DATA 18
#define CHAINED_IO_CLOCK 27
#define CHAINED_IO_STROBE 17 

// marker for init
int g_initialised =0; 



//
// This is a software loop to wait
// a short while.
//
void short_wait()
{ int w;
  for (w=0; w<50; w++)
  { w++;
    w--;
  }
} // short_wait

void high_wait()
{ int w;
  for (w=0; w<25; w++)
  { w++;
    w--;
  }
}

void high_wait_half()
{ int w;
  for (w=0; w<12; w++)
  { w++;
    w--;
  }
}

void low_wait()
{ int w;
  for (w=0; w<10; w++)
  { w++;
    w--;
  }
}



//
//  init io chain IOs 
//
int setup_iochain(int numboards)
{
   //error checking
   if(numboards < 1)
   {
      return ERR_PARAM;
   }

   if(chained_io_buffer != 0)
   {
      return ERR_INIT;
   }

   //calc amount of ios
   num_chained_io = 32* numboards;

   //alloc buffer
   if ((chained_io_buffer = malloc(num_chained_io/8)) == NULL) {
      printf("chained_io allocation error \n");
      exit (-1);
   }

   gpioSetFunction(CHAINED_IO_ENABLE,output);
   gpioSetFunction(CHAINED_IO_DATA,output);
   gpioSetFunction(CHAINED_IO_CLOCK,output);
   gpioSetFunction(CHAINED_IO_STROBE,output);
 
   //enable output stage
   gpioSetPin(CHAINED_IO_ENABLE,high);
   return 0;

}

//
// Undo the IO chain init
//
int deinit_iochain()
{
  free(chained_io_buffer);

   return 0;
}


//
// set a bit of the IO chain
//
int iochain_setbit(int bit)
{
   //error checking
   if(bit >= num_chained_io-1)
   {
      return ERR_PARAM;
   }
   if(chained_io_buffer == 0)
   {
     return ERR_INIT;
   }

   //calc bit and bytenum
   int bytenum = bit/8;
   int bitnum = bit%8;

   chained_io_buffer[bytenum] |= (1<<bitnum); 

   return 0;
}

//
// clear a bit in the IO chain
// 
int iochain_clearbit(int bit)
{
   if(bit >= num_chained_io-1)
   {
      return ERR_PARAM;
   }
   if(chained_io_buffer == 0)
   {
     return ERR_INIT;
   }

   //calc bit and bytenum
   int bytenum = bit/8;
   int bitnum = bit%8;
  
   chained_io_buffer[bytenum] &= ~(1<<bitnum); 

   return 0;
}

//
// update buffered outputs to hards. Blocks while sending.
//
int iochain_update()
{
 
   if(chained_io_buffer == 0)
   {
     return ERR_INIT;
   }

   int i=0;
   //check all bits
   for(i= num_chained_io-1; i >=0; i--)
   {
       int bytenum = i/8;
       int bitnum = i%8;  

       //set output if there is a one  
       if(chained_io_buffer[bytenum] & (1<<bitnum))
       {
         gpioSetPin(CHAINED_IO_DATA,high);       }
       high_wait_half();
       //clock
       gpioSetPin(CHAINED_IO_CLOCK,high);
       high_wait_half();
       gpioSetPin(CHAINED_IO_CLOCK,low);
       
       //set data to zero
       gpioSetPin(CHAINED_IO_DATA,low);
       low_wait();
   }

   //toggle strobe
   gpioSetPin(CHAINED_IO_STROBE,high);
   high_wait();
   gpioSetPin(CHAINED_IO_STROBE,low);

   return 0;
}

///////////////////////////////////////
// I2C Routinen
//////////////////////////////////////



int read_i2c(int address, char reg, int amount, char* data)
{
   int i=0;
   int err;
   //error checking
   // TODO
   err =gpioI2cSetClock(100000);
   if(err!= 0)
     printf(gpioErrToString(err));
   //set address
   err =gpioI2cSet7BitSlave(address);
   if(err!= 0)
     printf(gpioErrToString(err));

   err = gpioI2cWriteData(&reg,1);
   if(err!= 0)
     printf(gpioErrToString(err));

   //read data
   err= gpioI2cReadData(data,amount);
   if(err!= 0)
     printf(gpioErrToString(err));

   return 0;
}

int write_i2c(int address, char reg, int amount, char* data)
{
   int i=0;
   //error checking
   // TODO
   int err=0;
   
   char txbuf[100]={0};
   txbuf[0]= reg;

   //copy data
   memcpy(&txbuf[1],data,amount);

   err = gpioI2cSetClock(100000);
   if(err!= 0)
     printf(gpioErrToString(err));
   //set address
   err = gpioI2cSet7BitSlave(address);
   if(err!= 0)
     printf(gpioErrToString(err));

   //send data
   err = gpioI2cWriteData(txbuf,amount+1);
   if(err!= 0)
     printf(gpioErrToString(err));

   return 0;
}

////////////////////////////////////////////
//public main routines
////////////////////////////////////////////

// main setup routine
int setup_raspidapter(int numboards)
{
   //error checking
   if(numboards <1)
     return ERR_PARAM;

   if(g_initialised == 1)
	return ERR_INIT;

   //setup IO access
   gpioSetup();
  
   //setup iochain
   int ret = setup_iochain(numboards);
   if(ret != 0)
	return ret;

   //set up i2c
   gpioI2cSetup();

   g_initialised = 1;

   return 0;
}

int deinit_raspidapter()
{
  gpioI2cCleanup();
  gpioCleanup();
  deinit_iochain();
  return 0;
}
