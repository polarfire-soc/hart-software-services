#
# MPFS HSS Embedded Software
#
# Copyright 2019-2021 Microchip Corporation.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#
# Defines build targets
#

all: config.h $(TARGET)

##############################################################################
#
# KConfig support
#

defconfig:
	@$(ECHO) " CP       def_config";
	cp boards/${BOARD}/def_config .config
	@$(ECHO) " GENCONFIG"
	$(GENCONFIG)

menuconfig:
	@$(ECHO) " MENUCONFIG"
	$(MENUCONFIG)

config:
	@$(ECHO) " MENUCONFIG"
	$(MENUCONFIG)

# we can't run curses in SoftConsole, so just copy a pre-canned
# config, and the user can tweak if necessary
.config:
ifeq ($(HOST_WINDOWS), true)
	$(info Using boards/${BOARD}/def_config - edit as necessary.)
	@$(ECHO) " CP       def_config";
	cp boards/${BOARD}/def_config .config
	@$(ECHO) " GENCONFIG"
	$(GENCONFIG)
else
	@$(ECHO) " MENUCONFIG"
	$(MENUCONFIG)
endif

config.h: .config
	@$(ECHO) " GENCONFIG"
	$(GENCONFIG)

genconfig: config.h

##############################################################################
#
# Build Targets
#

#coverage: CORE_CFLAGS +=-fsanitize=address
coverage: CORE_CFLAGS +=-fprofile-arcs -ftest-coverage
coverage: OPT-y:=-O0 # lcov seg-faults without at least -O1 for some reason..
coverage: clean $(TARGET)

profile: CFLAGS +=-pg -DLOOP_COUNT=10000
profile: clean $(TARGET)
	./$(TARGET) >/dev/null 2>/dev/null
	$(GPROF) $(TARGET)  |less

.PHONY: clean cppcheck splint cscope cscope.files

clean: envm-wrapper_clean
	$(RM) $(TARGET) $(TEST_TARGET) cppcheck.log splint.log valgrind.log \
		$(OBJS:.o=.gcda) $(OBJS) $(OBJS:.o=.gcno) $(OBJS:.o=.c.gcov) $(OBJS:.o=.su) \
                $(EXTRA_OBJS) $(EXTRA_OBJS:.o=.c.gcov) $(EXTRA_OBJS:.o=.su) \
		$(TEST_OBJS) $(TEST_OBJS.o=.gcno) $(TEST_OBJS.o=.c.gcov) $(TEST_OBJS:.o=.su) \
		$(DEPENDENCIES) \
		gmon.out cscope.out \
		error.log flawfinder.log sparse.log $(BINDIR)/output-*.map config.h \
                $(TARGET:.elf=.hex) $(TARGET:.elf=.lss) $(TARGET:.elf=.sym) $(TARGET:.elf=.bin)
	$(RM) -r docs/DoxygenOutput
	$(RM) -r lcovOutput coverage.info
	$(RM) *.gcov
	$(RM) *.lss *.hex *.sym
	$(RM) $(BINDIR)/hss* $(BINDIR)/output.map

distclean:
	$(RM) $(OBJS) $(TARGET) cppcheck.log splint.log valgrind.log \
		 cscope.out cscope.files
	$(RM) -r docs/DoxygenOutput

cppcheck: $(SRCS-y)
	cppcheck --suppress=missingIncludeSystem -v --enable=all --inconclusive --std=posix \
                 -ibaremetal/polarfire-soc-bare-metal-library/examples -ithirdparty \
		 $(INCLUDES) -UDECLARE_CAUSE -UDECLARE_CSR -UDECLARE_INSN --force  . \
		 >cppcheck.log 2>&1

sparse: REAL_CC:=$(CC)
sparse: MCMODEL:=
sparse: CC=cgcc
sparse: clean $(TARGET)

smatch: $(SRCS-y) $(EXTRA_SRCS-y) $(TEST_SRCS)
	smatch $(INCLUDES) $(SRCS-y) $(EXTRA_SRCS-y) $(TEST_SRCS)

