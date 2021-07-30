/*****************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire and PolarFire SoC PCIe subsystem software driver implementation.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "pf_pcie.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/
/* PCIe enumeration values */
/* At Present PCIe supports one Root Port and End Point */
#define PCIE_CFG_MAX_NUM_OF_BUS     8u
#define PCIE_CFG_MAX_NUM_OF_DEV     2u
#define PCIE_CFG_MAX_NUM_OF_FUN     8u

#define PCIE_CFG_FUN_NOT_IMP_MASK   0xFFFFu
#define PCIE_CFG_HEADER_TYPE_MASK   0x00EF0000u
#define PCIE_CFG_MUL_FUN_DEV_MASK   0x00800000u

#define PCIE_CFG_HEADER_O_TYPE      0x00000000u
#define PCIE_CFG_CATCHE_SIZE        0x10u

/* ECAM Address Register bitmaps and masks */
/* Mask of all valid bits */
#define PCIE_ECAM_MASK              0x0FFFFFFFu
/* Bus Number Mask */
#define PCIE_ECAM_BUS_MASK          0x0FF00000u
/* Device Number Mask */
#define PCIE_ECAM_DEV_MASK          0x000F8000u
/* Function Number Mask */
#define PCIE_ECAM_FUN_MASK          0x00007000u
/* Bus Number Shift Value */
#define PCIE_ECAM_BUS_SHIFT         20u
/* Device Number Shift Value */
#define PCIE_ECAM_DEV_SHIFT         15u
/* Function Number Shift Value */
#define PCIE_ECAM_FUN_SHIFT         12u

#define BAR_MASK                    0xFFFFFFFFu
#define BAR_SIZE_MASK               0xFFFFFFF0u
#define BAR_LSB_MASK                0x0000000Fu
#define VENDOR_ID_MASK              0xFFFFu
/* 64-bit Address space of BAR */
#define ADDR_SPACE_64BIT            0x00000004u
/* Translated ID */
#define  PCIE_TX_RX_INTERFACE       0x00000000u
#define  PCIE_CONFIG_INTERFACE      0x00000001u
#define  AXI4_MASTER_0_INTERFACE    0x00000004u
/* Translation table size 256MB */
#define SIZE_256MB_TRANSLATE_TABLE_EN      0x00000037u
/*
 PCIe Root Port
 Control & Status Register: enable I/O, memory access,
                            bus master,Parity Error
                            Response and  SERR#
*/
#define RP_CFG_PRMSCR_DATA          0x00000147u
/*
 PCIe EndPoint
 Control & Status Register: enable Memory access, bus master
*/
#define EP_CFG_PRMSCR_DATA          0x00000006u

/* device conrol status register: maxrdreq=512, maxpayloadsize=256 */
#define RP_DEVICE_CTRL_STATUS_DATA  0x0000202Fu
#define EP_DEVICE_CTRL_STATUS_DATA  0x00FF2830u

/* PCIe Sub bus:FF, sec bus:0, prim bus:0 */
#define PRIM_SEC_SUB_BUS_DEFAULT    0x00FF0000u

/* Non-prefetchable Limit, Non-prefetchable Base */
#define RP_MEM_LIMIT_BASE_DATA      0xEEE0EEE0u
/* Prefetchable Limit, Prefetchable Base */
#define RP_PREF_MEM_LIMIT_BASE_DATA 0xFFF1FFF1u
/* Prefetchable Base upper */
#define RP_PREF_BASE_UPPER_DATA     0xFFFFFFFFu
/* Prefetchable upper limit*/
#define RP_PREF_LIMIT_UPPER_DATA    0x00000000u

/* PCIe Config space MSI capability structure */
#define PCIE_ENABLE_MSI             0x00010000u
#define PCIE_CAPB_ID_MSI            0x05u
#define PCIE_CAPB_ID_MASK           0xFFu
#define PCIE_CONFIG_CAPB_ID_MASK    0xFFFFFFFFFFFFFF00u
#define PCIE_MSI_64BIT_CAPB_ADDR    0x00800000u
#define PCIE_MSI_MULTI_VECTOR_SHIFT 17u
#define PCIE_MSI_MULTI_VECTOR_MASK  7u
#define PCIE_MSI_MULTI_ENABLE_SHIFT 20u

/* PCI Express Capability structure ID */
#define PCIE_CAPB_ID_STRUCT         0x10u
/****************************************************************************/
/* PCIe AXI master table init defines */
#define ATR0_PCIE_WIN0_OFFSET       0x600u
#define ATR_WIN_REG_TABLE_SIZE      0x20u
#define WIN0_SRCADDR_PARAM          0x00u
#define WIN0_SRC_ADDR               0x01u
#define WIN0_TRSL_ADDR_LSB          0x02u
#define WIN0_TRSL_ADDR_UDW          0x03u
#define WIN0_TRSL_PARAM             0x04u
#define ATR_ADDR_MASK               0xFFFFF000u
#define TLP_SHIFT                   16u
#define BAR_01_DW0                  0xE4u
/****************************************************************************/
/* PCIe AXI slave table init defines */
#define ATR0_AXI4_SLV0_OFFSET       0x800u
#define ATR_SLV_REG_TABLE_SIZE      0x20u
#define SLV0_SRCADDR_PARAM          0x00u
#define SLV0_SRC_ADDR               0x01u
#define SLV0_TRSL_ADDR_LSB          0x02u
#define SLV0_TRSL_ADDR_UDW          0x03u
#define SLV0_TRSL_PARAM             0x04u
/****************************************************************************/
/* DMA */
#define EP_DMA_START_DATA           0x00002321u
#define EP_DMA_INTERFACE_AXI        0x00000004u
#define EP_DMA_INTERFACE_PCIE       0x00000000u

/* EP DMA interrupt and error status */
#define DMA_INT_STATUS              0x00000003u
#define DMA_ERR_STATUS              0x00000300u

/* Enable PCIe Host MSI, INTx DMAx interrupts */
#define PCIE_HOST_INT_ENABLE        0x1F000FFFu
/* Enable PCIe local  DMAx, DMA error interrupts */
#define PCIE_LOCAL_INT_ENABLE       0x1F0000FFu
/****************************************************************************/
/* Clear PCIe interrupt events */
#define PCIE_EVENT_INT_DATA         0x00070007u
#define PCIE_ECC_DISABLE            0x0F000000u
#define PCIE_SEC_ERROR_INT_CLEAR    0x0000FFFFu
#define PCIE_DED_ERROR_INT_CLEAR    0x0000FFFFu
#define PCIE_ISTATUS_CLEAR          0xFFFFFFFFu
#define PCIE_CLEAR                  0x00000000u
#define PCIE_SET                    0x00000001u

#define ROOT_PORT_ENABLE            0x00000001u

#define NULL_POINTER                ((void *)0)

#define MASK_32BIT                  0xFFFFFFFFu
#define SHIFT_32BIT                 32u

struct pcie_dma_instance_t
{
  volatile pf_pcie_ep_dma_status_t state;
  pf_pcie_write_callback_t tx_complete_handler;
  pf_pcie_read_callback_t rx_complete_handler;
};

