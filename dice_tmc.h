//
// Raspidapter Library Code
//
// DICE TMC header 
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

#ifndef DICE_TMC_H
#define DICE_TMC_H

#include "dice_common.h"


//! return value for TMC26XStepper.getOverTemperature() if there is a overtemperature situation in the TMC chip
/*!
 * This warning indicates that the TCM chip is too warm. 
 * It is still working but some parameters may be inferior. 
 * You should do something against it.
 */
#define TMC26X_OVERTEMPERATURE_PREWARING 1
//! return value for TMC26XStepper.getOverTemperature() if there is a overtemperature shutdown in the TMC chip
/*!
 * This warning indicates that the TCM chip is too warm to operate and has shut down to prevent damage. 
 * It will stop working until it cools down again.
 * If you encouter this situation you must do something against it. Like reducing the current or improving the PCB layout 
 * and/or heat management.
 */
#define TMC26X_OVERTEMPERATURE_SHUTDOWN 2

//which values can be read out
/*!
 * Selects to readout the microstep position from the motor.
 *\sa readStatus()
 */
#define TMC26X_READOUT_POSITION 0
/*!
 * Selects to read out the StallGuard value of the motor.
 *\sa readStatus()
 */
#define TMC26X_READOUT_STALLGUARD 1
/*!
 * Selects to read out the current current setting (acc. to CoolStep) and the upper bits of the StallGuard value from the motor.
 *\sa readStatus(), setCurrent()
 */
#define TMC26X_READOUT_CURRENT 3

/*!
 * Define to set the minimum current for CoolStep operation to 1/2 of the selected CS minium.
 *\sa setCoolStepConfiguration()
 */
#define COOL_STEP_HALF_CS_LIMIT 0
/*!
 * Define to set the minimum current for CoolStep operation to 1/4 of the selected CS minium.
 *\sa setCoolStepConfiguration()
 */
#define COOL_STEP_QUARTDER_CS_LIMIT 1

// Setup a DICE in a specific slot  
// dice  - the dice struct to set up
// board - the board number where the dice is, counting from 1
// slot - the slot where the dice is on the board. Values between 1 and 4 are valid.
int dice_tmc_setup(struct DICE* dice,int board, int slot);

//needs to be called after all DICE_TMCs are setup. Sends some default values to the driver
// dice - the dice to start
int dice_tmc_start(struct DICE* dice);

// step the dice for one step
// dice - the dice to step
int dice_tmc_step(struct DICE* dice);

// change direction on a dice
// dice - the dice to work on
// dir - the direction, 1 or 0
int dice_tmc_dir(struct DICE* dice,int dir);

// Set the number of microsteps in 2^i values (rounded) up to 256
//
// This method set's the number of microsteps per step in 2^i interval.
// This means you can select 1, 2, 4, 16, 32, 64, 128 or 256 as valid microsteps.
// If you give any other value it will be rounded to the next smaller number (3 would give a microstepping of 2).
void dice_tmc_setMicrosteps(struct DICE* dice,int number_of_steps);

// Sets and configure the classical Constant Off Timer Chopper
// dice - the dice to work on
// constant_off_time  - The off time setting controls the minimum chopper frequency. For most applications an off time within the range of 5μs to 20μs will fit. Setting this parameter to zero completely disables all driver transistors and the motor can free-wheel. 0: chopper off, 1:15: off time setting (1 will work with minimum blank time of 24 clocks)
// blank_time -  Selects the comparator blank time. This time needs to safely cover the switching event and the duration of the ringing on the sense resistor. For most low current drivers, a setting of 1 or 2 is good. For high current applications with large MOSFETs, a setting of 2 or 3 will be required. 0 (min setting) … (3) amx setting
// fast_decay_time_setting - Fast decay time setting. Controls the portion of fast decay for each chopper cycle. 0: slow decay only, 1…15: duration of fast decay phase
// sine_wave_offset  - Sine wave offset. Controls the sine wave offset. A positive offset corrects for zero crossing error. -3…-1: negative offset, 0: no offset,1…12: positive offset
// use_curreent_comparator - Selects usage of the current comparator for termination of the fast decay cycle. If current comparator is enabled, it terminates the fast decay cycle in case the current reaches a higher negative value than the actual positive value. (0 disable, -1 enable).
//
// The classic constant off time chopper uses a fixed portion of fast decay following each on phase. 
// While the duration of the on time is determined by the chopper comparator, the fast decay time needs 
// to be set by the user in a way, that the current decay is enough for the driver to be able to follow 
// the falling slope of the sine wave, and on the other hand it should not be too long, in order to minimize 
// motor current ripple and power dissipation. This best can be tuned using an oscilloscope or 
// trying out motor smoothness at different velocities. A good starting value is a fast decay time setting 
// similar to the slow decay time setting.
// After tuning of the fast decay time, the offset should be determined, in order to have a smooth zero transition. 
// This is necessary, because the fast decay phase leads to the absolute value of the motor current being lower 
// than the target current (see figure 17). If the zero offset is too low, the motor stands still for a short 
// moment during current zero crossing, if it is set too high, it makes a larger microstep.
// Typically, a positive offset setting is required for optimum operation.
//
// See also dice_tmc_setSpreadCycleChoper() for other alternatives.
// See also dice_tmc_setRandomOffTime() for spreading the noise over a wider spectrum
void dice_tmc_setConstantOffTimeChopper(struct DICE* dice,char constant_off_time, char blank_time, char fast_decay_time_setting, char sine_wave_offset, unsigned char use_current_comparator);

