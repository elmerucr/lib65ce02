# lib65ce02
## Description
This code, written in c, aims to be a cycle exact emulation of the 65ce02 cpu. Its programming interface is inspired by [Musashi Motorola 680x0 emulator](https://github.com/kstenerud/Musashi) by Karl Stenerud. Typically this library will not take control of the applications' main thread, but in stead run a designated amount of cycles and return the consumed cycles.
## Why?
I am aware many emulation libraries for older cpu's already exist. I do it just for fun and development of my own virtual 65ce02 based computer system (another project in the works): C256 or 'Hitting the Virtual Metal'.
## Compiling
It doesn't compile into a shared library or similar. If you want the cpu emulation core into your own project, just copy all ````csg65ce02*.*```` files into your source tree. To compile the ````emulate_65ce02```` application: make a ````build```` folder in the source tree and enter it and run the following commands:
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