/* EP dma struct variable */
static struct pcie_dma_instance_t g_pcie_dma;

/* RP and EP bridge and controller structure variable */
static PCIE_BRIDGE * g_rp_pcie_bridge = PCIE_CLEAR;
static PCIE_BRIDGE * g_ep_bridge_reg = PCIE_CLEAR;
static PCIE_CTRL * g_ep_ctrl_reg = PCIE_CLEAR;
static PCIE_CTRL * g_rp_pcie_ctrl = PCIE_CLEAR;

/* Enumeration and BAR structure variable */
pf_pcie_ebuff_t g_pcie_enumeration;
pf_pcie_bar_info_t g_pcie_bar_allocate;

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 * This function is valid only when IP is configured as a root complex.
 *
 */
pf_pcie_ebuff_t *
PF_PCIE_enumeration
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr
)
{
    PCIE_BRIDGE * p_pcie_bridge;
    PCIE_END_CONF * p_pcie_config_space;
    PCIE_ROOT_CONF * p_pcie_root_config;
    PCIE_CTRL * p_pcie_ctrl;
    uint32_t * p_pcie_ep_config_space = PCIE_CLEAR;
    uint32_t pcie_header_type;
    uint32_t pcie_multi_fun;
    uint8_t pcie_bus_num;
    uint8_t pcie_dev_num;
    uint8_t pcie_fun_num;
    uint16_t pcie_vendor_id;
    uint64_t src_ecam_addr;
    uint32_t addr_inc;
    uint32_t p_reg;
    uint32_t ecam_addr_low;
    uint32_t ecam_addr_high;
    /* Default attached bridges and devices are 0 */
    uint8_t bridges_attached = PCIE_CLEAR;
    uint8_t devices_attached = PCIE_CLEAR;
    uint32_t prim_sec_num = PRIM_SEC_SUB_BUS_DEFAULT;
    /* Default bridge and device enumeration number is 0 */
    g_pcie_enumeration.no_of_bridges_attached = PCIE_CLEAR;
    g_pcie_enumeration.no_of_devices_attached = PCIE_CLEAR;
    /* Check PCIe Controller is 0 or 1 */
    if (PF_PCIE_CTRL_0 == pcie_ctrl_num)
    {
        /* PCIe Bridge Controller 0 */
        p_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET)));
        p_pcie_ctrl = ((PCIE_CTRL *)((uintptr_t)(apb_addr + PCIE0_CRTL_PHY_ADDR_OFFSET)));
    }
    else if (PF_PCIE_CTRL_1 == pcie_ctrl_num)
    {
        /* PCIe Bridge Controller 1 */
        p_pcie_bridge  = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET)));
        p_pcie_ctrl  = ((PCIE_CTRL *)((uintptr_t)(apb_addr + PCIE1_CRTL_PHY_ADDR_OFFSET)));
    }
    else
    {
        /* Not supports PCIe Bridge Controller */
        p_pcie_bridge = PCIE_CLEAR;
    }
    /* Check Root Port is enabled or not */
    if ((NULL_POINTER != p_pcie_bridge) &&
       (ROOT_PORT_ENABLE == ((p_pcie_bridge->GEN_SETTINGS) & ROOT_PORT_ENABLE)))
    {
        g_rp_pcie_bridge = p_pcie_bridge;
        g_rp_pcie_ctrl = p_pcie_ctrl;
        /* Clear interrupts on PCIe RootPort */
        p_pcie_ctrl->ECC_CONTROL = PCIE_ECC_DISABLE;
        p_pcie_ctrl->PCIE_EVENT_INT = PCIE_EVENT_INT_DATA;
        p_pcie_ctrl->SEC_ERROR_INT = PCIE_SEC_ERROR_INT_CLEAR;
        p_pcie_ctrl->DED_ERROR_INT = PCIE_DED_ERROR_INT_CLEAR;
        p_pcie_bridge->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
        p_pcie_bridge->IMASK_LOCAL = PCIE_CLEAR;
        p_pcie_bridge->ISTATUS_HOST = PCIE_ISTATUS_CLEAR;
        p_pcie_bridge->IMASK_HOST = PCIE_CLEAR;

        ecam_addr_low = (uint32_t)(ecam_addr & MASK_32BIT);
        ecam_addr_high = (uint32_t)((ecam_addr >> SHIFT_32BIT) & MASK_32BIT);
        /* Select PCIe Config space */
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_CONFIG_INTERFACE;
        /* Address Translation table setup */
        p_pcie_bridge->ATR0_AXI4_SLV0_SRCADDR_PARAM = ecam_addr_low | SIZE_256MB_TRANSLATE_TABLE_EN;
        p_pcie_bridge->ATR0_AXI4_SLV0_SRC_ADDR = ecam_addr_high;
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_LSB = ecam_addr_low;
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_UDW = ecam_addr_high;
        /* Enumeration starts here */
        for (pcie_bus_num = PCIE_CLEAR; pcie_bus_num < PCIE_CFG_MAX_NUM_OF_BUS; pcie_bus_num++)
        {
            for (pcie_dev_num = PCIE_CLEAR; pcie_dev_num < PCIE_CFG_MAX_NUM_OF_DEV; pcie_dev_num++)
            {
                for (pcie_fun_num = PCIE_CLEAR; pcie_fun_num < PCIE_CFG_MAX_NUM_OF_FUN; pcie_fun_num++)
                {
                    /* Calculate ECAM address based on AXI slave base address and bus, device and fun num */
                    src_ecam_addr = ecam_address_calc(ecam_addr, pcie_bus_num, pcie_dev_num, pcie_fun_num);
                    p_pcie_config_space = (PCIE_END_CONF *)((uintptr_t)src_ecam_addr);
                    /* Read Vendor ID of PCIe RP or EP */
                    pcie_vendor_id = ((uint16_t)((p_pcie_config_space->VID_DEVID) & VENDOR_ID_MASK));
                    /* Check Vendor ID is 0xFFFF then no need search */
                    if (PCIE_CFG_FUN_NOT_IMP_MASK == pcie_vendor_id)
                    {
                        if (PCIE_CLEAR == pcie_fun_num)
                        {
                            /*
                            * don't need to search
                            * further on this device.
                            */
                            break;
                        }
                    }
                    else
                    {
                        /* Header Type */
                        pcie_header_type = p_pcie_config_space->BIST_HEADER & PCIE_CFG_HEADER_TYPE_MASK;
                        pcie_multi_fun = p_pcie_config_space->BIST_HEADER & PCIE_CFG_MUL_FUN_DEV_MASK;
                        /* Check header type is type0 for PCIe EndPoint */
                        if (PCIE_CFG_HEADER_O_TYPE == pcie_header_type)
                        {
                            g_pcie_enumeration.devices[devices_attached].bus_num = pcie_bus_num;
                            g_pcie_enumeration.devices[devices_attached].dev_num = pcie_dev_num;
                            g_pcie_enumeration.devices[devices_attached].fun_num = pcie_fun_num;
                            g_pcie_enumeration.devices[devices_attached].vendor_id = pcie_vendor_id;
                            ++devices_attached;
                            g_pcie_enumeration.no_of_devices_attached = devices_attached;
                            /* Enable config space  memory access bus master and cache size */
                            p_pcie_config_space->CFG_PRMSCR |= (EP_CFG_PRMSCR_DATA);
                            p_pcie_config_space->BIST_HEADER = PCIE_CFG_CATCHE_SIZE;

                            p_pcie_ep_config_space = (uint32_t *)((uintptr_t)src_ecam_addr);
                            /* PCIe configuration header starts after standard size - 0x40/4 = 0x10 */
                            p_pcie_ep_config_space = p_pcie_ep_config_space + 0x10u;

                            for (addr_inc = PCIE_CLEAR; addr_inc < 48u; ++addr_inc)
                            {
                                /* Read Capability ID 10h for PCI Express Capability structure */
                                p_reg = *(p_pcie_ep_config_space + addr_inc);
                                if (PCIE_CAPB_ID_STRUCT == (p_reg & PCIE_CAPB_ID_MASK))
                                {
                                    break;
                                }
                            }
                            if (addr_inc < 48u)
                            {
                                /* Device control and status offset */
                                addr_inc = addr_inc + 2u;
                                /* End Point control and status configure */
                                *(p_pcie_ep_config_space + addr_inc) = EP_DEVICE_CTRL_STATUS_DATA;
                            }
                        }
                        else
                        {
                            /* Header type is Type1
                             * This is a bridge or Root Port
                             */
                            /* Check bus num is '0' for Root Port*/
                            if (PCIE_CLEAR == pcie_bus_num)
                            {
                                p_pcie_root_config = (PCIE_ROOT_CONF *)((uintptr_t)src_ecam_addr);
                                prim_sec_num |= (uint32_t)(((((uint32_t)pcie_bus_num) + PCIE_SET) << 8u) | pcie_bus_num);
                                /* Sub bus num 0xFF, sec bus num 1, and prime bus num 0 */
                                p_pcie_root_config->PRIM_SEC_BUS_NUM = prim_sec_num;
                                /* Control & Status Register: enable bus master,
                                 * Parity Error Response and  SERR#
                                 */
                                p_pcie_root_config->CFG_PRMSCR = RP_CFG_PRMSCR_DATA;
                                /* Non-prefetchable Limit, Non-prefetchable Base */
                                p_pcie_root_config->MEM_LIMIT_BASE = RP_MEM_LIMIT_BASE_DATA;
                                /* Prefetchable Limit, Prefetchable Base */
                                p_pcie_root_config->PREF_MEM_LIMIT_BASE = RP_PREF_MEM_LIMIT_BASE_DATA;
                                /* Prefetchable Base upper */
                                p_pcie_root_config->PREF_BASE_UPPER = RP_PREF_BASE_UPPER_DATA;
                                /* Prefetchable upper limit*/
                                p_pcie_root_config->PREF_LIMIT_UPPER = RP_PREF_LIMIT_UPPER_DATA;
                                p_pcie_root_config->DEVICE_CTRL_STAT = RP_DEVICE_CTRL_STATUS_DATA;
                            }
                            else
                            {
                                /* The PCIe bridge primary and secondary bus setup */
                                prim_sec_num |= (uint32_t)(((((uint32_t)pcie_bus_num) + PCIE_SET) << 8u) | pcie_bus_num);
                                p_pcie_root_config = (PCIE_ROOT_CONF *)((uintptr_t)src_ecam_addr);
                                p_pcie_root_config->PRIM_SEC_BUS_NUM = prim_sec_num;
                                p_pcie_root_config->CFG_PRMSCR = RP_CFG_PRMSCR_DATA;
                            }
                            g_pcie_enumeration.bridges[bridges_attached].bus_num = pcie_bus_num;
                            g_pcie_enumeration.bridges[bridges_attached].dev_num = pcie_dev_num;
                            g_pcie_enumeration.bridges[bridges_attached].fun_num = pcie_fun_num;
                            g_pcie_enumeration.bridges[bridges_attached].vendor_id = pcie_vendor_id;
                            ++bridges_attached;
                            g_pcie_enumeration.no_of_bridges_attached = bridges_attached;
                        }
                        if ((PCIE_CLEAR == pcie_fun_num) && (PCIE_CLEAR == pcie_multi_fun))
                        {
                            /*
                            * If it is function 0 and it is not a
                            * multi function device, don't need
                            * to search further on this device
                            */
                            break;
                        }
                    }
                }
            }
        }
        /* Selects PCIe Tx/RxInterface */
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM  = PCIE_TX_RX_INTERFACE;
    }
    return &g_pcie_enumeration;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
