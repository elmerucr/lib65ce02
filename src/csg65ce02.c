//  csg65ce02.c
//
//  Created by elmerucr from 11/01/2017 - 11/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include "csg65ce02.h"
#include "csg65ce02_macros.h"

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
	REL,BP_IND_Y,BP_IND_Z,WREL,BP_X,BP_X,BP_Y,BP,IMPLIED,ABS_Y,IMPLIED,ABS_X,ABS,ABS_X,ABS_X,BPREL,
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
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
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

void csg65ce02_init(csg65ce02 *thisCPU, uint8_t *mem) {
    thisCPU->ram = mem;
}

void csg65ce02_reset(csg65ce02 *thisCPU) {
    //aReg = 0x00;
    //xReg = 0x00;
    //yReg = 0x00;
	zReg = 0x00;					// is actively set to 0 to emulate 65c02 stz instructions store zero
	bReg = 0x00;					// init to 0 for correct emulation of earlier 65xx cpus ("zero-page")
    spReg = 0x01fd;					// is this the correct value => yes (see pagetable ...)
									// free to set this to another value manually

	thisCPU->nFlag = 0x00;
	thisCPU->vFlag = 0x00;
	thisCPU->eFlag = eFlagValue;	// starts flagged (8 bit sp on page $01) for 6502 compatible behav.
	thisCPU->dFlag = 0x00;			// must be zero after reset!
	thisCPU->iFlag = 0x00;
	thisCPU->zFlag = 0x00;
	thisCPU->cFlag = 0x00;

    pcReg = memory[0xfffc] | memory[0xfffd] << 8;

	thisCPU->cycles_last_executed_instruction = 1;	// safe value after reset
}

inline void csg65ce02_push_byte(csg65ce02 *thisCPU, uint8_t byte) {
	if( thisCPU->eFlag ) {							// 8 bit stack pointer
		memory[(spReg & 0x00ff) | 0x0100] = byte;
		uint16_t temp_word = spReg & 0xff00;
		spReg--;
		spReg = (spReg & 0x00ff) | temp_word;
	} else {								// 16 bit stack pointer
		memory[spReg] = byte;
		spReg--;
	}
}

inline uint8_t csg65ce02_pull_byte(csg65ce02 *thisCPU) {
	if( thisCPU->eFlag ) {							// 8 bit stack pointer
		uint16_t temp_word = spReg & 0xff00;			// sph must keep same value
		spReg++;										// increase the sp
		spReg = (spReg & 0x00ff) | temp_word;			// correct it if it crossed a page border
		return memory[(spReg & 0x00ff) | 0x0100];		// pull one byte and return it
	} else {										// 16 bit stack pointer
		spReg++;
		return memory[spReg];
	}
}

