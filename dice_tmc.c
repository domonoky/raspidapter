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
#define LOWER_SG_THRESHOLD 6
#define CONSTANT_OFFTIME 7

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
int getReadoutValue(struct DICE* dice);

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
	
   //setting the default register values
   dice->userValues[DRIVER_CONTROL_REGISTER_VALUE]=DRIVER_CONTROL_REGISTER|INITIAL_MICROSTEPPING;
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

   return 0;
}

int dice_tmc_step(struct DICE* dice)
{
   iochain_setbit(dice->step);
   iochain_update();
   // todo wait ?
   iochain_clearbit(dice->step);
   iochain_update();
   
}

int dice_tmc_dir(struct DICE* dice,int dir)
{
   if(dir) iochain_setbit(dice->dir);
   else iochain_clearbit(dice->dir);
   iochain_update();
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

unsigned int dice_tmc_getCurrent(struct DICE* dice)
{
    //we calculate the current according to the datasheet to be on the safe side
    //this is not the fastest but the most accurate and illustrative way
    double result = (double)(dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] & CURRENT_SCALING_PATTERN);
    double resistor_value = SENSE_RESISTOR;
    double voltage = (dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] & VSENSE)? 0.165:0.31;
    result = (result+1.0)/32.0*voltage/SENSE_RESISTOR*1000.0*1000.0;
    return (unsigned int)result;
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
   // save offtime
   dice->userValues[CONSTANT_OFFTIME] = constant_off_time;

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

/*
 * constant_off_time: The off time setting controls the minimum chopper frequency. 
 * For most applications an off time within	the range of 5μs to 20μs will fit.
 *		2...15: off time setting
 *
 * blank_time: Selects the comparator blank time. This time needs to safely cover the switching event and the
 * duration of the ringing on the sense resistor. For
 *		0: min. setting 3: max. setting
 *
 * hysteresis_start: Hysteresis start setting. Please remark, that this value is an offset to the hysteresis end value HEND.
 *		1...8
 *
 * hysteresis_end: Hysteresis end setting. Sets the hysteresis end value after a number of decrements. Decrement interval time is controlled by HDEC. 
 * The sum HSTRT+HEND must be <16. At a current setting CS of max. 30 (amplitude reduced to 240), the sum is not limited.
 *		-3..-1: negative HEND 0: zero HEND 1...12: positive HEND
 *
 * hysteresis_decrement: Hysteresis decrement setting. This setting determines the slope of the hysteresis during on time and during fast decay time.
 *		0: fast decrement 3: very slow decrement
 */

void dice_tmc_setSpreadCycleChopper(struct DICE* dice,char constant_off_time, char blank_time, char hysteresis_start, char hysteresis_end, char hysteresis_decrement) 
{
   //perform some sanity checks
   if (constant_off_time<2) {
	constant_off_time=2;
   } else if (constant_off_time>15) {
 	constant_off_time=15;
   }
   // save offtime
   dice->userValues[CONSTANT_OFFTIME] = constant_off_time;

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
   if (hysteresis_start<1) {
  	hysteresis_start=1;
   } else if (hysteresis_start>8) {
	hysteresis_start=8;
   }
   hysteresis_start--;

   if (hysteresis_end < -3) {
 	hysteresis_end = -3;
   } else if (hysteresis_end>12) {
 	hysteresis_end = 12;
   }
   //shift the hysteresis_end
   hysteresis_end +=3;

   if (hysteresis_decrement<0) {
 	hysteresis_decrement=0;
   } else if (hysteresis_decrement>3) {
	hysteresis_decrement=3;
   }
	
   //first of all delete all the values for this
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] &= ~(CHOPPER_MODE_T_OFF_FAST_DECAY | BLANK_TIMING_PATTERN | HYSTERESIS_DECREMENT_PATTERN | HYSTERESIS_LOW_VALUE_PATTERN | HYSTERESIS_START_VALUE_PATTERN | T_OFF_TIMING_PATERN);

   //set the blank timing value
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= ((unsigned long)blank_value) << BLANK_TIMING_SHIFT;
   //setting the constant off time
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= constant_off_time;
   //set the hysteresis_start
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= ((unsigned long)hysteresis_start) << HYSTERESIS_START_VALUE_SHIFT;
   //set the hysteresis end
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= ((unsigned long)hysteresis_end) << HYSTERESIS_LOW_SHIFT;
   //set the hystereis decrement
   dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= ((unsigned long)blank_value) << BLANK_TIMING_SHIFT;
   
   send262(dice,dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]);
}