pf_pcie_bar_info_t *
PF_PCIE_allocate_memory
(
    uint64_t ecam_addr,
    uint64_t allocate_addr
)
{
    PCIE_END_CONF * p_pcie_config_space;
    uint32_t pcie_header_type;
    uint64_t axi_trns_base_addr = allocate_addr;
    uint32_t axi_trns_base_addr_low;
    uint32_t axi_trns_base_addr_high;
    uint32_t read_bar, lsb4bits_bar;

    axi_trns_base_addr_low = (uint32_t)(allocate_addr  & MASK_32BIT);
    axi_trns_base_addr_high = (uint32_t)((allocate_addr >> SHIFT_32BIT) & MASK_32BIT);
    /* Default BAR and BAR size is 0 */
    g_pcie_bar_allocate.bar0_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar0_size = PCIE_CLEAR;
    g_pcie_bar_allocate.bar1_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar1_size = PCIE_CLEAR;
    g_pcie_bar_allocate.bar2_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar2_size = PCIE_CLEAR;
    g_pcie_bar_allocate.bar3_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar3_size = PCIE_CLEAR;
    g_pcie_bar_allocate.bar4_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar4_size = PCIE_CLEAR;
    g_pcie_bar_allocate.bar5_address = PCIE_CLEAR;
    g_pcie_bar_allocate.bar5_size = PCIE_CLEAR;
    /* Check PCIe bridge is enabled */
    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        /* Select PCIe Config space */
        g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM  = PCIE_CONFIG_INTERFACE;

        p_pcie_config_space = (PCIE_END_CONF *)((uintptr_t)ecam_addr);
        pcie_header_type = PCIE_CLEAR;
        /* Header Type */
        pcie_header_type = p_pcie_config_space->BIST_HEADER & PCIE_CFG_HEADER_TYPE_MASK;
        /* Write 0xFFFFFFFF to read PCIe BAR0 */
        p_pcie_config_space->BAR0 = BAR_MASK;
        /* Read BAR0 */
        read_bar = p_pcie_config_space->BAR0;
        /* Lower 4-bits of BAR0 */
        lsb4bits_bar = read_bar & BAR_LSB_MASK;
        /* Calculate BAR0 size */
        read_bar &= BAR_SIZE_MASK;
        read_bar = ~(read_bar) + PCIE_SET;
        if (read_bar != PCIE_CLEAR)
        {
            /* Write Translation address in EP BAR0 */
            p_pcie_config_space->BAR0 = axi_trns_base_addr_low | lsb4bits_bar;
            if (ADDR_SPACE_64BIT == (lsb4bits_bar & ADDR_SPACE_64BIT))
            {
                 p_pcie_config_space->BAR1 = axi_trns_base_addr_high;
                 g_pcie_bar_allocate.bar1_address = axi_trns_base_addr_high;
            }
            g_pcie_bar_allocate.bar0_address = axi_trns_base_addr_low;
            g_pcie_bar_allocate.bar0_size = read_bar;
            axi_trns_base_addr = axi_trns_base_addr + read_bar;
        }
         /* Check BAR0 is not 64-bit address space */
        if (ADDR_SPACE_64BIT != (lsb4bits_bar & ADDR_SPACE_64BIT))
        {
            axi_trns_base_addr_low = (uint32_t)(axi_trns_base_addr  & MASK_32BIT);
            axi_trns_base_addr_high = (uint32_t)((axi_trns_base_addr >> SHIFT_32BIT) & MASK_32BIT);

            /* Write 0xFFFFFFFF to read PCIe BAR1 */
             p_pcie_config_space->BAR1 = BAR_MASK;
             /* Read BAR1 */
             read_bar = p_pcie_config_space->BAR1;
             lsb4bits_bar = read_bar & BAR_LSB_MASK;
             /* Calculate BAR1 size */
             read_bar &= BAR_SIZE_MASK;
             read_bar = ~(read_bar) + PCIE_SET;
             if (read_bar != PCIE_CLEAR)
             {
                 /* Write Translation address in EP BAR1 */
                 p_pcie_config_space->BAR1 = axi_trns_base_addr_low | lsb4bits_bar;
                 g_pcie_bar_allocate.bar1_address = axi_trns_base_addr_low;
                 g_pcie_bar_allocate.bar1_size = read_bar;
                 axi_trns_base_addr = axi_trns_base_addr + read_bar;
             }
        }
        if (PCIE_CFG_HEADER_O_TYPE == pcie_header_type)
        {
            axi_trns_base_addr_low = (uint32_t)(axi_trns_base_addr  & MASK_32BIT);
            axi_trns_base_addr_high = (uint32_t)((axi_trns_base_addr >> SHIFT_32BIT) & MASK_32BIT);
             /* Write 0xFFFFFFFF to read PCIe BAR2 */
            p_pcie_config_space->BAR2 = BAR_MASK;
            /* Read BAR2 */
            read_bar = p_pcie_config_space->BAR2;
            lsb4bits_bar = read_bar & BAR_LSB_MASK;

            /* Calculate BAR2 size */
            read_bar &= BAR_SIZE_MASK;
            read_bar = ~(read_bar) + PCIE_SET;
            if (read_bar != PCIE_CLEAR)
            {
                /* Write Translation address in EP BAR2 */
                p_pcie_config_space->BAR2 = axi_trns_base_addr_low | lsb4bits_bar;
               if (ADDR_SPACE_64BIT == (lsb4bits_bar & ADDR_SPACE_64BIT))
                {
                    p_pcie_config_space->BAR3 = axi_trns_base_addr_high;
                    g_pcie_bar_allocate.bar3_address = axi_trns_base_addr_high;
                }
                g_pcie_bar_allocate.bar2_address = axi_trns_base_addr_low;
                g_pcie_bar_allocate.bar2_size = read_bar;
                axi_trns_base_addr = axi_trns_base_addr + read_bar;
            }

            /* Check BAR2 is not 64-bit address space */
            if (ADDR_SPACE_64BIT != (lsb4bits_bar & ADDR_SPACE_64BIT))
            {
                axi_trns_base_addr_low = (uint32_t)(axi_trns_base_addr  & MASK_32BIT);
                axi_trns_base_addr_high = (uint32_t)((axi_trns_base_addr >> SHIFT_32BIT) & MASK_32BIT);
                /* Write 0xFFFFFFFF to read PCIe BAR3 */
                p_pcie_config_space->BAR3 = BAR_MASK;
                /* Read BAR3 */
                read_bar = p_pcie_config_space->BAR3;
                lsb4bits_bar = read_bar & BAR_LSB_MASK;

                /* Calculate BAR3 size */
                read_bar &= BAR_SIZE_MASK;
                read_bar = ~(read_bar) + PCIE_SET;
                if (read_bar != PCIE_CLEAR)
                {
                    /* Write Translation address in EP BAR3 */
                    p_pcie_config_space->BAR3 = axi_trns_base_addr_low | lsb4bits_bar;
                    g_pcie_bar_allocate.bar3_address = axi_trns_base_addr_low;
                    g_pcie_bar_allocate.bar3_size = read_bar;
                    axi_trns_base_addr = axi_trns_base_addr + read_bar;
                }
            }
            axi_trns_base_addr_low = (uint32_t)(axi_trns_base_addr  & MASK_32BIT);
            axi_trns_base_addr_high = (uint32_t)((axi_trns_base_addr >> SHIFT_32BIT) & MASK_32BIT);
            /* Write 0xFFFFFFFF to read PCIe BAR4 */
            p_pcie_config_space->BAR4 = BAR_MASK;
            /* Read BAR4 */
            read_bar = p_pcie_config_space->BAR4;
            lsb4bits_bar = read_bar & BAR_LSB_MASK;

            /* Calculate BAR4 size */
            read_bar &= BAR_SIZE_MASK;
            read_bar = ~(read_bar) + PCIE_SET;
            if (read_bar != PCIE_CLEAR)
            {
                /* Write Translation address in EP BAR4 */
                p_pcie_config_space->BAR4 = axi_trns_base_addr_low | lsb4bits_bar;
               if (ADDR_SPACE_64BIT == (lsb4bits_bar & ADDR_SPACE_64BIT))
                {
                    p_pcie_config_space->BAR5 = axi_trns_base_addr_high;
                    g_pcie_bar_allocate.bar5_address = axi_trns_base_addr_high;
                }
                g_pcie_bar_allocate.bar4_address = axi_trns_base_addr_low;
                g_pcie_bar_allocate.bar4_size = read_bar;
                axi_trns_base_addr = axi_trns_base_addr + read_bar;
            }

            /* Check BAR4 is not 64-bit address space */
            if (ADDR_SPACE_64BIT != (lsb4bits_bar & ADDR_SPACE_64BIT))
            {
                axi_trns_base_addr_low = (uint32_t)(axi_trns_base_addr  & MASK_32BIT);
                axi_trns_base_addr_high = (uint32_t)((axi_trns_base_addr >> SHIFT_32BIT) & MASK_32BIT);
                /* Write 0xFFFFFFFF to read PCIe BAR5 */
                p_pcie_config_space->BAR5 = BAR_MASK;
                /* Read BAR5 */
                read_bar = p_pcie_config_space->BAR5;
                lsb4bits_bar = read_bar & BAR_LSB_MASK;

                /* Calculate BAR5 size */
                read_bar &= BAR_SIZE_MASK;
                read_bar = ~(read_bar) + PCIE_SET;
                if (read_bar != PCIE_CLEAR)
                {
                    /* Write Translation address in EP BAR5 */
                    p_pcie_config_space->BAR5 = axi_trns_base_addr_low | lsb4bits_bar;
                    g_pcie_bar_allocate.bar5_address = axi_trns_base_addr_low;
                    g_pcie_bar_allocate.bar5_size = read_bar;
                    axi_trns_base_addr = axi_trns_base_addr + read_bar;
                }
            }
        }
        /* Selects PCIe Tx/RxInterface */
        g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_TX_RX_INTERFACE;
    }
    return &g_pcie_bar_allocate;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void
