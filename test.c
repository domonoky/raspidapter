//
//
// Raspidater test suite
//
//
//
// This file is part of raspidapter test suite.
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
// Try to strike a balance between keep code simple for
// novice programmers but still have reasonable quality code
//

#include "raspidapter_common.h"
#include "dice_stk.h"
#include "dice_9555.h"
#include "dice_vn.h"


struct DICE dice_stk;
struct DICE dice_9555;
struct DICE dice_vn;
struct DICE dice_tmc;

//
// Simple SW wait loop
//
void long_wait(int v)
{ int w;
  while (v--)
    for (w=-800000; w<800000; w++)
    { w++;
      w--;
    }
} // long_wait

// Test
//
int main(void)
{ 
  unsigned int pinstate;

  printf ("Test Program:\n");
  printf ("When ready hit enter.\n");
  (void) getchar();

  // Init raspidapter IOs
  setup_raspidapter(1);
  
   printf("setup DICE STK\n");
  //init DICE-STK 0
 if(dice_stk_setup(&dice_stk,1,1) !=0)
  {
    printf("dice_stk_setup failed");
    return -1;
  }

  //set enable
  if(dice_stk_enable(&dice_stk,1) !=0)
  {
    printf("dice_stk_enable failed");
    return -1;
  }
  // set stubstepping
  if(dice_stk_substepping(&dice_stk,32) !=0)
  {
    printf("dice_stk_substepping failed");
    return -1;
  }

  // init DICE_9555
  printf("setup DICE 9555\n");
 if(dice_9555_setup(&dice_9555,1,2,1) !=0)
  {
    printf("dice_9555_setup failed");
    return -1;
  }

  //set pins to output
  pinstate = 0xffff;
  if(dice_9555_setoutput(&dice_9555,pinstate) !=0)
  {
    printf("dice_9555_setoutput failed");
    return -1;
  }

 // init DICE_VN in port 3
  printf("setup DICE VN\n");
  if(dice_vn_setup(&dice_vn,1,3,1) !=0)
  {
    printf("dice_vn_setup failed");
    return -1;
  }
  //set pins to output
  pinstate = 0x0;
  if(dice_vn_setoutput(&dice_vn,pinstate) !=0)
  {
    printf("dice_vn_setoutput failed");
    return -1;
  } 

  // init dice TMC in slot 4
   printf("setup DICE TMC\n");
  if(dice_tmc_setup(&dice_tmc,1,4) !=0)
  {
    printf("dice_tm_setup failed");
    return -1;
  }
  //start the driver
  dice_tmc_start(&dice_tmc);

 printf("enable DICE TMC\n");
  dice_tmc_setEnabled(&dice_tmc,0);

/////////////////////////////////////
   dice_tmc_getMotorPosition(&dice_tmc);
   int test = dice_tmc_isOpenLoadA(&dice_tmc);
   printf("dice tmc open load A check: %x\n",test);

  // do something
  unsigned char dice_vn_pins = 1;
  unsigned short dice_9555_pins = 1;

  int loopcounter =0;
  while(1)
  {
    dice_tmc_step(&dice_tmc);

    // step the stk motor
    if(dice_stk_step(&dice_stk) !=0)
    {
       printf("dice_stk_step failed");
       return -1;
    }
  
    //change dice_vn outputs
    if(dice_vn_set(&dice_vn,dice_vn_pins) !=0)
    {
       printf("dice_vn_set failed");
       return -1;
    }
   //shift vn output
   if((loopcounter %1000) ==0)
   {
    dice_vn_pins  = dice_vn_pins <<1;
    if(dice_vn_pins == 0) dice_vn_pins =1;
   }

 
    //change dice_9555 outputs
    if(dice_9555_set(&dice_9555,dice_9555_pins) != 0)
    {
       printf("dice_9555_set failed");
       return -1;
    }

    //shift 9555 output
   if((loopcounter %1000) ==0)
   {
     dice_9555_pins = dice_9555_pins <<1;
     if(dice_9555_pins == 0) dice_9555_pins =1;
   }


   loopcounter++;
  }

  restore_io();
} // main



