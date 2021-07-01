CC=gcc

all: deflate dump_header deflate validate

deflate: miniz.c miniz.h main.c ../../../modules/misc/hss_crc32.c
	$(CC) -o deflate miniz.c main.c -I ../../../include ../../../modules/misc/hss_crc32.c -I../../../ -I../../../thirdparty/opensbi/include/ -D__riscv_xlen=64

dump_header: dump_header.c
	$(CC) -o dump_header dump_header.c

validate: validate.c
	$(CC) -g -o validate validate.c miniz.c

clean:
	-$(RM) deflate bootImageBlobg.bin.deflate dump_header validate out.bin
