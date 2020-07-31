#!/usr/bin/env python3
#==============================================================================
# This script takes an xml file which describes hardware options and produces
# header files in the directory hardware which are used by the embedded
# software.
# Currently there is an option to produce the xml file from .csv files, and
# this option will be available during the development phase.
#==============================================================================
import datetime
import os
import os.path
import xml.etree.ElementTree as ET
import sys


# -----------------------------------------------------------------------------
# mpfs_configuration_generator.py version
# 0.3.4 fixed comment formatting bug in hw_memory.h generation
# 0.3.3 updated copyright format
# 0.3.2 removed leading zeros from decimal values ( clock rates)
# -----------------------------------------------------------------------------
def get_script_ver():
    '''
    This changes anytime anytime the mpfs_configuration_generator.py script
    changes. This does not necessarily mean the xml format has been updated in
    get_xml_ver()
    :return: script version
    '''
    return "0.4.1"


# -----------------------------------------------------------------------------
# xml description version
#
# -----------------------------------------------------------------------------
def get_xml_ver():
    '''
    version 0.2.9  Added regs SUBBLK_CLOCK_CR, SOFT_RESET_CR, GPIO_CR, USB_CR,
                   MESH_SEED_CR
    version 0.2.7  Added reset vector addresses for the five harts
    This changes anytime the description changes. It is used when generating the
    reference xml script
    :return: xml version
    '''
    return "0.3.1"


# -----------------------------------------------------------------------------
# xml file to parse
# Also an xml files listing tags used for reference
# -----------------------------------------------------------------------------
reference_xml_file = \
    ('hardware_des_xml,src_example,mpfs_hw_ref_defaults.xml,default',
     'hardware_des_xml,src_example,mpfs_hw_ref_ddr3_100Mhz_ext_clk.xml,ddr3_100Mhz_ref')

xml_tag_file = 'hardware_des_xml,src_example,mpfs_hw_tag_reference.xml'


# -----------------------------------------------------------------------------
# xml tags, the structure here should follow the readme.md description
# contained in the root folder for tags
# Please note: The tag in the first column ( mss_xxx) is the same as the
# directory name (/soc_config/mss_xxx)
# the fourth item lets program know how to format info in header file
# the six item lets program know how to format value, decimal or hex
# -----------------------------------------------------------------------------
xml_tags = ('mss_memory_map,map,mem_elements,fm_define,none,hex',
            'mss_memory_map,apb_split,registers,fm_struct,none,hex',
            'mss_memory_map,cache,registers,fm_struct,none,hex',
            'mss_memory_map,pmp_h0,registers,fm_struct,HART0_,hex64',
            'mss_memory_map,pmp_h1,registers,fm_struct,HART1_,hex64',
            'mss_memory_map,pmp_h2,registers,fm_struct,HART2_,hex64',
            'mss_memory_map,pmp_h3,registers,fm_struct,HART3_,hex64',
            'mss_memory_map,pmp_h4,registers,fm_struct,HART4_,hex64',
            'mss_memory_map,mpu_fic0,registers,fm_struct,FIC0_,hex64',
            'mss_memory_map,mpu_fic1,registers,fm_struct,FIC1_,hex64',
            'mss_memory_map,mpu_fic2,registers,fm_struct,FIC2_,hex64',
            'mss_memory_map,mpu_crypto,registers,fm_struct,CRYPTO_,hex64',
            'mss_memory_map,mpu_gem0,registers,fm_struct,GEM0_,hex64',
            'mss_memory_map,mpu_gem1,registers,fm_struct,GEM1_,hex64',
            'mss_memory_map,mpu_usb,registers,fm_struct,USB_,hex64',
            'mss_memory_map,mpu_mmc,registers,fm_struct,MMC_,hex64',
            'mss_memory_map,mpu_scb,registers,fm_struct,SCB_,hex64',
            'mss_memory_map,mpu_trace,registers,fm_struct,TRACE_,hex64',
            'mss_io,io_mux,registers,fm_reg,none,hex',
            'mss_io,hsio,registers,fm_reg,none,hex',
            'mss_sgmii,tip,registers,fm_reg,none,hex',
            'mss_ddr,options,registers,fm_reg,none,hex',
            'mss_ddr,io_bank,registers,fm_reg,none,hex',
            'mss_ddr,mode,registers,fm_reg,none,hex',
            'mss_ddr,off_mode,registers,fm_reg,none,hex',
            'mss_ddr,segs,registers,fm_reg,none,hex',
            'mss_ddr,ddrc,registers,fm_reg,none,hex',
            'mss_clocks,clocks,registers,fm_define,none,decimal',
            'mss_clocks,mss_sys,registers,fm_define,MSS_,hex',
            'mss_clocks,mss_pll,registers,fm_define,MSS_,hex',
            'mss_clocks,sgmii_pll,registers,fm_reg,SGMII_,hex',
            'mss_clocks,ddr_pll,registers,fm_reg,DDR_,hex',
            'mss_clocks,mss_cfm,registers,fm_reg,MSS_,hex',
            'mss_clocks,sgmii_cfm,registers,fm_reg,SGMII_,hex',
            'mss_general,mss_peripherals,registers,fm_reg,none,hex',)


