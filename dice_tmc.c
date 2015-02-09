//
// Raspidapter library
//
// DICE TMC implementation 
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
#include "dice_tmc.h"

// common defines
#define SENSE_RESISTOR 91  // in mOhm
#define DEFAULT_CURRENT 1000  //in mAmps
#define DEFAULT_MICROSTEPPING 32

//register buffers are in the user value of the DICE structs
#define DRIVER_CONTROL_REGISTER_VALUE 0
#define CHOPPER_CONFIG_REGISTER_VALUE 1
#define COOL_STEP_REGISTER_VALUE 2
#define STALL_GUARD2_CURRENT_REGISTER_VALUE 3
#define DRIVER_CONFIGURATION_REGISTER_VALUE 4
#define DRIVER_STATUS_RESULT 5

//register defines
#define DRIVER_CONTROL_REGISTER 0x0ul
#define CHOPPER_CONFIG_REGISTER 0x80000ul
#define COOL_STEP_REGISTER  0xA0000ul
#define STALL_GUARD2_LOAD_MEASURE_REGISTER 0xC0000ul
#define DRIVER_CONFIG_REGISTER 0xE0000ul

#define REGISTER_BIT_PATTERN 0xFFFFFul

//definitions for the driver control register
#define MICROSTEPPING_PATTERN 0xFul
#define STEP_INTERPOLATION 0x200ul
#define DOUBLE_EDGE_STEP 0x100ul
#define VSENSE 0x40ul
#define READ_MICROSTEP_POSTION 0x0ul
#define READ_STALL_GUARD_READING 0x10ul
#define READ_STALL_GUARD_AND_COOL_STEP 0x20ul
#define READ_SELECTION_PATTERN 0x30ul

//definitions for the chopper config register
#define CHOPPER_MODE_STANDARD 0x0ul
#define CHOPPER_MODE_T_OFF_FAST_DECAY 0x4000ul
#define T_OFF_PATTERN 0xful
#define RANDOM_TOFF_TIME 0x2000ul
#define BLANK_TIMING_PATTERN 0x18000ul
#define BLANK_TIMING_SHIFT 15
#define HYSTERESIS_DECREMENT_PATTERN 0x1800ul
#define HYSTERESIS_DECREMENT_SHIFT 11
#define HYSTERESIS_LOW_VALUE_PATTERN 0x780ul
#define HYSTERESIS_LOW_SHIFT 7
#define HYSTERESIS_START_VALUE_PATTERN 0x78ul
#define HYSTERESIS_START_VALUE_SHIFT 4
#define T_OFF_TIMING_PATERN 0xFul

//definitions for cool step register
#define MINIMUM_CURRENT_FOURTH 0x8000ul
#define CURRENT_DOWN_STEP_SPEED_PATTERN 0x6000ul
#define SE_MAX_PATTERN 0xF00ul
#define SE_CURRENT_STEP_WIDTH_PATTERN 0x60ul
#define SE_MIN_PATTERN 0xful

//definitions for stall guard2 current register
#define STALL_GUARD_FILTER_ENABLED 0x10000ul
#define STALL_GUARD_TRESHHOLD_VALUE_PATTERN 0x17F00ul
#define CURRENT_SCALING_PATTERN 0x1Ful
#define STALL_GUARD_CONFIG_PATTERN 0x17F00ul
#define STALL_GUARD_VALUE_PATTERN 0x7F00ul

//definitions for the input from the TCM260
#define STATUS_STALL_GUARD_STATUS 0x1ul
#define STATUS_OVER_TEMPERATURE_SHUTDOWN 0x2ul
#define STATUS_OVER_TEMPERATURE_WARNING 0x4ul
#define STATUS_SHORT_TO_GROUND_A 0x8ul
#define STATUS_SHORT_TO_GROUND_B 0x10ul
#define STATUS_OPEN_LOAD_A 0x20ul
#define STATUS_OPEN_LOAD_B 0x40ul
#define STATUS_STAND_STILL 0x80ul
#define READOUT_VALUE_PATTERN 0xFFC00ul

//default values
#define INITIAL_MICROSTEPPING 0x3ul //32th microstepping


//internal function defines
void send262(struct DICE* dice,unsigned long datagram);


