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
	csg65ce02_ram = malloc(1024 * 1024 * sizeof(uint8_t));			// get 1mb of ram storage space
	// fill ram with alternating 0x00's and 0xff's
	for(int i=0; i<(1024*1024); i++) {
		csg65ce02_ram[i] = (i & 64) ? 0xff : 0x00;
	}
	for(int i=0; i<16; i++) {
		csg65ce02_mmu_registers[i] = i;
	}
	csg65ce02_mmu_registers[0x0f] = 0x80;
}

void csg65ce02_mmu_cleanup() {
	free(csg65ce02_ram);
}

// memory access functions implementation:
uint8_t csg65ce02_read_byte(uint16_t address) {
	uint8_t result;
	switch((address & 0xff00) >> 8) {
		default :
			result =  csg65ce02_ram[(csg65ce02_mmu_registers[address >> 12] << 12) | (address & 0x0fff)];
	}
	return result;
}

inline void csg65ce02_write_byte(uint16_t address, uint8_t byte) {
	uint32_t phys_addr = (csg65ce02_mmu_registers[address >> 12] << 12) | (address & 0x0fff);
	if(!(phys_addr & 0x00080000)) csg65ce02_ram[phys_addr] = byte;
}