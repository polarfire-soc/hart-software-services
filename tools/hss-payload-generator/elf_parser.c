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
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <elf.h>
#include <gelf.h>
#include <libelf.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "elf_parser.h"
#include "elf_strings.h"
#include "hss_types.h"
#include "debug_printf.h"
#include "generate_payload.h"
#include "crc32.h"


extern bool wide_output;
extern struct HSS_BootImage bootImage;

static size_t numChunks = 0u;
static size_t numZIChunks = 0u;

/////////////////////////////////////////////////////////////////////////////
//
// Local Function Prototypes
//

static bool is_section_in_segment(GElf_Shdr *pShdr, GElf_Phdr *pPhdr) __attribute__((nonnull));
static void process_sections_in_segment(Elf *pElf, GElf_Phdr *pPhdr, size_t owner) __attribute__((nonnull));
static void parse_elf_program_headers(Elf *pElf, size_t owner) __attribute__((nonnull(1)));

/////////////////////////////////////////////////////////////////////////////
//
// Local Functions
//

static bool is_section_in_segment(GElf_Shdr *pShdr, GElf_Phdr *pPhdr)
{
	bool result;

	assert(pShdr);
	assert(pPhdr);

	uint64_t addr = pPhdr->p_vaddr;
	uint64_t size = pPhdr->p_memsz;

	result = ((pShdr->sh_addr >= addr) && ((pShdr->sh_addr + pShdr->sh_size) <= (addr + size)));

	return result;
}

