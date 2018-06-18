//  csg65ce02_mmu.h
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

// Memory contains 512kb ram, physically addressable by mmu, divided in 128 x 4kb pages
// Memory may contain a max of 128 4kb roms.
// Implementation is as 1 array of exactly 1mb. The top half (msb put to 1) is rom
//
//

#ifndef CSG65CE02_MMU_H
#define CSG65CE02_MMU_H

uint8_t *csg65ce02_memoryblock;		// will be publically visible (for dma purposes)

void csg65ce02_mmu_init();
void csg65ce02_mmu_cleanup();

#endif