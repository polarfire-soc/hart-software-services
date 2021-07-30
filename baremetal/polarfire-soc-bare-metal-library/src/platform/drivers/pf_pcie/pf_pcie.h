/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
 *
 * PolarFire and PolarFire SoC PCIe subsystem software driver public API
 * and data structure.
 *
 * SVN $Revision$
 * SVN $Date$
 */
/*=========================================================================*//**
  @mainpage PolarFire and PolarFire SoC PCIe Bare Metal Driver.

  @section intro_sec Introduction
    Microsemi PolarFire(tm) FPGAs(PolarFire and PolarFire SoC devices) contain
    a fully integrated PCIe endpoint and root port subsystems with optimized
    embedded controller blocks that connect to the physical layer interface of
    the PolarFire transceiver. Each PolarFire device includes two PolarFire
    embedded PCIe subsystem (PCIESS) blocks. The PCIESS is a hard PCI Express
    protocol stack embedded within every PolarFire device. It includes the
    transaction layer, data link layer, and physical layer. The PolarFire
    PCIESS includes both a physical coding sublayer (PCS) and a physical
    medium attachment (PMA) sublayer that supports x1, x2, and x4 endpoint
    and root port configurations at up to 5 Gbps (Gen 2) speeds.

    Note: All references to PolarFire in this document include both
    PolarFire and PolarFire SoC devices unless stated otherwise.

    The PolarFire PCIe software driver, provided as C source code, supports a
    set of functions for controlling PCIe as part of a bare-metal system where
    no operating system is available. The driver can be adapted for use as part
    of an operating system, but the implementation of the adaptation layer
    between the driver and the operating system's driver model is outside the
    scope of the driver.

  @section theory_op Theory of Operation

    The PolarFire PCIe driver functions are grouped into the following categories:
        * Enumeration
        * Memory Allocation
        * MSI Enabling
        * Configuration Space Read and Write
        * Address Translation Table
        * Interrupt Control
        * End Point DMA Operation
        * Supports PCIe for both PolarFire and PolarFire SoC devices

    Enumeration
    If the PolarFire PCI Express is configured as a root port, the application
    must call the PF_PCIE_enumeration() function to enumerate the PCIe system
    components. The PF_PCIE_enumeration() function uses the PCIe base address,
    the dual PCIe controller number, and the ECAM base address as its
    parameters, and returns the number of PCIe bridges/switches and endpoint
    devices attached to the PCIe system. The initial ECAM base address for
    the enumeration must start with 0 for bus, device, and function numbers.

    Memory Allocation
    The PF_PCIE_allocate_memory() function allocates memory on the PCIe root
    port host processor for the PCIe endpoints. The application must call this
    function after it calls the PF_PCIE_enumeration() function. This function
    uses the ECAM base address and allocated memory address from the host
    processor memory map as its parameters.

    MSI Enabling
    The PF_PCIE_enable_config_space_msi() enables MSI in the PCIe configuration
    space of the MSI capability register set. The application must call this
    function after it calls the PF_PCIE_enumeration() function. The function must
    be called separately to enable MSI in the PCIe root port and PCIe endpoint.

    Configuration Space Read and Write
    The following functions are used for configuration space read and write:
        * PF_PCIE_type1_header_read()
        * PF_PCIE_config_space_atr_table_init()
        * PF_PCIE_config_space_atr_table_terminate()
        * PF_PCIE_config_space_read()
        * PF_PCIE_config_space_write()

    The PF_PCIE_config_space_read() and PF_PCIE_config_space_write() functions
    are used by the application to read and write data to the PCIe type 0 or
    type 1 configuration space registers. Before calling the configuration space
    read or write function, the application must initialize the address
    translation table for the configuration space using the
    PF_PCIE_config_space_atr_table_init() function. The application can terminate
    the configuration space read or write operation using the
    PF_PCIE_config_space_atr_table_terminate() function. The application can
    read the entire PCIe type 1 configuration space header information using the
    PF_PCIE_type1_header_read() function.

    Address Translation table
    The following functions are used for address translation table setup:
        * PF_PCIE_master_atr_table_init()
        * PF_PCIE_slave_atr_table_init()
    The PF_PCIE_master_atr_table_init() function initializes the address
    translation table for the PCIe to perform address translation from the
    PCIe address space (BAR) to the AXI4 master.
    The PF_PCIE_slave_atr_table_init() function initializes the address
    translation table for the PCIe to perform address translation from the
    AXI4 slave to the PCIe address space.

    Interrupt Control
    Interrupts generated by the PCIe controller configured for MSI, INTx, and
    DMA transfer using the following functions:
        * PF_PCIE_enable_interrupts()
        * PF_PCIE_disable_interrupts()
        * PF_PCIE_isr()
    The PF_PCIE_enable_interrupts() function is used to enable the local MSI,
    INTx, and DMA transfer interrupts on the PCIe Root Port.
    The PF_PCIE_disable_interrupts() function is used to disable the local MSI,
    INTx, and DMA transfer interrupts on the PCIe Root Port.
    The PF_PCIE_isr() function is used to handle the PCIe interrupts. The user
    must call the PF_PCIE_isr() function from the system level interrupt handler.

    End Point DMA Operation
    The Application on the PCIe Root Port host processor initializes and configures
    the PolarFire PCIe End Point DMA engine for DMA transfer.

    The following functions are used for DMA transfer:
        * PF_PCIE_dma_init()
        * PF_PCIE_dma_read()
        * PF_PCIE_dma_write()
        * PF_PCIE_dma_abort()
        * PF_PCIE_set_dma_write_callback()
        * PF_PCIE_set_dma_read_callback()
        * PF_PCIE_dma_get_transfer_status()

    Initialization
    The PF_PCIE_dma_init() function in the PCIe root port application initializes
    the PolarFire PCIe endpoint DMA. This function must be called before any
    other PolarFire PCIe DMA driver functions. This function uses the allocated
    memory address from the memory map for the PCIe endpoint BAR on the host
    processor as its parameter. The application can call this function after
    the PF_PCIE_enumeration() and PF_PCIE_allocate_memory() functions.

    Data Transfer Control
    The PF_PCIE_dma_read() function starts a DMA transfer from the PCIe endpoint
    memory to the PCIe root port memory. It reads the data from the PCIe
    endpoint memory and writes it to the PCIe root port memory.

    The PF_PCIE_dma_write() function starts a DMA transfer from PCIe root port
    memory to the PCIe endpoint memory. It reads the data from the PCIe root
    port memory and writes it to the PCIe endpoint memory.
    For all DMA transfers, the user must provide the source and destination
    address, along with the transfer size.

    The PF_PCIE_dma_abort() function aborts a dma transfer that is in progress.

    Data transfer status
    The status of the PCIe DMA transfer initiated by the last call to PF_PCIE_dma_read()
    or PF_PCIE_dma_write() can be retrieved using the PF_PCIE_dma_get_transfer_status()
    function.

    Interrupt Handling
    PCIe DMA read and write operations are interrupt driven. The application must
    register read and write callback functions with the driver. This relevant callback
    function is then called by the PCIe driver every time data is written to or read
    from the destination address.
    The PF_PCIE_set_dma_read_callback() and PF_PCIE_set_dma_write_callback()
    functions are used to register a handler function that is called by the
    driver when the endpoint DMA transfer is completed. The driver passes the
    outcome of the transfer to the completion handler in the form of a
    pf_pcie_ep_dma_status_t parameter, which indicates whether the transfer
    was successful, and in case of an error during the transfer, indicates
    the type of error that occurred. The user must create and register
    transfer completion handler functions suitable for the application.

    The user must call the PF_PCIE_enable_interupts() function or the
    PF_PCIE_disable_interupts() function to enable or disable PCIe interrupts.
    And also, the user must call the PF_PCIE_isr() function from the system
    level interrupt handler for handling DMA interrupts.

*//*=========================================================================*/
#ifndef PF_PCIESS_H_
#define PF_PCIESS_H_

