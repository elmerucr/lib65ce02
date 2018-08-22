//  csg65ce02_mmu.h
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

// Memory contains 1024kb ram, physically addressable by mmu, divided in 256 x 4kb blocks
//
// Please note:
// Memory access functions are declared in csg65ce02.h, implementation is in csg65ce02_mmu.c
//

#ifndef CSG65CE02_MMU_H
#define CSG65CE02_MMU_H

#include "csg65ce02.h"

uint8_t *csg65ce02_ram;		// will be publically visible (for dma purposes)
							// should not be used directly by cpu

void csg65ce02_mmu_init();
void csg65ce02_mmu_cleanup();

#endif