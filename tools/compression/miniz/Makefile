CC=gcc

all: deflate dump_header deflate validate

deflate: miniz.c miniz.h main.c ../../../modules/misc/hss_crc32.c
	$(CC) -o deflate miniz.c main.c -I/home/unikie/Desktop/polarfire/hss/work/clean/hart-software-services/build/boards/mpfs-icicle-kit-es/fpga_design_config/ -I /home/unikie/Desktop/polarfire/hss/work/clean/hart-software-services/thirdparty/opensbi/include -I ../../../include ../../../modules/misc/hss_crc32.c -I../../../ -I../../../thirdparty/opensbi/include/-I -D__riscv_xlen=64

dump_header: dump_header.c
	$(CC) -o dump_header dump_header.c -I /home/unikie/Desktop/polarfire/hss/work/clean/hart-software-services/thirdparty/opensbi/include

validate: validate.c
	$(CC) -g -o validate validate.c miniz.c

clean:
	-$(RM) deflate bootImageBlobg.bin.deflate dump_header validate out.bin