PF_PCIE_enable_config_space_msi
(
    uint64_t ecam_addr,
    uint64_t msi_addr,
    uint16_t msi_data
)
{
    uint32_t * p_pcie_config_space;
    uint32_t addr_inc;
    uint32_t p_reg;
    uint32_t msi_addr_low;
    uint32_t msi_addr_high;

    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        msi_addr_low = (uint32_t)(msi_addr & MASK_32BIT);
        msi_addr_high = (uint32_t)((msi_addr >> SHIFT_32BIT) & MASK_32BIT);

        /* Select PCIe Config space */
        g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_CONFIG_INTERFACE;

        p_pcie_config_space = (uint32_t *)((uintptr_t)(ecam_addr & PCIE_CONFIG_CAPB_ID_MASK));

        /* PCIe configuration header starts after standard size - 0x40/4 = 0x10*/
        p_pcie_config_space = p_pcie_config_space + 0x10u;

        for (addr_inc = PCIE_CLEAR; addr_inc < 48u; ++addr_inc)
        {
            /* Read Capability ID 05h for MSI */
            p_reg = *(p_pcie_config_space + addr_inc);
            if (PCIE_CAPB_ID_MSI == (p_reg & PCIE_CAPB_ID_MASK))
            {
                break;
            }
        }
        if (addr_inc < 48u)
        {
            p_reg = *(p_pcie_config_space + addr_inc);

            /* Read number of requested vectors(multiple messages capable) */
            p_reg = (p_reg >> PCIE_MSI_MULTI_VECTOR_SHIFT) & PCIE_MSI_MULTI_VECTOR_MASK;

            /* Numbers of allocated vectors (multiple messages enable) */
            p_reg = p_reg << PCIE_MSI_MULTI_ENABLE_SHIFT;

            /* Enable MSI */
            *(p_pcie_config_space + addr_inc) |= (p_reg | PCIE_ENABLE_MSI);
            p_reg = *(p_pcie_config_space + addr_inc);
            ++addr_inc;

            /* MSI message lower address */
            *(p_pcie_config_space + addr_inc) = msi_addr_low;
            ++addr_inc;

            /* MSI mesage upper address */
            if (PCIE_MSI_64BIT_CAPB_ADDR == (p_reg & PCIE_MSI_64BIT_CAPB_ADDR))
            {
                *(p_pcie_config_space + addr_inc) = msi_addr_high;
                ++addr_inc;
            }
            /* MSI message data */
            *(p_pcie_config_space + addr_inc) = msi_data;
        }
        /* Selects PCIe Tx/RxInterface */
        g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_TX_RX_INTERFACE;
    }

}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 * PCIe reads type1 configuration space of pcie bridge or switch.
 */