int dice_tmc_setup(struct DICE* dice,int board, int slot)
{
   //error checking
   if(dice == NULL)
     return ERR_PARAM;

   if(board == 0)
     return ERR_PARAM;

   if(slot <1 || slot > 4)
     return ERR_PARAM;

   //set type
   dice->type = DICE_TMC;

   //calc bit numbers
   dice->enable = (board-1)*32 + (slot-1)*8 + 0;
   dice->ms1 = (board-1)*32 + (slot-1)*8 + 4;
   dice->ms2 = (board-1)*32 + (slot-1)*8 + 5;
   dice->ms3 = (board-1)*32 + (slot-1)*8 + 1;
   dice->rs = (board-1)*32 + (slot-1)*8 + 2;
   dice->dir = (board-1)*32 + (slot-1)*8 + 3;
   dice->step = (board-1)*32 + (slot-1)*8 + 6;
   dice->slp = (board-1)*32 + (slot-1)*8 + 7;

   //set up the register buffers with initial values
   dice->userValues[DRIVER_CONTROL_REGISTER_VALUE]=DRIVER_CONTROL_REGISTER | INITIAL_MICROSTEPPING;
   dice->userValues[CHOPPER_CONFIG_REGISTER]=CHOPPER_CONFIG_REGISTER;
	
   //setting the default register values
   dice->userValues[DRIVER_CONTROL_REGISTER_VALUE]=DRIVER_CONTROL_REGISTER|INITIAL_MICROSTEPPING;
   // microsteps = (1 << INITIAL_MICROSTEPPING);
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]=CHOPPER_CONFIG_REGISTER;
   dice->userValues[COOL_STEP_REGISTER_VALUE]=COOL_STEP_REGISTER;
   dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE]=STALL_GUARD2_LOAD_MEASURE_REGISTER;
   dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] = DRIVER_CONFIG_REGISTER | READ_STALL_GUARD_READING;

   //unselect CS
   iochain_setbit(dice->enable);
   iochain_update();


   return 0;
}

int dice_tmc_start(struct DICE* dice)
{
   //send initial values
   send262(dice,dice->userValues[DRIVER_CONTROL_REGISTER_VALUE]); 
   send262(dice,dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]);
   send262(dice,dice->userValues[COOL_STEP_REGISTER_VALUE]);
   send262(dice,dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE]);
   send262(dice,dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]);

   //set the current
   dice_tmc_setCurrent(dice,DEFAULT_CURRENT);
   //set to a conservative start value
   dice_tmc_setConstantOffTimeChopper(dice,7, 54, 13,12,1);
   //set a nice microstepping value
   dice_tmc_setMicrosteps(dice,DEFAULT_MICROSTEPPING);
}


void dice_tmc_setCurrent(struct DICE* dice,unsigned int current) 
{
    unsigned char current_scaling = 0;
    //calculate the current scaling from the max current setting (in mA)
    double mASetting = (double)current;
    double resistor_value = SENSE_RESISTOR;

    // remove vesense flag
    dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] &= ~(VSENSE);	
    //this is derrived from I=(cs+1)/32*(Vsense/Rsense)
    //leading to cs = CS = 32*R*I/V (with V = 0,31V oder 0,165V  and I = 1000*current)
    //with Rsense=0,15
    //for vsense = 0,310V (VSENSE not set)
    //or vsense = 0,165V (VSENSE set)
    current_scaling = (unsigned char)((resistor_value*mASetting*32.0/(0.31*1000.0*1000.0))-0.5); //theoretically - 1.0 for better rounding it is 0.5
	
    //check if the current scalingis too low
    if (current_scaling<16) 
    {
        //set the csense bit to get a use half the sense voltage (to support lower motor currents)
	dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] |= VSENSE;
        //and recalculate the current setting
        current_scaling = (unsigned char)((resistor_value*mASetting*32.0/(0.165*1000.0*1000.0))-0.5); //theoretically - 1.0 for better rounding it is 0.5
    }

    //do some sanity checks
    if (current_scaling>31)
    {
	current_scaling=31;
    }

    //delete the old value
    dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] &= ~(CURRENT_SCALING_PATTERN);
    //set the new current scaling
    dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] |= current_scaling;
    
    //send 
    send262(dice,dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]);
    send262(dice,dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE]);
   
}

/*
 * Set the number of microsteps per step.
 * 0,2,4,8,16,32,64,128,256 is supported
 * any value in between will be mapped to the next smaller value
 * 0 and 1 set the motor in full step mode
 */
