//  csg65ce02_disassemble.c
//
//  Created by elmerucr on 24/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include "csg65ce02.h"
#include "csg65ce02_dasm.h"

// all mnemonics (instructions)
const char *mnemonics[122] = {
	"adc ","and ","asl ","asr ","asw ","aug ","bbr 0,","bbr 1,","bbr 2,","bbr 3,",
	"bbr 4,","bbr 5,","bbr 6,","bbr 7,","bbs 0,","bbs 1,","bbs 2,","bbs 3,","bbs 4,","bbs 5,",
	"bbs 6,","bbs 7,","bcc ","bcs ","beq ","bit ","bmi ","bne ","bpl ","bra ",
	"brk ","bsr ","bvc ","bvs ","clc ","cld ","cle ","cli ","clv ","cmp ",
	"cpx ","cpy ","cpz ","dec ","dec a ","dec x ","dec y ","dec z ","dew ","eor ",
	"inc ","inc a ","inc x ","inc y ","inc z ","inw ","jmp ","jsr ","lda ","ldx ",
	"ldy ","ldz ","lsr ","neg ","nop ","ora ","pha ","php ","phw ","phx ",
	"phy ","phz ","pla ","plp ","plx ","ply ","plz ","rmb 0,","rmb 1,","rmb 2,",
	"rmb 3,","rmb 4,","rmb 5,","rmb 6,","rmb 7,","rol ","ror ","row ","rti ","rtn ",
	"rts ","sbc ","sec ","sed ","see ","sei ","smb 0,","smb 1,","smb 2,","smb 3,",
	"smb 4,","smb 5,","smb 6,","smb 7,","sta ","stx ","sty ","stz ","tab ","tax ",
	"tay ","taz ","tba ","trb ","tsb ","tsx ","tsy ","txa ","txs ","tya ",
	"tys ","tza "
};

const int mnemonic_per_instruction[256] = {
	30,6,36,94,114,65,2,77,67,65,2,116,114,65,2,6,
	28,65,65,28,113,65,2,78,34,65,51,54,113,65,2,7,
	57,1,57,57,25,1,85,79,73,1,85,120,25,1,85,8,
	26,1,1,26,25,1,85,80,92,1,44,47,25,1,85,9,
	88,49,63,3,3,49,62,81,66,49,62,111,56,49,62,10,
	32,49,49,32,3,49,62,82,37,49,70,108,5,49,62,11,
	90,0,89,31,107,0,86,83,72,0,86,121,56,0,86,12,
	33,0,0,33,107,0,86,84,95,0,75,112,56,0,86,13,
	29,104,104,29,106,104,105,96,46,25,117,106,106,104,105,14,
	22,104,104,22,106,104,105,97,119,104,118,105,107,104,107,15,
	60,58,59,61,60,58,59,98,110,58,109,61,60,58,59,16,
	23,58,58,23,60,58,59,99,38,58,115,61,60,58,59,17,
	41,39,42,48,41,39,43,100,53,39,45,4,41,39,43,18,
	27,39,39,27,42,39,43,101,35,39,69,71,42,39,43,19,
	40,91,58,55,40,91,50,102,52,91,64,87,40,91,50,20,
	24,91,91,24,68,91,50,103,93,91,74,76,68,91,50,21
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
