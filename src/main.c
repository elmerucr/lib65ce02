//  main.c
//
//  Created by elmerucr on 26/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02.h"
#include "csg65ce02_dasm.h"

#define TEXT_BUFFER_SIZE 64

int main() {
	// prepare 64k memory
	uint8_t memory0[65536];
	// fill it with alternating pattern
	for(int i=0; i<65536; i++) {
		memory0[i] = (i & 64) ? 0xff : 0x00;
	}

	// set nmi, reset & break vectors
	memory0[0xfffa] = 0x00;		// nmi $2000
	memory0[0xfffb] = 0x20;
	memory0[0xfffc] = 0x00;		// reset $0400
	memory0[0xfffd] = 0xc0;
	memory0[0xfffe] = 0x00;		// brk $a900
	memory0[0xffff] = 0xa9;

	// 'interesting' data in bp
	memory0[0x0011] = 0xff;
	memory0[0x001e] = 0x58;

	// program
	memory0[0xc000] = 0xa0;
	memory0[0xc001] = 0x00;
	memory0[0xc002] = 0x8c;
	memory0[0xc003] = 0x37;
	memory0[0xc004] = 0xc0;
	memory0[0xc005] = 0xb9;
	memory0[0xc006] = 0x31;
	memory0[0xc007] = 0xc0;
	memory0[0xc008] = 0xaa;
	memory0[0xc009] = 0xc8;
	memory0[0xc00a] = 0xca;
	memory0[0xc00b] = 0xb9;
	memory0[0xc00c] = 0x31;
	memory0[0xc00d] = 0xc0;
	memory0[0xc00e] = 0xc8;
	memory0[0xc00f] = 0xd9;
	memory0[0xc010] = 0x31;
	memory0[0xc011] = 0xc0;
	memory0[0xc012] = 0x90;
	memory0[0xc013] = 0x14;
	memory0[0xc014] = 0xf0;
	memory0[0xc015] = 0x12;
	memory0[0xc016] = 0x4b;
	memory0[0xc017] = 0xb9;
	memory0[0xc018] = 0x31;
	memory0[0xc019] = 0xc0;
	memory0[0xc01a] = 0x88;
	memory0[0xc01b] = 0x99;
	memory0[0xc01c] = 0x31;
	memory0[0xc01d] = 0xc0;
	memory0[0xc01e] = 0x6b;
	memory0[0xc01f] = 0xc8;
	memory0[0xc020] = 0x99;
	memory0[0xc021] = 0x31;
	memory0[0xc022] = 0xc0;
	memory0[0xc023] = 0xa9;
	memory0[0xc024] = 0xff;
	memory0[0xc025] = 0x8d;
	memory0[0xc026] = 0x37;
	memory0[0xc027] = 0xc0;
	memory0[0xc028] = 0xca;
	memory0[0xc029] = 0xd0;
	memory0[0xc02a] = 0xe0;
	memory0[0xc02b] = 0x2c;
	memory0[0xc02c] = 0x37;
	memory0[0xc02d] = 0xc0;
	memory0[0xc02e] = 0x30;
	memory0[0xc02f] = 0xd0;
	memory0[0xc030] = 0xea;
	memory0[0xc031] = 0x05;
	memory0[0xc032] = 0x05;
	memory0[0xc033] = 0x04;
	memory0[0xc034] = 0x03;
	memory0[0xc035] = 0x02;
	memory0[0xc036] = 0x01;

	printf("\nemulate_65ce02 (C)2018 by elmerucr v20180522.0\n");

	char text_buffer[TEXT_BUFFER_SIZE];	// allocate storage for text text_buffer to print strings
	csg65ce02 cpu0;
	csg65ce02_init(&cpu0,memory0);

	// reset and print message
	printf("\nResetting 65ce02\n");
	csg65ce02_reset(&cpu0);
	csg65ce02_dump_status(&cpu0);
	csg65ce02_dasm(memory0,cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
	printf("%s <--> %i cycle(s)\n", text_buffer, cycles_per_instruction[memory0[cpu0.pc]]);
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
						start += csg65ce02_dasm(memory0, start, text_buffer, TEXT_BUFFER_SIZE);
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
				csg65ce02_dasm(memory0,cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				printf("%s <--> %i cycle(s)\n",text_buffer,cycles_per_instruction[memory0[cpu0.pc]]);
				break;
			case 'r' :
				printf("Resetting 65ce02\n");
				csg65ce02_reset(&cpu0);
				csg65ce02_dump_status(&cpu0);
				csg65ce02_dasm(memory0,cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				puts(text_buffer);
				break;
			case 's' :
				csg65ce02_dump_status(&cpu0);
				csg65ce02_dasm(memory0,cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
				printf("%s <--> %i cycle(s)\n",text_buffer,cycles_per_instruction[memory0[cpu0.pc]]);
				break;
			case 't' :
				if(cpu0.eFlag) {			// stack page always $01
					printf("e flag is set, stack in 6502 mode\n");
					temp_byte = 0x01;
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