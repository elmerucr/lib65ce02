//  csg65ce02.c
//
//  Created by elmerucr on 11/01/2017.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02.h"
#include "csg65ce02_macros.h"

// library internal function
void csg65ce02_calculate_effective_address(csg65ce02 *thisCPU, uint8_t opcode, uint16_t *eal, uint16_t *eah);
void csg65ce02_handle_opcode(csg65ce02 *thisCPU, uint8_t opcode, uint16_t eal, uint16_t eah);

const int addressing_mode_per_instruction[256] = {
	IMPLIED,BP_X_IND,IMPLIED,IMPLIED,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP,BP_X,BP_X,BP,IMPLIED,ABS_Y,ACCUM,IMPLIED,ABS,ABS_X,ABS_X,BPREL,
	ABS,BP_X_IND,ABS_IND,ABS_X_IND,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_X,BP,IMPLIED,ABS_Y,ACCUM,IMPLIED,ABS_X,ABS_X,ABS_X,BPREL,
	IMPLIED,BP_X_IND,ACCUM,ACCUM,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS,ABS,ABS,BPREL,
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_X,BP,IMPLIED,ABS_Y,IMPLIED,IMPLIED,IMPLIED,ABS_X,ABS_X,BPREL,
	IMPLIED,BP_X_IND,IMPLIED,WREL,BP,BP,BP,BP,IMPLIED,IMM,ACCUM,IMPLIED,ABS_IND,ABS,ABS,BPREL,
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

const uint8_t bytes_per_instruction[256] = {
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

const uint8_t cycles_per_instruction[256] = {
	7,5,2,2,4,3,4,4,3,2,1,1,5,4,5,4,
	2,5,5,3,4,3,4,4,1,4,1,1,5,4,5,4,
	5,5,7,7,4,3,4,4,3,2,1,1,5,4,5,4,
	2,5,5,3,4,3,4,4,1,4,1,1,5,4,5,4,
	5,5,2,2,4,3,4,4,3,2,1,1,3,4,5,4,
	2,5,5,3,4,3,4,4,1,4,3,1,4,4,5,4,
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

const uint8_t modify_pc_per_instruction[256] = {
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
void csg65ce02_init(csg65ce02 *thisCPU) {
	thisCPU->breakpoints_active = false;
	thisCPU->breakpoint_array = (bool *)malloc(65536 * sizeof(bool));
	for(int i=0; i<65536; i++) {
		thisCPU->breakpoint_array[i] = false;
	}
}

// Cleanup procedure
void csg65ce02_cleanup(csg65ce02 *thisCPU) {
	free(thisCPU->breakpoint_array);
}

void csg65ce02_reset(csg65ce02 *thisCPU) {
    //aReg = 0x00;			// don't care
    //xReg = 0x00;			// don't care
    //yReg = 0x00;			// don't care
	zReg = 0x00;			// is actively set to 0 to emulate 65c02 stz instructions store zero
	bReg = 0x00;			// init to 0 for correct emulation of earlier 65xx cpus ("zero-page")
    spReg = 0x01fd;			// is this the correct value => yes (see pagetable ...)
							// if e=1 then wraps around same page, if e=0 wraps around memory

	thisCPU->nFlag = 0x00;
	thisCPU->vFlag = 0x00;
	thisCPU->eFlag = eFlagValue;	// starts flagged (8 bit sp on page $01) for 6502 compatible behaviour
	thisCPU->dFlag = 0x00;			// must be zero after reset!
	thisCPU->iFlag = iFlagValue;	// starts flagged, to avoid irq's from happening - bootup code must 'cli' after setup
	thisCPU->zFlag = 0x00;
	thisCPU->cFlag = 0x00;

	// default states of irq and nmi pins
	thisCPU->irq_pin = true;
	thisCPU->irq_pending = false;

	thisCPU->nmi_pin = true;
	thisCPU->nmi_pin_last_state = true;
	thisCPU->nmi_pending = false;

	// load reset vector into pc
    pcReg = csg65ce02_read_byte(0xfffc) | (csg65ce02_read_byte(0xfffd) << 8);

	thisCPU->cycles_last_executed_instruction = 1;	// safe value after reset, so interrupts can't be acknowledged
													// this is useful for setting up an extended stack (cle, see)
}

// Breakpoint functions
void csg65ce02_enable_breakpoints(csg65ce02 *thisCPU) {
	thisCPU->breakpoints_active = true;
}

void csg65ce02_disable_breakpoints(csg65ce02 *thisCPU) {
	thisCPU->breakpoints_active = false;
}

void csg65ce02_add_breakpoint(csg65ce02 *thisCPU, uint16_t address) {
	thisCPU->breakpoint_array[address] = true;
}

void csg65ce02_remove_breakpoint(csg65ce02 *thisCPU, uint16_t address) {
	thisCPU->breakpoint_array[address] = false;
}

// stack operation push (stack always points to the current available position)
inline void csg65ce02_push_byte(csg65ce02 *thisCPU, uint8_t byte) {
	if( thisCPU->eFlag ) {							// 8 bit stack pointer
		csg65ce02_write_byte(spReg, byte);			// store the byte
		uint16_t temp_word = spReg & 0xff00;		// keep track of current msb of 8 bit stack pointer
		spReg--;									// lower the stack pointer by one
		spReg = (spReg & 0x00ff) | temp_word;		// make sure msb keeps it old value

	} else {										// 16 bit stack pointer, much simpler!
		csg65ce02_write_byte(spReg, byte);			// store the byte
		spReg--;									// lower the stack pointer by one
	}
}

// stack operation pull
inline uint8_t csg65ce02_pull_byte(csg65ce02 *thisCPU) {
	if( thisCPU->eFlag ) {										// 8 bit stack pointer
		uint16_t temp_word = spReg & 0xff00;					// sph must keep same value
		spReg++;												// increase the sp
		spReg = (spReg & 0x00ff) | temp_word;					// correct it if it crossed a page border
		return csg65ce02_read_byte(spReg);						// pull one byte and return it
	} else {													// 16 bit stack pointer, again much simpler
		spReg++;
		return csg65ce02_read_byte(spReg);
	}
}

//	Main function, execute a number of cycles on the virtual cpu
unsigned int csg65ce02_execute(csg65ce02 *thisCPU, unsigned int no_cycles) {
	uint8_t  current_opcode;
	uint16_t effective_address_l;		// low byte address of the effective address, normally used
	uint16_t effective_address_h;		// high byte address of the effective address (for IMMW / ABSW addressing)

	thisCPU->cycle_count = 0;
	thisCPU->instruction_counter = 0;

	// logic to initiate irq's
	if(!(thisCPU->irq_pin) && !(thisCPU->iFlag) ) {
		thisCPU->irq_pending = true;
	}
	// logic to initiate nmi's
	if(!(thisCPU->nmi_pin) && thisCPU->nmi_pin_last_state) {	// state changed from 1 to 0, edge!
		thisCPU->nmi_pending = true;
		thisCPU->nmi_pin_last_state = false;					// this will avoid multiple nmi's
	}

	// actual instruction loop
	do {
		current_opcode = thisCPU->irq_pending ? 0x00 : csg65ce02_read_byte(pcReg);

		csg65ce02_calculate_effective_address(thisCPU, current_opcode, &effective_address_l, &effective_address_h);

		csg65ce02_handle_opcode(thisCPU, current_opcode, effective_address_l, effective_address_h);

		thisCPU->cycles_last_executed_instruction = cycles_per_instruction[current_opcode];
		thisCPU->cycle_count += thisCPU->cycles_last_executed_instruction;

		// increase pc only if the instruction does not actively change the pc by itself
		if(!modify_pc_per_instruction[current_opcode]) {
			pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
		}
		thisCPU->instruction_counter++;
	// Three conditions must be met to keep running:
	//    (1) enough cycles?
	//    (2) no breakpoint?
	//    (3) breakpoints activated?
    } while(	(thisCPU->cycle_count < no_cycles) &&
				!((thisCPU->breakpoint_array[pcReg] == true) && thisCPU->breakpoints_active) );

    return thisCPU->cycle_count;
}

inline void csg65ce02_calculate_effective_address(csg65ce02 *thisCPU, uint8_t opcode, uint16_t *eal, uint16_t *eah) {
	// temporary storage possibilities
	uint16_t	temp_word;
	uint8_t		temp_byte;
	// calculate effective address
	switch( addressing_mode_per_instruction[opcode]) {
		case IMM :
			*eal = pcReg1;
			break;
		case ABS :
			*eal = op1 | (op2 << 8);
			break;
		case BP :
			*eal = op1 | (bReg << 8);
			break;
		case ACCUM :
			// IMPLEMENTED IN INSTRUCTION
			break;
		case IMPLIED :
			// IMPLEMENTED IN INSTRUCTION
			break;
		case BP_X :
			*eal = ((uint8_t)(op1 + xReg)) | (bReg << 8);
			break;
		case BP_Y :
			*eal = ((uint8_t)(op1 + yReg)) | (bReg << 8);
			break;
		case ABS_X :
			*eal = (op1 | (op2 << 8)) + xReg;
			break;
		case ABS_Y :
			*eal = (op1 | (op2 << 8)) + yReg;
			break;
		case BP_X_IND :
			*eal = csg65ce02_read_byte((op1 + xReg) | (bReg << 8)) |
					(csg65ce02_read_byte((op1 + xReg + 1) | (bReg << 8)) << 8);
			break;
		case BP_IND_Y :
			*eal = (csg65ce02_read_byte( op1 | (bReg << 8) ) |
					(csg65ce02_read_byte( ((uint8_t)(op1 + 1)) | (bReg << 8) ) << 8)) + yReg;
			break;
		case BP_IND_Z :
			*eal = (csg65ce02_read_byte( op1 | (bReg << 8) ) |
					(csg65ce02_read_byte( ((uint8_t)(op1 + 1)) | (bReg << 8) ) << 8)) + zReg;
			break;
		case D_SP_IND_Y :
			// IMPLEMENT!!!!!
			break;
		case REL :
			temp_byte = op1;
			temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
			*eal = (uint16_t)(pcReg + 2 + temp_word);					// note "2"!
			break;
		case WREL :
			temp_word = ( (uint16_t)op1 ) | ( (uint16_t)op2 << 8 );
			*eal = (uint16_t)(pcReg + 2 + temp_word);					// note "2"!
			break;
		case ABS_IND :
			*eal = csg65ce02_read_byte(op1 | (op2 << 8)) | (csg65ce02_read_byte((op1 | (op2 << 8))+1) << 8);
			break;
		case BPREL :
			temp_byte = op2;															// note "op2"!
			temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
			*eal = (uint16_t)(pcReg + 3 + temp_word);					// note "3"!
			break;
		case ABS_X_IND :
			temp_word = (uint16_t)((op1 | (op2 << 8)) + xReg);
			*eal = csg65ce02_read_byte(temp_word) | (csg65ce02_read_byte((uint16_t)(temp_word+1)) << 8);
			break;
		case IMMW :
			*eal = pcReg1;
			*eah = pcReg2;
			break;
		case ABSW :
			*eal = op1 | (op2 << 8);
			*eah = (uint16_t)(*eal + 1);
			break;
		default :
			break;
	};
}

inline void csg65ce02_handle_opcode(csg65ce02 *thisCPU, uint8_t opcode, uint16_t eal, uint16_t eah) {
	uint8_t temp_byte;
	uint8_t temp_byte2;
	uint16_t temp_word;

	uint16_t effective_address_l = eal;
	uint16_t effective_address_h = eah;

	switch( opcode ) {
		case 0x00 :								// brk instruction (or irq/nmi)
			if(thisCPU->irq_pending) {
				// push high byte of the pc on the stack (note rti will not increase pc on return)
				csg65ce02_push_byte(thisCPU, msb((uint16_t)(pcReg)));
				// push low byte of the pc on the stack
				csg65ce02_push_byte(thisCPU, lsb((uint16_t)(pcReg)));

				temp_byte2 = 0x00;
				thisCPU->irq_pending = false;
				thisCPU->irq_pin = true;
			} else {
				// push high byte of the pc+2 on the stack (note rti will not increase pc on return)
				csg65ce02_push_byte(thisCPU, msb((uint16_t)(pcReg+2)));
				// push low byte of the pc+2 on the stack
				csg65ce02_push_byte(thisCPU, lsb((uint16_t)(pcReg+2)));

				temp_byte2 = bFlagValue;
			}

			// push sr onto stack
			temp_byte =	thisCPU->nFlag |
						thisCPU->vFlag |
						thisCPU->eFlag |
						temp_byte2     |	// pushes bFlagValue when cause was brk instruction
						thisCPU->dFlag |
						thisCPU->iFlag |    // yes, the i flag as it was during last instruction is pushed!
						thisCPU->zFlag |
						thisCPU->cFlag;
			csg65ce02_push_byte(thisCPU, temp_byte);
			// set interrupt disable flag
			thisCPU->iFlag = iFlagValue;

			// load pc vector for irq/brk
			thisCPU->pc = csg65ce02_read_byte(0xfffe) | (csg65ce02_read_byte(0xffff) << 8);

			// clear the decimal flag (it will be restored by the rti instruction) (Eyes, p338)
			// note: in older versions of the cpu (nmos) this didn't happen, resulting in difficult to find bugs
			thisCPU->dFlag = 0x00;
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
			aReg = aReg | (csg65ce02_read_byte(effective_address_l));
			setStatusForNZ(aReg);
			break;
		case 0x02 :								// cle
			thisCPU->eFlag = 0x00;
			break;
		case 0x03 :								// see
			thisCPU->eFlag = eFlagValue;
			break;
		// case 0x04 :								// tsb bp
		// case 0x0c :								// tsb abs
		// 	temp_byte = aReg & memory[effective_address_l];
		// 	if(temp_byte) {						// some of the bit locs of both values were set
		// 		thisCPU->zFlag = 0x00;
		// 	} else {
		// 		thisCPU->zFlag = zFlagValue;
		// 	}
		// 	memory[effective_address_l] = aReg | memory[effective_address_l];
		// 	break;
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
			if(opcode & 0x80) {						// set memory bit
				csg65ce02_write_byte(effective_address_l, csg65ce02_read_byte(effective_address_l) | (0x01 << temp_byte) );
			} else {								// reset (clear) memory bit
				csg65ce02_write_byte(effective_address_l, csg65ce02_read_byte(effective_address_l) & (0xff - (0x01 << temp_byte)));
			}
			break;
		case 0x0b :								// tsy
			yReg = (spReg & 0xff00) >> 8;
			setStatusForNZ(yReg);
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
			temp_word = op1 | (bReg << 8);
			// Which bit will be tested for? And store it in temp_byte
			temp_byte = (opcode & 0x70) >> 4;
			// effect. address (rel) was already calculated
			if(opcode & 0x80) {						// test for set
				if(csg65ce02_read_byte(temp_word) & (0x01<<temp_byte)) {
					pcReg = effective_address_l;
				} else {
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
				}
			} else {								// test for clear
				if(!(csg65ce02_read_byte(temp_word) & (0x01<<temp_byte))) {
					pcReg = effective_address_l;
				} else {
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
				}
			}
			break;
		case 0x10 :								// bpl rel
		case 0x13 :								// bpl wrel
			if(thisCPU->nFlag) {			// n flag is set, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			} else {						// n flag not set, take relative jump
				pcReg = effective_address_l;
			}
			break;
		case 0x18 :								// clc instruction
			thisCPU->cFlag = 0x00;
			break;
		case 0x1a :								// inc a instruction
			aReg++;
			setStatusForNZ(aReg);
			break;
		case 0x20 :								// jsr abs instruction (push last byte addr of instr
		case 0x23 :								// jsr (abs,x)
			csg65ce02_push_byte(thisCPU, msb((uint16_t)(pcReg+2)));
			csg65ce02_push_byte(thisCPU, lsb((uint16_t)(pcReg+2)));
			pcReg = effective_address_l;
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
			aReg = aReg & (csg65ce02_read_byte(effective_address_l));
			setStatusForNZ(aReg);
			break;
		case 0x24 :								// bit bp
		case 0x2c :								// bit abs
		case 0x34 :								// bit bp,x
		case 0x3c :								// bit abs,x
		case 0x89 :								// bit immediate (note 65c02 not n&v flags!)
			temp_byte = csg65ce02_read_byte(effective_address_l);
			setStatusForZ(temp_byte & aReg);
			thisCPU->nFlag = temp_byte & nFlagValue;
			thisCPU->vFlag = temp_byte & vFlagValue;
			break;
		case 0x2b :								// tys
			spReg = (spReg & 0x00ff) | (yReg << 8);
			break;
		case 0x30 :								// bmi rel
		case 0x33 :								// bmi wrel
			if(thisCPU->nFlag) {			// n flag is set, take relative jump
				pcReg = effective_address_l;
			} else {						// n flag not set, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			}
			break;
		case 0x38 :								// sec
			thisCPU->cFlag = cFlagValue;
			break;
		case 0x3a :								// dec a
			aReg--;
			setStatusForNZ(aReg);
			break;
		case 0x40 :								// rti
			temp_byte = csg65ce02_pull_byte(thisCPU);
			thisCPU->nFlag = temp_byte & nFlagValue;
			thisCPU->vFlag = temp_byte & vFlagValue;
			thisCPU->eFlag = temp_byte & eFlagValue;
					// no b flag to set of course...
			thisCPU->dFlag = temp_byte & dFlagValue;
			thisCPU->iFlag = temp_byte & iFlagValue;
			thisCPU->zFlag = temp_byte & zFlagValue;
			thisCPU->cFlag = temp_byte & cFlagValue;
			// restore the program counter
			pcReg = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
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
			aReg = aReg ^ csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(aReg);
			break;
		case 0x42 :								// neg
			aReg = ~aReg;
			aReg++;
			setStatusForNZ(aReg);
			break;
		case 0x48 :								// pha
			csg65ce02_push_byte(thisCPU, aReg);
			break;
		case 0x4b :								// taz
			zReg = aReg;
			setStatusForNZ(zReg);
			break;
		case 0x4c :								// jmp abs
		case 0x6c :								// jmp (abs)
		case 0x7c :								// jmp (abs,x)
			pcReg = effective_address_l;
			break;
		case 0x58 :								// cli
			thisCPU->iFlag = 0x00;
			break;
		case 0x5a :								// phy
			csg65ce02_push_byte(thisCPU, yReg);
			break;
		case 0x5b :								// tab
			bReg = aReg;
			break;
		case 0x60 :								// rts instruction
			pcReg = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
			pcReg = (uint16_t)(pcReg+1);		// increase pc by 1 and wrap if necessary
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
			temp_word = aReg + temp_byte + thisCPU->cFlag;
			if(temp_word & 0xff00) thisCPU->cFlag = cFlagValue; else thisCPU->cFlag = 0;
			temp_byte2 = temp_word & 0xff;
			if(((aReg^temp_byte2) & (temp_byte^temp_byte2)) & 0x80) {
				thisCPU->vFlag = vFlagValue;
			} else {
				thisCPU->vFlag = 0x00;
			}
			aReg = temp_byte2;
			setStatusForNZ(aReg);
			break;
		case 0x64 :								// stz bp
			csg65ce02_write_byte(effective_address_l, zReg);
			break;
		case 0x68 :								// pla
			aReg = csg65ce02_pull_byte(thisCPU);
			setStatusForNZ(aReg);
			break;
		case 0x6b :								// tza
			aReg = zReg;
			setStatusForNZ(aReg);
			break;
		case 0x78 :								// sei
			thisCPU->iFlag = iFlagValue;
			break;
		case 0x7a :								// ply
			yReg = csg65ce02_pull_byte(thisCPU);
			setStatusForNZ(yReg);
			break;
		case 0x80 :								// bra rel
		case 0x83 :								// bra wrel
			pcReg = effective_address_l;
			break;
		case 0x81 :								// sta (bp,x)
		case 0x85 :								// sta bp
		case 0x8d :								// sta absolute
		case 0x91 :								// sta (bp),y
		case 0x99 :								// sta abs,y
			csg65ce02_write_byte(effective_address_l, aReg);
			break;
		case 0x84 :								// sty bp
		case 0x8b :								// sty abs,x
		case 0x8c :								// sty abs
		case 0x94 :								// sty bp,x
			csg65ce02_write_byte(effective_address_l, yReg);
			break;
		case 0x86 :								// stx bp
		case 0x8e :								// stx absolute
		case 0x96 :								// stx bp,y
		case 0x9b :								// stx absolute,y
			csg65ce02_write_byte(effective_address_l, xReg);
			break;

		case 0x88 :								// dey
			yReg--;
			setStatusForNZ(yReg);
			break;
		case 0x90 :								// bcc rel
		case 0x93 :								// bcc wrel
			if(thisCPU->cFlag) {			// carry set, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			} else {						// carry not set, take relative jump
				pcReg = effective_address_l;
			}
			break;
		case 0x9a :								// txs
			spReg = (spReg & 0xff00) | xReg;
			break;
		case 0xa0 :								// ldy immediate
		case 0xa4 :								// ldy bp
		case 0xac :								// ldy abs
		case 0xb4 :								// ldy bp,x
		case 0xbc :								// ldy abs,x
			yReg = csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(yReg);
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
			aReg = csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(aReg);
			break;
		case 0xa2 :								// ldx immediate
		case 0xa6 :								// ldx bp
		case 0xae :								// ldx abs
		case 0xb6 :								// ldx bp,y
		case 0xbe :								// ldx abs,y
			xReg = csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(xReg);
			break;
		case 0xa3 :								// ldz immediate
		case 0xab :								// ldz abs
		case 0xbb :								// ldz abs,x
			zReg = csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(zReg);
			break;
		case 0xaa :								// tax
			xReg = aReg;
			setStatusForNZ(xReg);
			break;
		case 0xb0 :								// bcs rel
		case 0xb3 :								// bcs wrel
			if(thisCPU->cFlag) {			// carry set, take relative jump
				pcReg = effective_address_l;
			} else {						// carry not set, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			}
			break;
		case 0xba :								// tsx instruction
			xReg = (spReg & 0x00ff);
			setStatusForNZ(xReg);
			break;
		case 0xc0 :								// cpy immediate
		case 0xc4 :								// cpy bp
		case 0xcc :								// cpy abs
			temp_word = (0x0100 | yReg) - csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(0x00ff & temp_word);
			if(temp_word & 0xff00) {		// yReg >= operand
				thisCPU->cFlag = cFlagValue;
			} else {						// yReg < operand
				thisCPU->cFlag = 0;
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
			temp_word = (0x0100 | aReg) - csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(0x00ff & temp_word);
			if(temp_word & 0xff00) {		// aReg >= operand
				thisCPU->cFlag = cFlagValue;
			} else {						// aReg < operand
				thisCPU->cFlag = 0;
			}
			break;
		case 0xc2 :								// cpz immediate
		case 0xd4 :								// cpz bp
		case 0xdc :								// cpz absolute
			temp_word = (0x0100 | zReg) - csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(0x00ff & temp_word);
			if(temp_word & 0xff00) {		// zReg >= operand
				thisCPU->cFlag = cFlagValue;
			} else {						// zReg < operand
				thisCPU->cFlag = 0;
			}
			break;
		case 0xc8 :								// inc y
			yReg++;
			setStatusForNZ(yReg);
			break;
		case 0xca :								// dec x
			xReg--;
			setStatusForNZ(xReg);
			break;
		case 0xd0 :								// bne rel
		case 0xd3 :								// bne wrel
			if(thisCPU->zFlag) {			// equal, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			} else {						// not equal, take relative jump
				pcReg = effective_address_l;
			}
			break;
		case 0xda :								// phx
			csg65ce02_push_byte(thisCPU, xReg);
			break;
		case 0xdb :								// phz
			csg65ce02_push_byte(thisCPU, zReg);
			break;
		case 0xe0 :								// cpx immediate
		case 0xe4 :								// cpx bp
		case 0xec :								// cpx abs
			temp_word = (0x0100 | xReg) - csg65ce02_read_byte(effective_address_l);
			setStatusForNZ(0x00ff & temp_word);
			if(temp_word & 0xff00) {		// xReg >= operand
				thisCPU->cFlag = cFlagValue;
			} else {						// xReg < operand
				thisCPU->cFlag = 0;
			}
			break;
		case 0xe6 :								// inc bp
		case 0xee :								// inc abs
		case 0xf6 :								// inc bp,x
		case 0xfe :								// inc abs,x
			temp_byte = csg65ce02_read_byte(effective_address_l);
			temp_byte++;
			csg65ce02_write_byte(effective_address_l, temp_byte);
			setStatusForNZ(temp_byte);
			break;
		case 0xe8 :								// inc x
			xReg++;
			setStatusForNZ(xReg);
			break;
		case 0xea :								// nop instruction
			break;
		case 0xf0 :								// beq rel
		case 0xf3 :								// beq wrel
			if(thisCPU->zFlag) {			// equal, take relative jump
				pcReg = effective_address_l;
			} else {						// not equal, skip to next instruction
				pcReg = (uint16_t)(pcReg+bytes_per_instruction[opcode]);
			}
			break;
		case 0xfa :								// plx
			xReg = csg65ce02_pull_byte(thisCPU);
			setStatusForNZ(xReg);
			break;
		case 0xfb :								// plz
			zReg = csg65ce02_pull_byte(thisCPU);
			setStatusForNZ(zReg);
			break;
		case 0xf4 :								// phw immediate
		case 0xfc :								// phw absolute
			csg65ce02_push_byte(thisCPU, csg65ce02_read_byte(effective_address_l));
			csg65ce02_push_byte(thisCPU, csg65ce02_read_byte(effective_address_h));
			break;
		default :								// opcode not implemented
			printf("error: opcode 0x%02x not implemented\n", opcode);
	}
}

void csg65ce02_pull_irq_pin(csg65ce02 *thisCPU) {
	thisCPU->irq_pin = false;
}
void csg65ce02_release_irq_pin(csg65ce02 *thisCPU) {
	thisCPU->irq_pin = true;
}

void csg65ce02_dump_status(csg65ce02 *thisCPU, char *temp_string) {
	snprintf(temp_string, 256, " pc  ac xr yr zr bp shsl nvebdizc in\n%04x %02x %02x %02x %02x %02x %02x%02x %s%s%s %s%s%s%s %s%s", pcReg, aReg, xReg, yReg, zReg, bReg, spReg >> 8, spReg & 0x00ff, thisCPU->nFlag ? "*" : ".", thisCPU->vFlag ? "*" : ".", thisCPU->eFlag ? "*" : ".", thisCPU->dFlag ? "*" : ".", thisCPU->iFlag ? "*" : ".", thisCPU->zFlag ? "*" : ".", thisCPU->cFlag ? "*" : ".", thisCPU->irq_pin ? "1" : "0", thisCPU->nmi_pin ? "1" : "0");
}

void csg65ce02_dump_page(csg65ce02 *thisCPU, uint8_t pageNo) {
    for(int i=0; i<0x100; i++) {
        if(i%16 == 0) printf("\n%04x", pageNo<<8 | i);
        printf(" %02x", csg65ce02_read_byte(pageNo << 8 | i));
    }
    printf("\n");
}