#include "pf_pcie_regs.h"
#include "pf_pcie_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* PCIe Controller 0 */
#define PF_PCIE_CTRL_0                 0u
/* PCIe Controller 1 */
#define PF_PCIE_CTRL_1                 1u

/* It indicates that the ATR table is enabled */
#define PF_PCIE_ATR_TABLE_ENABLE       1u
/* It indicates that the the ATR table is disabled */
#define PF_PCIE_ATR_TABLE_DISABLE      0u
/*****************************************************************************
  The address translation table initialization is successfully assigned to
  a ATR table of AXI4 master/slave
*/
#define PF_PCIE_ATR_TABLE_INIT_SUCCESS  0u
/*****************************************************************************
  The address translation table initialization is not successfully assigned to
  a ATR table of AXI4 master/slave
*/
#define PF_PCIE_ATR_TABLE_INIT_FAILURE  1u

/******************************************************************************
  The PF_PCIE_enumeration() function enumerates all the components in a PCIe
  system, including endpoints, bridges, and switches connected to the system.

  @param apb_addr
    Specifies the base address in the processor's memory map for the registers
    of the PCI Express hardware instance being initialized.

  @param pcie_ctrl_num
    Specifies the PCIe controller number 0 or 1.

  @param ecam_addr
    Specifies the ECAM address of PCIe system. The address is  calculated based
    on the bus number, device number, function number, and the PCIe AXI4 slave
    base address from the processor's memory map. The bus, device, and function
    number should be 0 so the enumeration starts with bus number 0 and ends with
    bus number 8.

  @return
    The PF_PCIE_enumeration() function returns a pointer to the PCIe enumerate
    buffer structure for all the devices and bridges connected to the PCIe system.

  @code
        uint64_t apb_addr = 0x60000000

        uint64_t ecam_addr = 0x70000000;
        uint8_t slv_table_no = 0;
        uint8_t tlp = PF_PCIE_TLP_MEM;

        pf_pcie_slave_atr_cfg_t  s_cfg;
        pf_pcie_ebuff_t * p_pcie_enum_data;

        s_cfg.state = PF_PCIE_ATR_TABLE_ENABLE;
        s_cfg.size = PF_PCIE_SIZE_256MB;
        s_cfg.src_addr = 0x70000000;
        s_cfg.src_addr_msb = 0;
        s_cfg.trns_addr = 0x70000000;
        s_cfg.trns_addr_msb = 0;

        PF_PCIE_slave_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &s_cfg, slv_table_no, tlp);

        p_pcie_enumer_data = PF_PCIE_enumeration(apb_addr, PF_PCIE_CTRL_1, ecam_addr);

        if(p_pcie_enum_data->no_of_devices_attached != 0)
        {

        }
  @endcode
*/
pf_pcie_ebuff_t *
PF_PCIE_enumeration
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr
);

