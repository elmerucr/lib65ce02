//  csg65ce02.c
//
//  Created by elmerucr on 11/01/2017.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "csg65ce02.h"

// general macros
#define MSB(n)      (uint8_t)((n & 0xff00) >> 8)         // get MSB from a word
#define LSB(n)      (uint8_t)(n & 0x00ff)                // get LSB from a word

// flag position definition macros
#define N_FLAG_VALUE       0x80	// Negative flag
#define V_FLAG_VALUE       0x40	// Overflow
#define E_FLAG_VALUE       0x20	// Extend flag (16bit, spl and sph togeth), when set, only spl is used
#define B_FLAG_VALUE       0x10	// Break - does NOT exist. At interrupt, save bit value in stack
#define D_FLAG_VALUE       0x08	// Decimal flag - please note the 65ce02 bug that's been found (there is one)
#define I_FLAG_VALUE       0x04	// Interrupt
#define Z_FLAG_VALUE       0x02	// Zero
#define C_FLAG_VALUE       0x01	// Carry

// register access macros
#define PC_REG		thisCPU->pc
#define A_REG		thisCPU->a
#define X_REG		thisCPU->x
#define Y_REG		thisCPU->y
#define Z_REG		thisCPU->z
#define B_REG		thisCPU->b
#define SP_REG		thisCPU->sp

// flag set pseudofunctions
#define SET_STATUS_FOR_N(n)								\
            if(n & N_FLAG_VALUE) {						\
                thisCPU->n_flag = N_FLAG_VALUE;			\
            } else {									\
                thisCPU->n_flag = 0x00;					\
            }

#define SET_STATUS_FOR_Z(n)								\
            if(n) {										\
                thisCPU->z_flag = 0x00;					\
            } else {									\
                thisCPU->z_flag = Z_FLAG_VALUE;			\
            }

// combined flag set pseudofunctions
#define SET_STATUS_FOR_NZ(n)   SET_STATUS_FOR_N(n); SET_STATUS_FOR_Z(n)

// memory access macros
#define PC_REG1		(uint16_t)(PC_REG+1)
#define PC_REG2		(uint16_t)(PC_REG+2)
#define op1			csg65ce02_read_byte(PC_REG1)
#define op2			csg65ce02_read_byte(PC_REG2)

// library internal function
void csg65ce02_calculate_effective_address(csg65ce02 *thisCPU, uint8_t opcode, uint16_t *eal, uint16_t *eah);
void csg65ce02_handle_opcode(csg65ce02 *thisCPU, uint8_t opcode, uint16_t eal, uint16_t eah);

const int addressing_mode_per_instruction[256] =
{
	IMM,BP_X_IND,IMPLIED,IMPLIED,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP,BP_X,BP_X,BP,IMPLIED,ABS_Y,ACCUM,IMPLIED,ABS,ABS_X,ABS_X,BPREL,
	ABS,BP_X_IND,ABS_IND,ABS_X_IND,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_X,BP,IMPLIED,ABS_Y,ACCUM,IMPLIED,ABS_X,ABS_X,ABS_X,BPREL,
	IMPLIED,BP_X_IND,ACCUM,ACCUM,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_X,BP,IMPLIED,ABS_Y,IMPLIED,IMPLIED,IMPLIED,ABS_X,ABS_X,BPREL,
	IMPLIED,BP_X_IND,IMM,WREL,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS_IND,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_X,BP,IMPLIED,ABS_Y,IMPLIED,IMPLIED,ABS_X_IND,ABS_X,ABS_X,BPREL,
	REL,BP_X_IND,D_SP_IND_Y,WREL,BP,BP,BP,BP,IMPLIED,IMM,IMPLIED,ABS_X,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_Y,BP,IMPLIED,ABS_Y,IMPLIED,ABS_Y,ABS,ABS_X,ABS_X,BPREL,
	IMM,BP_X_IND,IMM,IMM,BP,BP,BP,BP,IMPLIED,IMM,IMPLIED,ABS,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_Y,BP,IMPLIED,ABS_Y,IMPLIED,ABS_X,ABS_X,ABS_X,ABS_Y,BPREL,
	IMM,BP_X_IND,IMM,BP,BP,BP,BP,BP,IMPLIED,IMM,IMPLIED,ABS,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP,BP_X,BP_X,BP,IMPLIED,ABS_Y,IMPLIED,IMPLIED,ABS,ABS_X,ABS_X,BPREL,
	IMM,BP_X_IND,D_SP_IND_Y,BP,BP,BP,BP,BP,IMPLIED,IMM,IMPLIED,ABS,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,IMMW,BP_X,BP_X,BP,IMPLIED,ABS_Y,IMPLIED,IMPLIED,ABSW,ABS_X,ABS_X,BPREL
};

const uint8_t bytes_per_instruction[256] =
{
	2,2,1,1,2,2,2,2,1,2,1,1,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,1,3,3,3,3,
	3,2,3,3,2,2,2,2,1,2,1,1,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,1,3,3,3,3,
	1,2,1,1,2,2,2,2,1,2,1,1,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,1,4,3,3,3,
	1,2,2,3,2,2,2,2,1,2,1,1,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,1,3,3,3,3,
	2,2,2,3,2,2,2,2,1,2,1,3,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,3,3,3,3,3,
	2,2,2,2,2,2,2,2,1,2,1,3,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,3,3,3,3,3,
	2,2,2,2,2,2,2,2,1,2,1,3,3,3,3,3,
	2,2,2,3,2,2,2,2,1,3,1,1,3,3,3,3,
	2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
	2,2,2,3,3,2,2,2,1,3,1,1,3,3,3,3
};