splint: $(SRCS-y)
	find . -name \*.c -exec splint +posixlib -DPRIu64=\"llu\" -Dtarget_ulong="unsigned long" \
		 -Dasm="(void)" -weak $(INCLUDES) \
		 -D__riscv_xlen=64 -D__SIZEOF_POINTER__=8 -D__SIZEOF_INT__=4 -D__SIZEOF_SHORT__=2 \
		 \{\} \;  >splint.log 2>&1

cscope: cscope.files cscope.out
cscope.files:
	find . -name \*.[chS] > cscope.files

cscope.out: cscope.files
	cscope -b

lcov: coverage
	./$(TARGET) | tee foo
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/include/*' 'test/*' --output-file coverage2.info
	mv coverage2.info coverage.info
	genhtml coverage.info --output-directory lcovOutput

showloc:
	@$(ECHO) -n "There are " && \
		 $(ECHO) -n `find . -name \*.[chs] -exec wc -l \{\}  \; | awk '{count += $$1;} END { print count;}'` && \
		 $(ECHO) " lines of source code"

valgrind: CC=cc
valgrind: MCMODEL:=
valgrind: $(TARGET)
	valgrind -v --tool=memcheck --show-reachable=yes --leak-check=yes --track-origins=yes \
		./$(TARGET) 2> valgrind.log

doxygen:
	doxygen docs/doxygen.cfg
	$(MAKE) -C docs/DoxygenOutput/latex all

flawfinder:
	flawfinder `find . -type d | grep -v thirdparty | grep -v .git | grep -v baremetal | tail -n +2` baremetal/drivers baremetal/polarfire-soc-bare-metal-library/src >flawfinder.log

showsize: $(BINDIR)/hss-l2lim.elf
	@echo
	@$(SIZE) $(BINDIR)/hss-l2lim.elf

showfullsize: $(BINDIR)/hss-l2lim.elf
	@echo
	@$(NM) --print-size --size-sort --radix=x $(BINDIR)/hss-l2lim.elf
	@$(READELF) -e $(BINDIR)/hss-l2lim.elf

ifdef CONFIG_CC_DUMP_STACKSIZE
showstack: hss-envm.elf
	@echo
	@cat `find . -name \*.su` | awk '{print $$2 " " $$0}' | sort -rn | cut -d " " -f 2- | head -20

showmaxstack:  hss-envm.elf
	@echo
	@cat `find . -name \*.su` | awk '$$2>a {a=$$2; b=$$0} END {print b}'
endif
	

.PHONY: config clean docs distclean doxygen showsize \
        cppcheck splint sparse smatch cscope lcov valgrind flawfinder

help:
	@$(ECHO) "Valid targets include:"
	@$(ECHO) "	$ make" $(TARGET) "		# Build binary"
	@$(ECHO) "	$ make clean		# Delete all built files"
	@$(ECHO) "	$ make distclean		# Fully clean all built files and test outputs"
	
	@$(ECHO) ""
	@$(ECHO) "	$ make doxygen		# Generate doxygen documentation"
	@$(ECHO) "	$ make cppcheck		# Run cppcheck on source code"
	@$(ECHO) "	$ make splint 		# Run splint on source code"
	@$(ECHO) "	$ make sparse 		# Run sparse on source code"
	@$(ECHO) "	$ make smatch 		# Run smatch on source code"
	@$(ECHO) "	$ make cscope 		# Build cscope database"
	@$(ECHO) "	$ make lcov 		# Rebuild instrumented binaries and run lcov"
	@$(ECHO) "	$ make valgrind		# Run valgrind on binary"
	@$(ECHO) "	$ make flawfinder		# Run flawfinder on this directory"
	
	@$(ECHO) ""
	@$(ECHO) "	$ make showloc		# Print number of lines of code"
	@$(ECHO) "	$ make showsize		# Print information about binary size"
	@$(ECHO) "	$ make showfullsize	# Dump detailed information about object sizes"
ifdef CONFIG_CC_DUMP_STACKSIZE
	@$(ECHO) "	$ make showstack	# Print top 10 functions by stack usage"
	@$(ECHO) "	$ make showmaxstack	# Print top function by stack usage"
endif
	@$(ECHO) "	$ make dep		# Remake dependencies"