# -----------------------------------------------------------------------------
#  Header files to generate
# -----------------------------------------------------------------------------
header_files = ('soc_config,memory_map,hw_memory.h',
                'soc_config,memory_map,hw_apb_split.h',
                'soc_config,memory_map,hw_cache.h',
                'soc_config,memory_map,hw_pmp_hart0.h',
                'soc_config,memory_map,hw_pmp_hart1.h',
                'soc_config,memory_map,hw_pmp_hart2.h',
                'soc_config,memory_map,hw_pmp_hart3.h',
                'soc_config,memory_map,hw_pmp_hart4.h',
                'soc_config,memory_map,hw_mpu_fic0.h',
                'soc_config,memory_map,hw_mpu_fic1.h',
                'soc_config,memory_map,hw_mpu_fic2.h',
                'soc_config,memory_map,hw_mpu_crypto.h',
                'soc_config,memory_map,hw_mpu_gem0.h',
                'soc_config,memory_map,hw_mpu_gem1.h',
                'soc_config,memory_map,hw_mpu_usb.h',
                'soc_config,memory_map,hw_mpu_mmc.h',
                'soc_config,memory_map,hw_mpu_scb.h',
                'soc_config,memory_map,hw_mpu_trace.h',
                'soc_config,io,hw_mssio_mux.h',
                'soc_config,io,hw_hsio_mux.h',
                'soc_config,sgmii,hw_sgmii_tip.h',
                'soc_config,ddr,hw_ddr_options.h',
                'soc_config,ddr,hw_ddr_io_bank.h',
                'soc_config,ddr,hw_ddr_mode.h',
                'soc_config,ddr,hw_ddr_off_mode.h',
                'soc_config,ddr,hw_ddr_segs.h',
                'soc_config,ddr,hw_ddrc.h',
                'soc_config,clocks,hw_mss_clks.h',
                'soc_config,clocks,hw_clk_sysreg.h',
                'soc_config,clocks,hw_clk_mss_pll.h',
                'soc_config,clocks,hw_clk_sgmii_pll.h',
                'soc_config,clocks,hw_clk_ddr_pll.h',
                'soc_config,clocks,hw_clk_mss_cfm.h',
                'soc_config,clocks,hw_clk_sgmii_cfm.h',
                'soc_config,general,hw_gen_peripherals.h')

MAX_LINE_WIDTH = 80


# -----------------------------------------------------------------------------
# Read the xml file into ET
# -----------------------------------------------------------------------------
def read_xml_file(s):
    file_dir = os.path.join(*s)
    tree = ET.parse(file_dir.strip())
    root = tree.getroot()  # type: object
    return root


# -----------------------------------------------------------------------------
#  Routine to make a folder
# -----------------------------------------------------------------------------
def safe_make_folder(i):
    '''Makes a folder (and its parents) if not present'''
    try:
        os.makedirs(i)
    except:
        pass


