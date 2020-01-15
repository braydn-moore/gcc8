all: build/asm build/bin

build/asm:
	mkdir -p build
	gcc asm.c utils/binary_tree.c utils/string_utils.c -o build/asm

build/bin:
	mkdir -p build
	gcc utils/binary_tree.c utils/string_utils.c bin.c -o build/bin