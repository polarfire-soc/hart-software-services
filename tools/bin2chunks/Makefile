CC=gcc
CROSS_COMPILER=riscv64-unknown-elf-
CFLAGS=-Wall --std=gnu11 -O2 -g -DNR_CPUs=4

CWD=$(shell pwd)

all: bin2chunks dump_header

-include ../../.config

bin2chunks: bin2chunks.c ../../misc/hss_crc32.c
	$(CC) $(CFLAGS) -o bin2chunks bin2chunks.c ../../misc/hss_crc32.c -I../../include -I../../ssmb/ipi/ -I../../ -I../../thirdparty/opensbi/include -D__riscv_xlen=64

dump_header: dump_header.c 
	$(CC) $(CFLAGS) -o dump_header dump_header.c -I../../include -I../../ssmb/ipi/ -I../..  -I../../thirdparty/opensbi/include -D__riscv_xlen=64
clean:
	-$(RM) bin2chunks dump_header