# -----------------------------------------------------------------------------
# Create the directory structure
# -----------------------------------------------------------------------------
def create_hw_dir_struct(root_folder, TOP):
    '''Creates directory structure off root, subdirectories passed in a tupple'''
    for folder in TOP:
        safe_make_folder(root_folder + '/' + folder)


# -----------------------------------------------------------------------------
#  Generate the copyright notice at the top of the header file
# -----------------------------------------------------------------------------
def WriteCopyright(root, theFile, filename, creator):
    '''
    generate copyright notice based on the following:
    #/*******************************************************************************
    # * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
    # *
    # * SPDX-License-Identifier: MIT
    # *
    # * MPFS HAL Embedded Software
    # *
    # */
    :param root:
    :param theFile:
    :param filename:
    :param creator:
    :return:
    '''
    theFile.write('/**********************************************************'
                  '*********************\n')
    theFile.write(" * Copyright 2019-" + str(datetime.datetime.now().year) + " Microchip FPGA Embedded Systems Solutions.\n")
    theFile.write(' * \n')
    theFile.write(' * SPDX-License-Identifier: MIT\n')
    theFile.write(' * \n')
    theFile.write(" * @file " + filename + "\n")
    theFile.write(" * @author " + creator + "\n")
    theFile.write(' * \n')
    for child in root:
        if child.tag == "design_information":
            for child1 in child:
                if child1.tag == "design_name":
                    theFile.write(' * Libero design name: ' + child1.text.strip() + "\n")
                if child1.tag == "libero_version":
                    theFile.write(' * Generated using Libero version: ' + child1.text.strip() + "\n")
                if child1.tag == "mpfs_part_no":
                    theFile.write(' * MPFS part number used in design: ' + child1.text.strip() + "\n")
                if child1.tag == "creation_date_time":
                    theFile.write(' * Date generated by Libero: ' + child1.text.strip() + "\n")
                if child1.tag == "xml_format_version":
                    theFile.write(' * Format version of XML description: ' + child1.text.strip() + "\n")
    theFile.write(' * PolarFire SoC Configuration Generator version: ' + get_script_ver() + "\n")
    strings = ('',
    'Note 1: This file should not be edited. If you need to modify a parameter,',
    'without going through the Libero flow or editing the associated xml file,',
    'the following method is recommended:',
    '  1. edit the file platform//config//software//mpfs_hal//mss_sw_config.h',
    '  2. define the value you want to override there. (Note: There is a ',
    '     commented example in mss_sw_config.h)',
    'Note 2: The definition in mss_sw_config.h takes precedence, as ',
    'mss_sw_config.h is included prior to the ' + filename + ' in the hal ',
    '(see platform//mpfs_hal//mss_hal.h)',
           )
    for string in strings:
        theFile.write(' * ' + string + "\n")
    theFile.write(' *\n */ \n')


# -----------------------------------------------------------------------------
#  the header start define
# -----------------------------------------------------------------------------
def start_define(theFile, filename):
    filename = filename[:-2]  # remove .h from file name
    theFile.write('\n#ifndef ' + filename.upper() + '_H_')
    theFile.write('\n#define ' + filename.upper() + '_H_\n\n')


# -----------------------------------------------------------------------------
#  start c plus define
# -----------------------------------------------------------------------------
def start_cplus(theFile, filename):
    theFile.write('\n#ifdef __cplusplus\n')
    theFile.write('extern ' + ' \"C\"' + ' {\n')
    theFile.write('#endif\n\n')


# -----------------------------------------------------------------------------
#  end define associated with header start define
# -----------------------------------------------------------------------------
def end_define(theFile, filename):
    filename = filename[:-2]  # remove .h from file name
    theFile.write('\n#endif /*' + ' #ifdef ' + filename.upper() + '_H_ */\n\n')


# -----------------------------------------------------------------------------
#  end c++ define
# -----------------------------------------------------------------------------
def end_cplus(theFile, filename):
    theFile.write('\n#ifdef __cplusplus\n}\n#endif\n\n')