/*
 * In a constant off time chopper scheme both coil choppers run freely, i.e. are not synchronized. 
 * The frequency of each chopper mainly depends on the coil current and the position dependant motor coil inductivity, thus it depends on the microstep position. 
 * With some motors a slightly audible beat can occur between the chopper frequencies, especially when they are near to each other. This typically occurs at a 
 * few microstep positions within each quarter wave. This effect normally is not audible when compared to mechanical noise generated by ball bearings, etc. 
 * Further factors which can cause a similar effect are a poor layout of sense resistor GND connection.
 * Hint: A common factor, which can cause motor noise, is a bad PCB layout causing coupling of both sense resistor voltages 
 * (please refer to sense resistor layout hint in chapter 8.1).
 * In order to minimize the effect of a beat between both chopper frequencies, an internal random generator is provided. 
 * It modulates the slow decay time setting when switched on by the RNDTF bit. The RNDTF feature further spreads the chopper spectrum, 
 * reducing electromagnetic emission on single frequencies.
 */
void dice_tmc_setRandomOffTime(struct DICE* dice,char value) 
{
   if (value) {
	dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]|= RANDOM_TOFF_TIME;
   } else {
	dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] &= ~(RANDOM_TOFF_TIME);
   }	

   send262(dice,dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]);	
}


void dice_tmc_setStallGuardThreshold(struct DICE* dice,char stall_guard_threshold, char stall_guard_filter_enabled) 
{
   if (stall_guard_threshold<-64) {
	stall_guard_threshold = -64;
   //We just have 5 bits	
   } else if (stall_guard_threshold > 63) {
 	stall_guard_threshold = 63;
   }
   //add trim down to 7 bits
   stall_guard_threshold &=0x7f;
   //delete old stall guard settings
   dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] &= ~(STALL_GUARD_CONFIG_PATTERN);
   if (stall_guard_filter_enabled) {
 	dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] |= STALL_GUARD_FILTER_ENABLED;
   }
   //Set the new stall guard threshold
   dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] |= (((unsigned long)stall_guard_threshold << 8) & STALL_GUARD_CONFIG_PATTERN);

   send262(dice,dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE]);
}

char dice_tmc_getStallGuardThreshold(struct DICE* dice) 
{
    unsigned long stall_guard_threshold = dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE] & STALL_GUARD_VALUE_PATTERN;
    //shift it down to bit 0
    stall_guard_threshold >>=8;
    //convert the value to an int to correctly handle the negative numbers
    char result = stall_guard_threshold;
    //check if it is negative and fill it up with leading 1 for proper negative number representation
    if (result & (1<<6)) {
        result |= 0xC0;
    }
    return result;
}

char dice_tmc_getStallGuardFilter(struct DICE* dice) {
    if (dice->userValues[STALL_GUARD2_CURRENT_REGISTER_VALUE]& STALL_GUARD_FILTER_ENABLED) {
        return -1;
    } else {
        return 0;
    }
}

void dice_tmc_setCoolStepConfiguration(struct DICE* dice,unsigned int lower_SG_threshold, unsigned int SG_hysteresis, unsigned char current_decrement_step_size,
                              unsigned char current_increment_step_size, unsigned char lower_current_limit) {
    //sanitize the input values
    if (lower_SG_threshold>480) {
        lower_SG_threshold = 480;
    }
    //divide by 32
    lower_SG_threshold >>=5;
    if (SG_hysteresis>480) {
        SG_hysteresis=480;
    }
    //divide by 32
    SG_hysteresis >>=5;
    if (current_decrement_step_size>3) {
        current_decrement_step_size=3;
    }
    if (current_increment_step_size>3) {
        current_increment_step_size=3;
    }
    if (lower_current_limit>1) {
        lower_current_limit=1;
    }
    //store the lower level in order to enable/disable the cool step
    dice->userValues[LOWER_SG_THRESHOLD]=lower_SG_threshold;
    
    //the good news is that we can start with a complete new cool step register value
    //and simply set the values in the register
   dice->userValues[COOL_STEP_REGISTER_VALUE]= ((unsigned long)lower_SG_threshold) | (((unsigned long)SG_hysteresis)<<8) | (((unsigned long)current_decrement_step_size)<<5)
        | (((unsigned long)current_increment_step_size)<<13) | (((unsigned long)lower_current_limit)<<15)
        //and of course we have to include the signature of the register
        | COOL_STEP_REGISTER;
    
    send262(dice,dice->userValues[COOL_STEP_REGISTER_VALUE]);
}