/****************************************************************************
  The PF_PCIE_allocate_memory() function allocates memory on the PCIe root
  port host processor for the PCIe endpoint BARs. This function must be called
  after the PF_PCIE_enumeration() function.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4 slave
    base address from the processor's memory map.

  @param allocate_addr
    Provides the translated address from the PCIe root port AXI4 slave address
    translation table. This address is assigned to the PCIe endpoint BAR.

  @return
    The PF_PCIE_allocate_memory() function returns a pointer to the PCIe bar
    info structure for the assigned BAR address and BAR size to the PCIe endpoint.

   @code

        uint64_t apb_addr = 0x60000000
        uint64_t ecam_addr = 0x70000000;
        uint64_t ecam_addr1 = 0x70100000;
        uint8_t slv_table_no = 0;
        uint8_t tlp = PF_PCIE_TLP_MEM;

        pf_pcie_slave_atr_cfg_t  s_cfg;
        pf_pcie_bar_info_t *p_pcie_bar_data;
        pf_pcie_ebuff_t * p_pcie_enum_data;

        s_cfg.state = PF_PCIE_ATR_TABLE_ENABLE;
        s_cfg.size = PF_PCIE_SIZE_256MB;
        s_cfg.src_addr = 0x70000000;
        s_cfg.src_addr_msb = 0;
        s_cfg.trns_addr = 0x70000000;
        s_cfg.trns_addr_msb = 0;

        PF_PCIE_slave_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &s_cfg, slv_table_no, tlp);

        p_pcie_enumer_data = PF_PCIE_enumeration(apb_addr, PF_PCIE_CTRL_1, ecam_addr);
        if(p_pcie_enum_data->no_of_devices_attached != 0)
        {
            p_pcie_bar_data = PF_PCIE_allocate_memory(ecam_addr1, 0x70000000);
        }

  @endcode
*/
pf_pcie_bar_info_t *
PF_PCIE_allocate_memory
(
    uint64_t ecam_addr,
    uint64_t allocate_addr
);

