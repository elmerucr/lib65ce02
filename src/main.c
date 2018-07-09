//  main.c
//
//  Created by elmerucr on 26/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02.h"
#include "csg65ce02_dasm.h"
#include "csg65ce02_mmu.h"

#define TEXT_BUFFER_SIZE 64

int main() {
	csg65ce02_mmu_init();		// first, initialize memory management unit and memory

	// set nmi, reset & break vectors
	csg65ce02_ram[0x80ffa] = 0x00;		// nmi $2000
	csg65ce02_ram[0x80ffb] = 0x20;
	csg65ce02_ram[0x80ffc] = 0x00;		// reset $c000
	csg65ce02_ram[0x80ffd] = 0xc0;
	csg65ce02_ram[0x80ffe] = 0x00;		// brk $a900
	csg65ce02_ram[0x80fff] = 0xa9;

	// program
	csg65ce02_ram[0x0c000] = 0x02;		// cle
	csg65ce02_ram[0x0c001] = 0xa9;		// lda #$34
	csg65ce02_ram[0x0c002] = 0x34;
	csg65ce02_ram[0x0c003] = 0x48;		// pha
	csg65ce02_ram[0x0c004] = 0x48;		// pha
	csg65ce02_ram[0x0c005] = 0x48;		// pha
	csg65ce02_ram[0x0c006] = 0xea;		// nop
	csg65ce02_ram[0x0c007] = 0xf4;		// phw #$e432
	csg65ce02_ram[0x0c008] = 0x32;
	csg65ce02_ram[0x0c009] = 0xe4;
	csg65ce02_ram[0x0c00a] = 0xfc;		// phw $ffff
	csg65ce02_ram[0x0c00a] = 0xff;
	csg65ce02_ram[0x0c00b] = 0xff;

	printf("\nemulate_65ce02 (C)2018 by elmerucr v20180522.0\n");

	char text_buffer[TEXT_BUFFER_SIZE];	// allocate storage for text text_buffer to print strings
	csg65ce02 cpu0;

	// reset and print message
	printf("\nResetting 65ce02\n");
	csg65ce02_reset(&cpu0);
	csg65ce02_dump_status(&cpu0);
	csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
	printf("%s <--> %i cycle(s)\n", text_buffer, cycles_per_instruction[csg65ce02_ram[cpu0.pc]]);
	printf("\nType 'h' for help\n");

	char prompt = '.';
	char input_command;

	int running = 1;

	uint8_t temp_byte;

	do {
		putchar('\n');
		putchar(prompt);
		input_command = getchar(); getchar();
		switch( input_command ) {
			case 'b' :
				printf("Dump current basepage:\n");
				csg65ce02_dump_page(&cpu0,cpu0.b);
				break;
			case 'd' : {
				uint16_t start = cpu0.pc;
					for(int i=0; i<8; i++) {
						start += csg65ce02_dasm(start, text_buffer, TEXT_BUFFER_SIZE);
						puts(text_buffer);
					}
				}
				break;
			case 'h' :
				printf("b - Dump basepage\n");
				printf("d - Disassemble next 8 instructions\n");
				printf("h - Prints this help message\n");
				printf("n - Execute next instruction\n");
				printf("r - Reset 65ce02\n");
				printf("s - Prints processor status\n");
				printf("t - Dump current stack page\n");
				printf("x - Exit emulate_65ce02\n");
				break;
			case 'n' :
				csg65ce02_execute(&cpu0,0);
				csg65ce02_dump_status(&cpu0);
				csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				printf("%s <--> %i cycle(s)\n",text_buffer,cycles_per_instruction[csg65ce02_ram[cpu0.pc]]);
				break;
			case 'r' :
				printf("Resetting 65ce02\n");
				csg65ce02_reset(&cpu0);
				csg65ce02_dump_status(&cpu0);
				csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				puts(text_buffer);
				break;
			case 's' :
				csg65ce02_dump_status(&cpu0);
				csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				printf("%s <--> %i cycle(s)\n",text_buffer,cycles_per_instruction[csg65ce02_ram[cpu0.pc]]);
				break;
			case 't' :
				if(cpu0.eFlag) {			// stack page always $01
					printf("e flag is set, stack in 6502 mode\n");
					temp_byte = (cpu0.sp & 0xff00) >> 8;
				} else {				//
					printf("e flag is not set, stack in extended mode\n");
					temp_byte = (cpu0.sp & 0xff00) >> 8;
				}
				csg65ce02_dump_page(&cpu0,temp_byte);	// dump stack
				break;
			case 'x' :
				running = 0;
				break;
			default :
				printf("Error: unknown command '%c'\n",input_command);
		}
	} while(running);

	printf("\nbye\n\n");

	return 0;
}