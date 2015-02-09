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

// Setup a DICE in a specific slot  
int dice_tmc_setup(struct DICE* dice,int board, int slot);

//needs to be called after all DICE_TMCs are setup
int dice_tmc_start(struct DICE* dice);

int dice_tmc_step(struct DICE* dice);
int dice_tmc_dir(struct DICE* dice,int dir);

void dice_tmc_setMicrosteps(struct DICE* dice,int number_of_steps);
void dice_tmc_setConstantOffTimeChopper(struct DICE* dice,char constant_off_time, char blank_time, char fast_decay_time_setting, char sine_wave_offset, unsigned char use_current_comparator);
void dice_tmc_setSpreadCycleChopper(struct DICE* dice,char constant_off_time, char blank_time, char hysteresis_start, char hysteresis_end, char hysteresis_decrement);
void dice_tmc_setRandomOffTime(struct DICE* dice,char value);
void dice_tmc_setCurrent(struct DICE* dice,unsigned int current);    
void dice_tmc_setStallGuardThreshold(struct DICE* dice,char stall_guard_threshold, char stall_guard_filter_enabled);	
void dice_tmc_setCoolStepConfiguration(struct DICE* dice,unsigned int lower_SG_threshold, unsigned int SG_hysteresis, unsigned char current_decrement_step_size,
                                  unsigned char current_increment_step_size, unsigned char lower_current_limit);    
void dice_tmc_setCoolStepEnabled(struct DICE* dice,char enabled);
void dice_tmc_readStatus(struct DICE* dice,char read_value);

#endif