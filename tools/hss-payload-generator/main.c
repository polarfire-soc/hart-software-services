/******************************************************************************************
 *
 * MPFS HSS Embedded Software - tools/hss-payload-generator
 *
 * Copyright 2020-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#include "hss_types.h"
#include "blob_handler.h"
#include "elf_parser.h"
#include "yaml_parser.h"
#include "generate_payload.h"
#include "dump_payload.h"
#include "debug_printf.h"

#define GEN_VERSION_STRING "0.99.26"

struct HSS_BootImage bootImage;
struct HSS_BootChunkDesc *pChunkDescs;

bool wide_output = false;

/*
 * Local function prototypes
 */
static void intro_banner(void);
static void print_usage(char **argv) __attribute__((nonnull));

static void intro_banner(void)
{
	puts("Hart Software Service formatted boot image generator v" GEN_VERSION_STRING);
	puts("Copyright (c) 2021 Microchip FPGA Embedded Systems Solutions.\n");
}

static void print_usage(char **argv)
{
	printf("Usage: %s [-v] [-w] [-h] [[-c <configfile.yaml> <output.bin>] [-n <override-set-name>]] [-d <output.bin>]\n\n", argv[0]);
	printf("\nMultiple '-v' arguments increases verbosity of output.\n\n");

	printf(" -c		Run generator and specify path to configuration YAML\n");
	printf(" -d		Run analyzer and specifiy path to payload binary\n");
	printf(" -h		print this help\n");
	printf(" -v		Increase verbosity of output\n");
	printf(" -w		Extra-wide output (used with verbosity)\n\n");

	exit(1);
}

int main(int argc, char **argv)
{
	intro_banner();

	// these must come before we run the YAML parser to ensure all correctly setup
	elf_parser_init();
	blob_handler_init();
	generate_init();

	//
	// parse command line arguments
	//
	int opt;
	char *config_filename = NULL;
	char *dump_payload_filename = NULL;
	while ((opt = getopt(argc, argv, (const char *)"c:d:hvw")) != -1) {
		switch (opt) {
		case 'c':
			config_filename = optarg;
			break;

		case 'd':
			dump_payload_filename = optarg;
			break;

		case 'h':
			print_usage(argv);
			exit(EXIT_SUCCESS);
			break;

		case 'v':
			debug_increaseLogLevel();
			break;

		case 'w':
			wide_output = true;
			break;

		default:
			print_usage(argv);
			break;
		}
	}

	if ((config_filename) && (dump_payload_filename)) {
		fprintf(stderr, "%s: Only one of -c or -d allowed\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//
	// now that we know what we are about, let's go to work...
	//
	if ((config_filename) && (argc > optind)) {
		yaml_parser(config_filename);
		generate_payload(argv[optind]);
	} else if (dump_payload_filename) {
		dump_payload(dump_payload_filename);
	} else {
		print_usage(argv);
		exit(EXIT_FAILURE);
	}

	return 0;
}