# -----------------------------------------------------------------------------
#  write line, break into chunks
# -----------------------------------------------------------------------------
def write_line(headerFile , reg_description):
    ''' write line, break into chunks '''
    word_list = reg_description.split()  # list of words
    sentence = word_list[0] + ' '
    word_list.pop(0)
    for word in word_list:
        if (len(sentence + word + ' ') > MAX_LINE_WIDTH):
            headerFile.write(sentence + '\n')
            sentence = word + ' '
        else:
            sentence = sentence + word + ' ';
    if len(sentence) > 0:
        headerFile.write(sentence + '\n')


# -----------------------------------------------------------------------------
# Iterate through registers and produce header file output
# -----------------------------------------------------------------------------
def generate_register(headerFile, registers, tags):
    '''
    Parse registers tag for register tags and print to header file
    :param headerFile: header file to print to
    :param registers: registers in a tag
    :param tags: Some tags used to determine print format
    :return:
    '''
    for register in registers:
        # if tag 4 is set, pre-append register name with tag[4] value
        if tags[4] != 'none':
            pre_append = tags[4]
            name = 'LIBERO_SETTING_' + pre_append + register.get('name')
        else:
            name = 'LIBERO_SETTING_' + register.get('name')
        name_of_reg = name
        description = register.get('description')
        name_gap = 15
        if len(name) > 15:
            name_gap = len(name)
        s = '#define' + ' ' + name.ljust(name_gap, ' ')
        name = register.get('name') + "_OFF_MODE"
        name_gap = 15
        if len(name) > 15:
            name_gap = len(name)
        stest1 = '#define' + ' ' + name.ljust(name_gap, ' ')
        field_list = []
        reg_value = 0
        reg_value_default = 0
        for field in register:
            if field.tag == "field":
                gap = 30
                if len(field.get('name')) > gap:
                    gap = len(field.get('name')) + 4
                sfield = '    /* ' + field.get('name').ljust(gap, ' ')
                stemp = '    [' + field.get('offset') + ':' + field.get('width') + ']'
                stemp = stemp.ljust(12, ' ')
                sfield += stemp
                sfield += field.get('Type')
                if (field.get('Type') == 'RW'):
                    sfield += ' value= ' + field.text.strip()
                    temp_val = ((int(field.text.strip(), 16)) << int(field.get('offset')))
                    reg_value += temp_val
                sfield += ' */ \n'
                # add the field to list of fields
                field_list.extend([sfield])
        if tags[5] == 'decimal':
            value = format(reg_value, '01X')
            default_value = format(reg_value_default, '08X')
        elif tags[5] == 'hex64':
            value = '0x' + format(reg_value, '016X')  + 'ULL'
            default_value = '0x' + format(reg_value_default, '08X')
        else :
            value = '0x' + format(reg_value, '08X')  + 'UL'
            default_value = '0x' + format(reg_value_default, '08X')
        name_gap = 4
        if len(s) >= name_gap:
            name_gap = len(s) + 4
        s = s.ljust(name_gap, ' ') + value + '\n'
        reg_description = '/*' + description + ' */ \n'
        headerFile.write('#if !defined ' + '(' + name_of_reg + ')\n')
        # Write out the register description, max chars per line 80
        write_line(headerFile , reg_description)
        headerFile.write(s)
        for x in range(len(field_list)):
            headerFile.write(field_list[x])
        headerFile.write('#endif\n')


