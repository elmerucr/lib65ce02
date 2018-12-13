# lib65ce02
## Description
This code, written in c, aims to become a cycle exact emulation of the 65ce02 cpu. Its programming interface is inspired by [Musashi Motorola 680x0 emulator](https://github.com/kstenerud/Musashi) by Karl Stenerud.
## Motivation
Many emulation libraries for older cpu's already exist. This is for my own learning experience and for development of a virtual 65ce02 based computer system.
## Core functions
Typically, this library will not take control of the main thread, in stead it will run a designated amount of cycles using the ````csg65ce02_execute(csg65ce02 *thisCPU, unsigned int noCycles)```` function that returns the amount of consumed cycles. It is possible to run only one cpu instruction by calling the same function with 0 cycles as an argument.

Besides the emulation core, the library also contains a disassembler. The disassemble function ````csg65ce02_dasm(uint16_t address, char *buffer, int length)```` depends on the memory read function from the emulation core. That way, memory will be viewed the same way as the cpu does.
## Compiling
It doesn't compile into a shared library or something similar. If you want the cpu emulation core in your own project, just copy and add all ````csg65ce02*.*```` files into your source tree. To compile the supplied ````emulate_65ce02```` application: make a ````build```` folder in the ````lib65ce02```` project tree, enter it and run the following commands from the shell:
````
cmake ..
make
./emulate_65ce02
````
The source code in ````main.c```` will be quite self-explanatory.
## MIT License
Copyright (c) 2018 elmerucr

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.