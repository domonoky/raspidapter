//
// Raspidapter test suite
//
// dice STK implementation 
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
#include "dice_stk.h"

int dice_stk_setup(struct DICE* dice,int board, int slot)
{
   //error checking
   if(dice == NULL)
     return ERR_PARAM;

   if(board == 0)
     return ERR_PARAM;

   if(slot <1 || slot > 4)
     return ERR_PARAM;

   //set type
   dice->type = DICE_STK;

   //calc bit numbers
   dice->enable = (board-1)*32 + (slot-1)*8 + 0;
   dice->ms1 = (board-1)*32 + (slot-1)*8 + 4;
   dice->ms2 = (board-1)*32 + (slot-1)*8 + 5;
   dice->ms3 = (board-1)*32 + (slot-1)*8 + 1;
   dice->rs = (board-1)*32 + (slot-1)*8 + 2;
   dice->dir = (board-1)*32 + (slot-1)*8 + 3;
   dice->step = (board-1)*32 + (slot-1)*8 + 6;
   dice->slp = (board-1)*32 + (slot-1)*8 + 7;

   return 0;
}

int dice_stk_step(struct DICE* dice)
{
  //error checking
  if(dice == NULL)
    return ERR_PARAM;
  if(dice->type != DICE_STK)
    return ERR_PARAM;

  iochain_setbit(dice->step);
  iochain_update();
    
    
  // wait ?
  iochain_clearbit(dice->step);
  
  return iochain_update();
}

int dice_stk_dir(struct DICE* dice,int dir)
{
  //error checking
  if(dice == NULL)
    return ERR_PARAM;
  if(dice->type != DICE_STK)
    return ERR_PARAM;

  if(dir) iochain_setbit(dice->dir);
  else iochain_clearbit(dice->dir);
  
  return iochain_update();
}

int dice_stk_enable(struct DICE* dice,int enable)
{
  //error checking
  if(dice == NULL)
    return ERR_PARAM;
  if(dice->type != DICE_STK)
    return ERR_PARAM;

  if(enable) iochain_setbit(dice->enable);
  else iochain_clearbit(dice->enable);
  
  return iochain_update();
}

int dice_stk_substepping(struct DICE* dice,int substepping)
{
  //error checking
  if(dice == NULL)
    return ERR_PARAM;
  if(dice->type != DICE_STK)
    return ERR_PARAM;

  //analyse substepping
  switch(substepping)
  {
      case 1:
      {
         // no substepping
         iochain_clearbit(dice->ms1);
         iochain_clearbit(dice->ms2);
         iochain_clearbit(dice->ms3);
         break;
      }
      case 2:
      {
         // 1/2 substepping
         iochain_setbit(dice->ms1);
         iochain_clearbit(dice->ms2);
         iochain_clearbit(dice->ms3);
         break;
      }
      case 4:
      {
         // 1/4 substepping
         iochain_clearbit(dice->ms1);
         iochain_setbit(dice->ms2);
         iochain_clearbit(dice->ms3);
         break;
      }
      case 8:
      {
         // 1/8 substepping
         iochain_setbit(dice->ms1);
         iochain_setbit(dice->ms2);
         iochain_clearbit(dice->ms3);
         break;
      }
      case 16:
      {
         // 1/16 substepping
         iochain_clearbit(dice->ms1);
         iochain_clearbit(dice->ms2);
         iochain_setbit(dice->ms3);
         break;
      }
      case 32:
      {
         // 1/32 substepping
         iochain_setbit(dice->ms1);
         iochain_clearbit(dice->ms2);
         iochain_setbit(dice->ms3);
         break;
      }
      case 64:
      {
         // 1/64 substepping
         iochain_clearbit(dice->ms1);
         iochain_setbit(dice->ms2);
         iochain_setbit(dice->ms3);
         break;
      }
      case 128:
      {
         // 1/64 substepping
         iochain_setbit(dice->ms1);
         iochain_setbit(dice->ms2);
         iochain_setbit(dice->ms3);
         break;
      }
      default:
        return ERR_PARAM;
  }

  //write to ios
  return iochain_update();
}