/****************************************************************************
  The PF_PCIE_enable_config_space_msi() function enables the PCIe root port or
  endpoint MSI in the PCIe configuration space. It also sets up the message
  address and other data required to generate MSI.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4
    slave base address from the processor's memory map.

  @param msi_addr
    Specifies the target memory address on the PCIe root port to generate MSI
    from the PCIe endpoint.

  @param msi_data
    Specifies the data value to write into the msi_addr    parameter.

  @return
    The PF_PCIE_enable_config_space_msi() function does not return a value.

   @code
        uint64_t apb_addr = 0x60000000
        uint64_t ecam_addr = 0x70000000;
        uint64_t ecam_addr1 = 0x70100000;
        uint8_t slv_table_no = 0;
        uint8_t tlp = PF_PCIE_TLP_MEM;

        pf_pcie_slave_atr_cfg_t  s_cfg;
        pf_pcie_master_atr_cfg_t  m_cfg;
        pf_pcie_bar_info_t *p_pcie_bar_data;
        pf_pcie_ebuff_t * p_pcie_enum_data;

        s_cfg.state = PF_PCIE_ATR_TABLE_ENABLE;
        s_cfg.size = PF_PCIE_SIZE_256MB;
        s_cfg.src_addr = 0x70000000;
        s_cfg.src_addr_msb = 0;
        s_cfg.trns_addr = 0x70000000;
        s_cfg.trns_addr_msb = 0;

        PF_PCIE_slave_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &s_cfg, slv_table_no, tlp);

        m_cfg.state =  PF_PCIE_ATR_TABLE_ENABLE;
        m_cfg.bar_type = PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM;
        m_cfg.bar_size = PF_PCIE_SIZE_64KB;
        m_cfg.table_size = PF_PCIE_SIZE_64KB;
        m_cfg.src_addr = 0x10000000;
        m_cfg.src_addr_msb = 0;
        m_cfg.trns_addr = 0x00000000;
        m_cfg.trns_addr_msb = 0;

        PF_PCIE_master_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &m_cfg, slv_table_no, tlp);

        p_pcie_enumer_data = PF_PCIE_enumeration(apb_addr, PF_PCIE_CTRL_1, ecam_addr);
        if(p_pcie_enum_data->no_of_devices_attached != 0)
        {
            PF_PCIE_enable_config_space_msi(ecam_addr, 0x190, 0x120);
            PF_PCIE_enable_config_space_msi(ecam_addr1, 0x190, 0x120);
        }

  @endcode
*/
void
PF_PCIE_enable_config_space_msi
(
    uint64_t ecam_addr,
    uint64_t msi_addr,
    uint16_t msi_data
);

/****************************************************************************
  The PF_PCIE_type1_header_read() function reads the PCIe type1 (bridge or switch)
  configuration space header information.

  @param apb_addr
    Specifies the base address in the processor's memory map for the registers
    of the PCI Express hardware instance being initialized.

  @param pcie_ctrl_num
    Specifies the PCIe controller number, 0 or 1.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4
    slave base address from the processor's memory map.

  @param p_type1_header
    Contains the output of the PCIe type1 header. The PCIe bridge/switch type 1
    header information is stored in this parameter.

  @return
   The PF_PCIE_type1_header_read() function does not return a value.

  @code

        uint64_t apb_addr = 0x60000000
        uint64_t ecam_addr = 0x70000000;
        PCIE_ROOT_CONF read_type1_header;

        PF_PCIE_type1_header_read(apb_addr, PF_PCIE_CTRL_1, ecam_addr, &read_type1_header)

  @endcode
*/
void
PF_PCIE_type1_header_read
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr,
    PCIE_ROOT_CONF * p_type1_header
);

/****************************************************************************
  The PF_PCIE_config_space_atr_table_init() function initializes the PCIe AXI4
  slave address translation table using the ecam_addr parameter, and enables
  the PCIe configuration interface to read or write data to the configuration
  space registers.

  @param apb_addr
    Specifies the base address in the processor's memory map for the registers
    of the PCI Express hardware instance being initialized.

  @param pcie_ctrl_num
    Specifies the PCIe controller number, 0 or 1.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4
    slave base address from the processor's memory map.

  @return
    The PF_PCIE_config_space_atr_table_init() function returns success or failure
    of the PCIe ATR table initialization. The two possible return values are:
        -PF_PCIE_ATR_TABLE_INIT_SUCCESS
        -PF_PCIE_ATR_TABLE_INIT_FAILURE
*/
uint8_t
PF_PCIE_config_space_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr
);

