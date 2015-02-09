//
// Raspidapter Library Code
//
// DICE STK header 
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

#ifndef DICE_STK_H
#define DICE_STK_H

#include "dice_common.h"

// Setup a DICE in a specific slot  
int dice_stk_setup(struct DICE* dice,int board, int slot);

// set the DICE
int dice_stk_step(struct DICE* dice);
//change direction
int dice_stk_dir(struct DICE* dice,int dir);
// enable the driver
int dice_stk_enable(struct DICE* dice,int enable);
//change substepping - only 1,2,4,8,16,32,64,128 is possible
int dice_stk_substepping(struct DICE* dice,int substepping);

#endif