// Sets and configures with spread cycle chopper.
// dice - the dice to work on
// constant_off_time - The off time setting controls the minimum chopper frequency. For most applications an off time within the range of 5μs to 20μs will fit. Setting this parameter to zero completely disables all driver transistors and the motor can free-wheel. 0: chopper off, 1:15: off time setting (1 will work with minimum blank time of 24 clocks)
// blank_time - Selects the comparator blank time. This time needs to safely cover the switching event and the duration of the ringing on the sense resistor. For most low current drivers, a setting of 1 or 2 is good. For high current applications with large MOSFETs, a setting of 2 or 3 will be required. 0 (min setting) … (3) amx setting
// hysteresis_start - Hysteresis start setting. Please remark, that this value is an offset to the hysteresis end value. 1 … 8
// hysteresis_end - Hysteresis end setting. Sets the hysteresis end value after a number of decrements. Decrement interval time is controlled by hysteresis_decrement. The sum hysteresis_start + hysteresis_end must be <16. At a current setting CS of max. 30 (amplitude reduced to 240), the sum is not limited.
// hysteresis_decrement - Hysteresis decrement setting. This setting determines the slope of the hysteresis during on time and during fast decay time. 0 (fast decrement) … 3 (slow decrement).
//
// The spreadCycle chopper scheme (pat.fil.) is a precise and simple to use chopper principle, which automatically determines 
// the optimum fast decay portion for the motor. Anyhow, a number of settings can be made in order to optimally fit the driver 
// to the motor.
// Each chopper cycle is comprised of an on-phase, a slow decay phase, a fast decay phase and a second slow decay phase.
// The slow decay phases limit the maximum chopper frequency and are important for low motor and driver power dissipation. 
// The hysteresis start setting limits the chopper frequency by forcing the driver to introduce a minimum amount of 
// current ripple into the motor coils. The motor inductivity determines the ability to follow a changing motor current. 
// The duration of the on- and fast decay phase needs to cover at least the blank time, because the current comparator is 
// disabled during this time.
// 
// See also: setRandomOffTime() for spreading the noise over a wider spectrum
void dice_tmc_setSpreadCycleChopper(struct DICE* dice,char constant_off_time, char blank_time, char hysteresis_start, char hysteresis_end, char hysteresis_decrement);

// Use random off time for noise reduction (0 for off, -1 for on).
// value 0 for off, -1 for on
//
// In a constant off time chopper scheme both coil choppers run freely, i.e. are not synchronized. 
// The frequency of each chopper mainly depends on the coil current and the position dependant motor coil inductivity, 
// thus it depends on the microstep position. With some motors a slightly audible beat can occur between the chopper 
// frequencies, especially when they are near to each other. This typically occurs at a few microstep positions within 
// each quarter wave. 
// This effect normally is not audible when compared to mechanical noise generated by ball bearings, 
// etc. Further factors which can cause a similar effect are a poor layout of sense resistor GND connection.
// In order to minimize the effect of a beat between both chopper frequencies, an internal random generator is provided. 
// It modulates the slow decay time setting when switched on. The random off time feature further spreads the chopper spectrum,
// reducing electromagnetic emission on single frequencies.
void dice_tmc_setRandomOffTime(struct DICE* dice,char value);

// Set the maximum motor current in mA (1000 is 1 Amp)
// Keep in mind this is the maximum peak Current. The RMS current will be 1/sqrt(2) smaller. The actual current can also be smaller
// by employing CoolStep.
// dice - the dice to work on
// current - the maximum motor current in mA
void dice_tmc_setCurrent(struct DICE* dice,unsigned int current);    

// readout the motor maximum current in mA (1000 is an Amp)
// This is the maximum current. to get the current current - which may be affected by CoolStep us getCurrentCurrent()
// returns the maximum motor current in milli amps
// See also getCurrentCurrent()
unsigned int dice_tmc_getCurrent(struct DICE* dice);

