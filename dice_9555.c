//
// Raspidapter test suite
//
// dice 9555 implementation 
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
#include "dice_9555.h"

#define PTR_INPUT_REG 0
#define PTR_OUTPUT_REG 2
#define PTR_POL_REG 4
#define PTR_CONFIG_REG 6

#define DEV_BASE_ADDR 0x20

int dice_9555_setup(struct DICE* dice,int board, int slot,int number)
{
   //error checking
   if(dice == NULL)
     return ERR_PARAM;

   if(board == 0)
     return ERR_PARAM;

   if(slot <1 || slot > 4)
     return ERR_PARAM;

   if(number <1 && number > 8)
     return ERR_PARAM;

   //set type
   dice->type = DICE_9555;

   //calc bit numbers
   dice->enable = (board-1)*32 + (slot-1)*8 + 0;
   dice->ms1 = (board-1)*32 + (slot-1)*8 + 4;
   dice->ms2 = (board-1)*32 + (slot-1)*8 + 5;
   dice->ms3 = (board-1)*32 + (slot-1)*8 + 1;
   dice->rs = (board-1)*32 + (slot-1)*8 + 2;
   dice->dir = (board-1)*32 + (slot-1)*8 + 3;
   dice->step = (board-1)*32 + (slot-1)*8 + 6;
   dice->slp = (board-1)*32 + (slot-1)*8 + 7;

   // set address bits
   if( (number-1) & (1<<0))
     iochain_setbit(dice->ms1);
   else iochain_clearbit(dice->ms1);
   if( (number-1) && (1<<1))
     iochain_setbit(dice->ms2);
   else iochain_clearbit(dice->ms1);
   if( (number-1) && (1<<2))
     iochain_setbit(dice->ms3);
   else
     iochain_clearbit(dice->ms1);

   iochain_update();
   //store address
   dice->i2c_addr = DEV_BASE_ADDR | (number -1);

   return 0;
}


int dice_9555_setoutput(struct DICE* dice,int pins)
{
   return write_i2c(dice->i2c_addr,PTR_CONFIG_REG,2,(char*) &pins);
}

int dice_9555_setpolarity(struct DICE* dice,int pins)
{
    return write_i2c(dice->i2c_addr,PTR_POL_REG,2,(char*) &pins);
}

int dice_9555_set(struct DICE* dice, int pins)
{
   return write_i2c(dice->i2c_addr,PTR_OUTPUT_REG,2,(char*) &pins);
}

int dice_9555_read(struct DICE* dice, int pins)
{
   return read_i2c(dice->i2c_addr,PTR_INPUT_REG,2,(char*) &pins);
}