void
PF_PCIE_type1_header_read
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr,
    PCIE_ROOT_CONF * p_type1_header
)
{
    PCIE_BRIDGE * p_pcie_bridge;
    PCIE_ROOT_CONF * p_pcie_type1_header;

    uint32_t pcie_header_type;
    uint32_t ecam_addr_low;
    uint32_t ecam_addr_high;

    if (PF_PCIE_CTRL_0 == pcie_ctrl_num)
    {
        p_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET)));
    }
    else if (PF_PCIE_CTRL_1 == pcie_ctrl_num)
    {
        p_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET)));
    }
    else
    {
        p_pcie_bridge = NULL_POINTER;
    }

    /* Check Root Port */
    if ((NULL_POINTER != p_pcie_bridge) &&
        (ROOT_PORT_ENABLE == (p_pcie_bridge->GEN_SETTINGS & ROOT_PORT_ENABLE)))
    {
        ecam_addr_low = (uint32_t)(ecam_addr & MASK_32BIT);
        ecam_addr_high = (uint32_t)((ecam_addr >> SHIFT_32BIT) & MASK_32BIT);

        /* Selects PCIe Config space */
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_CONFIG_INTERFACE;

        p_pcie_bridge->ATR0_AXI4_SLV0_SRCADDR_PARAM = ecam_addr_low | SIZE_256MB_TRANSLATE_TABLE_EN ;
        p_pcie_bridge->ATR0_AXI4_SLV0_SRC_ADDR = ecam_addr_high;
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_LSB = ecam_addr_low;
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_UDW = ecam_addr_high;

        p_pcie_type1_header = (PCIE_ROOT_CONF *)((uintptr_t)ecam_addr);
        /* Header Type */
        pcie_header_type = p_pcie_type1_header->BIST_HEADER & PCIE_CFG_HEADER_TYPE_MASK;
        if (PCIE_CFG_HEADER_O_TYPE != pcie_header_type)
        {
            p_type1_header->VID_DEVID = p_pcie_type1_header->VID_DEVID;
            p_type1_header->CFG_PRMSCR = p_pcie_type1_header->CFG_PRMSCR;
            p_type1_header->CLASS_CODE = p_pcie_type1_header->CLASS_CODE;
            p_type1_header->BIST_HEADER = p_pcie_type1_header->BIST_HEADER;
            p_type1_header->BAR0 = p_pcie_type1_header->BAR0;
            p_type1_header->BAR1 = p_pcie_type1_header->BAR1;
            p_type1_header->PRIM_SEC_BUS_NUM = p_pcie_type1_header->PRIM_SEC_BUS_NUM;
            p_type1_header->IO_LIMIT_BASE = p_pcie_type1_header->IO_LIMIT_BASE;
            p_type1_header->MEM_LIMIT_BASE = p_pcie_type1_header->MEM_LIMIT_BASE;
            p_type1_header->PREF_MEM_LIMIT_BASE = p_pcie_type1_header->PREF_MEM_LIMIT_BASE;
            p_type1_header->PREF_BASE_UPPER = p_pcie_type1_header->PREF_BASE_UPPER;
            p_type1_header->PREF_LIMIT_UPPER = p_pcie_type1_header->PREF_LIMIT_UPPER;
            p_type1_header->IO_LIMIT_BASE_UPPER = p_pcie_type1_header->IO_LIMIT_BASE_UPPER;
            p_type1_header->CAPAB_POINTER = p_pcie_type1_header->CAPAB_POINTER;
            p_type1_header->EXPAN_ROM_BASE = p_pcie_type1_header->EXPAN_ROM_BASE;
            p_type1_header->INT_LINE_PIN = p_pcie_type1_header->INT_LINE_PIN;
        }
        /* Selects PCIe Tx/RxInterface */
        p_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_TX_RX_INTERFACE;
    }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