// set the StallGuard threshold in order to get sensible StallGuard readings.
// dice - the dice to work on
// stall_guard_threshold-  -64 … 63 the StallGuard threshold
// stall_guard_filter_enabled - 0 if the filter is disabled, -1 if it is enabled
//
// The StallGuard threshold is used to optimize the StallGuard reading to sensible values. It should be at 0 at
// the maximum allowable load on the otor (but not before). = is a good starting point (and the default)
// If you get Stall Gaurd readings of 0 without any load or with too little laod increase the value.
// If you get readings of 1023 even with load decrease the setting.
//
// If you switch on the filter the StallGuard reading is only updated each 4th full step to reduce the noise in the
// reading.
void dice_tmc_setStallGuardThreshold(struct DICE* dice,char stall_guard_threshold, char stall_guard_filter_enabled);

// reads out the StallGuard threshold
// return a number between -64 and 63.
char dice_tmc_getStallGuardThreshold(struct DICE* dice);

// returns the current setting of the StallGuard filter
// return 0 if not set, -1 if set
char dice_tmc_getStallGuardFilter(struct DICE* dice);

// This method configures the CoolStep smart energy operation. You must have a proper StallGuard configuration for the motor situation (current, voltage, speed) in order to use this feature. This also enables cool step.
// dice - the dice to work on
// lower_SG_threshold - Sets the lower threshold for stallGuard2TM reading. Below this value, the motor current becomes increased. Allowed values are 0...480
// SG_hysteresis - Sets the distance between the lower and the upper threshold for stallGuard2TM reading. Above the upper threshold (which is lower_SG_threshold+SG_hysteresis+1) the motor current becomes decreased. Allowed values are 0...480
// current_decrement_step_size - Sets the current decrement steps. If the StallGuard value is above the threshold the current gets decremented by this step size. 0...32
// current_increment_step_size - Sets the current increment step. The current becomes incremented for each measured stallGuard2TM value below the lower threshold. 0...8
// lower_current_limit - Sets the lower motor current limit for coolStepTM operation by scaling the CS value. Values can be COOL_STEP_HALF_CS_LIMIT, COOL_STEP_QUARTER_CS_LIMIT
//
// The CoolStep smart energy operation automatically adjust the current sent into the motor according to the current load,
// read out by the StallGuard in order to provide the optimum torque with the minimal current consumption.
// You configure the CoolStep current regulator by defining upper and lower bounds of StallGuard readouts. If the readout is above the 
// limit the current gets increased, below the limit the current gets decreased.
// You can specify the upper an lower threshold of the StallGuard readout in order to adjust the current. You can also set the number of
// StallGuard readings neccessary above or below the limit to get a more stable current adjustement.
// The current adjustement itself is configured by the number of steps the current gets in- or decreased and the absolut minimum current
// (1/2 or 1/4th otf the configured current).
// See also: COOL_STEP_HALF_CS_LIMIT, COOL_STEP_QUARTER_CS_LIMIT
void dice_tmc_setCoolStepConfiguration(struct DICE* dice,unsigned int lower_SG_threshold, unsigned int SG_hysteresis, unsigned char current_decrement_step_size,
                                  unsigned char current_increment_step_size, unsigned char lower_current_limit);  

// enables or disables the CoolStep smart energy operation feature. It must be configured before enabling it.
// dice - the dice to work on
// enabled - true if CoolStep should be enabled, false if not.
// See also: dice_tmc_setCoolStepConfiguration() 
void dice_tmc_setCoolStepEnabled(struct DICE* dice,char enabled);

// returns the lower StallGuard threshold for the CoolStep operation
// See also dice_tmc_setCoolStepConfiguration()
unsigned int dice_tmc_getCoolStepLowerSgThreshold(struct DICE* dice);
    
// returns the upper StallGuard threshold for the CoolStep operation
// See also dice_tmc_setCoolStepConfiguration()
unsigned int dice_tmc_getCoolStepUpperSgThreshold(struct DICE* dice);
    
// returns the number of StallGuard readings befor CoolStep adjusts the motor current.
//See also dice_tmc_setCoolStepConfiguration()
unsigned char dice_tmc_getCoolStepNumberOfSGReadings(struct DICE* dice);
    
//returns the increment steps for the current for the CoolStep operation
// See also dice_tmc_setCoolStepConfiguration()
unsigned char dice_tmc_getCoolStepCurrentIncrementSize(struct DICE* dice);
    
// returns the absolut minium current for the CoolStep operation
// See also  dice_tmc_setCoolStepConfiguration()
// see also COOL_STEP_HALF_CS_LIMIT, COOL_STEP_QUARTER_CS_LIMIT
unsigned char dice_tmc_getCoolStepLowerCurrentLimit(struct DICE* dice);

