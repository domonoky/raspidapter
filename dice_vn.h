//
// Raspidapter Library Code
//
// DICE VN header 
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

#ifndef DICE_VN_H
#define DICE_VN_H

#include "dice_common.h"

// Setup a DICE in a specific slot  
int dice_vn_setup(struct DICE* dice,int board, int slot, int number);

// configure pins for output or input - a 0 means output a 1 means input
int dice_vn_setoutput(struct DICE* dice,int pins);

//configure the pin polarity
int dice_vn_setpolarity(struct DICE* dice,int pins);

//set pins - pins should be set as output
int dice_vn_set(struct DICE* dice, int pins);

//read pins - pins should be set as input
int dice_vn_read(struct DICE* dice, int pins);


#endif