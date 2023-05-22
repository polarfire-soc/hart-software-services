#!/usr/bin/env python3

"""
MPFS HSS Generate Profiling Report tool

This script takes the CSV profiling output from the HSS console
which contains function addresses, and tick counts, and converts
the function addresses into function names.

"""

#
#
# MPFS HSS Generate Profiling Report tool
#
# Copyright 2023 Microchip Corporation.
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
#

import argparse
import csv
import sys

try:
    from elftools.elf.elffile import ELFFile
except ImportError:
    print("Unable to import the pyelftools package, " +
          "please check your Python installation", file=sys.stderr)
    print("Ensure that pyelftools is installed", file=sys.stderr)
    sys.exit(1)


def build_symbol_cache(elf_filepath: str):
    '''takes a path to an ELF file, and builds a symbol cache'''
    if args.verbose:
        print('Building ELF symbol cache from ' + args.elffile,
              file=sys.stderr)

    with open(elf_filepath, 'rb') as f:
        elf = ELFFile(f)

        if args.verbose:
            print('- reading symbols', file=sys.stderr)
        symtab = elf.get_section_by_name('.symtab')

        global symbol_cache
        symbol_cache = {}
        if args.verbose:
            print('- building cache dictionary', file=sys.stderr)
        for symbol in symtab.iter_symbols():
            symbol_cache[symbol['st_value']] = symbol.name


def process_csv(csvfile):
    '''process input CSV'''
    if args.verbose:
        print('Loading sorted CSV', file=sys.stderr)

    with open(csvfile, newline='') as csvfile:
        filtered = (line for line in csvfile if not line.startswith("#"))
        reader = csv.reader(filtered)
        rows = list(reader)
        rows.sort(key=lambda row: int(row[1]), reverse=True)

        if args.verbose:
            print('Searching for function names in symbol cache',
                  file=sys.stderr)

        for row in rows:
            process_row(row)


def process_row(row):
    '''takes a tuple (function address, tick count) and replaces the
    function address with its function name'''
    funcaddr = int(row[0], 16)
    tickcount = int(row[1])

    if funcaddr in symbol_cache:
        print(symbol_cache[funcaddr], tickcount, sep=', ')
    else:
        print(funcaddr, tickcount, sep=', ')
        pass


def main():
    '''main function'''
    parser = argparse.ArgumentParser(description='Generate Profile Report')
    parser.add_argument('--verbose', '-v', action='count', default=0)
    parser.add_argument('elffile')
    parser.add_argument('csvfile')

    global args
    args = parser.parse_args()

    build_symbol_cache(args.elffile)
    process_csv(args.csvfile)


#
#
#

if __name__ == "__main__":
    main()
