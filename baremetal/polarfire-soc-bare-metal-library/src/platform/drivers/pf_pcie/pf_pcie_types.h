/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire and PolarFire SoC PCIe subsystem software driver public
 * data structures.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef PF_PCIESS_TYPES_H_
#define PF_PCIESS_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
  PCIe AXI master and slave table size enum i.e PCIe ATR_SIZE
  PCIe ATR_SIZE is 6 bits long and defines the Address Translation Space Size.
  This space size in bytes is equal to 2^(ATR_SIZE +1).

  The pf_pcie_atr_size_t type specifies the table sizes supported by the
  PolarFire PCIe driver for the initialization of the PCIe AXI4 master and
  slave address translation tables.
*/
typedef enum
{
    PF_PCIE_SIZE_4KB = 11,
    PF_PCIE_SIZE_8KB,
    PF_PCIE_SIZE_16KB,
    PF_PCIE_SIZE_32KB,
    PF_PCIE_SIZE_64KB,
    PF_PCIE_SIZE_128KB,
    PF_PCIE_SIZE_256KB,
    PF_PCIE_SIZE_512KB,
    PF_PCIE_SIZE_1MB,
    PF_PCIE_SIZE_2MB,
    PF_PCIE_SIZE_4MB,
    PF_PCIE_SIZE_8MB,
    PF_PCIE_SIZE_16MB,
    PF_PCIE_SIZE_32MB,
    PF_PCIE_SIZE_64MB,
    PF_PCIE_SIZE_128MB,
    PF_PCIE_SIZE_256MB,
    PF_PCIE_SIZE_512MB,
    PF_PCIE_SIZE_1GB,
    PF_PCIE_SIZE_2GB,
    PF_PCIE_SIZE_4GB,
    PF_PCIE_SIZE_8GB,
    PF_PCIE_SIZE_16GB,
    PF_PCIE_SIZE_32GB,
    PF_PCIE_SIZE_64GB,
    PF_PCIE_SIZE_128GB,
    PF_PCIE_SIZE_256GB,
    PF_PCIE_SIZE_512GB,
    PF_PCIE_SIZE_1TB,
    PF_PCIE_SIZE_2TB,
    PF_PCIE_SIZE_4TB,
    PF_PCIE_SIZE_8TB,
    PF_PCIE_SIZE_16TB,
    PF_PCIE_SIZE_32TB,
    PF_PCIE_SIZE_64TB,
    PF_PCIE_SIZE_128TB,
    PF_PCIE_SIZE_256TB,
    PF_PCIE_SIZE_512TB
} pf_pcie_atr_size_t;

/*****************************************************************************
  The pf_pcie_bar_type_t type specifies the bar types supported by the driver
  for the initialization of the PCIe AXI4 master address translation table.

  PCIe BAR type enum for 32-bit and 64-bit memory.
*/
typedef enum
{
    PF_PCIE_BAR_TYPE_32BIT_MEM = 0x0,
    PF_PCIE_BAR_TYPE_32BIT_PREFET_MEM = 0x8,
    PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM = 0xC
} pf_pcie_bar_type_t;

/*****************************************************************************
  The pf_pcie_tlp_type_t type specifies the transaction layer packet types
  supported by the driver for the initialization of the PCIe AXI4 master and
  slave address translation tables.
*/
typedef enum
{
    PF_PCIE_TLP_MEM = 0,
    PF_PCIE_TLP_MEM_LOCKED = 1,
    PF_PCIE_TLP_IO = 2,
    PF_PCIE_TLP_TRSNL_REQUEST = 3,
    PF_PCIE_TLP_MESSAGE = 4,
    PF_PCIE_TLP_MEM_TRSNL_REQUEST = 5
} pf_pcie_tlp_type_t;

/*****************************************************************************
  The pf_pcie_ep_dma_status_t type communicates the status of the DMA transfer
  initiated by the most recent call to the PF_PCIE_dma_write() or
  PF_PCIE_dma_read() function. It indicates if a transfer is in progress, and
  if this is not the case, indicates the outcome of the latest transfer. This
  type is returned by the PF_PCIE_dma_get_transfer_status() function and used
  as a parameter for the handler functions registered with the PCIe driver.
  The following table shows the different statuses of an endpoint DMA transfer
  indicated by the pf_pcie_ep_dma_status_t type.

  - PF_PCIE_EP_DMA_NOT_INITIALIZED   - The DMA controller is not initialized
  - PF_PCIE_EP_DMA_IN_PROGRESS       - A DMA transfer is in progress.
  - PF_PCIE_EP_DMA_COMPLETED         - The most recent DMA transfer initiated
                                       by a call to PF_PCIE_dma_write() or
                                       PF_PCIE_dma_read() has completed successfully.
  - PF_PCIE_EP_DMA_ERROR             - An error is detected in a DMA controller
                                       transfer, source, or destination completion.
 */