const uint8_t cycles_per_instruction[256] =
{
	7,5,2,2,4,3,4,4,3,2,1,1,5,4,5,4,
	2,5,5,3,4,3,4,4,1,4,1,1,5,4,5,4,
	5,5,7,7,4,3,4,4,3,2,1,1,5,4,5,4,
	2,5,5,3,4,3,4,4,1,4,1,1,5,4,5,4,
	5,5,2,2,4,3,4,4,3,2,1,1,3,4,5,4,
	2,5,5,3,4,3,4,4,2,4,3,1,4,4,5,4,
	4,5,7,5,3,3,4,4,3,2,1,1,5,4,5,4,
	2,5,5,3,3,3,4,4,2,4,3,1,5,4,5,4,
	2,5,6,3,3,3,3,4,1,2,1,4,4,4,4,4,
	2,5,5,3,3,3,3,4,1,4,1,4,4,4,4,4,
	2,5,2,2,3,3,3,4,1,2,1,4,4,4,4,4,
	2,5,5,3,3,3,3,4,1,4,1,4,4,4,4,4,
	2,5,2,6,3,3,4,4,1,2,1,7,4,4,5,4,
	2,5,5,3,3,3,4,4,1,4,3,3,4,4,5,4,
	2,5,6,6,3,3,4,4,1,2,1,6,4,4,5,4,
	2,5,5,3,5,3,4,4,1,4,3,3,7,4,5,4
};

const uint8_t modify_pc_per_instruction[256] =
{
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,0,0,0,0,0,0,0,0,1,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1
};

// Init procedure
void csg65ce02_init(csg65ce02 *thisCPU)
{
	thisCPU->breakpoints_active = false;
	thisCPU->breakpoint_array = (bool *)malloc(65536 * sizeof(bool));
	for(int i=0; i<65536; i++) thisCPU->breakpoint_array[i] = false;
}

// Cleanup procedure
void csg65ce02_cleanup(csg65ce02 *thisCPU)
{
	free(thisCPU->breakpoint_array);
}

void csg65ce02_reset(csg65ce02 *thisCPU)
{
    //A_REG = 0x00;			// don't care
    //X_REG = 0x00;			// don't care
    //Y_REG = 0x00;			// don't care
	Z_REG = 0x00;			// is actively set to 0 to emulate 65c02 stz instructions store zero
	B_REG = 0x00;			// init to 0 for correct emulation of earlier 65xx cpus ("zero-page")
    SP_REG = 0x01fd;			// is this the correct value => yes (see pagetable ...)
							// if e=1 then wraps around same page, if e=0 wraps around memory

	thisCPU->n_flag = 0x00;
	thisCPU->v_flag = 0x00;
	thisCPU->e_flag = E_FLAG_VALUE;	// starts flagged (8 bit sp on page $01) for 6502 compatible behaviour
	thisCPU->d_flag = 0x00;			// must be zero after reset!
	thisCPU->i_flag = I_FLAG_VALUE;	// starts flagged, to avoid irq's from happening - bootup code must 'cli' after setup
	thisCPU->z_flag = 0x00;
	thisCPU->c_flag = 0x00;

	// default states
	thisCPU->exception_type = NONE;
	thisCPU->nmi_pin_previous_state = true;

	// load reset vector into pc
    PC_REG = csg65ce02_read_byte(0xfffc) | (csg65ce02_read_byte(0xfffd) << 8);

	thisCPU->cycles_last_executed_instruction = 1;	// safe value after reset, so interrupts can't be acknowledged
													// this is useful for setting up an extended stack (cle, see)
}

// Breakpoint functions
void csg65ce02_enable_breakpoints(csg65ce02 *thisCPU)
{
	thisCPU->breakpoints_active = true;
}

void csg65ce02_disable_breakpoints(csg65ce02 *thisCPU)
{
	thisCPU->breakpoints_active = false;
}

void csg65ce02_add_breakpoint(csg65ce02 *thisCPU, uint16_t address)
{
	thisCPU->breakpoint_array[address] = true;
}

void csg65ce02_remove_breakpoint(csg65ce02 *thisCPU, uint16_t address)
{
	thisCPU->breakpoint_array[address] = false;
}

// stack operation push (stack always points to the current available position)
inline void csg65ce02_push_byte(csg65ce02 *thisCPU, uint8_t byte)
{
	if( thisCPU->e_flag )
	{												// 8 bit stack pointer
		csg65ce02_write_byte(SP_REG, byte);			// store the byte
		uint16_t temp_word = SP_REG & 0xff00;		// keep track of current MSB of 8 bit stack pointer
		SP_REG--;									// lower the stack pointer by one
		SP_REG = (SP_REG & 0x00ff) | temp_word;		// make sure MSB keeps it old value
	}
	else
	{												// 16 bit stack pointer, much simpler!
		csg65ce02_write_byte(SP_REG, byte);			// store the byte
		SP_REG--;									// lower the stack pointer by one
	}
}

// stack operation pull
inline uint8_t csg65ce02_pull_byte(csg65ce02 *thisCPU)
{
	if( thisCPU->e_flag )
	{															// 8 bit stack pointer
		uint16_t temp_word = SP_REG & 0xff00;					// sph must keep same value
		SP_REG++;												// increase the sp
		SP_REG = (SP_REG & 0x00ff) | temp_word;					// correct it if it crossed a page border
		return csg65ce02_read_byte(SP_REG);						// pull one byte and return it
	}
	else
	{															// 16 bit stack pointer, again much simpler
		SP_REG++;
		return csg65ce02_read_byte(SP_REG);
	}
}