/****************************************************************************
  The PF_PCIE_config_space_atr_table_terminate() function disables the PCIe
  configuration interface in the address translation table and enables the PCIe
  Tx/Rx interface for PCIe transactions.

  @param
    The PF_PCIE_config_space_atr_table_terminate() function does not have parameters.

  @return
    The PF_PCIE_config_space_atr_table_terminate() function does not return a value.
*/
void PF_PCIE_config_space_atr_table_terminate(void);

/****************************************************************************
  The PF_PCIE_config_space_read() function reads the data from configuration
  space of PCIe device register, bridge/switch register.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4
    slave base address from the processor's memory map.

  @param config_space_offset
    Specifies a PCIe type 0/1 configuration space address offset.

  @param value
    Specifies the output value to read from the PCIe type 0/1 configuration space.

  @return
   The PF_PCIE_config_space_read() function does not return a value.

  @code
        uint64_t apb_addr = 0x60000000
        uint64_t ecam_addr = 0x70100000;
        uint32_t read_value0;
        uint32_t read_value1;

        PF_PCIE_config_space_atr_table_init(apb_addr, PF_PCIE_CTRL_1, ecam_addr);

        PF_PCIE_config_space_read(ecam_addr, DEVICE_VID_DEVID, &read_value0);

        PF_PCIE_config_space_read(ecam_addr, DEVICE_CFG_PRMSCR, &read_value1);

        PF_PCIE_config_space_atr_table_terminate();

  @endcode
*/
void
PF_PCIE_config_space_read
(
    uint64_t ecam_addr,
    uint16_t config_space_offset,
    uint32_t *value
);

/****************************************************************************
  The PF_PCIE_config_space_write() function writes the data on configuration
  space of PCIe device, bridge/switch register.

  @param ecam_addr
    Specifies the ECAM address of the PCIe system. The address is calculated
    based on the bus number, device number, function number, and PCIe AXI4
    slave base address from the processor's memory map.

  @param config_space_offset
    Specifies a PCIe type 0/1 configuration    space address offset.

  @param value
    Specifies the input value to write on the PCIe type 0/1 configuration space.

  @return
   The PF_PCIE_config_space_write() function does not return a value.

  @code
        uint64_t apb_addr = 0x60000000
        uint64_t ecam_addr = 0x70100000;
        uint32_t write_value0 = 0x11AA;
        uint32_t write_value1 = 0x6;

        PF_PCIE_config_space_atr_table_init(apb_addr, PF_PCIE_CTRL_1, ecam_addr);

        PF_PCIE_config_space_write(ecam_addr, DEVICE_VID_DEVID, write_value0);

        PF_PCIE_config_space_write(ecam_addr, DEVICE_CFG_PRMSCR, write_value1);

        PF_PCIE_config_space_atr_table_terminate();

  @endcode
*/
void
PF_PCIE_config_space_write
(
    uint64_t ecam_addr,
    uint16_t config_space_offset,
    uint32_t value
);