unsigned int csg65ce02_execute(csg65ce02 *thisCPU, unsigned int noCycles) {
	uint8_t current_opcode;
	uint16_t effective_address_l;
	uint16_t effective_address_h;

	// temporary storage possibility
	uint16_t	temp_word;
	uint8_t		temp_byte;

	unsigned int cycle_count = 0;
	int running = 1;

	do {
		current_opcode = memory[pcReg];		// fetch opcode at current pc

		switch( addressing_mode_per_instruction[current_opcode]) {
			case IMM :
				effective_address_l = pcReg1;
				break;
			case ABS :
				effective_address_l = op1 | (op2 << 8);
				break;
			case BP :
				effective_address_l = op1 | (bReg << 8);
				break;
			case ACCUM :
				// IMPLEMENTED IN INSTRUCTION
				break;
			case IMPLIED :
				// IMPLEMENTED IN INSTRUCTION
				break;
			case BP_X :
				effective_address_l = ((uint8_t)(op1 + xReg)) | (bReg << 8);
				break;
			case BP_Y :
				effective_address_l = ((uint8_t)(op1 + yReg)) | (bReg << 8);
				break;
			case ABS_X :
				effective_address_l = (op1 | (op2 << 8)) + xReg;
				break;
			case ABS_Y :
				effective_address_l = (op1 | (op2 << 8)) + yReg;
				break;
			case BP_X_IND :
				effective_address_l =	memory[(op1 + xReg) | (bReg << 8)] |
										(memory[(op1 + xReg + 1) | (bReg << 8)] << 8);
				break;
			case BP_IND_Y :
				effective_address_l =	(memory[ op1 | (bReg << 8) ] |
										(memory[ ((uint8_t)(op1 + 1)) | (bReg << 8) ] << 8)) + yReg;
				break;
			case BP_IND_Z :
				effective_address_l =	(memory[ op1 | (bReg << 8) ] |
										(memory[ ((uint8_t)(op1 + 1)) | (bReg << 8) ] << 8)) + zReg;
				break;
			case D_SP_IND_Y :
				// IMPLEMENT!!!!!
				break;
			case REL :
				temp_byte = op1;
				temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
				effective_address_l = (uint16_t)(pcReg + 2 + temp_word);					// note "2"!
				break;
			case WREL :
				temp_word = ( (uint16_t)op1 ) | ( (uint16_t)op2 << 8 );
				effective_address_l = (uint16_t)(pcReg + 2 + temp_word);					// note "2"!
				break;
			case ABS_IND :
				effective_address_l = memory[op1 | (op2 << 8)] | (memory[(op1 | (op2 << 8))+1] << 8);
				break;
			case BPREL :
				temp_byte = op2;															// note "op2"!
				temp_word = (temp_byte & 0x80) ? 0xff00 | temp_byte : 0x0000 | temp_byte;
				effective_address_l = (uint16_t)(pcReg + 3 + temp_word);					// note "3"!
				break;
			case ABS_X_IND :
				temp_word = (uint16_t)((op1 | (op2 << 8)) + xReg);
				effective_address_l = memory[temp_word] | (memory[(uint16_t)(temp_word+1)] << 8);
				break;
			case IMMW :
				// IMPLEMENT!
				break;
			case ABSW :
				// IMPLEMENT!
				break;
			default :
				break;
		};

		switch( current_opcode ) {
			case 0x00 :								// brk instruction, still a fake handling routine
				running = 0;
				break;
			case 0x01 :								// ora (bp,x)
			case 0x05 :								// ora bp
			case 0x09 :								// ora immediate
			case 0x0d :								// ora abs
			case 0x11 :								// (bp),y
			case 0x12 :								// (bp),z
			case 0x15 :								// bp,x
			case 0x19 :								// abs,y
			case 0x1d :								// abs,x
				aReg = aReg | (memory[effective_address_l]);
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
				// Which bit from bp addres is going to be tested? Store it in temp_byte
				temp_byte = (current_opcode & 0x70) >> 4;
				if(current_opcode & 0x80) {				// set memory bit
					memory[effective_address_l] = memory[effective_address_l] | (0x01 << temp_byte);
				} else {								// reset (clear) memory bit
					memory[effective_address_l] = memory[effective_address_l] & (0xff - (0x01 << temp_byte));
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
				temp_byte = (current_opcode & 0x70) >> 4;
				// effect. address (rel) was already calculated
				if(current_opcode & 0x80) {				// test for set
					if(memory[temp_word] & (0x01<<temp_byte)) {
						pcReg = effective_address_l;
					} else {
						pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
					}
				} else {								// test for clear
					if(!(memory[temp_word] & (0x01<<temp_byte))) {
						pcReg = effective_address_l;
					} else {
						pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
					}
				}
				break;
			case 0x10 :								// bpl rel
			case 0x13 :								// bpl wrel
				if(thisCPU->nFlag) {			// n flag is set, skip to next instruction
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
				} else {						// n flag not set, take relative jump
					pcReg = effective_address_l;
				}
				break;
			case 0x1a :								// inc instruction
				aReg++;
				setStatusForNZ(aReg);
				break;
			case 0x20 :								// jsr abs instruction (push last byte addr of instr
			case 0x23 :								// jsr (abs,x)
				csg65ce02_push_byte(thisCPU, msb((uint16_t)(pcReg+2)));
				csg65ce02_push_byte(thisCPU, lsb((uint16_t)(pcReg+2)));
				pcReg = effective_address_l;
				break;
			case 0x24 :								// bit bp
			case 0x2c :								// bit abs
			case 0x34 :								// bit bp,x
			case 0x3c :								// bit abs,x
			case 0x89 :								// bit immediate
				temp_byte = memory[effective_address_l];
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
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
				}
				break;
			case 0x3a :								// dec
				aReg--;
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
			case 0x5b :								// tab
				bReg = aReg;
				break;
            case 0x60 :								// rts instruction
				pcReg = csg65ce02_pull_byte(thisCPU) | ( csg65ce02_pull_byte(thisCPU) << 8 );
				pcReg = (uint16_t)(pcReg+1);		// increase pc by 1 and wrap if necessary
				break;
			case 0x64 :								// stz bp
				memory[effective_address_l] = zReg;
				break;
			case 0x6b :								// tza
				aReg = zReg;
				setStatusForNZ(aReg);
				break;
			case 0x80 :								// bra rel
			case 0x83 :								// bra wrel
				pcReg = effective_address_l;
				break;
			case 0x81 :								// sta (bp,x)
			case 0x8d :								// sta absolute
			case 0x99 :								// sta abs,y
				memory[effective_address_l] = aReg;
				break;
			case 0x84 :								// sty bp
			case 0x8b :								// sty abs,x
			case 0x8c :								// sty abs
			case 0x94 :								// sty bp,x
				memory[effective_address_l] = yReg;
				break;
			case 0x88 :								// dey
				yReg--;
				setStatusForNZ(yReg);
				break;
			case 0x90 :								// bcc rel
			case 0x93 :								// bcc wrel
				if(thisCPU->cFlag) {			// carry set, skip to next instruction
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
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
				yReg = memory[effective_address_l];
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
				aReg = memory[effective_address_l];
				setStatusForNZ(aReg);
				break;
			case 0xa2 :								// ldx immediate
			case 0xa6 :								// ldx bp
			case 0xae :								// ldx abs
			case 0xb6 :								// ldx bp,y
			case 0xbe :								// ldx abs,y
				xReg = memory[effective_address_l];
				setStatusForNZ(xReg);
				break;
			case 0xa3 :								// ldz immediate
			case 0xab :								// ldz abs
			case 0xbb :								// ldz abs,x
				zReg = memory[effective_address_l];
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
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
				}
				break;
			case 0xba :								// tsx instruction
				xReg = (spReg & 0x00ff);
				setStatusForNZ(xReg);
				break;
			case 0xc0 :								// cpy immediate
			case 0xc4 :								// cpy bp
			case 0xcc :								// cpy abs
				temp_word = (0x0100 | yReg) - memory[effective_address_l];
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
				temp_word = (0x0100 | aReg) - memory[effective_address_l];
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
				temp_word = (0x0100 | zReg) - memory[effective_address_l];
				setStatusForNZ(0x00ff & temp_word);
				if(temp_word & 0xff00) {		// zReg >= operand
					thisCPU->cFlag = cFlagValue;
				} else {						// zReg < operand
					thisCPU->cFlag = 0;
				}
				break;
			case 0xc8 :								// iny
				yReg++;
				setStatusForNZ(yReg);
				break;
			case 0xca :								// dex
				xReg--;
				setStatusForNZ(xReg);
				break;
			case 0xd0 :								// bne rel
			case 0xd3 :								// bne wrel
				if(thisCPU->zFlag) {			// equal, skip to next instruction
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
				} else {						// not equal, take relative jump
					pcReg = effective_address_l;
				}
				break;
			case 0xe0 :								// cpx immediate
			case 0xe4 :								// cpx bp
			case 0xec :								// cpx abs
				temp_word = (0x0100 | xReg) - memory[effective_address_l];
				setStatusForNZ(0x00ff & temp_word);
				if(temp_word & 0xff00) {		// xReg >= operand
					thisCPU->cFlag = cFlagValue;
				} else {						// xReg < operand
					thisCPU->cFlag = 0;
				}
				break;
			case 0xe8 :								// inx
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
					pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
				}
				break;
            default :								// opcode not implemented
				printf("error: opcode not implemented\n");
        }
		thisCPU->cycles_last_executed_instruction = cycles_per_instruction[current_opcode];
		cycle_count += thisCPU->cycles_last_executed_instruction;

		// increase pc only if the instruction does not actively change the pc by itself
		if(!modify_pc_per_instruction[current_opcode]) {
			pcReg = (uint16_t)(pcReg+bytes_per_instruction[current_opcode]);
		}
    } while( (cycle_count < noCycles) && running);

    return cycle_count;
}

