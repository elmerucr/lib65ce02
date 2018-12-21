//  csg65ce02.c
//
//  Created by elmerucr on 11/01/2017.
//  Copyright © 2018 elmerucr. All rights reserved.

#ifndef CSG65CE02_H
#define CSG65CE02_H

#include <stdint.h>
#include <stdbool.h>

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
	//uint8_t		bFlag;	// Break - does exist as a permanent '1'
							// doesn't make sense to implement at this stage.
							// At interrupt, just push the 'cause' in stack
	uint8_t		dFlag;		// Decimal flag - please note the 65ce02 bug that's been found
	uint8_t		iFlag;		// Interrupt
	uint8_t 	zFlag;		// Zero
	uint8_t		cFlag;		// Carry

	// other things to keep track of and need to be accessible from 'outside'
	uint16_t		instruction_counter;
	unsigned int	cycle_count;
	uint8_t			cycles_last_executed_instruction;	// necessary to decide if an irq will be acknowledged
	unsigned int	remaining_cycles;

	// info and pointer to a 64k array with breakpoint information
	bool		breakpoints_active;
	bool		*breakpoint_array;
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

// Init procedure, disables breakpoints and creates dynamically internal breakpoint array of 64k
void csg65ce02_init(csg65ce02 *thisCPU);

// Cleanup procedure, frees memory allocated by init function
void csg65ce02_cleanup(csg65ce02 *thisCPU);

// Reset procedure
void csg65ce02_reset(csg65ce02 *thisCPU);

// Breakpoint functions
void csg65ce02_enable_breakpoints(csg65ce02 *thisCPU);
void csg65ce02_disable_breakpoints(csg65ce02 *thisCPU);
void csg65ce02_add_breakpoint(csg65ce02 *thisCPU, uint16_t address);
void csg65ce02_remove_breakpoint(csg65ce02 *thisCPU, uint16_t address);

//	Memory Operations as seen from the CPU
uint8_t	csg65ce02_read_byte(uint16_t address);
void	csg65ce02_write_byte(uint16_t address, uint8_t byte);

// stack operations, take correctly into account status of e flag
void	csg65ce02_push_byte(csg65ce02 *thisCPU, uint8_t byte);
uint8_t	csg65ce02_pull_byte(csg65ce02 *thisCPU);

//	Execute a number of cycles on the virtual cpu.
//	When called with 0 cycles, only one instruction will be executed
//	The function returns the number of cycles taken by the cpu
//
//	Note: when an instr takes only 1 cycle, a pending irq will not be acknowledged, it has to wait
//	--> How to implement this? --> just store the duration of the currently executed instruction
unsigned int csg65ce02_execute(csg65ce02 *thisCPU, unsigned int no_cycles);

// returns 1 on success, else 0
unsigned int csg65ce02_trigger_irq(csg65ce02 *thisCPU);

// must always be acknowledged
void csg65ce02_trigger_nmi(csg65ce02 *thisCPU);


//	Help functions for convenience
//	It currently uses printf to dump the status of the cpu
void csg65ce02_dump_status(csg65ce02 *thisCPU);

void csg65ce02_dump_page(csg65ce02 *thisCPU, uint8_t pageNo);

#endif
