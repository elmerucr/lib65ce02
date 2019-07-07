//  csg65ce02_mmu.c
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02_mmu.h"
#include "defs.h"

void csg65ce02_mmu_init()
{
	csg65ce02_ram = malloc(65536 * sizeof(uint8_t));			// get 64kb of ram storage space

	// fill ram with alternating 0x00's and 0xff's
	for(int i=0; i<(65536*sizeof(uint8_t)); i++)
	{
		csg65ce02_ram[i] = (i & 64) ? 0xff : 0x00;
	}
}

void csg65ce02_mmu_cleanup()
{
	free(csg65ce02_ram);
}

// memory access functions implementation:
uint8_t csg65ce02_read_byte(uint16_t address)
{
	return csg65ce02_ram[address];
}

inline void csg65ce02_write_byte(uint16_t address, uint8_t byte)
{
	csg65ce02_ram[address] = byte;
}
