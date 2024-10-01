/*******************************************************************************
 * Copyright 2019-2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Health Monitor State Machine
 * \brief E51-Assisted Health Monitor
 */

#include "config.h"
#include "hss_types.h"
#include "healthmon_service.h"

const struct HealthMonitor monitors[] =
{
    { "IOSCB_PLL_MSS:PLL_CTRL",		0x3E001004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 1u },
    { "IOSCB_PLL_DDR:PLL_CTRL",		0x3E010004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 1u },
    { "IOSCB_PLL_SGMII:PLL_CTRL",	0x3E001004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 1u },
    { "IOSCB_PLL:pll_nw_0:PLL_CTRL",	0x38100004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 1u },
    //{ "IOSCB_PLL:pll_se_0:PLL_CTRL",	0x38010004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_se_1:PLL_CTRL",	0x38020004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_0:PLL_CTRL",	0x38040004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_1:PLL_CTRL",	0x38080004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_nw_1:PLL_CTRL",	0x38200004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_0:PLL_CTRL",	0x38400004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_1:PLL_CTRL",	0x38800004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },

    { "SYSREG:BOOT_FAIL_CR",            0x20002014, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  1u,       healthmon_nop_trigger, 5u },
    { "SYSREG:DEVICE_STATUS",           0x20002024, NOT_EQUAL_TO_VALUE, 0x1F09u, 0u, 0u,  0x1FFF,   healthmon_nop_trigger, 5u },
    { "SYSREG:MPU_VIOLATION_SR",        0x200020F0, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  1u,       healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_SR",                 0x20002100, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  0x3FFF,   healthmon_nop_trigger, 5u },

    { "SYSREG:EDAC_CNT_MMC",            0x20002108, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_DDRC",           0x2000210C, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_MAC0",           0x20002110, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_MAC1",           0x20002114, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_USB",            0x20002118, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_CAN0",           0x2000211c, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_CAN1",           0x20002120, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     healthmon_nop_trigger, 5u },

    { "SYSREG:MAINTENANCE_INT_SR",      0x20002148, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1FFFFF, healthmon_nop_trigger, 5u },// [20:0] == some cleared by writing 1, some y writing to PLL_STATUS
    { "SYSREG:PLL_STATUS_SR",           0x2000214c, NOT_EQUAL_TO_VALUE, 0x707u,  0u, 0u,  0x7FF,    healthmon_nop_trigger, 5u },
    { "SYSREG:MISC_SR",                 0x20002154, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  2u,       healthmon_nop_trigger, 5u },
    { "SYSREG:DLL_STATUS_SR",           0x2000215c, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1FFFFF, healthmon_nop_trigger, 5u },

    { "IOSCBCFG:STATUS",                0x37080004, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  0xEu,     healthmon_nop_trigger, 5u },// [3:1] => scb_buserr, timeout, scb_error

    // unknown what the following should be...
    { "IOSCB_PLL:pll_se_0:PLL_CTRL",	0x38010004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_se_1:PLL_CTRL",	0x38020004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_ne_0:PLL_CTRL",	0x38040004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_ne_1:PLL_CTRL",	0x38080004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_nw_1:PLL_CTRL",	0x38200004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_sw_0:PLL_CTRL",	0x38400004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },
    { "IOSCB_PLL:pll_sw_1:PLL_CTRL",	0x38800004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       healthmon_nop_trigger, 5u },

    { "L2:Config:ECCDirFixCount",       0x02010108, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, healthmon_nop_trigger, 1u },
    { "L2:Config:ECCDataFixCount",      0x02010148, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, healthmon_nop_trigger, 1u },
    { "L2:Config:ECCDataFailCount",     0x02010168, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, healthmon_nop_trigger, 1u },
};

struct HealthMonitor_Status monitor_status[ARRAY_SIZE(monitors)] =
{
   { 0u, 0u, 0u, false }
};

const size_t monitors_array_size = ARRAY_SIZE(monitors);