static void process_sections_in_segment(Elf *pElf, GElf_Phdr *pPhdr, size_t owner)
{
	Elf_Scn *pScn = NULL;
	GElf_Shdr shdr;
	GElf_Shdr *pShdr = &shdr;
	char *name;

	assert(pElf);
	assert(pPhdr);

	while ((pScn = elf_nextscn(pElf, pScn)) != NULL) {
		if (gelf_getshdr(pScn, &shdr) != &shdr) {
			fprintf(stderr, "gelf_getshdr() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}

		size_t shstrndx;

		if (elf_getshdrstrndx(pElf, &shstrndx) != 0) {
			fprintf(stderr, "elf_getshdrstrndx() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}

		if (is_section_in_segment(&shdr, pPhdr)) {
			name = elf_strptr(pElf, shstrndx, shdr.sh_name);
			if (name != NULL) {
				debug_printf(5, "SECTION:: Name: %s, sh_type: %lx, sh_flags: %lx,"
					" sh_addr: %lx, sh_offset: %lx, sh_size: %lx, sh_link: %lx,"
					" sh_info: %lx, sh_addralign: %lx, sh_entsize: %lx\n",
					 name, shdr.sh_type, shdr.sh_flags, shdr.sh_addr, shdr.sh_offset,
					 shdr.sh_size, shdr.sh_link, shdr.sh_info, shdr.sh_addralign,
					 shdr.sh_entsize);

				debug_printf(2, "%s ", name);
			} else {
				fprintf(stderr, "elf_strptr() failed: %s\n", elf_errmsg(-1));
				exit(EXIT_FAILURE);
			}

			if (pPhdr->p_type == PT_LOAD) {
				if (shdr.sh_type != SHT_NOBITS) {
					Elf_Data *pData = NULL;
					pData = elf_getdata(pScn, pData);
					if (pData) {
						void *pBuffer = malloc(shdr.sh_size);
						assert(pBuffer);
						assert(shdr.sh_size == pData->d_size);

						memcpy(pBuffer, pData->d_buf, pData->d_size);

						struct HSS_BootChunkDesc chunk = {
    							.owner = owner,
    							.loadAddr = 0u,
    							.execAddr = (uintptr_t)shdr.sh_addr,
    							.size = pData->d_size,
    							.crc32 = CRC32_calculate(pBuffer, pData->d_size)
						};

						numChunks = generate_add_chunk(chunk, pBuffer);
					}
				} else {
					struct HSS_BootZIChunkDesc ziChunk = {
    						.owner = owner,
    						.execAddr = (void *)shdr.sh_addr,
    						.size = shdr.sh_size
					};

					numZIChunks = generate_add_ziChunk(ziChunk);
				}
			}
		} else { // if (verbose > 4) {
			name = elf_strptr(pElf, shstrndx, shdr.sh_name);
			if (name != NULL) {
				debug_printf(4, "Skipping %s\n", name);
				debug_printf(4, "\t%.16" PRIx64 "-%.16" PRIx64, pPhdr->p_vaddr, (pPhdr->p_vaddr + pPhdr->p_memsz));
				debug_printf(4, " vs ");
				debug_printf(4, "%.16" PRIx64 "-%.16" PRIx64 "\n", pShdr->sh_addr, (pShdr->sh_addr + pShdr->sh_size));
			} else {
				fprintf(stderr, "elf_strptr() failed: %s\n", elf_errmsg(-1));
				exit(EXIT_FAILURE);
			}
		}
	}
}

static void parse_elf_program_headers(Elf *pElf, size_t owner)
{
	assert(pElf);

	//
	//  some sanity testing
	//
	if (gelf_getclass(pElf) == ELFCLASSNONE) {
		fprintf(stderr, "gelf_getclass() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	}

	GElf_Ehdr ehdr;
	if (!gelf_getehdr(pElf, &ehdr)) {
		fprintf(stderr, "gelf_getehdr() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	}

	if (ehdr.e_machine != EM_RISCV) {
		fprintf(stderr, "machine type check failed: Machine type is >>%s<<.\n"
			"Only RISC-V payloads are supported.\n",
			ElfMachineToString(ehdr.e_machine));
		exit(EXIT_FAILURE);
	}

	if (ehdr.e_type != ET_EXEC) {
		fprintf(stderr, "ELF type check failed: ELF type is >>%s<<.\n"
			"Only executable payloads are supported.\n",
			ElfTypeToString(ehdr.e_type));
		exit(EXIT_FAILURE);
	}


	//
	// output ELF details
	debug_printf(2, "Elf file type is %s (%s)\n", ElfTypeCodeToString(ehdr.e_type), ElfTypeToString(ehdr.e_type));
	debug_printf(2, "Entry point 0x%" PRIx64 "\n", ehdr.e_entry);
	debug_printf(2, "There are %d program header%s, starting at offset %" PRIu64 "\n\n", ehdr.e_phnum, ehdr.e_phnum ? "s":"", ehdr.e_phoff);

	debug_printf(2, "Program Headers:\n");
	debug_printf(2, "       Type           Offset             VirtAddr           PhysAddr");

	if (wide_output) {
	 	debug_printf(2, "    ");
	} else {
		debug_printf(2, "\n");
	}

	debug_printf(2,"       FileSiz            MemSiz              Flags  Align\n");

	//
	// parse each program header
	for (int i = 0u; i < (int)ehdr.e_phnum; i++) {
		GElf_Phdr phdr;

		if (gelf_getphdr(pElf, i, &phdr) != &phdr) {
			fprintf(stderr, "elf_getphdr() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}
		debug_printf(2, "   %02" PRIu64"  %-14s 0x%.16" PRIx64" 0x%.16" PRIx64" 0x%.16" PRIx64,
			i, ElfProgramTypeToString(phdr.p_type), phdr.p_offset, phdr.p_vaddr,
			phdr.p_paddr);

		if (wide_output) {
			;
		} else {
			debug_printf(2, "\n                     ");
		}

		debug_printf(2, " 0x%.16" PRIx64" 0x%.16" PRIx64"  %c%c%c    0x%" PRIx64 "\n",
			phdr.p_filesz, phdr.p_memsz,
			(phdr.p_flags & PF_R) ? 'R':' ', (phdr.p_flags & PF_W) ? 'W':' ',
			(phdr.p_flags & PF_X) ? 'E':' ', phdr.p_align);
	}

	debug_printf(2, "\n"
		" Section to Segment mapping:\n"
		"  Segment Sections...\n");

	for (int i = 0u; i < (int)ehdr.e_phnum; i++) {
		GElf_Phdr phdr;
		if (gelf_getphdr(pElf, i, &phdr) != &phdr) {
			fprintf(stderr, "elf_getphdr() failed: %s\n", elf_errmsg(-1));
			exit(EXIT_FAILURE);
		}

		debug_printf(2, "   %02" PRIu64 "     ", i);
		process_sections_in_segment(pElf, &phdr, owner);
		debug_printf(2, "\n");
	}

	debug_printf(2, "\n");
}


/////////////////////////////////////////////////////////////////////////////
//
// Module Inteface
//

void elf_parser_init(void)
{
	if (elf_version(EV_CURRENT) == EV_NONE) {
		perror("elf_version()");
		exit(EXIT_FAILURE);
	}
}

bool elf_parser(char const * const filename, size_t owner)
{
	bool result = true;

	assert(filename);

	debug_printf(1, "\nProcessing ELF >>%s<<\n", filename);
	if (!bootImage.hart[owner-1].firstChunk) {
		bootImage.hart[owner-1].firstChunk= numChunks;
	}

	int fd = open(filename, O_RDONLY, 0);
	if (fd < 0) {
		char buffer[300];
		snprintf(buffer, 299, "open(\"%s\")", filename);
		buffer[299] = 0;
		perror(buffer);
		exit(EXIT_FAILURE);
	}

	Elf *pElf = elf_begin(fd, ELF_C_READ, NULL);
	if (!pElf) {
		fprintf(stderr, "elf_begin() failed: %s\n", elf_errmsg(-1));
		exit(EXIT_FAILURE);
	}

	if (elf_kind(pElf) != ELF_K_ELF) {
		fprintf(stderr, ">>%s<< is not an ELF object\n", filename);
		result = false;
	} else {
		parse_elf_program_headers(pElf, owner);

		bootImage.hart[owner-1].lastChunk = numChunks - 1u;
		bootImage.hart[owner-1].numChunks = bootImage.hart[owner-1].lastChunk - bootImage.hart[owner-1].firstChunk + 1u;

		elf_end(pElf);
	}

	close(fd);

	return result;
}