void dice_tmc_setMicrosteps(struct DICE* dice,int number_of_steps) {
	long setting_pattern;
	//poor mans log
	if (number_of_steps>=256) {
		setting_pattern=0;
	} else if (number_of_steps>=128) {
		setting_pattern=1;
	} else if (number_of_steps>=64) {
		setting_pattern=2;
	} else if (number_of_steps>=32) {
		setting_pattern=3;
	} else if (number_of_steps>=16) {
		setting_pattern=4;
	} else if (number_of_steps>=8) {
		setting_pattern=5;
	} else if (number_of_steps>=4) {
		setting_pattern=6;
	} else if (number_of_steps>=2) {
		setting_pattern=7;
    //1 and 0 lead to full step
	} else if (number_of_steps<=1) {
		setting_pattern=8;
	}

	//delete the old value
	dice->userValues[DRIVER_CONTROL_REGISTER_VALUE] &=0xFFFF0ul;
	//set the new value
	dice->userValues[DRIVER_CONTROL_REGISTER_VALUE] |=setting_pattern;
	
	send262(dice,dice->userValues[DRIVER_CONTROL_REGISTER_VALUE]);

}

/*
 * constant_off_time: The off time setting controls the minimum chopper frequency. 
 * For most applications an off time within	the range of 5μs to 20μs will fit.
 *		2...15: off time setting
 *
 * blank_time: Selects the comparator blank time. This time needs to safely cover the switching event and the
 * duration of the ringing on the sense resistor. For
 *		0: min. setting 3: max. setting
 *
 * fast_decay_time_setting: Fast decay time setting. With CHM=1, these bits control the portion of fast decay for each chopper cycle.
 *		0: slow decay only
 *		1...15: duration of fast decay phase
 *
 * sine_wave_offset: Sine wave offset. With CHM=1, these bits control the sine wave offset. 
 * A positive offset corrects for zero crossing error.
 *		-3..-1: negative offset 0: no offset 1...12: positive offset
 *
 * use_current_comparator: Selects usage of the current comparator for termination of the fast decay cycle. 
 * If current comparator is enabled, it terminates the fast decay cycle in case the current 
 * reaches a higher negative value than the actual positive value.
 *		1: enable comparator termination of fast decay cycle
 *		0: end by time only
 */
void dice_tmc_setConstantOffTimeChopper(struct DICE* dice,char constant_off_time, char blank_time, char fast_decay_time_setting, char sine_wave_offset, unsigned char use_current_comparator)
{
   //perform some sanity checks
   if (constant_off_time<2) {
   	constant_off_time=2;
   } else if (constant_off_time>15) {
  	constant_off_time=15;
   }
  
   char blank_value;
   //calculate the value acc to the clock cycles
   if (blank_time>=54) {
 	blank_value=3;
   } else if (blank_time>=36) {
 	blank_value=2;
   } else if (blank_time>=24) {
 	blank_value=1;
   } else {
 	blank_value=0;
   }
   if (fast_decay_time_setting<0) {
	fast_decay_time_setting=0;
   } else if (fast_decay_time_setting>15) {
	fast_decay_time_setting=15;
   }
   if (sine_wave_offset < -3) {
        sine_wave_offset = -3;
   } else if (sine_wave_offset>12) {
	sine_wave_offset = 12;
   }
   //shift the sine_wave_offset
   sine_wave_offset +=3;
	
   //calculate the register setting
   //first of all delete all the values for this 
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] &= ~((1<<12) | BLANK_TIMING_PATTERN | HYSTERESIS_DECREMENT_PATTERN | HYSTERESIS_LOW_VALUE_PATTERN | HYSTERESIS_START_VALUE_PATTERN | T_OFF_TIMING_PATERN);
   //set the constant off pattern
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= CHOPPER_MODE_T_OFF_FAST_DECAY;
   //set the blank timing value
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= ((unsigned long)blank_value) << BLANK_TIMING_SHIFT;
   //setting the constant off time
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= constant_off_time;
   //set the fast decay time
   //set msb
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= (((unsigned long)(fast_decay_time_setting & 0x8))<<HYSTERESIS_DECREMENT_SHIFT);
   //other bits
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= (((unsigned long)(fast_decay_time_setting & 0x7))<<HYSTERESIS_START_VALUE_SHIFT);
   //set the sine wave offset
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= (unsigned long)sine_wave_offset << HYSTERESIS_LOW_SHIFT;
   //using the current comparator?
   if (!use_current_comparator) {
 	dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= (1<<12);
   }
  
   send262(dice,dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]);
	
}


void send262(struct DICE* dice,unsigned long datagram)
{
    unsigned long i_datagram;
    
    //select the TMC driver
    iochain_setbit((1<<dice->enable));
    iochain_update();

    //ensure that only valid bit are set (0-19)
    //datagram &=REGISTER_BIT_PATTERN;
	
    //write/read the values
    spi_transfernb((unsigned char*)&datagram,(unsigned char*)&i_datagram,4);

    //deselect the TMC chip
    iochain_clearbit((1<<dice->enable));
    iochain_update();
 
    //store the datagram as status result
    dice->userValues[DRIVER_STATUS_RESULT] = i_datagram;
}