typedef enum
{
    PF_PCIE_EP_DMA_NOT_INITIALIZED = 0,
    PF_PCIE_EP_DMA_IN_PROGRESS,
    PF_PCIE_EP_DMA_COMPLETED,
    PF_PCIE_EP_DMA_ERROR
}pf_pcie_ep_dma_status_t;

/***************************************************************************//**
  The pf_pcie_write_callback_t type defines the function prototype that must be
  followed by PolarFire PCIe End Point DMA write completion handler function.

  Declaring and Implementing the write call-back function:
  The write call-back function should follow the following prototype:
        void transmit_callback(pf_pcie_ep_dma_status_t status);
  The actual name of the call-back function is unimportant. You can use any name
  of your choice for the write call-back function.

 */
typedef void (*pf_pcie_write_callback_t)(pf_pcie_ep_dma_status_t status);

/***************************************************************************//**
  The pf_pcie_read_callback_t type defines the function prototype that must be
  followed by PolarFire PCIe End Point DMA read completion handler function.

  Declaring and Implementing the read call-back function:
  The read call-back function should follow the following prototype:
    void receive_callback(pf_pcie_ep_dma_status_t status);
  The actual name of the call-back function is unimportant. You can use any
  name of your choice for the read call-back function.

 */
typedef void (*pf_pcie_read_callback_t)(pf_pcie_ep_dma_status_t status);

/*****************************************************************************
  The pf_pcie_info_t structure contains the PCIe system(device or bridge) vendor id,
  bus, device and function number.

  bus_num
    Specifies the bus number of the PCIe system.

  device_num
    Specifies the device number of the PCIe system.

  fun_num
    Specifies the function number of the PCIe system.

  vendor_id
    Specifies the vendor id of PCIe endpoint or bridge or switch.

*/
typedef struct
{
    uint16_t bus_num;
    uint8_t  dev_num;
    uint8_t  fun_num;
    uint16_t vendor_id;
} pf_pcie_info_t;

/*****************************************************************************
  The pf_pcie_bar_info_t structure contains information about the memory
  allocated on the host processor for the PCIe endpoint. It is used in the
  PF_PCIE_allocate_memory() function.

  bar_address
    Specifies the memory address allocated on the host processor for the
    PCIe endpoint BAR.

  bar_size
    Specifies the size of the memory allocated on the host processor for
    the PCIe endpoint BAR.

*/
typedef struct
{
    uint32_t bar0_address;
    uint32_t bar0_size;
    uint32_t bar1_address;
    uint32_t bar1_size;
    uint32_t bar2_address;
    uint32_t bar2_size;
    uint32_t bar3_address;
    uint32_t bar3_size;
    uint32_t bar4_address;
    uint32_t bar4_size;
    uint32_t bar5_address;
    uint32_t bar5_size;
}pf_pcie_bar_info_t;

