//  csg65ce02_mmu.h
//
//  Created by elmerucr on 16/06/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

// Memory contains 64kb ram, physically addressable by mmu
//
// Please note:
// Memory access functions are declared in csg65ce02.h, implementation is in csg65ce02_mmu.c
//

#ifdef __cplusplus
extern "C"
{
#endif

	#ifndef CSG65CE02_MMU_H
	#define CSG65CE02_MMU_H

	#include "csg65ce02.h"

	uint8_t *csg65ce02_ram;		// will be publically visible (for dma purposes)
								// should not be used directly by cpu (cpu uses access functions)

	void csg65ce02_mmu_init();
	void csg65ce02_mmu_cleanup();

	#endif

#ifdef __cplusplus
}
#endif