cmake_minimum_required (VERSION 2.6)
project(lib65ce02)
set(CMAKE_BUILD_TYPE Debug)
add_executable(emulate_65ce02 ./src/csg65ce02.c ./src/main.c ./src/csg65ce02_dasm.c ./src/csg65ce02_mmu.c)