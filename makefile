craft: craft.s craft_test.c
	riscv64-unknown-elf-clang -c craft.s
	riscv64-unknown-elf-clang -c craft_test.c
	rm -f ./craft_test.out
	riscv64-unknown-elf-clang craft.o craft_test.o -o craft_test.out -w
	spike /home/shakti/riscv/riscv64-unknown-elf/bin/pk ./craft_test.out

safemalloc: craft.s safemalloc.c safemalloc_test.c
	riscv64-unknown-elf-clang -c craft.s
	riscv64-unknown-elf-clang -c safemalloc.c -w
	riscv64-unknown-elf-clang -c safemalloc.c -w -S -o safemalloc.asm
	riscv64-unknown-elf-clang -c safemalloc_test.c
	rm -f ./safemalloc_test.out
	riscv64-unknown-elf-clang safemalloc_test.o safemalloc.o craft.o -o safemalloc_test.out
	spike /home/shakti/riscv/riscv64-unknown-elf/bin/pk ./safemalloc_test.out

clean:
	rm -f *.o *.out *.asm
