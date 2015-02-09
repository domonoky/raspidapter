//
// Raspidapter test suite
//
// dice header 
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

#ifndef DICE_H
#define DICE_H

// define all sorts of DICE Types

#define DICE_NO 0
#define DICE_STK 1
#define DICE_9555 2
#define DICE_VN 3
#define DICE_TMC 4


// ERROR codes
// ERR_PARAM and ERR_INIT are define in the rapidapter_common header

#define NUM_USER_VALUES 6

//common information for all dices
struct DICE 
{
   //general IOs
   char type;
   int enable;
   int ms1;
   int ms2;
   int ms3;
   int rs;
   int dir;
   int step;
   int slp;

   int i2c_addr;

   unsigned long userValues[NUM_USER_VALUES];
};

#endif