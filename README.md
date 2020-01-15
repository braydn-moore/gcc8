## gcc8
A simple assembly compiler for the chip8 specification. It is in its early stages and may be buggy, you have been warned

## Getting Started
To compile the compiler on a Unix based system using gcc you can use the makefile provided by running 

    make

in the main directory

A chip8 assembly file can be assembled using the following steps:

    ./asm <input file> <output file>
    ./bin <output file from asm> <output binary>

