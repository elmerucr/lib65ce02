# todo
- Implement all missing instructions.
- What to do with AUG instruction?
- What's the precise difference between the init and the reset function?
- The ````brk```` instruction needs special attention. Several functions that implement interrupts (or exceptions / "special flows") need to be written and one will be called by the ````brk```` instruction.
- Currently, memory is defined as a simple array of 64k. Although access seems quick and easy, it doesn't allow real memory management by a virtual mmu (emulate ram/rom/illegal access/...). Memory access functions are defined in the header files,  but must be implemented by a memory management unit (mmu). Currently these functions are still the simple versions
- Check ````pla```` opcode in simple and extended stack mode
- Please Implement: IRQ's can be acknowledged only if the last executed instruction took more than 1 cycle. This is to prevent errors when changing the stack point from small to large.