void dice_tmc_setCoolStepEnabled(struct DICE* dice,char enabled) {
    //simply delete the lower limit to disable the cool step
    dice->userValues[COOL_STEP_REGISTER_VALUE] &= ~SE_MIN_PATTERN;
    //and set it to the proper value if cool step is to be enabled
    if (enabled) {
        dice->userValues[COOL_STEP_REGISTER_VALUE] |=dice->userValues[LOWER_SG_THRESHOLD];
    }

    send262(dice,dice->userValues[COOL_STEP_REGISTER_VALUE]);
}

unsigned int dice_tmc_getCoolStepLowerSgThreshold(struct DICE* dice) {
    //we return our internally stored value - in order to provide the correct setting even if cool step is not enabled
    return dice->userValues[LOWER_SG_THRESHOLD]<<5;
}

unsigned int dice_tmc_getCoolStepUpperSgThreshold(struct DICE* dice) {
    return (unsigned char)((dice->userValues[COOL_STEP_REGISTER_VALUE]& SE_MAX_PATTERN)>>8)<<5;
}

unsigned char dice_tmc_getCoolStepCurrentIncrementSize(struct DICE* dice) {
    return (unsigned char)((dice->userValues[COOL_STEP_REGISTER_VALUE] & CURRENT_DOWN_STEP_SPEED_PATTERN)>>13);
}

unsigned char dice_tmc_getCoolStepNumberOfSGReadings(struct DICE* dice) {
    return (unsigned char)((dice->userValues[COOL_STEP_REGISTER_VALUE] & SE_CURRENT_STEP_WIDTH_PATTERN)>>5);
}

unsigned char dice_tmc_getCoolStepLowerCurrentLimit(struct DICE* dice) {
    return (unsigned char)((dice->userValues[COOL_STEP_REGISTER_VALUE] & MINIMUM_CURRENT_FOURTH)>>15);
}

int dice_tmc_getMotorPosition(struct DICE* dice) {
   //we read it out even if we are not started yet - perhaps it is useful information for somebody 
    dice_tmc_readStatus(dice,TMC26X_READOUT_POSITION);
    return dice_tmc_getReadoutValue(dice);
}

//reads the stall guard setting from last status
//returns -1 if stallguard information is not present
int dice_tmc_getCurrentStallGuardReading(struct DICE* dice) 
{
  //not time optimal, but solution optiomal:
  //first read out the stall guard value
  dice_tmc_readStatus(dice,TMC26X_READOUT_STALLGUARD);
  return dice_tmc_getReadoutValue(dice);
}

//reads the stall guard setting from last status
//returns -1 if stallguard information is not present
int dice_tmc_setCurrentStallGuardReading(struct DICE* dice) 
{
  //not time optimal, but solution optiomal:
  //first read out the stall guard value
  dice_tmc_readStatus(dice,TMC26X_READOUT_STALLGUARD);
  return dice_tmc_getReadoutValue(dice);
}

unsigned char dice_tmc_getCurrentCSReading(struct DICE* dice) 
{
  //not time optimal, but solution optiomal:
  //first read out the stall guard value
  dice_tmc_readStatus(dice,TMC26X_READOUT_CURRENT);
  return (dice_tmc_getReadoutValue(dice) & 0x1f);
}

unsigned int dice_tmc_getCurrentCurrent(struct DICE* dice) {
    double result = (double)dice_tmc_getCurrentCSReading(dice);
    double resistor_value = (double)SENSE_RESISTOR;
    double voltage = (dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] & VSENSE)? 0.165:0.31;
    result = (result+1.0)/32.0*voltage/resistor_value*1000.0*1000.0;
    return (unsigned int)result;
}


// return true if the stallguard threshold has been reached
char dice_tmc_isStallGuardOverThreshold(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_STALL_GUARD_STATUS);
}


// returns if there is any over temperature condition:
// OVER_TEMPERATURE_PREWARING if pre warning level has been reached
// OVER_TEMPERATURE_SHUTDOWN if the temperature is so hot that the driver is shut down
// Any of those levels are not too good.