// Main function, run a number of cycles on the virtual cpu, returns the no of processed cycles
// Exit code of the run function is stored in exit_code_run_function inside struct (0 on normal exit, 1 on ext breakpoint)
int csg65ce02_run(csg65ce02 *thisCPU, unsigned int no_cycles)
{
	uint8_t  current_opcode;
	uint16_t effective_address_l;		// low byte address of the effective address, normally used
	uint16_t effective_address_h;		// high byte address of the effective address (for IMMW / ABSW addressing)

	thisCPU->initial_cycles = no_cycles;
	thisCPU->remaining_cycles = no_cycles;

	// actual instruction loop
	do
	{
		// check exception conditions
		// MIGHT BE REPLACED BY A TABLE IN THE FUTURE
		if(thisCPU->cycles_last_executed_instruction == 1)
		{   // last instr took 1 cycle, skip all exceptions
			thisCPU->exception_type = NONE;
			// note that nmi previous state is not updated here
			// if we would do so, the nmi wouldn't be catched after the first non-1-cycle instruction
		}
		else
		{   // last instr took more than 1 cycle
            if(( *thisCPU->nmi_pin == false) && (thisCPU->nmi_pin_previous_state == true))
			{
                thisCPU->exception_type = NMI;
            }
			else
			{
                if(*thisCPU->irq_pin == true)
				{						            		// irq pin is up
                    thisCPU->exception_type = NONE;
                }
				else
				{									        // irq pin is down
                    if(thisCPU->i_flag)
					{						            	// irq masked by flag
                        thisCPU->exception_type = NONE;
                    }
					else
					{									    // irq not masked by flag
                        thisCPU->exception_type = IRQ;
                    }
                }
            }
			// update nmi pin previous state to current state
        	thisCPU->nmi_pin_previous_state = *thisCPU->nmi_pin;
        }

		current_opcode = thisCPU->exception_type ? 0x00 : csg65ce02_read_byte(PC_REG);

		csg65ce02_calculate_effective_address(thisCPU, current_opcode, &effective_address_l, &effective_address_h);

		csg65ce02_handle_opcode(thisCPU, current_opcode, effective_address_l, effective_address_h);

		thisCPU->cycles_last_executed_instruction = cycles_per_instruction[current_opcode];
		thisCPU->remaining_cycles -= thisCPU->cycles_last_executed_instruction;

		// increase pc only if the instruction does not actively change the pc by itself
		if( !modify_pc_per_instruction[current_opcode] )
		{
			PC_REG += bytes_per_instruction[current_opcode];
		}

		// check for breakpoint conditions
		if( (thisCPU->breakpoints_active == true ) && (thisCPU->breakpoint_array[PC_REG] == true) )
		{
			csg65ce02_end_timeslice(thisCPU);
		}
        // Three conditions must be met to keep running:
        //    (1) enough cycles?
        //    (2) no breakpoint?
        //    (3) breakpoints activated?
    }
	while(thisCPU->remaining_cycles > 0);

	thisCPU->exit_code_run_function = thisCPU->breakpoint_array[PC_REG] ? 1 : 0;

    return thisCPU->initial_cycles - thisCPU->remaining_cycles;
}

int csg65ce02_cycles_run(csg65ce02 *thisCPU)
{
	return thisCPU->initial_cycles = thisCPU->remaining_cycles;
}

int csg65ce02_cycles_remaining(csg65ce02 *thisCPU)
{
	return thisCPU->remaining_cycles;
}

void csg65ce02_add_cycles(csg65ce02 *thisCPU, unsigned int cycles)
{
	thisCPU->initial_cycles += cycles;
	thisCPU->remaining_cycles += cycles;
}

void csg65ce02_end_timeslice(csg65ce02 *thisCPU)
{
	thisCPU->initial_cycles -= thisCPU->remaining_cycles;
	thisCPU->remaining_cycles = 0;
}

