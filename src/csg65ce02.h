//  csg65ce02.c
//
//  Created by elmerucr from 11/01/2017 - 11/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#ifndef CSG65CE02_H
#define CSG65CE02_H

#include <stdint.h>

typedef struct {
	uint16_t	pc;			// program counter
	uint8_t		a;			// accumulator
	uint8_t		x;			// x register
	uint8_t		y;			// y register
	uint8_t		z;			// z register
	uint8_t		b;			// b register, high register of basepage
	uint16_t	sp;			// stack pointer 16 bits

	// status register (7 individual flags)
	uint8_t		nFlag;		// Negative flag
	uint8_t		vFlag;		// Overflow
	uint8_t		eFlag;		// Extend flag (16bit composed of spl and sph), when set, only spl is used
	//uint8_t		bFlag;		// Break - does NOT exist. When brk instruction, just save a value in stack
	uint8_t		dFlag;		// Decimal flag - please note the 65ce02 bug that's been found
	uint8_t		iFlag;		// Interrupt
	uint8_t 	zFlag;		// Zero
	uint8_t		cFlag;		// Carry

	// other things to keep track of
	uint8_t		*ram;								// pointer to memory array
	uint8_t		cycles_last_executed_instruction;	// necessary to check if irq will be ackn.

} csg65ce02;

enum addressing_modes {
	IMM,
	ABS,
	BP,
	ACCUM,
	IMPLIED,
	BP_X,
	BP_Y,
	ABS_X,
	ABS_Y,
	BP_X_IND,
	BP_IND_Y,
	BP_IND_Z,
	D_SP_IND_Y,
	REL,
	WREL,
	ABS_IND,
	BPREL,
	ABS_X_IND,
	IMMW,
	ABSW
};

//	Tables supplying instruction specific information
extern const int addressing_mode_per_instruction[];
extern const uint8_t bytes_per_instruction[];
extern const uint8_t cycles_per_instruction[];
extern const uint8_t modify_pc_per_instruction[];

//	For now, init just connects a memory pointer to the cpu
void csg65ce02_init(csg65ce02 *thisCPU, uint8_t *mem);

//	Reset procedure
void csg65ce02_reset(csg65ce02 *thisCPU);

// stack operations, take correctly into account status of e flag
void	csg65ce02_push_byte(csg65ce02 *thisCPU, uint8_t byte);
uint8_t	csg65ce02_pull_byte(csg65ce02 *thisCPU);

//	Execute a number of cycles on the virtual cpu.
//	When called with 0 cycles, it executes only one instruction
//	The function returns the number of cycles taken by the cpu
//
//	Note: when an instr takes only 1 cycle, a pending irq will not be acknowledged, it has to wait
//	--> How to implement this? --> just store the duration of the currently executed instruction
unsigned int csg65ce02_execute(csg65ce02 *thisCPU, unsigned int noCycles);

// returns 1 on success, else 0
unsigned int csg65ce02_trigger_irq(csg65ce02 *thisCPU);

// always successful
void csg65ce02_trigger_nmi(csg65ce02 *thisCPU);

//	It currently uses printf to dump the status of the cpu
void csg65ce02_dump_status(csg65ce02 *thisCPU);

void csg65ce02_dump_page(csg65ce02 *thisCPU, uint8_t pageNo);

#endif