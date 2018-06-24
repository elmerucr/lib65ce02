//  csg65ce02_mmu.c
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02_mmu.h"

uint8_t csg65ce02_mmu_registers[16];	// internal mem registers of mmu
									// will be memory addressable from 65ce02 cpu

void csg65ce02_mmu_init() {
	csg65ce02_memoryblock = malloc(1024 * 1024 * sizeof(uint8_t));			// get 1mb of ram storage space
	// fill memory it with alternating 0x00 and 0xff
	for(int i=0; i<(1024*1024); i++) {
		csg65ce02_memoryblock[i] = (i & 64) ? 0xff : 0x00;
	}
	for(int i=0; i<16; i++) {
		csg65ce02_mmu_registers[i] = i;
	}

	// OBSOLETE --> needs removal, etc...
	csg65ce02_memoryblock[0x00000] = 0x00;
	csg65ce02_memoryblock[0x00001] = 0x01;
	csg65ce02_memoryblock[0x00002] = 0x02;
	csg65ce02_memoryblock[0x00003] = 0x03;
	csg65ce02_memoryblock[0x00004] = 0x04;
	csg65ce02_memoryblock[0x00005] = 0x05;
	csg65ce02_memoryblock[0x00006] = 0x06;
	csg65ce02_memoryblock[0x00007] = 0x07;
	csg65ce02_memoryblock[0x00008] = 0x08;
	csg65ce02_memoryblock[0x00009] = 0x09;
	csg65ce02_memoryblock[0x0000a] = 0x0a;
	csg65ce02_memoryblock[0x0000b] = 0x0b;
	csg65ce02_memoryblock[0x0000c] = 0x0c;
	csg65ce02_memoryblock[0x0000d] = 0x0d;
	csg65ce02_memoryblock[0x0000e] = 0x0e;
	csg65ce02_memoryblock[0x0000f] = 0x80;		// bit7=1 -> refers to ROM, bank 0
}

void csg65ce02_mmu_cleanup() {
	free(csg65ce02_memoryblock);
}

// memory access functions need to be implemented:
uint8_t csg65ce02_read_byte(uint16_t address) {
	uint8_t result;
	switch((address & 0xff00) >> 8) {
		default :
			result =  csg65ce02_memoryblock[(csg65ce02_memoryblock[address >> 12] << 12) | (address & 0x0fff)];
	}
	return result;
}

inline void csg65ce02_write_byte(uint16_t address, uint8_t byte) {
	uint32_t phys_addr = (csg65ce02_memoryblock[address >> 12] << 12) | (address & 0x0fff);
	if(!(phys_addr & 0x00080000)) csg65ce02_memoryblock[phys_addr] = byte;
}