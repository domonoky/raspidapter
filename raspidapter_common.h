//
// Raspidapter IO code
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



#include <stdio.h>


// Error codes
#define ERR_PARAM -1
#define ERR_INIT -2
#define ERR_I2C -3

// main setup routine
// param: number of connected boards
int setup_raspidapter(int numboards);

//frees allocated resources
int deinit_raspidapter();


//functions to access the serial clocked IOs - normally only used by the DICE functions

// set a bit in the IO chain
int iochain_setbit(int bit);

//clear a bit in the IO chain
int iochain_clearbit(int bit);

// update buffered IOs to the hardware - blocks whiel sending
int iochain_update();

// function to access the i2C 
int read_i2c(int address, char reg, int amount, char* data);

int write_i2c(int address, char reg, int amount, char* data);