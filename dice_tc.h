//
// Raspidapter Library Code
//
// DICE TC header 
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

#ifndef DICE_TC_H
#define DICE_TC_H

#include "dice_common.h"

// Setup a DICE in a specific slot  
// dice  - the dice struct to set up
// board - the board number where the dice is, counting from 1
// slot - the slot where the dice is on the board. Values between 1 and 4 are valid.
int dice_tc_setup(struct DICE* dice,int board, int slot);

double dice_tc_readInternalTemp(struct DICE* dice,unsigned char chipnum);
double dice_tc_readCelsius(struct DICE* dice,unsigned char chipnum);
double dice_tc_readFarenheit(struct DICE* dice,unsigned char chipnum);
unsigned char dice_tc_readError(struct DICE* dice,unsigned char chipnum);

#endif