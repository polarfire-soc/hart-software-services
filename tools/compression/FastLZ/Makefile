all: lzc bootImageBlob.bin.lz77 dump_header lzc bootImageBlob.bin.lz77.o

lzc: fastlz.c fastlz.h main.c ../../../hart-software-services/misc/hss_crc32.c
	$(CC) -o lzc fastlz.c main.c -I ../../../hart-software-services/include ../../../hart-software-services/misc/hss_crc32.c -I../../../hart-software-services

bootImageBlob.bin.lz77: lzc ../../bin2chunks/bootImageBlob.bin
	./lzc ../../bin2chunks/bootImageBlob.bin bootImageBlob.bin.lz77

# generate a binary for linking
bootImageBlob.bin.lz77.o: bootImageBlob.bin.lz77
	riscv64-unknown-elf-ld -r -b binary bootImageBlob.bin.lz77 -o bootImageBlob.bin.lz77.o

dump_header: dump_header.c
	$(CC) -o dump_header dump_header.c

clean:
	-$(RM) lzc bootImageBlobg.bin.lz77 dump_header