inline void csg65ce02_calculate_effective_address(csg65ce02 *thisCPU, uint8_t opcode, uint16_t *eal, uint16_t *eah)
{
	// temporary storage possibilities
	uint16_t	temp_word;
	uint8_t		temp_byte;
	// calculate effective address
	switch( addressing_mode_per_instruction[opcode])
	{
		case IMM :
			*eal = PC_REG1;
			break;
		case ABS :
			*eal = op1 | (op2 << 8);
			break;
		case BP :
			*eal = op1 | (B_REG << 8);
			break;
		case ACCUM :
			// IMPLEMENTED IN INSTRUCTION
			break;
		case IMPLIED :
			// IMPLEMENTED IN INSTRUCTION
			break;
		case BP_X :
			*eal = ((uint8_t)(op1 + X_REG)) | (B_REG << 8);
			break;
		case BP_Y :
			*eal = ((uint8_t)(op1 + Y_REG)) | (B_REG << 8);
			break;
		case ABS_X :
			*eal = (op1 | (op2 << 8)) + X_REG;
			break;
		case ABS_Y :
			*eal = (op1 | (op2 << 8)) + Y_REG;
			break;
		case BP_X_IND :
			*eal = csg65ce02_read_byte((op1 + X_REG) | (B_REG << 8)) |
					(csg65ce02_read_byte((op1 + X_REG + 1) | (B_REG << 8)) << 8);
			break;
		case BP_IND_Y :
			*eal = (csg65ce02_read_byte( op1 | (B_REG << 8) ) |
					(csg65ce02_read_byte( ((uint8_t)(op1 + 1)) | (B_REG << 8) ) << 8)) + Y_REG;
			break;
		case BP_IND_Z :
			*eal = (csg65ce02_read_byte( op1 | (B_REG << 8) ) |
					(csg65ce02_read_byte( ((uint8_t)(op1 + 1)) | (B_REG << 8) ) << 8)) + Z_REG;
			break;
		case D_SP_IND_Y :
			// NOTE: SEE MEGA65 MANUAL ON GITHUB!
			// Stack Pointer Indirect, indexed by Y OPR (d,SP),Y (new)
			//
			// The second byte of the two-byte instruction contains an unsigned offset value, d,
			// which is added to the stack pointer (word) to form the address of two memory locations
			// whose contents are added to the unsigned Y register to form the address of the memory
			// location to be used by the operation.
			*eal = (uint16_t)((csg65ce02_read_byte((uint16_t)(SP_REG + op1)) | (csg65ce02_read_byte((uint16_t)(SP_REG + op1+ 1)) << 8)) + Y_REG);
			break;
		case REL :
			temp_byte = op1;
			temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
			*eal = (uint16_t)(PC_REG + 2 + temp_word);					// note "2"!
			break;
		case WREL :
			temp_word = ( (uint16_t)op1 ) | ( (uint16_t)op2 << 8 );
			*eal = (uint16_t)(PC_REG + 2 + temp_word);					// note "2"!
			break;
		case ABS_IND :
			*eal = csg65ce02_read_byte(op1 | (op2 << 8)) | (csg65ce02_read_byte((op1 | (op2 << 8))+1) << 8);
			break;
		case BPREL :
			temp_byte = op2;															// note "op2"!
			temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
			*eal = (uint16_t)(PC_REG + 3 + temp_word);					// note "3"!
			break;
		case ABS_X_IND :
			temp_word = (uint16_t)((op1 | (op2 << 8)) + X_REG);
			*eal = csg65ce02_read_byte(temp_word) | (csg65ce02_read_byte((uint16_t)(temp_word+1)) << 8);
			break;
		case IMMW :
			*eal = PC_REG1;
			*eah = PC_REG2;
			break;
		case ABSW :
			*eal = op1 | (op2 << 8);
			*eah = (uint16_t)(*eal + 1);
			break;
		default :
			break;
	};
}