uint8_t
PF_PCIE_config_space_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    uint64_t ecam_addr
)
{
    uint8_t returnval = PF_PCIE_ATR_TABLE_INIT_SUCCESS;
    uint32_t ecam_addr_low;
    uint32_t ecam_addr_high;

    if (NULL_POINTER == g_rp_pcie_bridge)
    {
        if (PF_PCIE_CTRL_0 == pcie_ctrl_num)
        {
            g_rp_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET)));
            g_rp_pcie_ctrl = ((PCIE_CTRL *)((uintptr_t)(apb_addr + PCIE0_CRTL_PHY_ADDR_OFFSET)));
        }
        else if (PF_PCIE_CTRL_1 == pcie_ctrl_num)
        {
            g_rp_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET)));
            g_rp_pcie_ctrl = ((PCIE_CTRL *)((uintptr_t)(apb_addr + PCIE1_CRTL_PHY_ADDR_OFFSET)));
        }
        else
        {
            g_rp_pcie_bridge = NULL_POINTER;
            returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
        }

        if (NULL_POINTER != g_rp_pcie_bridge)
        {
            /* Clear interrupts on PCIe RootPort */
            g_rp_pcie_ctrl->ECC_CONTROL = PCIE_ECC_DISABLE;
            g_rp_pcie_ctrl->PCIE_EVENT_INT = PCIE_EVENT_INT_DATA;
            g_rp_pcie_ctrl->SEC_ERROR_INT = PCIE_SEC_ERROR_INT_CLEAR;
            g_rp_pcie_ctrl->DED_ERROR_INT = PCIE_DED_ERROR_INT_CLEAR;

            g_rp_pcie_bridge->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
            g_rp_pcie_bridge->IMASK_LOCAL = PCIE_CLEAR;
            g_rp_pcie_bridge->ISTATUS_HOST = PCIE_ISTATUS_CLEAR;
            g_rp_pcie_bridge->IMASK_HOST = PCIE_CLEAR;
        }
    }

    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        /* Check Root Port */
        if (ROOT_PORT_ENABLE == (g_rp_pcie_bridge->GEN_SETTINGS & ROOT_PORT_ENABLE))
        {
            ecam_addr_low = (uint32_t)(ecam_addr & MASK_32BIT);
            ecam_addr_high = (uint32_t)((ecam_addr >> SHIFT_32BIT) & MASK_32BIT);
            /* Selects PCIe Config space */
            g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM  = PCIE_CONFIG_INTERFACE;

            g_rp_pcie_bridge->ATR0_AXI4_SLV0_SRCADDR_PARAM = ecam_addr_low |
                                                       SIZE_256MB_TRANSLATE_TABLE_EN;
            g_rp_pcie_bridge->ATR0_AXI4_SLV0_SRC_ADDR = ecam_addr_high;
            g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_LSB = ecam_addr_low;
            g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_ADDR_UDW = ecam_addr_high;
        }
        else
        {
            returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
        }
    }
    return returnval;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_config_space_atr_table_terminate(void)
{
    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        /* Selects PCIe Tx/Rx Interface, disable PCIe Config space */
        g_rp_pcie_bridge->ATR0_AXI4_SLV0_TRSL_PARAM = PCIE_TX_RX_INTERFACE;
     }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 * PCIe reads configuration space of end point or root port.
 */
void
PF_PCIE_config_space_read
(
    uint64_t ecam_addr,
    uint16_t config_space_offset,
    uint32_t * value
)
{
    volatile uint32_t * cfg_reg = (uint32_t *)((uintptr_t)(ecam_addr + config_space_offset));
    *value = *(cfg_reg);
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 * PCIe writes configuration space of end point or root port.
 */
void
PF_PCIE_config_space_write
(
    uint64_t ecam_addr,
    uint16_t config_space_offset,
    uint32_t value
)
{
    volatile uint32_t * cfg_reg = (uint32_t *)((uintptr_t)(ecam_addr + config_space_offset));
    *(cfg_reg) = value;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
uint8_t
PF_PCIE_master_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    pf_pcie_master_atr_cfg_t * cfg,
    uint8_t master_table_num,
    uint8_t tlp_type
)
{
    uint32_t * p_pcie_master_table;
    uint32_t * p_pcie_bar;
    PCIE_BRIDGE * p_pcie_bridge;
    uint32_t phy_reg;
    uint8_t returnval = PF_PCIE_ATR_TABLE_INIT_SUCCESS;

    /* Set pcie bridge base address for ATR master table based on pcie controller 0/1 */
    if (PF_PCIE_CTRL_0 == pcie_ctrl_num)
    {
        p_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET)));
        p_pcie_master_table = ((uint32_t *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET +
                            ATR0_PCIE_WIN0_OFFSET + (master_table_num * ATR_WIN_REG_TABLE_SIZE))));

        if (cfg->bar_type == PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM)
        {
            p_pcie_bar = ((uint32_t *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET + BAR_01_DW0 + (master_table_num * 8u))));
        }
        else
        {
            p_pcie_bar  = ((uint32_t *)((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET + BAR_01_DW0 + (master_table_num * 4u))));
        }
    }
    else if (PF_PCIE_CTRL_1 == pcie_ctrl_num)
    {
        p_pcie_bridge = ((PCIE_BRIDGE *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET)));
        p_pcie_master_table = ((uint32_t *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET +
                            ATR0_PCIE_WIN0_OFFSET + (master_table_num * ATR_WIN_REG_TABLE_SIZE))));

        if (cfg->bar_type == PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM)
        {
            p_pcie_bar = ((uint32_t *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET + BAR_01_DW0 + (master_table_num * 8u))));
        }
        else
        {
            p_pcie_bar = ((uint32_t *)((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET + BAR_01_DW0 + (master_table_num * 4u))));
        }
    }
    else
    {
        p_pcie_bridge = NULL_POINTER;
    }

    /* Check for pcie bridge controller base address */
    if (NULL_POINTER != p_pcie_bridge)
    {
        /* Check for PCIe Root Port */
        if (ROOT_PORT_ENABLE == (p_pcie_bridge->GEN_SETTINGS & ROOT_PORT_ENABLE))
        {
            if (master_table_num < 2u)
            {
                /* Write BAR_01 or BAR_23 size and bar type */
                phy_reg = ~((1u << (cfg->bar_size + 1u)) -1u);
                *p_pcie_bar = (phy_reg | cfg->bar_type);

                if (cfg->bar_type == PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM)
                {
                      /* Bar size mask in BAR_01/23_DW1 [64:32] bit */
                    *(p_pcie_bar + 1u) = BAR_MASK;
                }
            }
            else
            {
                returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
            }
        }
        else /* End Point */
        {
            if (master_table_num < 6u)
            {
                /* Write BAR_01 or BAR_23 or BAR_45 size and bar type */
                phy_reg = ~((1u << (cfg->bar_size + 1u)) -1u);
                *p_pcie_bar = (phy_reg | cfg->bar_type);

                if (cfg->bar_type == PF_PCIE_BAR_TYPE_64BIT_PREFET_MEM)
                {
                      /* Bar size mask in BAR_01/23/45_DW1 [64:32] bit */
                    *(p_pcie_bar + 1u) = BAR_MASK;
                }
            }
            else
            {
                returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
            }
        }

        if (PF_PCIE_ATR_TABLE_INIT_SUCCESS == returnval)
        {
            phy_reg =  (uint32_t)(cfg->src_addr & ATR_ADDR_MASK);
            phy_reg |= (uint32_t)((cfg->table_size) << PCIE_SET);
            phy_reg |=  (uint32_t)(cfg->state);

            /* Set ATR Master SRC LSB address */
            *(p_pcie_master_table + WIN0_SRCADDR_PARAM) = phy_reg;
            /* Set ATR Master SRC MSB address */
            *(p_pcie_master_table + WIN0_SRC_ADDR) = (cfg->src_addr_msb);
            /* Set ATR Master TRNSL LSB address */
            *(p_pcie_master_table + WIN0_TRSL_ADDR_LSB) = (cfg->trns_addr & ATR_ADDR_MASK);
            /* Set ATR Master TRNSL MSB address */
            *(p_pcie_master_table + WIN0_TRSL_ADDR_UDW) = (cfg->trns_addr_msb);
            /* Set TLP type and TRSL_ID value */
            *(p_pcie_master_table + WIN0_TRSL_PARAM) = ((uint32_t)tlp_type << TLP_SHIFT) | AXI4_MASTER_0_INTERFACE;
        }
    }
    else
    {
        returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
    }

    return returnval;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