// Get the current microstep position for phase A
// return The current microstep position for phase A 0…255
//
// Keep in mind that this routine reads and writes a value via SPI - so this may take a bit time.
int dice_tmc_getMotorPosition(struct DICE* dice);

// Reads the current StallGuard value.
// return The current StallGuard value, lesser values indicate higher load, 0 means stall detected.
// Keep in mind that this routine reads and writes a value via SPI - so this may take a bit time.
// See also setStallGuardThreshold() for tuning the readout to sensible ranges.
int dice_tmc_getCurrentStallGuardReading(struct DICE* dice);

// Reads the current current setting value as fraction of the maximum current
// Returns values between 0 and 31, representing 1/32 to 32/32 (=1)
// See also setCoolStepConfiguration()
unsigned char dice_tmc_getCurrentCSReading(struct DICE* dice);

// a convenience method to determine if the current scaling uses 0.31V or 0.165V as reference.
// return false if 0.13V is the reference voltage, true if 0.165V is used.
char dice_tmc_isCurrentScalingHalfed(struct DICE* dice);

// Reads the current current setting value and recalculates the absolute current in mA (1A would be 1000).
// This method calculates the currently used current setting (either by setting or by CoolStep) and reconstructs
// the current in mA by usinge the VSENSE and resistor value. This method uses floating point math - so it 
// may not be the fastest.
// See also getCurrentCSReading(), getResistor(), isCurrentScalingHalfed(), getCurrent()
unsigned int dice_tmc_getCurrentCurrent(struct DICE* dice);
    
// checks if there is a StallGuard warning in the last status
//return 0 if there was no warning, -1 if there was some warning.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
//
// See also setStallGuardThreshold() for tuning the readout to sensible ranges.
char dice_tmc_isStallGuardOverThreshold(struct DICE* dice);
    
// Return over temperature status of the last status readout
// return 0 is everything is OK, TMC26X_OVERTEMPERATURE_PREWARING if status is reached, TMC26X_OVERTEMPERATURE_SHUTDOWN is the chip is shutdown, -1 if the status is unknown.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_getOverTemperature(struct DICE* dice);

// Is motor channel A shorted to ground detected in the last status readout.
// return true is yes, false if not.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_isShortToGroundA(struct DICE* dice);

// Is motor channel B shorted to ground detected in the last status readout.
// return true is yes, false if not.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_isShortToGroundB(struct DICE* dice);

// Is motor channel A connected according to the last statu readout.
// return true is yes, false if not.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_isOpenLoadA(struct DICE* dice);

// Is motor channel A connected according to the last statu readout.
// return true is yes, false if not.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_isOpenLoadB(struct DICE* dice);
    
// Is chopper inactive since 2^20 clock cycles - defaults to ~0,08s
// return true is yes, false if not.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
char dice_tmc_isStandStill(struct DICE* dice);

// checks if there is a StallGuard warning in the last status
// return 0 if there was no warning, -1 if there was some warning.
// Keep in mind that this method does not enforce a readout but uses the value of the last status readout.
// You may want to use getMotorPosition() or getCurrentStallGuardReading() to enforce an updated status readout.
// See also isStallGuardOverThreshold()
// See also setStallGuardThreshold() for tuning the readout to sensible ranges.
char dice_tmc_isStallGuardReached(struct DICE* dice);
    
// enables or disables the motor driver bridges. If disabled the motor can run freely. If enabled not.
// enabled a boolean value true if the motor should be enabled, false otherwise.
void dice_tmc_setEnabled(struct DICE* dice,char enabled);

// checks if the output bridges are enabled. If the bridges are not enabled the motor can run freely
// return true if the bridges and by that the motor driver are enabled, false if not.
// See also setEnabled()
char dice_tmc_isEnabled(struct DICE* dice);

// Manually read out the status register
// This function sends a byte to the motor driver in order to get the current readout. The parameter read_value
// seletcs which value will get returned. If the read_vlaue changes in respect to the previous readout this method
// automatically send two bytes to the motor: one to set the redout and one to get the actual readout. So this method 
// may take time to send and read one or two bits - depending on the previous readout.
//
// read_value - selects which value to read out (0..3). You can use the defines TMC26X_READOUT_POSITION, TMC_262_READOUT_STALLGUARD, or TMC_262_READOUT_CURRENT
// See also TMC26X_READOUT_POSITION, TMC_262_READOUT_STALLGUARD, TMC_262_READOUT_CURRENT
void dice_tmc_readStatus(struct DICE* dice,char read_value);

#endif