# -----------------------------------------------------------------------------
# Iterate through tag mem_elements looking for mem elements produce header file
# output
# -----------------------------------------------------------------------------
def generate_mem_elements(headerFile, mem_elements, tags):
    '''
    Parse registers tag for mem tags and print to header file
    :param headerFile:
    :param registers:
    :return:
    '''
    for mem in mem_elements:
        name = 'LIBERO_SETTING_' + mem.get('name')
        name_of_reg = name
        name_size = name + '_SIZE'
        description = mem.get('description')
        name_gap = 15
        if len(name) > 15:
            name_gap = len(name)
        s = '#define' + ' ' + name.ljust(name_gap, ' ')
        s1 = '#define' + ' ' + name_size.ljust(name_gap, ' ')
        # get the values
        mem_value = mem.text.strip()
        mem_size = mem.get('size')
        # make sure space between name and value 4 spaces
        name_gap = 4
        if len(s) >= name_gap:
            name_gap = len(s) + 4
        # make sure space between name and value 4 spaces
        name_size_gap = 4
        if len(s1) >= name_size_gap:
            name_size_gap = len(s1) + 4
        # create the strings for writing
        s = s.ljust(name_gap, ' ') + mem_value +  '\n'
        reg_description = '/*' + description + ' */ \n'
        s1 = s1.ljust(name_size_gap, ' ') + mem_size \
             + '    /* Length of memory block*/ \n'
        headerFile.write('#if !defined ' + '(' + name_of_reg + ')\n')
        headerFile.write(reg_description)
        headerFile.write(s)
        headerFile.write(s1)
        headerFile.write('#endif\n')


# -----------------------------------------------------------------------------
# generate a header file
# -----------------------------------------------------------------------------
def generate_header( file, real_root, root, file_name, tags):
    creator = "Microchip-FPGA Embedded Systems Solutions"
    with open(file, 'w+') as headerFile:
        # write the copyright header
        WriteCopyright(real_root, headerFile, file_name, creator)
        start_define(headerFile, file_name)
        start_cplus(headerFile, file_name)
        for child in root:
            if child.tag == "registers":
                generate_register(headerFile, child, tags)
            if child.tag == "mem_elements":
                generate_mem_elements(headerFile, child, tags)
            for child2 in child:
                if child2.tag == "registers":
                    generate_register(headerFile, child2, tags)
        end_cplus(headerFile, file_name)
        end_define(headerFile, file_name)


# -----------------------------------------------------------------------------
#
# -----------------------------------------------------------------------------
def generate_reference_header_file(ref_header_file, root, header_files):
    creator = "Embedded Software"
    # itemName ="io_mux configuration"
    s = ref_header_file.split(',')
    file = os.path.join(*s)
    file_name = s[-1]
    with open(file, 'w+') as headerFile:
        # write the copyright header
        WriteCopyright(root, headerFile, file_name, creator)
        # add exclusive define
        start_define(headerFile, file_name)
        # include all the headers
        index = 0
        for child in header_files:
            c = header_files[index].split(',')
            c.remove('soc_config')
            # include_file = os.path.join(*c)
            # as we need formatting correct for linux and windows
            include_file = c[0] + '/' + c[1]
            headerFile.write('#include \"' + include_file + '\" \n')
            index += 1
        # add the c++ define
        start_cplus(headerFile, file_name)
        # no content in this case
        comment = '/* No content in this file, used for referencing header */\n'
        headerFile.write(comment)
        # end the c++ define
        end_cplus(headerFile, file_name)
        end_define(headerFile, file_name)


# -----------------------------------------------------------------------------
#  Generate all the header files, passed in output_header_files
# -----------------------------------------------------------------------------
def generate_header_files(output_header_files, input_xml_file, input_xml_tags):
    # read in an xml file
    s = input_xml_file.split(',')
    root = read_xml_file(s)
    index = 0
    while index < len(input_xml_tags):
        ref_tags = input_xml_tags[index].split(',')
        s = output_header_files[index].split(',')
        file_name = s[-1]
        dir_name = s[-2]
        file_dir = os.path.join(*s)
        found_match = 0
        for child in root:
            if child.tag == 'mss_' + dir_name:
                for child1 in child:
                    if child1.tag == ref_tags[1]:
                        found_match = 1
                        break
        #
        # Next, create file based on xml content
        #
        if found_match == 1:
            generate_header(file_dir, root, child1, file_name, ref_tags)
        index += 1

    '''
    generate a header which references all the generated headers
    '''
    file_name = 'soc_config,hw_platform.h'
    generate_reference_header_file(file_name, root, output_header_files)


