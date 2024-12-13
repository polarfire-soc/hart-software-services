#!/usr/bin/env python3

"""
RETRIEVE MPFS PART Number from XML

This script takes fpga design folder as input and retrieves the part number
"""


#
#
# MPFS PART NUMBER RETRIEVER
#
# Copyright 2021-2022 Microchip Corporation.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the 'Software'), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#
#
#

import argparse
import os
import os.path
import xml.etree.ElementTree as ET
import sys


def extract_mpfs_part_no(path):
    tree = ET.parse(path)
    root = tree.getroot()
    # Find the part number
    part_no_element = root.find(".//mpfs_part_no")
    if part_no_element is not None:
        mpfs_part_no = part_no_element.text
        print(part_no_element.text)
    else:
        print("MPFS_PART_NO field not found in XML")

        return mpfs_part_no


def main():
    parser = argparse.ArgumentParser(description="extract\
            mpfs_part_no field from design xml")
    parser.add_argument('--verbose', '-v', action='count', default=0)
    path_help = "Path to soc fpga design xml file"
    parser.add_argument("path", type=str, help=path_help)
    global args
    args = parser.parse_args()

    directory = sys.argv[1]
    xml_files = [os.path.join(directory, f)
                 for f in os.listdir(directory) if f.endswith('.xml')]
    if not xml_files:
        print("No XML files found in the directory")
        sys.exit(1)
    for xml_file in xml_files:
        extract_mpfs_part_no(xml_file)


if __name__ == '__main__':
    main()
