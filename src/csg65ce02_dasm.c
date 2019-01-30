//  csg65ce02_disassemble.c
//
//  Created by elmerucr on 24/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include "csg65ce02.h"
#include "csg65ce02_dasm.h"

// all mnemonics (instructions)
const char *mnemonics[120] = {
	"adc ","and ","asl ","asr ","asw ","aug ","bbr 0,","bbr 1,","bbr 2,","bbr 3,",
	"bbr 4,","bbr 5,","bbr 6,","bbr 7,","bbs 0,","bbs 1,","bbs 2,","bbs 3,","bbs 4,","bbs 5,",
	"bbs 6,","bbs 7,","bcc ","bcs ","beq ","bit ","bmi ","bne ","bpl ","bra ",
	"brk ","bsr ","bvc ","bvs ","clc ","cld ","cle ","cli ","clv ","cmp ",
	"cpx ","cpy ","cpz ","dea ","dew ","dex ","dey ","dez ","eor ","ina ",
	"inw ","inx ","iny ","inz ","jmp ","jsr ","lda ","ldx ","ldy ","ldz ",
	"lsr ","neg ","nop ","ora ","pha ","php ","phw ","phx ","phy ","phz ",
	"pla ","plp ","plx ","ply ","plz ","rmb 0,","rmb 1,","rmb 2,","rmb 3,","rmb 4,",
	"rmb 5,","rmb 6,","rmb 7,","rol ","ror ","row ","rti ","rtn ","rts ","sbc ",
	"sec ","sed ","see ","sei ","smb 0,","smb 1,","smb 2,","smb 3,","smb 4,","smb 5,",
	"smb 6,","smb 7,","sta ","stx ","sty ","stz ","tab ","tax ","tay ","taz ",
	"tba ","trb ","tsb ","tsx ","tsy ","txa ","txs ","tya ","tys ","tza "
};

const int mnemonic_per_instruction[256] = {
	30,63,36,92,112,63,2,75,65,63,2,114,112,63,2,6,
	28,63,63,28,111,63,2,76,34,63,49,53,111,63,2,7,
	55,1,55,55,25,1,83,77,71,1,83,118,25,1,83,8,
	26,1,1,26,25,1,83,78,90,1,43,47,25,1,83,9,
	86,48,61,3,3,48,60,79,64,48,60,109,54,48,60,10,
	32,48,48,32,3,48,60,80,37,48,68,106,5,48,60,11,
	88,0,87,31,105,0,84,81,70,0,84,119,54,0,84,12,
	33,0,0,33,105,0,84,82,93,0,73,110,54,0,84,13,
	29,102,102,29,104,102,103,94,46,25,115,104,104,102,103,14,
	22,102,102,22,104,102,103,95,117,102,116,103,105,102,105,15,
	58,56,57,59,58,56,57,96,108,56,107,59,58,56,57,16,
	23,56,56,23,58,56,57,97,38,56,113,59,58,56,57,17,
	41,39,42,44,41,39,43,98,52,39,45,4,41,39,43,18,
	27,39,39,27,42,39,43,99,35,39,67,69,42,39,43,19,
	40,89,56,50,40,89,49,100,51,89,62,85,40,89,49,20,
	24,89,89,24,66,89,49,101,91,89,72,74,66,89,49,21
};

const char *operand_modes[20][2] = {
	{ "#$", "" },
	{ "$", "" },
	{ "$", "" },
	{ "", "" },
	{ "", "" },
	{ "$",",x" },
	{ "$",",y" },
	{ "$",",x" },
	{ "$",",y" },
	{ "($",",x)" },
	{ "($","),y" },
	{ "($","),z" },
	{ "($",",sp),y" },
	{ "$", "" },
	{ "$", "" },
	{ "($",")" },
	{ "$",",$" },
	{ "($",",x)" },
	{ "#$","" },
	{ "$","" }
};

uint8_t csg65ce02_dasm(uint16_t address, char *buffer, int length) {
	int addressing_mode = addressing_mode_per_instruction[csg65ce02_read_byte(address)];
	int mnemonic_index = mnemonic_per_instruction[csg65ce02_read_byte(address)];
	int instr_length = bytes_per_instruction[csg65ce02_read_byte(address)];

	int opcode = csg65ce02_read_byte(address);
	uint8_t operand_byte0 = csg65ce02_read_byte((uint16_t)(address+1));
	uint8_t operand_byte1 = csg65ce02_read_byte((uint16_t)(address+2));

	switch( addressing_mode ) {
		case IMM :			// imm
		case BP :			// bp
		case BP_X :			// bp,x
		case BP_Y :			// bp,y
		case BP_X_IND :		// (bp,x)
		case BP_IND_Y :		// (bp),y
		case BP_IND_Z :		// (bp),z
		case D_SP_IND_Y :	// (d,sp),y
			snprintf(buffer,length,",%04x %02x %02x    %s%s%02x%s",
				address,
				opcode,
				operand_byte0,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				operand_byte0,
				operand_modes[addressing_mode][1]);
			break;
		case ABS :			// abs
		case ABS_X :		// abs,x
		case ABS_Y :		// abs,y
		case ABS_IND :		// (abs)
		case ABS_X_IND :	// (abs,x)
		case IMMW :			// immw
		case ABSW :			// absw
			snprintf(buffer,length,",%04x %02x %02x %02x %s%s%02x%02x%s",
				address,
				opcode,
				operand_byte0,
				operand_byte1,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				operand_byte1,
				operand_byte0,
				operand_modes[addressing_mode][1]);
			break;
		case ACCUM :		// accum
		case IMPLIED :		// implied
			snprintf(buffer,length,",%04x %02x       %s%s%s",
				address,
				opcode,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				operand_modes[addressing_mode][1]);
			break;
		case REL : {		// rel
			uint16_t result = (operand_byte0 & 0x80) ? 0xff00 | operand_byte0 : 0x0000 | operand_byte0;
			snprintf(buffer,length,",%04x %02x %02x    %s%s%04x%s",
				address,
				opcode,
				operand_byte0,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				(uint16_t)(address+2+result),
				operand_modes[addressing_mode][1]);
			}
			break;
		case WREL : {		// wrel
			uint16_t result = operand_byte0 | (operand_byte1 << 8);
			snprintf(buffer,length,",%04x %02x %02x %02x %s%s%04x%s",
				address,
				opcode,
				operand_byte0,
				operand_byte1,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				(uint16_t)(address+2+result),
				operand_modes[addressing_mode][1]);
			}
			break;
		case BPREL : {		// bprel (bbr* and bbs* instructions)
			uint16_t result = (operand_byte1 & 0x80) ? 0xff00 | operand_byte1 : 0x0000 | operand_byte1;
			snprintf(buffer,length,",%04x %02x %02x %02x %s%s%02x%s%04x",
				address,
				opcode,
				operand_byte0,
				operand_byte1,
				mnemonics[mnemonic_index],
				operand_modes[addressing_mode][0],
				operand_byte0,
				operand_modes[addressing_mode][1],
				(uint16_t)(address+3+result));
			}
			break;
		default :
			snprintf(buffer,length,"error: addressing mode not recognized");
	}
	return instr_length;
}