/****************************************************************************
  The PF_PCIE_master_atr_table_init() function sets up the address translation
  table for the PCIe AXI4 master.

  @param apb_addr
    Specifies the base address in the processor's memory map for the registers
    of the PCI Express hardware instance being initialized.

  @param pcie_ctrl_num
    Specifies the PCIe controller number, 0 or 1.

  @param cfg
    Specifies the configuration data structure of the PCIe AXI4 Master address
    translation table.

  @param master_table_num
    Specifies the PCIe to AXI4 master address translation table number. There
    are a total of six AXI master address translation tables in the PCIESS.

  @param tlp_type
    Specifies the Transaction Layer Packet(TLP) type. Available options are:
        * PF_PCIE_TLP_MEM
        * PF_PCIE_TLP_MEM_LOCKED
        * PF_PCIE_TLP_IO
        * PF_PCIE_TLP_TRSNL_REQUEST
        * PF_PCIE_TLP_MESSAGE
        * PF_PCIE_TLP_MEM_TRSNL_REQUEST

  @return
    The PF_PCIE_master_atr_table_init() function returns success or failure of
    the PCIe ATR table initialization. The two possible return values are:
        -PF_PCIE_ATR_TABLE_INIT_SUCCESS
        -PF_PCIE_ATR_TABLE_INIT_FAILURE

  @code
        pf_pcie_master_atr_cfg_t m_cfg;
        uint64_t apb_addr = 0x60000000
        uint8_t mst_table_no = 0;
        uint8_t tlp = PF_PCIE_TLP_MEM;

        m_cfg.state = PF_PCIE_ATR_TABLE_ENABLE;
        m_cfg.bar_type = PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM;
        m_cfg.bar_size = PF_PCIE_SIZE_64KB;
        m_cfg.table_size = PF_PCIE_SIZE_64KB;
        m_cfg.src_addr = 0x10000000;
        m_cfg.src_addr_msb = 0;
        m_cfg.trns_addr = 0x00000000;
        m_cfg.trns_addr_msb = 0;

        PF_PCIE_master_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &m_cfg, mst_table_no, tlp);


  @endcode
*/
uint8_t
PF_PCIE_master_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    pf_pcie_master_atr_cfg_t * cfg,
    uint8_t master_table_num,
    uint8_t tlp_type
);

/****************************************************************************
  The PF_PCIE_slave_atr_table_init() function sets up the address translation table
  for the PCIe AXI4 slave.

  @param apb_addr
    Specifies the base address in the processor's memory map for the registers
    of the PCI Express hardware instance being initialized.

  @param pcie_ctrl_num
    Specifies the PCIe controller number, 0 or 1.

  @param cfg
    Specifies the configuration data structure of AXI4 slave to PCIe address
    translation table.

  @param slave_table_num
    Specifies the AXI4 slave to PCIe transaction address translation table
    number. There are a total of eight AXI slave address translation tables
    in the PCIESS.

  @param tlp_type
    Specifies the transaction layer packet(TLP) type.
        * PF_PCIE_TLP_MEM
        * PF_PCIE_TLP_MEM_LOCKED
        * PF_PCIE_TLP_IO
        * PF_PCIE_TLP_TRSNL_REQUEST
        * PF_PCIE_TLP_MESSAGE
        * PF_PCIE_TLP_MEM_TRSNL_REQUEST


  @return
    The PF_PCIE_slave_atr_table_init() function returns success or failure of
    the PCIe ATR table initialization. The two possible return values are:
        -PF_PCIE_ATR_TABLE_INIT_SUCCESS
        -PF_PCIE_ATR_TABLE_INIT_FAILURE

  @code
        uint64_t apb_addr = 0x60000000
        uint8_t slv_table_no = 0;
        uint8_t tlp = PF_PCIE_TLP_MEM;
        pf_pcie_slave_atr_cfg_t  s_cfg;

        s_cfg.state = PF_PCIE_ATR_TABLE_ENABLE;
        s_cfg.size = PF_PCIE_SIZE_256MB;
        s_cfg.src_addr = 0x70000000;
        s_cfg.src_addr_msb = 0;
        s_cfg.trns_addr = 0x70000000;
        s_cfg.trns_addr_msb = 0;

        PF_PCIE_slave_atr_table_init(apb_addr, PF_PCIE_CTRL_1, &s_cfg, slv_table_no, tlp);


  @endcode
*/
uint8_t
PF_PCIE_slave_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    pf_pcie_slave_atr_cfg_t * cfg,
    uint8_t slave_table_num,
    uint8_t tlp_type
);

/****************************************************************************
  The PF_PCIE_dma_init() function initializes the PCIe endpoint DMA from the
  PCIe root port host processor.

  @param allocated_addr
    Specifies the memory address allocated on the PCIe root port host processor
    for PCIe endpoint BAR. The BAR accesses    the endpoint DMA engine.

  @return
   The PF_PCIE_dma_init() function does not return a value.
*/
void PF_PCIE_dma_init(uint64_t  allocated_addr);

