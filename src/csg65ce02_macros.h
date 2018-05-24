//  csg65ce02_macros.h
//
//  Created by elmerucr on 11/01/2017 - 11/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.
//
//  This file is only to be included from csg65ce02.c, nowhere else!

// general macros
#define msb(n)      (uint8_t)((n & 0xff00) >> 8)         // get msb from a word
#define lsb(n)      (uint8_t)(n & 0x00ff)                // get lsb from a word

// register access macros
#define pcReg       thisCPU->pc
#define aReg        thisCPU->a
#define xReg        thisCPU->x
#define yReg        thisCPU->y
#define zReg		thisCPU->z
#define bReg		thisCPU->b
#define spReg       thisCPU->sp

// flag position definition macros
#define nFlagValue       0x80	// Negative flag
#define vFlagValue       0x40	// Overflow
#define eFlagValue       0x20	// Extend flag (16bit composed of spl and sph), when set, only spl is used
#define bFlagValue       0x10	// Break - does NOT really exist. When brk instruction, just save in stack
#define dFlagValue       0x08	// Decimal flag - please note the 65ce02 bug that's been found
#define iFlagValue       0x04	// Interrupt
#define zFlagValue       0x02	// Zero
#define cFlagValue       0x01	// Carry

// flag set pseudofunctions
#define setStatusForN(n)								\
            if(n & nFlagValue) {						\
                thisCPU->nFlag = nFlagValue;			\
            } else {									\
                thisCPU->nFlag = 0x00;					\
            }

#define setStatusForZ(n)								\
            if(n) {										\
                thisCPU->zFlag = 0x00;					\
            } else {									\
                thisCPU->zFlag = zFlagValue;			\
            }

// combined flag set pseudofunctions
#define setStatusForNZ(n)   setStatusForN(n); setStatusForZ(n)

// memory access macros
#define memory      thisCPU->ram
#define pcReg1		(uint16_t)(pcReg+1)
#define pcReg2		(uint16_t)(pcReg+2)
#define op1			memory[pcReg1]
#define op2			memory[pcReg2]