void csg65ce02_dump_status(csg65ce02 *thisCPU) {
	if( thisCPU->eFlag ) {
		printf(" pc   ac xr yr zr bp sh   sl nvedizc\n");
		printf("%04x  %02x %02x %02x %02x %02x %02x 01%02x %s%s%s%s%s%s%s\n\n", pcReg, aReg, xReg, yReg, zReg, bReg, spReg >> 8, spReg & 0x00ff, thisCPU->nFlag ? "*" : ".", thisCPU->vFlag ? "*" : ".", thisCPU->eFlag ? "*" : ".", thisCPU->dFlag ? "*" : ".", thisCPU->iFlag ? "*" : ".", thisCPU->zFlag ? "*" : ".", thisCPU->cFlag ? "*" : "." );
	} else {
		printf(" pc   ac xr yr zr bp     sp  nvedizc\n");
		printf("%04x  %02x %02x %02x %02x %02x    %02x%02x %s%s%s%s%s%s%s\n\n", pcReg, aReg, xReg, yReg, zReg, bReg, spReg >> 8, spReg & 0x00ff, thisCPU->nFlag ? "*" : ".", thisCPU->vFlag ? "*" : ".", thisCPU->eFlag ? "*" : ".", thisCPU->dFlag ? "*" : ".", thisCPU->iFlag ? "*" : ".", thisCPU->zFlag ? "*" : ".", thisCPU->cFlag ? "*" : "." );
	}
}

void csg65ce02_dump_page(csg65ce02 *thisCPU, uint8_t pageNo) {
    for(int i=0; i<0x100; i++) {
        if(i%16 == 0) printf("\n%04x", pageNo<<8 | i);
        printf(" %02x", memory[pageNo << 8 | i]);
    }
    printf("\n");
}