/*******************************************************************************
  The PF_pcie_set_dma_write_callback() function registers the function called by
  the PCIe driver when the data has been written.

  @param write_callback
    Points to the function that will be called when the data is written by the PCIe DMA.

  @return
    The PF_PCIE_set_dma_write_callback() function does not return a value.

  @code
        void transfer_complete_handler(pf_pcie_ep_dma_status_t status);
        volatile uint32_t g_xfer_in_progress = 0;

        uint8_t External_30_IRQHandler(void)
        {
            PF_PCIE_isr();
            return(EXT_IRQ_KEEP_ENABLED);
        }
        void demo_transfer(void)
        {
            PF_PCIE_dma_init(0x70008000);
            PF_PCIE_set_dma_write_callback(transfer_complete_handler);
            g_xfer_in_progress = 1;
            PF_PCIE_enable_interrupts();
            HAL_enable_interrupts();
            PF_PCIE_dma_write(RP_SRAM_ADDR, EP_SRAM_ADDR, 128);
            while(g_xfer_in_progress)
            {
                ;
            }
        }
        void transfer_complete_handler(pf_pcie_ep_dma_status_t status)
        {
            g_xfer_in_progress = 0;
            switch(status)
            {
                case PF_PCIE_EP_DMA_COMPLETED:
                    display("Transfer complete");
                break;
                case PF_PCIE_EP_DMA_ERROR:
                    display("Transfer failed: error");
                break;
            }
        }

  @endcode
*/
void
PF_PCIE_set_dma_write_callback
(
    pf_pcie_write_callback_t write_callback
);

/*******************************************************************************
  The PF_PCIE_set_dma_read_callback() function registers the function called by
  the PCIe driver when the data has been read.

  @param read_callback
    Points to the function that will be called when the data is read by the PCIe DMA.

  @return
    The PF_PCIE_set_dma_read_callback() function does not return a value.
  @code
        void transfer_complete_handler(pf_pcie_ep_dma_status_t status);
        volatile uint32_t g_xfer_in_progress = 0;

        uint8_t External_30_IRQHandler(void)
        {
            PF_PCIE_isr();
            return(EXT_IRQ_KEEP_ENABLED);
        }
        void demo_transfer(void)
        {
            PF_PCIE_dma_init(0x70008000);
            PF_PCIE_set_dma_read_callback(transfer_complete_handler);
            g_xfer_in_progress = 1;
            PF_PCIE_enable_interrupts();
            HAL_enable_interrupts();
            PF_PCIE_dma_write(EP_SRAM_ADDR, RP_SRAM_ADDR, 128);
            while(g_xfer_in_progress)
            {
                ;
            }
        }
        void transfer_complete_handler(pf_pcie_ep_dma_status_t status)
        {
            g_xfer_in_progress = 0;
            switch(status)
            {
                case PF_PCIE_EP_DMA_COMPLETED:
                    display("Transfer complete");
                break;
                case PF_PCIE_EP_DMA_ERROR:
                    display("Transfer failed: error");
                break;
            }
        }

  @endcode
*/
void
PF_PCIE_set_dma_read_callback
(
    pf_pcie_read_callback_t read_callback
);

/****************************************************************************
  The PF_PCIE_dma_read() function initiates PCIe endpoint DMA data transfer
  from the PCIe root port host processor. Its parameters specify the source and
  destination addresses of the transfer, as well as its size. The PCIe DMA data
  transfers for read operations always use the PCIe endpoint memory as the
  source and the PCIe root port memory as the destination for the transfer.

  Note: A call to PF_PCIE_dma_read() while a transfer is in progress will not
  initiate a new transfer. Use the PF_PCIE_dma_get_transfer_status() function
  or a completion handler registered by the PF_PCIE_set_dma_read_callback()
  function  to check the status of the current transfer before calling the
  PF_PCIE_dma_read() function again.

  @param src_address
    Specifies the source address of the PCIe endpoint memory (AXI4 master ATR
    table sourceaddress).

  @param dest_address
    Specifies the destination address of the PCIe root port memory (AXI4 master
    ATR table destination address).

  @param rx_lenth
    Specifies the length (in bytes) of the data to be read.

  @return
   The PF_PCIE_dma_read() function does not return a value.
*/
void
PF_PCIE_dma_read
(
    uint64_t  src_address,
    uint64_t  dest_address,
    uint32_t  rx_lenth
);