/******************************************************************************
  The pf_pcie_ebuff_t structure is used in PCIe enumeration process to store
  the number of bridges and devices attached to the PCIe root port. The
  PF_PCIE_enumeration() function returns this structure to the application
  during PCIe enumeration.

  bridges
    Contains information about the attached PCIe bridge or switch, such as the
    bus number, device number, function number, and vendor ID.

  devices
    Contains information about the attached PCIe devices, such as the bus number,
    device number, function number, and vendor ID.

  no_of_bridges_attached
    Specifies the numbers of PCIe bridges attached to the PCIe system.

  no_of_devices_attached
    Specifies the numbers of PCIe endpoints attached on the PCIe system.

*/
typedef struct
{
    pf_pcie_info_t bridges[8u];
    pf_pcie_info_t devices[8u];
    uint8_t no_of_bridges_attached;
    uint8_t no_of_devices_attached;
} pf_pcie_ebuff_t;
/*****************************************************************************
PCIe AXI4 Master ATR table configuration structure.
  The user must create a record of the pf_pcie_master_atr_cfg_t type to hold
  the configuration of the PCIe AXI4 master ATR table. The
  PF_PCIE_master_atr_table_init() function is used to create this configuration
  record by entering the desired values.

  state
    Enables and disables the translation address table implementation.
        * PF_PCIE_ATR_TABLE_ENABLE
        * PF_PCIE_ATR_TABLE_DISABLE
  bar_type
    Sets the PCIe BAR type memory on AXI4 master ATR table to 32-bit or
    64-bit memory.
        * PF_PCIE_BAR_TYPE_32BIT_MEM
        * PF_PCIE_BAR_TYPE_32BIT_PREFET_MEM
        * PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM
  bar_size
    Specifies the size of the PCIe BAR space. The pf_pcie_atr_size_t type is
    used assign the bar size.

        * PF_PCIE_SIZE_4KB
        * PF_PCIE_SIZE_8KB
        * PF_PCIE_SIZE_16KB
        * PF_PCIE_SIZE_32KB
        * PF_PCIE_SIZE_64KB
        * PF_PCIE_SIZE_128KB
        ............
        ............
        * PF_PCIE_SIZE_512TB

  table_size
    Specifies the size of the PCIe AXI4 master address translation table.
    The pf_pcie_atr_size_t type is used to assign the table size.
        * PF_PCIE_SIZE_4KB
        * PF_PCIE_SIZE_8KB
        * PF_PCIE_SIZE_16KB
        * PF_PCIE_SIZE_32KB
        * PF_PCIE_SIZE_64KB
        * PF_PCIE_SIZE_128KB
        ............
        ............
        * PF_PCIE_SIZE_512TB
  src_addr
    Specifies the lower 32-bit source address of the PCIe AXI4 master address
    translation space.

  src_addr_msb
    Specifies the upper 32-bit (63:32-bit) source address of the PCIe AXI4
    master address translation space.

  trns_addr
    Specifies the translated lower 32-bit address of the PCIe AXI4 master
    address translation space.

  trns_addr_msb
    Specifies the translated upper 32-bit(63:32-bit) address of the PCIe AXI4
    master address translation space.

*/
typedef struct
{
    uint32_t state;
    pf_pcie_bar_type_t bar_type;
    pf_pcie_atr_size_t bar_size;
    pf_pcie_atr_size_t table_size;
    uint32_t src_addr;
    uint32_t src_addr_msb;
    uint32_t trns_addr;
    uint32_t trns_addr_msb;
} pf_pcie_master_atr_cfg_t;
/*****************************************************************************
PCIe AXI4 Slave ATR table configuration structure.
  The user must create a record of the pf_pcie_slave_atr_cfg_t type to hold the
  configuration of the PCIe AXI4 slave ATR table. The PF_PCIE_slave_atr_table_init()
  function is used to craete this configuration record by entering the desired values.

  state
    Enables and disables the translation address table implementation.
        * PF_PCIE_ATR_TABLE_ENABLE
        * PF_PCIE_ATR_TABLE_DISABLE
  size
    Specifies the size of the PCIe AXI4 slave address translation table.
    The pf_pcie_atr_size_t type is used to assign the table size.
        * PF_PCIE_SIZE_4KB
        * PF_PCIE_SIZE_8KB
        * PF_PCIE_SIZE_16KB
        * PF_PCIE_SIZE_32KB
        * PF_PCIE_SIZE_64KB
        * PF_PCIE_SIZE_128KB
        ............
        ............
        * PF_PCIE_SIZE_512TB

  src_addr
    Specifies the lower 32-bit source address of the PCIe AXI4 slave address
    translation space.

  src_addr_msb
    Specifies the upper 32-bit (63:32-bit) source address of the PCIe AXI4
    slave address translation space.

  trns_addr
    Specifies the translated lower 32-bit address of the PCIe AXI4 slave
    address translation space.

  trns_addr_msb
    Specifies the translated upper 32-bit (63:32-bit) address of the PCIe
    AXI4 slave address translation space.

*/
typedef struct
{
    uint32_t state;
    pf_pcie_atr_size_t size;
    uint32_t src_addr;
    uint32_t src_addr_msb;
    uint32_t trns_addr;
    uint32_t trns_addr_msb;
} pf_pcie_slave_atr_cfg_t;

#ifdef __cplusplus
}
#endif

#endif /* PF_PCIESS_H_ */