# -----------------------------------------------------------------------------
# Return absolute path created from working directory location and relative
# path passed as argument. Handles path to an XML file and path to a folder.
# -----------------------------------------------------------------------------
def get_full_path(in_path):
    cwd = os.getcwd()
    filename = ''
    if in_path.endswith('.xml'):
        path_comp = in_path.split('/')
        last = len(path_comp) - 1
        filename = path_comp[last]
        in_path = in_path.replace(filename, '')
    else:
        dir_entries = os.listdir(in_path)
        for dir_entry in dir_entries:
            print(dir_entry)
            if dir_entry.endswith('.xml'):
                filename = dir_entry
            else:
                if dir_entry.endswith('_mss_cfg.xml'):
                    filename = dir_entry
                    break

    try:
        os.chdir(in_path)
        full_path = os.getcwd()
    except IOError:
        sys.exit()

    os.chdir(cwd)
    full_path = full_path + '/' + filename
    return full_path


# -----------------------------------------------------------------------------
# helper for showing help information
# -----------------------------------------------------------------------------
def show_help():
    print ('no of args you entered = ' + str(len(sys.argv) - 1))
    print ('mpfs_configuration_generator.py :')
    print (' This program reads xml hardware definition, outputs: header files')
    print \
        (' Usage: python3 mpfs_configuration_generator.py [xml file name] [output folder location] ')
    input(' Please run again with correct arguments')


# -----------------------------------------------------------------------------
#    main function
#    todo: add options from the command line
# -----------------------------------------------------------------------------
def main():
    '''
    Currently four command line arguments
    arg0: required- the xml file to be parsed. Only one used in normal flow.
    arg1: name of the folder where the soc_config will be generated. This is optional unless arg2 and arg3 are required.
    arg2: Command parsed, if prent and equals 'generate_reference_xml' creates
        xml example file, If so, an example xml is generated from .csv
        definitions located in the reg_descriptions directory
    arg3: Command parsed, if present and equals 'debug_regs' outputs .csv with
        all reg names.
    '''

    #
    #  check/parse  arguments
    #
    nb_arguments = len(sys.argv) - 1
    if nb_arguments < 2:
        show_help()
        sys.exit()
    fullCmdArguments = sys.argv
    # - further arguments
    argumentList = fullCmdArguments[1:]
    input_xml_file = argumentList[0]
    input_xml_file = get_full_path(input_xml_file)

    if nb_arguments >= 2:
        output_folder_name = argumentList[1]
        output_folder_name = get_full_path(output_folder_name)
        os.chdir(output_folder_name)

    debug_reg_csv = False
    if nb_arguments >= 4:
        if argumentList[3] == 'debug_regs':
            debug_reg_csv = True
    if nb_arguments >= 3:
        if argumentList[2] == 'generate_refernce_xml':
            gen_xml = True
        else:
            gen_xml = False
    #
    # Check version of python interpreter, helps debugging
    # Currently runs on python version 2 and 3
    #
    print ('python interpreter details:',sys.version_info)
    if sys.version_info > (3, 0):
        # Python 3 code in this block
        print ('python interpreter running is version 3')
    else:
        # Python 2 code in this block
        print ('python interpreter running is version 2')
    #
    #  Create one xml file containing all xml information from .csv defines
    #  This is only used for internal testing. Not available for external use.
    #
    if len(sys.argv) >= 4:
        if gen_xml == True:
            import generate_xml_from_csv
            generate_xml_from_csv.generate_full_xml_file(reference_xml_file, xml_tags , get_xml_ver())
            # print('xml file created: ' + reference_xml_file.split(',')[-1])
    #
    # Create directory structure for the header files
    #
    root_folder = 'soc_config'
    TOP = ['clocks', 'ddr', 'io', 'memory_map', 'sgmii', 'general']
    create_hw_dir_struct(root_folder, TOP)
    #
    # Next, read in XML content and create header files
    #
    generate_header_files(header_files, input_xml_file, xml_tags)
    print('Hardware configuration header files created in directory:', os.path.join(output_folder_name, 'soc_config'))
    #
    #  generate an xml example with tags - only for reference
    #
#    generate_xml_from_csv.generate_xml_description_for_reference\
#        (xml_tag_file, xml_tags)


if __name__ == "__main__":
    main()