uint8_t
PF_PCIE_slave_atr_table_init
(
    uint64_t apb_addr,
    uint8_t pcie_ctrl_num,
    pf_pcie_slave_atr_cfg_t * cfg,
    uint8_t slave_table_num,
    uint8_t tlp_type
)
{
    uint32_t * p_pcie_slave;
    uint32_t phy_reg = PCIE_CLEAR;
    uint8_t returnval = PF_PCIE_ATR_TABLE_INIT_SUCCESS;

    /* Set pcie bridge base address for ATR slave table based on pcie controlle 0/1 */
    if (PF_PCIE_CTRL_0 == pcie_ctrl_num)
    {
        p_pcie_slave = ((uint32_t * )((uintptr_t)(apb_addr + PCIE0_BRIDGE_PHY_ADDR_OFFSET +
                            ATR0_AXI4_SLV0_OFFSET + (slave_table_num * ATR_SLV_REG_TABLE_SIZE))));
    }
    else if (PF_PCIE_CTRL_1 == pcie_ctrl_num)
    {
        p_pcie_slave = ((uint32_t * )((uintptr_t)(apb_addr + PCIE1_BRIDGE_PHY_ADDR_OFFSET +
                            ATR0_AXI4_SLV0_OFFSET + (slave_table_num * ATR_SLV_REG_TABLE_SIZE))));
    }
    else
    {
        p_pcie_slave = NULL_POINTER;
    }

   /* Check for pcie bridge controller base address */
    if (NULL_POINTER != p_pcie_slave)
    {
        phy_reg = (uint32_t)(cfg->src_addr & ATR_ADDR_MASK);
        phy_reg |= (uint32_t)((cfg->size) << PCIE_SET);
        phy_reg |= (uint32_t)(cfg->state);
        /* Set ATR slave SRC LSB address */
        *(p_pcie_slave + SLV0_SRCADDR_PARAM) = phy_reg;
        /* Set ATR slave SRC MSB address */
        *(p_pcie_slave + SLV0_SRC_ADDR) = (cfg->src_addr_msb);
        /* Set ATR slave TRNSL LSB address */
        phy_reg = (cfg->trns_addr & ATR_ADDR_MASK);
        *(p_pcie_slave + SLV0_TRSL_ADDR_LSB) = phy_reg;
        /* Set ATR slave TRNSL LSB address */
        *(p_pcie_slave + SLV0_TRSL_ADDR_UDW) = (cfg->trns_addr_msb);
        /* Set TLP type */
        *(p_pcie_slave + SLV0_TRSL_PARAM) = ((uint32_t)tlp_type << TLP_SHIFT) | PCIE_TX_RX_INTERFACE;
    }
    else
    {
        returnval = PF_PCIE_ATR_TABLE_INIT_FAILURE;
    }
    return returnval;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_dma_init(uint64_t  allocated_addr)
{
    g_ep_bridge_reg = (PCIE_BRIDGE *)((uintptr_t)allocated_addr);
    g_ep_ctrl_reg = (PCIE_CTRL *)((uintptr_t)(allocated_addr + 0x2000u));

    /* Disable EP ECC interrupts and clear status bits */
    g_ep_ctrl_reg->ECC_CONTROL = PCIE_ECC_DISABLE;
    g_ep_ctrl_reg->PCIE_EVENT_INT = PCIE_EVENT_INT_DATA;
    g_ep_ctrl_reg->SEC_ERROR_INT = PCIE_SEC_ERROR_INT_CLEAR;
    g_ep_ctrl_reg->DED_ERROR_INT = PCIE_DED_ERROR_INT_CLEAR;

    /* Disable and clear  Local and Host interrupts on EP */
    g_ep_bridge_reg->IMASK_LOCAL = PCIE_CLEAR;
    g_ep_bridge_reg->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
    g_ep_bridge_reg->IMASK_HOST = PCIE_CLEAR;
    g_ep_bridge_reg->ISTATUS_HOST = PCIE_ISTATUS_CLEAR;

    /* Enable PCIe Host MSI, INTx DMAx interrupts on EP */
    g_ep_bridge_reg->IMASK_HOST= PCIE_HOST_INT_ENABLE;
    g_ep_bridge_reg->ISTATUS_HOST = PCIE_ISTATUS_CLEAR;

    /* initialize default interrupt handlers */
    g_pcie_dma.tx_complete_handler = NULL_POINTER;
    g_pcie_dma.rx_complete_handler = NULL_POINTER;

    g_pcie_dma.state = PF_PCIE_EP_DMA_COMPLETED;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void
PF_PCIE_set_dma_write_callback
(
    pf_pcie_write_callback_t write_callback
)
{
    g_pcie_dma.tx_complete_handler = write_callback;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void
PF_PCIE_set_dma_read_callback
(
    pf_pcie_read_callback_t rx_callback
)
{
    g_pcie_dma.rx_complete_handler = rx_callback;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_dma_abort(void)
{
    if(NULL_POINTER != g_ep_bridge_reg)
    {
        g_ep_bridge_reg->DMA0_CONTROL = PCIE_CLEAR;
        g_ep_bridge_reg->DMA1_CONTROL = PCIE_CLEAR;
    }
    g_pcie_dma.state = PF_PCIE_EP_DMA_COMPLETED;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void
PF_PCIE_dma_read
(
    uint64_t src_address,
    uint64_t dest_address,
    uint32_t rx_lenth
)
{
    /* Check EP bridge access enabled with DMA */
    if (NULL_POINTER != g_ep_bridge_reg)
    {
        if ((PF_PCIE_EP_DMA_IN_PROGRESS != g_pcie_dma.state) && (rx_lenth > 0u))
        {
            g_ep_bridge_reg->DMA1_CONTROL = PCIE_CLEAR;
            /* DMA from EP to RP - source EP AXI-Master, destination PCIe - DMA1 */
            /* AXI4-Master Interface for Source*/
            g_ep_bridge_reg->DMA1_SRC_PARAM = EP_DMA_INTERFACE_AXI;
            /* PCIe Interface for Destination */
            g_ep_bridge_reg->DMA1_DESTPARAM = EP_DMA_INTERFACE_PCIE;
            /* Set source address */
            g_ep_bridge_reg->DMA1_SRCADDR_LDW = (uint32_t)(src_address & MASK_32BIT);
            g_ep_bridge_reg->DMA1_SRCADDR_UDW = (uint32_t)((src_address >> SHIFT_32BIT) & MASK_32BIT);
            /* Set destination address*/
            g_ep_bridge_reg->DMA1_DESTADDR_LDW = (uint32_t)(dest_address & MASK_32BIT);
            g_ep_bridge_reg->DMA1_DESTADDR_UDW = (uint32_t)((dest_address >> SHIFT_32BIT) & MASK_32BIT);
            /* Set dma size */
            g_ep_bridge_reg->DMA1_LENGTH = rx_lenth;
            /*Start dma transaction */
            g_ep_bridge_reg->DMA1_CONTROL = EP_DMA_START_DATA;

            g_pcie_dma.state = PF_PCIE_EP_DMA_IN_PROGRESS;
        }
    }
    else
    {
        g_pcie_dma.state = PF_PCIE_EP_DMA_NOT_INITIALIZED;
    }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void
PF_PCIE_dma_write
(
    uint64_t src_address,
    uint64_t dest_address,
    uint32_t tx_lenth
)
{
    /* Check EP bridge access enabled with DMA */
    if (NULL_POINTER != g_ep_bridge_reg)
    {
        if ((PF_PCIE_EP_DMA_IN_PROGRESS != g_pcie_dma.state)  && (tx_lenth > 0u))
        {
            g_ep_bridge_reg->DMA0_CONTROL = PCIE_CLEAR;
            /* DMA from RP to EP - source RP-PCIe, destination AXI-Master - DMA0 */
            /* PCIe Interface for Source */
            g_ep_bridge_reg->DMA0_SRC_PARAM = EP_DMA_INTERFACE_PCIE;
            /*AXI4-Master Interface for Destination*/
            g_ep_bridge_reg->DMA0_DESTPARAM = EP_DMA_INTERFACE_AXI;
            /* Set source address */
            g_ep_bridge_reg->DMA0_SRCADDR_LDW = (uint32_t)(src_address & MASK_32BIT);
            g_ep_bridge_reg->DMA0_SRCADDR_UDW = (uint32_t)((src_address >> SHIFT_32BIT) & MASK_32BIT);
            /* Set destination address*/
            g_ep_bridge_reg->DMA0_DESTADDR_LDW = (uint32_t)(dest_address & MASK_32BIT);
            g_ep_bridge_reg->DMA0_DESTADDR_UDW = (uint32_t)((dest_address >> SHIFT_32BIT) & MASK_32BIT);
            /* Set dma size */
            g_ep_bridge_reg->DMA0_LENGTH = tx_lenth;
            /*Start dma transaction */
            g_ep_bridge_reg->DMA0_CONTROL = EP_DMA_START_DATA;

            g_pcie_dma.state = PF_PCIE_EP_DMA_IN_PROGRESS;
        }
    }
    else
    {
        g_pcie_dma.state = PF_PCIE_EP_DMA_NOT_INITIALIZED;
    }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
pf_pcie_ep_dma_status_t
PF_PCIE_dma_get_transfer_status
(
    void
)
{
    return g_pcie_dma.state;
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_enable_interrupts(void)
{
    /* Check RP bridge access enabled */
    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        g_rp_pcie_bridge->IMASK_LOCAL = PCIE_LOCAL_INT_ENABLE;
        g_rp_pcie_bridge->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
        g_rp_pcie_bridge->ISTATUS_MSI = PCIE_ISTATUS_CLEAR;
    }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_disable_interrupts(void)
{
    /* Check RP bridge access enabled */
    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        g_rp_pcie_bridge->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
        g_rp_pcie_bridge->IMASK_LOCAL= PCIE_CLEAR;
    }
}

/**************************************************************************//**
 * See pf_pciess.h for details of how to use this function.
 *
 */
void PF_PCIE_isr(void)
{
    uint32_t phy_reg;
    /* Check RP bridge access enabled */
    if (NULL_POINTER != g_rp_pcie_bridge)
    {
        phy_reg = g_rp_pcie_bridge->ISTATUS_LOCAL;
        phy_reg = g_rp_pcie_bridge->ISTATUS_MSI;

        /* Check EP bridge access enabled with DMA */
        if (NULL_POINTER != g_ep_bridge_reg)
        {
            phy_reg = g_ep_bridge_reg->ISTATUS_HOST;

            /* Check EP DMA0/1 interrupt occurred */
            if (PCIE_CLEAR != (phy_reg & DMA_INT_STATUS))
            {
                g_ep_bridge_reg->ISTATUS_HOST = DMA_INT_STATUS;
                g_pcie_dma.state = PF_PCIE_EP_DMA_COMPLETED;

                if (NULL_POINTER != g_pcie_dma.tx_complete_handler)
                {
                    g_pcie_dma.tx_complete_handler(PF_PCIE_EP_DMA_COMPLETED);
                }
            }
            else if (PCIE_CLEAR != (phy_reg & DMA_ERR_STATUS))
            {
                g_ep_bridge_reg->ISTATUS_HOST = DMA_ERR_STATUS;
                g_pcie_dma.state = PF_PCIE_EP_DMA_ERROR;

                if (NULL_POINTER != g_pcie_dma.rx_complete_handler)
                {
                    g_pcie_dma.rx_complete_handler(PF_PCIE_EP_DMA_ERROR);
                }
            }
            else
            {
                g_ep_bridge_reg->ISTATUS_HOST = PCIE_ISTATUS_CLEAR;
            }
        }
        g_rp_pcie_bridge->ISTATUS_MSI = PCIE_ISTATUS_CLEAR;
        g_rp_pcie_bridge->ISTATUS_LOCAL = PCIE_ISTATUS_CLEAR;
    }
}

/****************************************************************************

 Compose an address to be written to configuration address port

 @param    Bus is the external PCIe function's Bus number.
 @param    Device is the external PCIe function's Device number.
 @param    Function is the external PCIe function's Function number.
 @return   32 bit composed value (address).

*/
uint64_t
ecam_address_calc
(
    uint64_t axi_addr,
    uint8_t bus,
    uint8_t device,
    uint8_t function
)
{
    uint64_t location = PCIE_CLEAR;

    location |= ((((uint64_t)bus) << PCIE_ECAM_BUS_SHIFT) & PCIE_ECAM_BUS_MASK);
    location |= ((((uint64_t)device) << PCIE_ECAM_DEV_SHIFT) & PCIE_ECAM_DEV_MASK);
    location |= ((((uint64_t)function) << PCIE_ECAM_FUN_SHIFT) & PCIE_ECAM_FUN_MASK);

    location &= PCIE_ECAM_MASK;
    axi_addr = axi_addr + location;
    return axi_addr;
}

#ifdef __cplusplus
}
#endif