/*******************************************************************************
  The PF_PCIE_dma_write() function initiates PCIe endpoint DMA data transfer
  from the PCIe root port host processor. Its parameters specify the source and
  destination addresses of the transfer as well as its size. The PCIe DMA data
  transfers for write operations always use the PCIe root port memory as the
  source and the PCIe endpoint memory as the destination for the transfer.

  Note: A call to PF_pcie_dma_write() while a transfer is in progress will not
  initiate a new transfer. Use the PF_pcie_dma_get_transfer_status() function
  or a completion handler registered by the PF_pcie_set_dma_write_callback()
  function  to check the status of the current transfer before calling the
  PF_pcie_dma_write() function again.

  @param src_address
    Specifies the source address of the PCIe root port memory (AXI4 master ATR
    table source address).

  @param dest_address
    Specifies the destination address of the PCIe endpoint memory (AXI4 master
    ATR table destination address).

  @param tx_lenth
    Specifies the length (in bytes) of the data to be written.

  @return
    The PF_PCIE_dma_write() function does not return a value.
*/
void
PF_PCIE_dma_write
(
    uint64_t  src_address,
    uint64_t  dest_address,
    uint32_t  tx_lenth
);

/*******************************************************************************
  The PF_PCIE_dma_abort() function aborts a PCIe DMA transfer that is in progress.

  @param
    The PF_PCIE_dma_abort() function has no parameters.

  @return
    The PF_PCIE_dma_abort() function does not return a value.
*/
void PF_PCIE_dma_abort(void);

/*******************************************************************************
  The PF_PCIE_dma_get_transfer_status() function indicates the status of a PCIe
  endpoint DMA transfer initiated by a call to the PF_PCIE_dma_write() or
  PF_PCIE_dma_read() function.

  @param
    The PF_PCIE_dma_get_transfer_status() function has no parameters.

  @return
    The PF_PCIE_dma_get_transfer_status() function returns the status of the PCIE DMA
    transfer as a value of  type pf_pcie_ep_dma_status_t. The possible return values are:
        - PF_PCIE_EP_DMA_NOT_INITIALIZED
        - PF_PCIE_EP_DMA_IN_PROGRESS
        - PF_PCIE_EP_DMA_COMPLETED
        - PF_PCIE_EP_DMA_ERROR
  @code
        void copy_ep_to_rp(void)
        {
            PF_PCIE_dma_init(0x70008000);
            PF_PCIE_dma_read(EP_SRAM_ADDR, RP_SRAM_ADDR, 128);
            do {
                xfer_state = PF_PCIE_dma_get_transfer_status();
            }while(PF_PCIE_EP_DMA_IN_PROGRESS == xfer_state);
        }

  @endcode

*/
pf_pcie_ep_dma_status_t
PF_PCIE_dma_get_transfer_status
(
    void
);

/****************************************************************************
  The PF_pcie_enable_interrupts() function enables local interrupts(MSI, INTx,
  DMAx) on the PCIe RootPort.

  @param
    The PF_PCIE_enable_interrupts() function has no parameters.

  @return
    The PF_PCIE_enable_interrupts() function does not return a value.
*/
void PF_PCIE_enable_interrupts(void);

/****************************************************************************
  The PF_pcie_disable_interrupts() function disables the local interrupts
  on the PCIe RootPort.

  @param
    The PF_PCIE_disable_interrupts() function has no parameters.

  @return
    The PF_PCIE_disable_interrupts() function does not return a value.
*/
void PF_PCIE_disable_interrupts(void);

/*******************************************************************************
 The PF_PCIE_isr() function is a PCIe root port local interrupt handler. The
 PF_PCIE_isr() function is the top level interrupt handler function for the
 PolarFire PCIe driver. The user must call the PF_PCIE_isr() function from
 the system level interrupt handler assigned to the interrupt triggered by
 the PF_PCIE/PCIE_#_INTERRUPT_OUT signal.

  @param
    The PF_PCIE_isr() function does not take any parameters

  @return
    The PF_PCIE_isr() function does not return a value.

  @code
        uint8_t External_30_IRQHandler(void)
        {
            PF_PCIE_isr();
            return(EXT_IRQ_KEEP_ENABLED);
        }
  @endcode
*/
void PF_PCIE_isr(void);

/*****************************************************************************/

uint64_t
ecam_address_calc
(
    uint64_t axi_addr,
    uint8_t bus,
    uint8_t device,
    uint8_t function
);
/*****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* PF_PCIESS_H_ */
