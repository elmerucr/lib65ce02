//  main.c
//
//  Created by elmerucr on 26/4/2018
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "csg65ce02.h"
#include "csg65ce02_dasm.h"
#include "csg65ce02_mmu.h"

#define TEXT_BUFFER_SIZE 64

// This function reads a line from stdin, and returns a pointer to a string. The caller needs to free allocated memory
char *read_line(void);

int main() {
	csg65ce02_mmu_init();		// first, initialize memory management unit and memory

	// set nmi, reset & break vectors
	csg65ce02_ram[0xfffa] = 0x00;		// nmi $a800
	csg65ce02_ram[0xfffb] = 0xa8;
	csg65ce02_ram[0xfffc] = 0x00;		// reset $c000
	csg65ce02_ram[0xfffd] = 0xc0;
	csg65ce02_ram[0xfffe] = 0x00;		// brk $a900
	csg65ce02_ram[0xffff] = 0xa9;

	// irq routine
	csg65ce02_ram[0xa900] = 0x48;		// pha, save processor state
	csg65ce02_ram[0xa901] = 0xda;		// phx
	csg65ce02_ram[0xa902] = 0x5a;		// phy
	csg65ce02_ram[0xa903] = 0xdb;		// phz
	// discover the cause of irq, if brk instruction, then take into account signature byte!
	// maybe better to use the stack pointer relative addressing mode!?
	csg65ce02_ram[0xa904] = 0xba;		// tsx, load lsb of sp into x
	csg65ce02_ram[0xa905] = 0x0b;		// tsy, load msb of sp into y
	csg65ce02_ram[0xa906] = 0x86;		// stx $f0
	csg65ce02_ram[0xa907] = 0xf0;
	csg65ce02_ram[0xa908] = 0x84;		// sty $f1
	csg65ce02_ram[0xa909] = 0xf1;
	csg65ce02_ram[0xa90a] = 0xa0;		// ldy #$05
	csg65ce02_ram[0xa90b] = 0x05;
	csg65ce02_ram[0xa90c] = 0xb1;		// lda ($f0),y
	csg65ce02_ram[0xa90d] = 0xf0;
	// do your interrupt handling code here
	// restore registers
	csg65ce02_ram[0xa90e] = 0xfb;		// plz
	csg65ce02_ram[0xa90f] = 0x7a;		// ply
	csg65ce02_ram[0xa910] = 0xfa;		// plx
	csg65ce02_ram[0xa911] = 0x68;		// pla
	csg65ce02_ram[0xa912] = 0x40;		// rti

	// main program in memory
	csg65ce02_ram[0xc000] = 0x58;		// cli
	csg65ce02_ram[0xc001] = 0x02;		// cle
	csg65ce02_ram[0xc002] = 0xa9;		// lda #$34
	csg65ce02_ram[0xc003] = 0x34;
	csg65ce02_ram[0xc004] = 0x8a;		// txa
	csg65ce02_ram[0xc005] = 0xaa;		// tax
	csg65ce02_ram[0xc006] = 0x48;		// pha
	csg65ce02_ram[0xc007] = 0xea;		// nop
	csg65ce02_ram[0xc008] = 0xf4;		// phw #$e432
	csg65ce02_ram[0xc009] = 0x32;
	csg65ce02_ram[0xc00a] = 0xe4;
	csg65ce02_ram[0xc00b] = 0xfc;		// phw $ffff
	csg65ce02_ram[0xc00c] = 0xff;
	csg65ce02_ram[0xc00d] = 0xff;
	csg65ce02_ram[0xc00e] = 0xad;		// lda $020f
	csg65ce02_ram[0xc00f] = 0x0f;
	csg65ce02_ram[0xc010] = 0x02;
	csg65ce02_ram[0xc011] = 0x00;		// brk #$01 (with signature byte #$01)
	csg65ce02_ram[0xc012] = 0x01;
	csg65ce02_ram[0xc013] = 0xa9;		// lda #$85
	csg65ce02_ram[0xc014] = 0x85;
	csg65ce02_ram[0xc015] = 0x0a;		// asl
	csg65ce02_ram[0xc016] = 0x4c;		// jmp $c002
	csg65ce02_ram[0xc017] = 0x02;
	csg65ce02_ram[0xc018] = 0xc0;


	printf("\nemulate_65ce02 (C)2019 by elmerucr v20190317.0\n");
	printf("type 'help' for a list of possible commands\n");

	char text_buffer[TEXT_BUFFER_SIZE];	// allocate storage for text_buffer to print strings
	char large_text_buffer[256];		// allocate more storage for print functions

	csg65ce02 cpu0;
	csg65ce02_init(&cpu0);
	csg65ce02_enable_breakpoints(&cpu0);		// in this setting, breakpoints are always enabled

	// reset system and print welcome message
	printf("resetting 65ce02...\n\n");
	csg65ce02_reset(&cpu0);
	csg65ce02_dump_status(&cpu0, large_text_buffer);
	printf("%s\n\n", large_text_buffer);
	csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
	printf("%s\n", text_buffer);

	// prepare command line interface
	char prompt = '.';
	uint8_t temp_byte;
	char *input_string;
	char *token0, *token1, *token2, *token3;
	bool finished = false;

	do {
		putchar(prompt);
		input_string = read_line();
		token0 = strtok( input_string, " ");
		token1 = strtok( NULL, " ");
		token2 = strtok( NULL, " ");
		token3 = strtok( NULL, " ");

		if( token0 == NULL ) {
			// do nothing, just catch the empty token, as strcmp with NULL pointer results in segfault
		} else if( strcmp(token0, "b") == 0 ) {
			if( token1 == NULL ) {
				bool breakpoints_present = false;
				for(int i=0; i<65536; i++) {
					if( cpu0.breakpoint_array[i] == true ) {
						printf("$%04x\n",i);
						breakpoints_present = true;
					}
				}
				if( !breakpoints_present) {
					printf("no breakpoints defined\n");
				}
			} else {
				unsigned int i;
				sscanf( token1, "%04x", &i);
				if(cpu0.breakpoint_array[i]) {
					printf("removing breakpoint at $%04x\n", i);
					cpu0.breakpoint_array[i] = false;
				} else {
					printf("adding breakpoint at $%04x\n", i);
					cpu0.breakpoint_array[i] = true;
				}
			}
		} else if( strcmp(token0, "base") == 0 ) {
			printf("basepage");
			csg65ce02_dump_page(&cpu0,cpu0.b);
		} else if( strcmp(token0, "d") == 0) {
			uint16_t start = cpu0.pc;
			for(int i=0; i<8; i++) {
				start += csg65ce02_dasm(start, text_buffer, TEXT_BUFFER_SIZE);
				putchar(',');
				puts(text_buffer);
			}
		} else if( strcmp(token0, "exit") == 0 ) {
			finished = true;
		} else if( strcmp(token0, "help") == 0) {
			printf("\nCommands:\n");
			printf("b - Breakpoint related commands\n");
			printf("d - Disassemble next 8 instructions\n");
			printf("i - Change irq pin state\n");
			printf("n - Execute next instruction\n");
			printf("r - Dump processor registers\n");
			printf("t - Dump current stack page\n\n");
			printf("base   - Dump current basepage\n");
			printf("exit   - Exit emulate_65ce02\n");
			printf("help   - Prints this help message\n");
			printf("reset  - Reset 65ce02\n\n");
			//printf("Type 'help <command name>' for more detailed info\n\n");
		} else if( strcmp(token0, "irq") == 0) {
			if( token1 == NULL ) {
				printf("Current status of irq pin is %1u\n", cpu0.irq_pin ? 1 : 0);
			} else {
				unsigned int i;
				sscanf( token1, "%1u", &i);
				if( i == 0 ) {
					csg65ce02_set_irq(&cpu0, false);
					printf("Current status of irq pin is %1u\n", cpu0.irq_pin ? 1 : 0);
				} else if( i == 1) {
					csg65ce02_set_irq(&cpu0, true);
					printf("Current status of irq pin is %1u\n", cpu0.irq_pin ? 1 : 0);
				} else {
					printf("Error: argument must be 0 or 1\n");
				}
			}
		} else if( strcmp(token0, "n") == 0) {
			unsigned int n = 0;
			if( token1 == NULL ) {
				// do nothing, n must remain 0
			} else {
				sscanf( token1, "%i", &n);
			}
			int i = csg65ce02_execute(&cpu0,n);
			printf("cpu ran %i cycles of %i demanded\n\n",i,n);
			csg65ce02_dump_status(&cpu0, large_text_buffer);
			printf("%s\n\n", large_text_buffer);
			csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
			printf("%s\n",text_buffer);
		} else if( strcmp(token0, "r") == 0 ) {
			csg65ce02_dump_status(&cpu0, large_text_buffer);
			printf("%s\n\n", large_text_buffer);
			csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
			printf("%s\n",text_buffer);
		} else if( strcmp(token0, "reset") == 0 ) {
			printf("Resetting 65ce02\n");
			csg65ce02_reset(&cpu0);
			csg65ce02_dump_status(&cpu0, large_text_buffer);
			printf("%s\n\n", large_text_buffer);
			csg65ce02_dasm(cpu0.pc,text_buffer, TEXT_BUFFER_SIZE);
			puts(text_buffer);
		} else if( strcmp(token0, "t") == 0 ) {
			if(cpu0.eFlag) {			// stack page always $01
				printf("e flag is set, stack is in 6502 mode\n");
			} else {				//
				printf("e flag is not set, stack is in extended mode\n");
			}
			temp_byte = (cpu0.sp & 0xff00) >> 8;
			csg65ce02_dump_page(&cpu0,temp_byte);	// dump stack
		} else {
			printf("Error: unknown command '%s'\n", input_string);
		}
	} while( !finished );

	free(input_string);
	free(cpu0.breakpoint_array);

	printf("bye\n\n");

	return 0;
}

char *read_line(void) {
	int bufsize = TEXT_BUFFER_SIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(1);	// failure
	}

	while (1) {
		// Read a character
		c = getchar();
		// If we hit EOF, replace it with a null character and return.
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

    	// If we have exceeded the buffer, reallocate.
    	if (position >= bufsize) {
    		bufsize += TEXT_BUFFER_SIZE;
    		buffer = realloc(buffer, bufsize);
    		if (!buffer) {
        		fprintf(stderr, "lsh: allocation error\n");
        		exit(1);		// failure
      		}
    	}
  	}
}