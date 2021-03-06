# lib65ce02
# ![lib65ce02](./doc/CSG65CE02.png)
## Description
The lib65ce02 library aims to become a cycle exact emulation of the CSG 65CE02 cpu. It is written in c and available on [GitHub](https://github.com/elmerucr/lib65ce02). The application programming interface is inspired by [Musashi Motorola 680x0 emulator](https://github.com/kstenerud/Musashi) by Karl Stenerud. Please realise that quite a few instructions needed guessing for their exact internal working since this cpu was never widely distributed and examined as for instance the MOS 6502.

Please have a look at the [manual](./doc/manual.md) (= a work in progress!) for the inner working of the cpu and its instructions.
## Motivation
Many emulation libraries for older cpu's already exist. This project serves my own learning experience and forms a crucial component of the [E64](https://github.com/elmerucr/E64) virtual computer system.
## Core functions
### Running cycles on the cpu
Typically, this library will not take control of the main thread, instead it will run a designated amount of cycles using the ````int csg65ce02_run(csg65ce02 *thisCPU, unsigned int no_cycles);```` function that returns the number of processed cycles. Variable ````exit_code_run_function```` inside the ````csg65ce02```` structure stores ````0```` on normal execution and ````1```` on a breakpoint encounter. It is possible to run only one cpu instruction by calling this function with 0 cycles as an argument.
### Disassembling instructions
Beside the emulation core, the library also contains a disassembler. The function ````uint8_t csg65ce02_dasm(uint16_t address, char *buffer, int length)```` uses the memory read function from the emulation core. That way, the disassembler reads memory the same way as the cpu.
## Compiling
It doesn't compile into a shared library or something similar. If you want the cpu emulation core in your own project, just copy and add all ````csg65ce02*.*```` files into your source tree. To compile the supplied ````emulate_65ce02```` application: make a ````build```` folder in the ````lib65ce02```` project tree, enter it and run the following commands from the shell:
````
cmake ..
make
./emulate_65ce02
````
The source code in ````main.c```` is quite self-explanatory.
## MIT License
Copyright (c) 2018 elmerucr

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.