//
// Raspidapter library
//
// DICE TC implementation 
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


#include "raspidapter_common.h"
#include "dice_tc.h"

#define ERROR_MASK 0x7

//internal function definitions
unsigned int spiread32(struct DICE* dice,unsigned char chipnum);



int dice_tc_setup(struct DICE* dice,int board, int slot)
{
   //error checking
   if(dice == NULL)
     return ERR_PARAM;

   if(board == 0)
     return ERR_PARAM;

   if(slot <1 || slot > 4)
     return ERR_PARAM;

   //set type
   dice->type = DICE_TC;

   //calc bit numbers
   dice->enable = (board-1)*32 + (slot-1)*8 + 0;
   dice->ms1 = (board-1)*32 + (slot-1)*8 + 4;
   dice->ms2 = (board-1)*32 + (slot-1)*8 + 5;
   dice->ms3 = (board-1)*32 + (slot-1)*8 + 1;
   dice->rs = (board-1)*32 + (slot-1)*8 + 2;
   dice->dir = (board-1)*32 + (slot-1)*8 + 3;
   dice->step = (board-1)*32 + (slot-1)*8 + 6;
   dice->slp = (board-1)*32 + (slot-1)*8 + 7;
	
   //unselect CS
   iochain_setbit(dice->enable);
   iochain_update();

   return 0;
}


double dice_tc_readInternalTemp(struct DICE* dice,unsigned char chipnum)
{
  unsigned int v;
  v=spiread32(dice,chipnum);
 //ignore bottom 4 bits
  v = v >> 4;
//pull the bottom 11 bits off
  double internal = v & 0x7ff;
  internal = internal* 0.0625; //LSB = 0.0625

  if(v&0x800) internal = internal * -1;
 return internal;
}

double dice_tc_readCelsius(struct DICE* dice,unsigned char chipnum)
{
  unsigned int v;

  v= spiread32(dice, chipnum);

  if(v & ERROR_MASK) 
  {
    printf("error reading temp: %x\n",v & ERROR_MASK);
    //chip return errors
    return 0;  
  }

  //get rid of internal temp
  v >>= 18;

  double centigrade = v;

  //LSB = 0.25 degree C
  centigrade *= 0.25;

  return centigrade;

}

double dice_tc_readFarenheit(struct DICE* dice,unsigned char chipnum)
{
  float f= dice_tc_readCelsius(dice,chipnum);
  f *= 9.0;
  f /= 5.0;
  f +=32;
  return f;
}

unsigned char dice_tc_readError(struct DICE* dice,unsigned char chipnum)
{
   return spiread32(dice,chipnum) & ERROR_MASK;
}

unsigned int spiread32(struct DICE* dice,unsigned char chipnum)
{
   unsigned int d=0;
   
   //select correct subchip
  switch(chipnum)
  {
     case 1:
        iochain_clearbit(dice->step);
        iochain_clearbit(dice->dir);
      break;
     case 2:
        iochain_setbit(dice->step);
        iochain_clearbit(dice->dir);
      break;
     case 3:
        iochain_clearbit(dice->step);
        iochain_setbit(dice->dir);
      break;
     default:
      printf("wrong chipnum\n");
      return 0;
  }
  iochain_update();

  // select chip
  iochain_clearbit(dice->enable);
  iochain_update();

   d= spi_transfer(0);
   d= d << 8;
   d |= spi_transfer(0);
   d= d << 8;
   d |= spi_transfer(0);
   d= d << 8;
   d |= spi_transfer(0);
   
  //deselect chip
  iochain_setbit(dice->enable);
  iochain_update();

  return d;
}