#
# Makefile for raspidapter test programs
#
#
# Copyright (C) Dominik Wenger 2015
# No rights reserved
# You may treat this program as if it was in the public domain
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Keep things simple for novice users
# so do not use any implicit rules!
#

all : test

clean :
	rm *.o test

test : raspidapter_common.o test.o dice_stk.o dice_9555.o dice_vn.o
	gcc -o test raspidapter_common.o dice_stk.o dice_9555.o dice_vn.o test.o -l bcm2835


# The next lines generate the various object files

dice_stk.o : dice_stk.c dice_stk.h dice_common.h raspidapter_common.h

dice_9555.o : dice_9555.c dice_9555.h dice_common.h raspidapter_common.h

dice_vn.o : dice_vn.c dice_vn.h dice_common.h raspidapter_common.h

raspidapter_common.o : raspidapter_common.c raspidapter_common.h 
	gcc -c raspidapter_common.c -I /usr/include/

test.o : test.c raspidapter_common.h
	gcc -c test.c