char dice_tmc_getOverTemperature(struct DICE* dice) 
{
   if (dice->userValues[DRIVER_STATUS_RESULT]  & STATUS_OVER_TEMPERATURE_SHUTDOWN) {
	return TMC26X_OVERTEMPERATURE_SHUTDOWN;
   }
   if (dice->userValues[DRIVER_STATUS_RESULT]  & STATUS_OVER_TEMPERATURE_WARNING) {
 	return TMC26X_OVERTEMPERATURE_PREWARING;
   }
  return 0;
}

//is motor channel A shorted to ground
char dice_tmc_isShortToGroundA(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_SHORT_TO_GROUND_A);
}

//is motor channel B shorted to ground
char dice_tmc_isShortToGroundB(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_SHORT_TO_GROUND_B);
}

//is motor channel A connected
char dice_tmc_isOpenLoadA(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_OPEN_LOAD_A);
}

//is motor channel B connected
char dice_tmc_isOpenLoadB(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_OPEN_LOAD_B);
}

//is chopper inactive since 2^20 clock cycles - defaults to ~0,08s
char dice_tmc_isStandStill(struct DICE* dice) {
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_STAND_STILL);
}

//is chopper inactive since 2^20 clock cycles - defaults to ~0,08s
char dice_tmc_isStallGuardReached(struct DICE* dice) 
{
   return (dice->userValues[DRIVER_STATUS_RESULT] & STATUS_STALL_GUARD_STATUS);
}

void dice_tmc_setEnabled(struct DICE* dice,char enabled) {
    //delete the t_off in the chopper config to get sure
    dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] &= ~(T_OFF_PATTERN);
    if (enabled) {
        //and set the t_off time
        dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] |= dice->userValues[CONSTANT_OFFTIME];
    }
    //if not enabled we don't have to do anything since we already delete t_off from the register
    send262(dice,dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE]);	
}

char dice_tmc_isEnabled(struct DICE* dice) {
    if (dice->userValues[CHOPPER_CONFIG_REGISTER_VALUE] & T_OFF_PATTERN) {
        return 1;
    } else {
        return 0;
    }
}

// reads a value from the TMC26X status register. The value is not obtained directly but can then 
// be read by the various status routines.
void dice_tmc_readStatus(struct DICE* dice,char read_value) 
{
    unsigned long old_driver_configuration_register_value = dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE];
    //reset the readout configuration
   dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] &= ~(READ_SELECTION_PATTERN);
   //this now equals TMC26X_READOUT_POSITION - so we just have to check the other two options
   if (read_value == TMC26X_READOUT_STALLGUARD) {
  	dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]|= READ_STALL_GUARD_READING;
   } else if (read_value == TMC26X_READOUT_CURRENT) {
 	dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE] |= READ_STALL_GUARD_AND_COOL_STEP;
   }
   //all other cases are ignored to prevent funny values
   //check if the readout is configured for the value we are interested in
   if (dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]!=old_driver_configuration_register_value) {
     //because then we need to write the value twice - one time for configuring, second time to get the value, see below
      send262(dice,dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]);
   }
   //write the configuration to get the last status    
   send262(dice,dice->userValues[DRIVER_CONFIGURATION_REGISTER_VALUE]);
}

int dice_tmc_getReadoutValue(struct DICE* dice) {
   return (int)(dice->userValues[DRIVER_STATUS_RESULT] >> 10);
}

void send262(struct DICE* dice,unsigned long datagram)
{
    unsigned long i_datagram=0;
    int i;
    //select the TMC driver
    iochain_clearbit(dice->enable);
    iochain_update();

    //ensure that only valid bit are set (0-19)
    //datagram &=REGISTER_BIT_PATTERN;
	
    //write/read the values
    i_datagram = spi_transfer((datagram >> 16) & 0xff);
    i_datagram <<= 8;
    i_datagram |= spi_transfer((datagram >>  8) & 0xff);
    i_datagram <<= 8;
    i_datagram |= spi_transfer((datagram) & 0xff);
    i_datagram >>= 4;
     
    //deselect the TMC chip
    iochain_setbit(dice->enable);
    iochain_update();

 
    //store the datagram as status result
    dice->userValues[DRIVER_STATUS_RESULT] = i_datagram;
}
