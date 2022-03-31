 /*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit offsets and masks definitions for PolarFire SoC(MPFS) MSS QSPI.
 *
 *
 */
#ifndef MSS_QSPI_REGS_H_
#define MSS_QSPI_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 Register Bit definitions
 */
#define CTRL_EN                                 0u
#define CTRL_XIP                                2u
#define CTRL_XIPADDR                            3u
#define CTRL_CLKIDL                             10u
#define CTRL_SAMPLE                             11u
#define CTRL_QMODE0                             13u
#define CTRL_QMODE12                            14u
#define CTRL_FLAGSX4                            16u
#define CTRL_CLKRATE                            24u

#define CTRL_EN_MASK                            (0x1u << CTRL_EN)
#define CTRL_XIP_MASK                           (0x1u << CTRL_XIP)
#define CTRL_XIPADDR_MASK                       (0x1u << CTRL_XIPADDR)
#define CTRL_CLKIDL_MASK                        (0x1u << CTRL_CLKIDL)
#define CTRL_SAMPLE_MASK                        (0x3u << CTRL_SAMPLE)
#define CTRL_QMODE0_MASK                        (0x1u << CTRL_QMODE0)
#define CTRL_QMODE12_MASK                       (0x3u << CTRL_QMODE12)
#define CTRL_FLAGSX4_MASK                       (0x1u << CTRL_FLAGSX4)
#define CTRL_CLKRATE_MASK                       (0xFu << CTRL_CLKRATE)

#define CTRL_SAMPLE_SCK                        (0x0u << CTRL_SAMPLE)
#define CTRL_SAMPLE_HCLKF                      (0x1u << CTRL_SAMPLE)
#define CTRL_SAMPLE_HCLKR                      (0x2u << CTRL_SAMPLE)

#define FRMS_TBYTES                             0u
#define FRMS_CBYTES                             16u
#define FRMS_QSPI                               25u
#define FRMS_IDLE                               26u
#define FRMS_FBYTE                              30u
#define FRMS_FWORD                              31u

#define FRMS_TBYTES_MASK                        (0xFFFFu << FRMS_TBYTES)
#define FRMS_CBYTES_MASK                        (0x1FFu << FRMS_CBYTES)
#define FRMS_QXIP_MASK                          (0x1u << FRMS_QXIP)
#define FRMS_IDLE_MASK                          (0xFu << FRMS_IDLE)
#define FRMS_FBYTE_MASK                         (0x1u << FRMS_FBYTE)
#define FRMS_FWORD_MASK                         (0x1u << FRMS_FWORD)

#define INTE_TDONE                              0u
#define INTE_RDONE                              1u
#define INTE_RAVLB                              2u
#define INTE_TAVLB                              3u
#define INTE_RFEMPTY                            4u
#define INTE_TFFULL                             5u

#define INTE_TDONE_MASK                         (0x1u << INTE_TDONE)
#define INTE_RDONE_MASK                         (0x1u << INTE_RDONE)
#define INTE_RAVLB_MASK                         (0x1u << INTE_RAVLB)
#define INTE_TAVLB_MASK                         (0x1u << INTE_TAVLB)
#define INTE_RFEMPTY_MASK                       (0x1u << INTE_RFEMPTY)
#define INTE_TFFULL_MASK                        (0x1u << INTE_TFFULL)


#define STTS_TDONE                              0u
#define STTS_RDONE                              1u
#define STTS_RAVLB                              2u
#define STTS_TAVLB                              3u
#define STTS_RFEMPTY                            4u
#define STTS_TFFULL                             5u
#define STTS_READY                              7u
#define STTS_FLAGSX4                            8u

#define STTS_TDONE_MASK                         (0x1u << STTS_TDONE)
#define STTS_RDONE_MASK                         (0x1u << STTS_RDONE)
#define STTS_RAVLB_MASK                         (0x1u << STTS_RAVLB)
#define STTS_TAVLB_MASK                         (0x1u << STTS_TAVLB)
#define STTS_RFEMPTY_MASK                       (0x1u << STTS_RFEMPTY)
#define STTS_TFFULL_MASK                        (0x1u << STTS_TFFULL)
#define STTS_READY_MASK                         (0x1u << STTS_READY)
#define STTS_FLAGSX4_MASK                       (0x1u << STTS_FLAGSX4)


#define RDAT                                    0u

#define RDAT_MASK                               0xFFu

#define TDAT                                    0u

#define TDAT_MASK                               0xFFu

#define X4RDAT                                  0u

#define X4RDAT_MASK                             0xFFFFFFFFu

#define X4TDAT                                  0u

#define X4TDAT_MASK                             0xFFFFFFFFu

#define DIRECT_EN_SSEL                          0u
#define DIRECT_OP_SSEL                          1u
#define DIRECT_EN_SCLK                          2u
#define DIRECT_OP_SCLK                          3u
#define DIRECT_EN_SDO                           4u
#define DIRECT_OP_SDO                           8u
#define DIRECT_OP_SDOE                          12u
#define DIRECT_IP_SDI                           16u
#define DIRECT_Reserved                         20u
#define DIRECT_IP_SCLK                          21u
#define DIRECT_IP_SSEL                          22u
#define DIRECT_IDLE                             23u

#define DIRECT_EN_SSEL_MASK                     (0x1u << DIRECT_EN_SSEL)
#define DIRECT_OP_SSEL_MASK                     (0x1u << DIRECT_OP_SSEL)
#define DIRECT_EN_SCLK_MASK                     (0x1u << DIRECT_EN_SCLK)
#define DIRECT_OP_SCLK_MASK                     (0x1u << DIRECT_OP_SCLK)
#define DIRECT_EN_SDO_MASK                      (0xFu << DIRECT_EN_SDO)
#define DIRECT_OP_SDO_MASK                      (0xFu << DIRECT_OP_SDO)
#define DIRECT_OP_SDOE_MASK                     (0xFu << DIRECT_OP_SDOE)
#define DIRECT_IP_SDI_MASK                      (0xFu << DIRECT_IP_SDI)
#define DIRECT_Reserved_MASK                    (0x1u << DIRECT_Reserved)
#define DIRECT_IP_SCLK_MASK                     (0x1u << DIRECT_IP_SCLK)
#define DIRECT_IP_SSEL_MASK                     (0x1u << DIRECT_IP_SSEL)
#define DIRECT_IDLE_MASK                        (0x1u << DIRECT_IDLE)


#define UADDAR                                  0u
#define UADDAR_MASK                             0xFFu

#ifdef __cplusplus
}
#endif

#endif /* MSS_QSPI_REGS_H_ */
