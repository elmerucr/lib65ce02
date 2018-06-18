//  csg65ce02_mmu.c
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdint.h>
#include <stdlib.h>
#include "csg65ce02_mmu.h"

void csg65ce02_mmu_init() {
	csg65ce02_memoryblock = malloc(1024 * 1024 * sizeof(uint8_t));			// get 1mb of storage space
	// fill memory it with alternating pattern
	for(int i=0; i<(1024*1024); i++) {
		// fill blocks alternating with a number and zeroes
		// lower half gets 0xff, upper half gets 0x80
		csg65ce02_memoryblock[i] = (i & 64) ? ((i & ((1024 * 1024) / 2)) ? 0xff : 0x80) : 0x00;
	}
}

void csg65ce02_mmu_cleanup() {
	free(csg65ce02_memoryblock);
}

// memory access functions need to be implemented:
// ...
// ...