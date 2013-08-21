# ----------------------------------------------------------------------------
#         :oCCCCOCoc.
#     .cCO8OOOOOOOOO8Oo:
#   .oOO8OOOOOOOOOOOOOOOCc
#  cO8888:         .:oOOOOC.                                                TM
# :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
# C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
# O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
# C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
# :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
#  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
#    cO@@@@@@@@@@@@@@@@@Oc0
#      :oO8@@@@@@@@@@Oo.
#         .oCOOOOOCc.                                      http://remood.org/
# ----------------------------------------------------------------------------
# Copyright (C) 2013-2013 ReMooD       <http://remood.org/>
# Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
#                                      <ghostlydeath@gmail.com>
# ----------------------------------------------------------------------------
# THERE ARE TWO LICENSES AVAILABLE FOR THIS FILE ONLY WITH ADDITIONAL TERMS:
#
# * THIS HEADER MUST NOT BE REMOVED, REGARDLESS OF WHICH LICENSE YOU CHOOSE.
# * KEEPING THESE LICENSE DESCRIPTIONS IN THE HEADER WILL NOT AFFECT NOR FORCE
# * THE CHOICE OF THE LICENSE AS LONG AS IT IS ONE OF THE FOLLOWING LISTED.
#
#  GNU GENERAL PUBLIC LICENSE 3 OR LATER
#  * This program is free software; you can redistribute it and/or
#  * modify it under the terms of the GNU General Public License
#  * as published by the Free Software Foundation; either version 3
#  * of the License, or (at your option) any later version.
#  * 
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
# 
#  SIMPLIFIED BSD LICENSE:
#  * Redistribution and use in source and binary forms, with or without
#  * modification, are permitted provided that the following conditions are
#  * met: 
#  * 
#  * 1. Redistributions of source code must retain the above copyright notice,
#  *    this list of conditions and the following disclaimer. 
#  * 2. Redistributions in binary form must reproduce the above copyright
#  *    notice, this list of conditions and the following disclaimer in the
#  *    documentation and/or other materials provided with the distribution. 
#  * 
#  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
#  * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#  * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
#  * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  * 
#  * The views and conclusions contained in the software and documentation are
#  * those of the authors and should not be interpreted as representing
#  * official policies, either expressed or implied, of the ReMooD Project.
# ----------------------------------------------------------------------------
# Makefile for bots
#
# Note that -G 1 is needed for GCC, because it will try to place unresolved
# external globals of really small size in the static area (ouch)

GCCDIR			?= ../usr/bin
GCCPREFIX		?= mips-elf-

__BOT_SOURCES	:= $(wildcard *.c)
__BOT_BASE		:= $(foreach __c,${__BOT_SOURCES},$(basename ${__c}))
__BOT_LMPS		:= $(foreach __c,${__BOT_SOURCES},$(basename ${__c}).lmp)
__BOT_OBJS		:= $(foreach __c,${__BOT_SOURCES},$(basename ${__c}).o)
__BOT_SES		:= $(foreach __c,${__BOT_SOURCES},$(basename ${__c}).s)
__BOT_INC		:= $(foreach __c,${__BOT_SOURCES},$(basename ${__c}).i)

.PHONY: all
all:			${__BOT_BASE}
				

define __rule

.PHONY: $1
$1:				$1.lmp
				

endef

$(foreach __in,${__BOT_BASE},$(eval $(call __rule,${__in})))

%.o:			%.c start.o ../../src/bot_lib.h
				${GCCDIR}/${GCCPREFIX}gcc -I ../../src/ -O0 -o $@ $< start.o -Wl,--just-symbols=extern.sym -T linker.ld -g -nostdlib -nostdinc -Wl,-Ttext,0x8000 -EL -march=mips32 -mabi=32 -G 1 -fno-merge-constants

%.lmp:			%.o
				${GCCDIR}/${GCCPREFIX}objcopy -O binary -j .text $< $@
				${GCCDIR}/${GCCPREFIX}objdump -M no-aliases -S -D -j .text $< > $(basename $@).od

start.o:		start.s
				${GCCDIR}/${GCCPREFIX}as -o $@ $< -EL -march=mips32

clean:			
				rm -f ${__BOT_LMPS} ${__BOT_OBJS} ${__BOT_SES} ${__BOT_INC} start.o