inline void csg65ce02_handle_opcode(csg65ce02 *thisCPU, uint8_t opcode, uint16_t eal, uint16_t eah)
{
	uint8_t temp_byte;
	uint8_t temp_byte2;
	uint16_t temp_word;

	uint16_t effective_address_l = eal;
	uint16_t effective_address_h = eah;

	switch( opcode )
	{
		case 0x00 :								// brk instruction (or irq/nmi)
			switch(thisCPU->exception_type)
			{
				case NONE:
					// push high byte of the pc+2 on the stack (note rti will not increase pc on return)
					csg65ce02_push_byte(thisCPU, MSB((uint16_t)(PC_REG+2)));
					// push low byte of the pc+2 on the stack
					csg65ce02_push_byte(thisCPU, LSB((uint16_t)(PC_REG+2)));
					// prepare break flag to be pushed onto stack
					temp_byte2 = B_FLAG_VALUE;
					break;
				case IRQ:
				case NMI:
					// push high byte of the pc on the stack (note rti will not increase pc on return)
					csg65ce02_push_byte(thisCPU, MSB((uint16_t)(PC_REG)));
					// push low byte of the pc on the stack
					csg65ce02_push_byte(thisCPU, LSB((uint16_t)(PC_REG)));
					// prepare EMPTY break flag to be pushed onto stack
					temp_byte2 = 0x00;
					break;
			}

			// push sr onto stack
			temp_byte =	thisCPU->n_flag |
						thisCPU->v_flag |
						thisCPU->e_flag |
						temp_byte2     |	// pushes B_FLAG_VALUE when cause was brk instruction
						thisCPU->d_flag |
						thisCPU->i_flag |    // yes, the i flag as it was during last instruction is pushed!
						thisCPU->z_flag |
						thisCPU->c_flag;
			csg65ce02_push_byte(thisCPU, temp_byte);
			// set interrupt disable flag
			thisCPU->i_flag = I_FLAG_VALUE;

			// load correct exception vector for brk/irq/nmi
			// put this code somewhere else?
			switch(thisCPU->exception_type)
			{
				case NONE:
				case IRQ:
					thisCPU->pc = csg65ce02_read_byte(0xfffe) | (csg65ce02_read_byte(0xffff) << 8);
					break;
				case NMI:
					thisCPU->pc = csg65ce02_read_byte(0xfffa) | (csg65ce02_read_byte(0xfffb) << 8);
					break;
			}

			// clear the decimal flag (it will be restored by the rti instruction) (Eyes, p338)
			// note: in older versions of the cpu (nmos) this didn't happen, resulting in difficult to trace bugs
			thisCPU->d_flag = 0x00;
			break;
		case 0x01 :								// ora (bp,x)
		case 0x05 :								// ora bp
		case 0x09 :								// ora immediate
		case 0x0d :								// ora abs
		case 0x11 :								// ora (bp),y
		case 0x12 :								// ora (bp),z
		case 0x15 :								// ora bp,x
		case 0x19 :								// ora abs,y
		case 0x1d :								// ora abs,x
			A_REG = A_REG | (csg65ce02_read_byte(effective_address_l));
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x02 :								// cle
			thisCPU->e_flag = 0x00;
			break;
		case 0x03 :								// see
			thisCPU->e_flag = E_FLAG_VALUE;
			break;
		case 0x04 :								// tsb bp
		case 0x0c :								// tsb abs
			temp_byte = A_REG;
			temp_byte2 = csg65ce02_read_byte(effective_address_l);
			csg65ce02_write_byte(effective_address_l, temp_byte | temp_byte2);
			SET_STATUS_FOR_Z(temp_byte & temp_byte2);
			break;
		case 0x06 :								// asl bp
		case 0x0e :								// asl abs
		case 0x16 :								// asl bp,x
		case 0x1e :								// asl abs,x
			temp_byte = csg65ce02_read_byte(effective_address_l);
			if(temp_byte & 0x80)
			{
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			csg65ce02_write_byte(effective_address_l, (temp_byte << 1) );
			SET_STATUS_FOR_NZ((temp_byte << 1))
			break;
		case 0x07 :								// rmb 0,bp
		case 0x17 :								// rmb 1,bp
		case 0x27 :								// rmb 2,bp
		case 0x37 :								// rmb 3,bp
		case 0x47 :								// rmb 4,bp
		case 0x57 :								// rmb 5,bp
		case 0x67 :								// rmb 6,bp
		case 0x77 :								// rmb 7,bp
		case 0x87 :								// smb 0,bp
		case 0x97 :								// smb 0,bp
		case 0xa7 :								// smb 0,bp
		case 0xb7 :								// smb 0,bp
		case 0xc7 :								// smb 0,bp
		case 0xd7 :								// smb 0,bp
		case 0xe7 :								// smb 0,bp
		case 0xf7 :								// smb 0,bp
			// Which bit from bp address is going to be tested? Store it in temp_byte
			temp_byte = (opcode & 0x70) >> 4;
			if(opcode & 0x80)
			{						// set memory bit
				csg65ce02_write_byte(effective_address_l, csg65ce02_read_byte(effective_address_l) | (0x01 << temp_byte) );
			}
			else
			{						// reset (clear) memory bit
				csg65ce02_write_byte(effective_address_l, csg65ce02_read_byte(effective_address_l) & (0xff - (0x01 << temp_byte)));
			}
			break;
		case 0x08 :								// php (b_flag does not exist, but it will push the value anyway!)
			temp_byte =	thisCPU->n_flag |
						thisCPU->v_flag |
						thisCPU->e_flag |
						B_FLAG_VALUE     |
						thisCPU->d_flag |
						thisCPU->i_flag |
						thisCPU->z_flag |
						thisCPU->c_flag;
			csg65ce02_push_byte(thisCPU, temp_byte);
			break;
		case 0x0a :								// asl accumulator
			temp_byte = thisCPU->a;
			if(temp_byte & 0x80)
			{
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			thisCPU->a = (temp_byte << 1);
			SET_STATUS_FOR_NZ(thisCPU->a);
			break;
		case 0x0b :								// tsy
			Y_REG = (SP_REG & 0xff00) >> 8;
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0x0f :								// bbr 0,bp,rel
		case 0x1f :								// bbr 1,bp,rel
		case 0x2f :								// bbr 2,bp,rel
		case 0x3f :								// bbr 3,bp,rel
		case 0x4f :								// bbr 4,bp,rel
		case 0x5f :								// bbr 5,bp,rel
		case 0x6f :								// bbr 6,bp,rel
		case 0x7f :								// bbr 7,bp,rel
		case 0x8f :								// bbs 0,bp,rel
		case 0x9f :								// bbs 1,bp,rel
		case 0xaf :								// bbs 2,bp,rel
		case 0xbf :								// bbs 3,bp,rel
		case 0xcf :								// bbs 4,bp,rel
		case 0xdf :								// bbs 5,bp,rel
		case 0xef :								// bbs 6,bp,rel
		case 0xff :								// bbs 7,bp,rel
			// Calculate effective basepage address, store it in temp_word
			temp_word = op1 | (B_REG << 8);
			// Which bit will be tested for? And store it in temp_byte
			temp_byte = (opcode & 0x70) >> 4;
			// effect. address (rel) was already calculated
			if(opcode & 0x80)
			{						// test for set
				if(csg65ce02_read_byte(temp_word) & (0x01<<temp_byte))
				{
					PC_REG = effective_address_l;
				}
				else
				{
					PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
				}
			}
			else
			{								// test for clear
				if(!(csg65ce02_read_byte(temp_word) & (0x01<<temp_byte)))
				{
					PC_REG = effective_address_l;
				}
				else
				{
					PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
				}
			}
			break;
		case 0x10 :								// bpl rel
		case 0x13 :								// bpl wrel
			if(thisCPU->n_flag)
			{			// n flag is set, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			}
			else
			{			// n flag not set, take relative jump
				PC_REG = effective_address_l;
			}
			break;
		case 0x14 :								// trb bp
		case 0x1c :								// trb abs
			temp_byte = A_REG;
			temp_byte2 = csg65ce02_read_byte(effective_address_l);
			csg65ce02_write_byte(effective_address_l, (~temp_byte) & temp_byte2);
			SET_STATUS_FOR_Z(temp_byte & temp_byte2);
			break;
		case 0x18 :								// clc instruction
			thisCPU->c_flag = 0x00;
			break;
		case 0x1a :								// inc a instruction
			A_REG++;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x1b :								// inc z instruction
			Z_REG++;
			SET_STATUS_FOR_NZ(Z_REG);
			break;
		case 0x20 :								// jsr abs instruction (push last byte addr of instr
		case 0x22 :								// jsr (abs)
		case 0x23 :								// jsr (abs,x)
			csg65ce02_push_byte(thisCPU, MSB((uint16_t)(PC_REG+2)));
			csg65ce02_push_byte(thisCPU, LSB((uint16_t)(PC_REG+2)));
			PC_REG = effective_address_l;
			break;
		case 0x21 :								// and (bp,x)
		case 0x25 :								// and bp
		case 0x29 :								// and immediate
		case 0x2d :								// and abs
		case 0x31 :								// and (bp),y
		case 0x32 :								// and (bp),z
		case 0x35 :								// and bp,x
		case 0x39 :								// and abs,y
		case 0x3d :								// and abs,x
			A_REG = A_REG & (csg65ce02_read_byte(effective_address_l));
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x24 :								// bit bp
		case 0x2c :								// bit abs
		case 0x34 :								// bit bp,x
		case 0x3c :								// bit abs,x
		case 0x89 :								// bit immediate (note 65c02 not n&v flags!)
			temp_byte = csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_Z(temp_byte & A_REG);
			thisCPU->n_flag = temp_byte & N_FLAG_VALUE;
			thisCPU->v_flag = temp_byte & V_FLAG_VALUE;
			break;
		case 0x28 :								// plp
			temp_byte = csg65ce02_pull_byte(thisCPU);
			thisCPU->n_flag = temp_byte & N_FLAG_VALUE;
			thisCPU->v_flag = temp_byte & V_FLAG_VALUE;
					// no e flag, only to be changed by cle and see instructions
					// no b flag of course...
			thisCPU->d_flag = temp_byte & D_FLAG_VALUE;
			thisCPU->i_flag = temp_byte & I_FLAG_VALUE;
			thisCPU->z_flag = temp_byte & Z_FLAG_VALUE;
			thisCPU->c_flag = temp_byte & C_FLAG_VALUE;
			break;
		case 0x2b :								// tys
			SP_REG = (SP_REG & 0x00ff) | (Y_REG << 8);
			break;
		case 0x30 :								// bmi rel
		case 0x33 :								// bmi wrel
			if(thisCPU->n_flag)
			{			// n flag is set, take relative jump
				PC_REG = effective_address_l;
			}
			else
			{						// n flag not set, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			}
			break;
		case 0x38 :								// sec
			thisCPU->c_flag = C_FLAG_VALUE;
			break;
		case 0x3a :								// dec a
			A_REG--;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x3b :								// dec z
			Z_REG--;
			SET_STATUS_FOR_NZ(Z_REG);
			break;
		case 0x40 :								// rti
			temp_byte = csg65ce02_pull_byte(thisCPU);
			thisCPU->n_flag = temp_byte & N_FLAG_VALUE;
			thisCPU->v_flag = temp_byte & V_FLAG_VALUE;
			thisCPU->e_flag = temp_byte & E_FLAG_VALUE;
					// no b flag to set of course...
			thisCPU->d_flag = temp_byte & D_FLAG_VALUE;
			thisCPU->i_flag = temp_byte & I_FLAG_VALUE;
			thisCPU->z_flag = temp_byte & Z_FLAG_VALUE;
			thisCPU->c_flag = temp_byte & C_FLAG_VALUE;
			// restore the program counter
			PC_REG = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
			// note: rti doesn't need a correction of pc afterwards (unlike rts)
			break;
		case 0x41 :								// eor (bp,x)
		case 0x45 :								// eor bp
		case 0x49 :								// eor imm
		case 0x4d :								// eor abs
		case 0x51 :								// eor (bp),y
		case 0x52 :								// eor (bp),z
		case 0x55 :								// eor bp,x
		case 0x59 :								// eor abs,y
		case 0x5d :								// eor abs,x
			A_REG = A_REG ^ csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x42 :								// neg
			A_REG = ~A_REG;
			A_REG++;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x48 :								// pha
			csg65ce02_push_byte(thisCPU, A_REG);
			break;
		case 0x4b :								// taz
			Z_REG = A_REG;
			SET_STATUS_FOR_NZ(Z_REG);
			break;
		case 0x4c :								// jmp abs
		case 0x6c :								// jmp (abs)
		case 0x7c :								// jmp (abs,x)
			PC_REG = effective_address_l;
			break;
		case 0x58 :								// cli
			thisCPU->i_flag = 0x00;
			break;
		case 0x5a :								// phy
			csg65ce02_push_byte(thisCPU, Y_REG);
			break;
		case 0x5b :								// tab
			B_REG = A_REG;
			break;
		case 0x60 :								// rts instruction
			PC_REG = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
			PC_REG = (uint16_t)(PC_REG+1);		// increase pc by 1 and wrap if necessary
			break;
		case 0x61 :								// adc (bp,x) - NEEDS SPECIAL CARE FOR DECIMAL MODE
		case 0x65 :								// adc bp
		case 0x69 :								// adc immediate
		case 0x6d :								// adc absolute
		case 0x71 :								// adc (bp),y
		case 0x72 :								// adc (bp),z
		case 0x75 :								// adc bp,x
		case 0x79 :								// adc abs,y
		case 0x7d :								// adc abs,x
			temp_byte = csg65ce02_read_byte(effective_address_l);
			temp_word = A_REG + temp_byte + thisCPU->c_flag;
			if(temp_word & 0xff00) thisCPU->c_flag = C_FLAG_VALUE; else thisCPU->c_flag = 0;
			temp_byte2 = temp_word & 0xff;
			if(((A_REG^temp_byte2) & (temp_byte^temp_byte2)) & 0x80)
			{
				thisCPU->v_flag = V_FLAG_VALUE;
			}
			else
			{
				thisCPU->v_flag = 0x00;
			}
			A_REG = temp_byte2;
			SET_STATUS_FOR_NZ(A_REG);
			break;
        case 0x62 :                             // rtn immediate
            temp_byte = op1;    // before manipulating PC_REG, we need to get the operand!!!! => Nasty bug otherwise!
            PC_REG = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
            PC_REG = (uint16_t)(PC_REG+1);        // increase pc by 1 and wrap if necessary
            for(int i=0; i<temp_byte; i++) csg65ce02_pull_byte(thisCPU);
            break;
		case 0x64 :								// stz bp
		case 0x74 :								// stz bp,x
		case 0x9c :								// stz abs
		case 0x9e :								// stz abs,x
			csg65ce02_write_byte(effective_address_l, Z_REG);
			break;
		case 0x68 :								// pla
			A_REG = csg65ce02_pull_byte(thisCPU);
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x6b :								// tza
			A_REG = Z_REG;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x78 :								// sei
			thisCPU->i_flag = I_FLAG_VALUE;
			break;
		case 0x7a :								// ply
			Y_REG = csg65ce02_pull_byte(thisCPU);
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0x7b :								// tba
			A_REG = B_REG;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x80 :								// bra rel
		case 0x83 :								// bra wrel
			PC_REG = effective_address_l;
			break;
		case 0x81 :								// sta (bp,x)
		case 0x82 :								// sta (d,s),y
		case 0x85 :								// sta bp
		case 0x8d :								// sta absolute
		case 0x91 :								// sta (bp),y
		case 0x92 :								// sta (bp),z
		case 0x95 :								// sta bp,x
		case 0x99 :								// sta abs,y
		case 0x9d :								// sta abs,x
			csg65ce02_write_byte(effective_address_l, A_REG);
			break;
		case 0x84 :								// sty bp
		case 0x8b :								// sty abs,x
		case 0x8c :								// sty abs
		case 0x94 :								// sty bp,x
			csg65ce02_write_byte(effective_address_l, Y_REG);
			break;
		case 0x86 :								// stx bp
		case 0x8e :								// stx absolute
		case 0x96 :								// stx bp,y
		case 0x9b :								// stx absolute,y
			csg65ce02_write_byte(effective_address_l, X_REG);
			break;
		case 0x88 :								// dec y
			Y_REG--;
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0x8a :								// txa
			A_REG = X_REG;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x90 :								// bcc rel
		case 0x93 :								// bcc wrel
			if(thisCPU->c_flag) {			// carry set, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			} else {						// carry not set, take relative jump
				PC_REG = effective_address_l;
			}
			break;
		case 0x98 :								// tya
			A_REG = Y_REG;
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0x9a :								// txs
			SP_REG = (SP_REG & 0xff00) | X_REG;
			break;
		case 0xa0 :								// ldy immediate
		case 0xa4 :								// ldy bp
		case 0xac :								// ldy abs
		case 0xb4 :								// ldy bp,x
		case 0xbc :								// ldy abs,x
			Y_REG = csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0xa1 :								// lda (bp,x)
		case 0xa5 :								// lda bp
		case 0xa9 :								// lda imm
		case 0xad :								// lda abs
		case 0xb1 :								// lda (bp),y
		case 0xb2 :								// lda (bp),z
		case 0xb5 :								// lda bp,x
		case 0xb9 :								// lda abs,y
		case 0xbd :								// lda abs,x
		case 0xe2 :								// lda (d,s),y
			A_REG = csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(A_REG);
			break;
		case 0xa2 :								// ldx immediate
		case 0xa6 :								// ldx bp
		case 0xae :								// ldx abs
		case 0xb6 :								// ldx bp,y
		case 0xbe :								// ldx abs,y
			X_REG = csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xa3 :								// ldz immediate
		case 0xab :								// ldz abs
		case 0xbb :								// ldz abs,x
			Z_REG = csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(Z_REG);
			break;
		case 0xa8 :								// tay
			Y_REG = A_REG;
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0xaa :								// tax
			X_REG = A_REG;
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xb0 :								// bcs rel
		case 0xb3 :								// bcs wrel
			if(thisCPU->c_flag) {			// carry set, take relative jump
				PC_REG = effective_address_l;
			} else {						// carry not set, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			}
			break;
		case 0xb8 :								// clv instruction
			thisCPU->v_flag = 0x00;
			break;
		case 0xba :								// tsx instruction
			X_REG = (SP_REG & 0x00ff);
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xc0 :								// cpy immediate
		case 0xc4 :								// cpy bp
		case 0xcc :								// cpy abs
			temp_word = (0x0100 | Y_REG) - csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(0x00ff & temp_word);
			if(temp_word & 0xff00)
			{		// Y_REG >= operand
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			else
			{									// Y_REG < operand
				thisCPU->c_flag = 0;
			}
			break;
		case 0xc1 :								// cmp (bp,x)
		case 0xc5 :								// cmp bp
		case 0xc9 :								// cmp immediate
		case 0xcd :								// cmp abs
		case 0xd1 :								// cmp (bp),y
		case 0xd2 :								// cmp (bp),z
		case 0xd5 :								// cmp bp,x
		case 0xd9 :								// cmp abs,y
		case 0xdd :								// cmp abs,x
			temp_word = (0x0100 | A_REG) - csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(0x00ff & temp_word);
			if(temp_word & 0xff00)
			{		// A_REG >= operand
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			else
			{									// A_REG < operand
				thisCPU->c_flag = 0;
			}
			break;
		case 0xc2 :								// cpz immediate
		case 0xd4 :								// cpz bp
		case 0xdc :								// cpz absolute
			temp_word = (0x0100 | Z_REG) - csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(0x00ff & temp_word);
			if(temp_word & 0xff00)
			{		// Z_REG >= operand
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			else
			{						// Z_REG < operand
				thisCPU->c_flag = 0;
			}
			break;
		case 0xc6 :								// dec bp
		case 0xce :								// dec abs
		case 0xd6 :								// dec bp,x
		case 0xde :								// dec abs,x
			temp_byte = csg65ce02_read_byte(effective_address_l);
			temp_byte--;
			csg65ce02_write_byte(effective_address_l, temp_byte);
			SET_STATUS_FOR_NZ(temp_byte);
			break;
		case 0xc8 :								// inc y
			Y_REG++;
			SET_STATUS_FOR_NZ(Y_REG);
			break;
		case 0xca :								// dec x
			X_REG--;
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xd0 :								// bne rel
		case 0xd3 :								// bne wrel
			if(thisCPU->z_flag)
			{			// equal, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			}
			else
			{						// not equal, take relative jump
				PC_REG = effective_address_l;
			}
			break;
		case 0xd8 :								// cld
			thisCPU->d_flag = 0x00;
			break;
		case 0xda :								// phx
			csg65ce02_push_byte(thisCPU, X_REG);
			break;
		case 0xdb :								// phz
			csg65ce02_push_byte(thisCPU, Z_REG);
			break;
		case 0xe0 :								// cpx immediate
		case 0xe4 :								// cpx bp
		case 0xec :								// cpx abs
			temp_word = (0x0100 | X_REG) - csg65ce02_read_byte(effective_address_l);
			SET_STATUS_FOR_NZ(0x00ff & temp_word);
			if(temp_word & 0xff00)
			{	// X_REG >= operand
				thisCPU->c_flag = C_FLAG_VALUE;
			}
			else
			{	// X_REG < operand
				thisCPU->c_flag = 0;
			}
			break;
		case 0xe6 :								// inc bp
		case 0xee :								// inc abs
		case 0xf6 :								// inc bp,x
		case 0xfe :								// inc abs,x
			temp_byte = csg65ce02_read_byte(effective_address_l);
			temp_byte++;
			csg65ce02_write_byte(effective_address_l, temp_byte);
			SET_STATUS_FOR_NZ(temp_byte);
			break;
		case 0xe8 :								// inc x
			X_REG++;
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xea :								// nop instruction
			break;
		case 0xf0 :								// beq rel
		case 0xf3 :								// beq wrel
			if(thisCPU->z_flag)
			{	// equal, take relative jump
				PC_REG = effective_address_l;
			}
			else
			{	// not equal, skip to next instruction
				PC_REG = (uint16_t)(PC_REG+bytes_per_instruction[opcode]);
			}
			break;
		case 0xf8 :								// sed
			thisCPU->d_flag = D_FLAG_VALUE;
			break;
		case 0xfa :								// plx
			X_REG = csg65ce02_pull_byte(thisCPU);
			SET_STATUS_FOR_NZ(X_REG);
			break;
		case 0xfb :								// plz
			Z_REG = csg65ce02_pull_byte(thisCPU);
			SET_STATUS_FOR_NZ(Z_REG);
			break;
		case 0xf4 :								// phw immediate
		case 0xfc :								// phw absolute
												// please note that the resulting address on stack is also little endian this way!
			csg65ce02_push_byte(thisCPU, csg65ce02_read_byte(effective_address_h));
			csg65ce02_push_byte(thisCPU, csg65ce02_read_byte(effective_address_l));
			break;
		default :								// opcode not implemented
			printf("error: opcode 0x%02x not implemented\n", opcode);
	}
}

void csg65ce02_assign_irq_pin(csg65ce02 *thisCPU, bool *pin)
{
	thisCPU->irq_pin = pin;
}

void csg65ce02_assign_nmi_pin(csg65ce02 *thisCPU, bool *pin)
{
	thisCPU->nmi_pin = pin;
}

void csg65ce02_dump_status(csg65ce02 *thisCPU, char *temp_string)
{
	snprintf(temp_string, 256, " pc  ac xr yr zr bp shsl nvebdizc\n%04x %02x %02x %02x %02x %02x %02x%02x %s%s%s %s%s%s%s", PC_REG, A_REG, X_REG, Y_REG, Z_REG, B_REG, SP_REG >> 8, SP_REG & 0x00ff, thisCPU->n_flag ? "*" : ".", thisCPU->v_flag ? "*" : ".", thisCPU->e_flag ? "*" : ".", thisCPU->d_flag ? "*" : ".", thisCPU->i_flag ? "*" : ".", thisCPU->z_flag ? "*" : ".", thisCPU->c_flag ? "*" : ".");
}

void csg65ce02_dump_page(csg65ce02 *thisCPU, uint8_t pageNo)
{
    for(int i=0; i<0x100; i++) {
        if(i%16 == 0) printf("\n%04x", pageNo<<8 | i);
        printf(" %02x", csg65ce02_read_byte(pageNo << 8 | i));
    }
    printf("\n");
}
