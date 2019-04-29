//  csg65ce02_disassemble.h
//
//  Created by elmerucr on 24/4/2018.
//  Copyright © 2018 elmerucr. All rights reserved.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CSG65CE02_DASM_H
#define CSG65CE02_DASM_H

// function disassembles one instr at given address and returns the total size of the instruction
uint8_t csg65ce02_dasm(uint16_t address, char *buffer, int length);

#endif

#ifdef __cplusplus
}
#endif
