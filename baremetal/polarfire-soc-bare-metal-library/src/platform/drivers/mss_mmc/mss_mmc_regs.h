 /*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register definitions of the PolarFire SoC MSS eMMC SD.
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */

#ifndef __MSS_MMC_REGS_H_
#define __MSS_MMC_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* PolarFire SoC MSS eMMC/SD/SDIO Host Controller register set */
typedef struct {
    volatile uint32_t HRS00;
    volatile uint32_t HRS01;
    volatile uint32_t HRS02;
    volatile uint32_t HRS03;
    volatile uint32_t HRS04;
    volatile uint32_t HRS05;
    volatile uint32_t HRS06;
    volatile uint32_t HRS07;
    volatile uint32_t RESERVED0[22u];
    volatile uint32_t HRS30;
    volatile uint32_t HRS31;
    volatile uint32_t HRS32;
    volatile uint32_t HRS33;
    volatile uint32_t HRS34;
    volatile uint32_t HRS35;
    volatile uint32_t HRS36;
    volatile uint32_t HRS37;
    volatile uint32_t HRS38;
    volatile uint32_t RESERVED1[20u];
    volatile uint32_t HRS59;
    volatile uint32_t RESERVED2[3u];
    volatile uint32_t CRS63;
    volatile uint32_t RESERVED3[64u];
    volatile uint32_t SRS00;
    volatile uint32_t SRS01;
    volatile uint32_t SRS02;
    volatile uint32_t SRS03;
    volatile uint32_t SRS04;
    volatile uint32_t SRS05;
    volatile uint32_t SRS06;
    volatile uint32_t SRS07;
    volatile uint32_t SRS08;
    volatile uint32_t SRS09;
    volatile uint32_t SRS10;
    volatile uint32_t SRS11;
    volatile uint32_t SRS12;
    volatile uint32_t SRS13;
    volatile uint32_t SRS14;
    volatile uint32_t SRS15;
    volatile uint32_t SRS16;
    volatile uint32_t SRS17;
    volatile uint32_t SRS18;
    volatile uint32_t SRS19;
    volatile uint32_t SRS20;
    volatile uint32_t SRS21;
    volatile uint32_t SRS22;
    volatile uint32_t SRS23;
    volatile uint32_t SRS24;
    volatile uint32_t SRS25;
    volatile uint32_t SRS26;
    volatile uint32_t SRS27;
    volatile uint32_t RESERVED4[100u];
    volatile uint32_t CQRS00;
    volatile uint32_t CQRS01;
    volatile uint32_t CQRS02;
    volatile uint32_t CQRS03;
    volatile uint32_t CQRS04;
    volatile uint32_t CQRS05;
    volatile uint32_t CQRS06;
    volatile uint32_t CQRS07;
    volatile uint32_t CQRS08;
    volatile uint32_t CQRS09;
    volatile uint32_t CQRS10;
    volatile uint32_t CQRS11;
    volatile uint32_t CQRS12;
    volatile uint32_t CQRS13;
    volatile uint32_t CQRS14;
    volatile uint32_t RESERVED5;
    volatile uint32_t CQRS16;
    volatile uint32_t CQRS17;
    volatile uint32_t CQRS18;
    volatile uint32_t RESERVED6;
    volatile uint32_t CQRS20;
    volatile uint32_t CQRS21;
    volatile uint32_t CQRS22;
    volatile uint32_t CQRS23;
}MMC_TypeDef;

#define MMC_BASE                        0x20008000u
#define MMC                             ((MMC_TypeDef *) MMC_BASE)

/*-----------------------------------------------------------------------------
    @name (HRS6) - masks and macros
 ------------------------------------------------------------------------------
*/
#define HRS6_EMMC_MODE_MASK             0x00000007u
#define HRS6_EMMC_MODE_SDCARD           (0x0u << 0)
#define HRS6_EMMC_MODE_SDR              (0x2u << 0)
#define HRS6_EMMC_MODE_DDR              (0x3u << 0)
#define HRS6_EMMC_MODE_HS200            (0x4u << 0)
#define HRS6_EMMC_MODE_HS400            (0x5u << 0)
#define HRS6_EMMC_MODE_HS400_ES         (0x6u << 0)
#define HRS6_EMMC_MODE_LEGACY           (0x7u << 0)
#define HRS6_EMMC_TUNE_VALUE_MASK       0x00003F00u
#define HRS6_EMMC_TUNE_REQUEST          0x00008000u

/*-----------------------------------------------------------------------------
    @name Block count and size register (SRS01) - masks
 ------------------------------------------------------------------------------
*/
/* Block count for current transfer mask */
#define SRS1_BLOCK_COUNT            0xFFFF0000u
/* DMA buffer size 4kB */
#define SRS1_DMA_BUFF_SIZE_4KB      0x00000000u
/* DMA buffer size 8kB */
#define SRS1_DMA_BUFF_SIZE_8KB      0x00001000u
/* DMA buffer size 16kB */
#define SRS1_DMA_BUFF_SIZE_16KB     0x00002000u
/* DMA buffer size 32kB */
#define SRS1_DMA_BUFF_SIZE_32KB     0x00003000u
/* DMA buffer size 64kB */
#define SRS1_DMA_BUFF_SIZE_64KB     0x00004000u
/* DMA buffer size 128kB */
#define SRS1_DMA_BUFF_SIZE_128KB    0x00005000u
/* DMA buffer size 265kB */
#define SRS1_DMA_BUFF_SIZE_256KB    0x00006000u
/* DMA buffer size 512kB */
#define SRS1_DMA_BUFF_SIZE_512KB    0x00007000u
/* DMA buffer size mask */
#define SRS1_DMA_BUFF_SIZE_MASK     0x00007000u
/* Transfer block size mask */
#define SRS1_BLOCK_SIZE             0x00000FFFu

/*-----------------------------------------------------------------------------
    @name Transfer mode and command information register (SRS3) - masks
 ------------------------------------------------------------------------------
*/
/* command type */
/* Abort CMD12, CMD52 for writing "I/O Abort" in CCCR */
#define SRS3_ABORT_CMD              (0x3u << 22)
/* Resume CMD52 for writing "Function Select" in CCCR */
#define SRS3_RESUME_CMD             (0x2u << 22)
/*Suspend CMD52 for writing "Bus Suspend" in CCCR */
#define SRS3_SUSPEND_CMD            (0x1u << 22)
/* data is present and will be transferred using the DAT line */
#define SRS3_DATA_PRESENT           0x00200000u
/* index check enable */
#define SRS3_INDEX_CHECK_EN         0x00100000u
/* response CRC check enable */
#define SRS3_CRC_CHECK_EN           0x00080000u
/* response type */
/* response type - no response */
#define SRS3_NO_RESPONSE            (0x0u << 16)
/* response type - response length 136 */
#define SRS3_RESP_LENGTH_136        (0x1u << 16)
/* response type - response length 48 */
#define SRS3_RESP_LENGTH_48         (0x2u << 16)
/* response type - response length 48 and check Busy after response */
#define SRS3_RESP_LENGTH_48B        (0x3u << 16)
/* RID - Response Interrupt Disable
   When set to 1, the Command Complete Interrupt (SRS12.CC)
   will be disabled */
#define SRS3_RESP_INTER_DISABLE     0x00000100u
/* RECE - Response Error Check Enable.
  When set 1, the host will look after R1/R5 responses. */
#define SRS3_RESP_ERR_CHECK_EN      0x00000080u
/* Response type
  Response type R1 for the response content checker */
#define SRS3_RESPONSE_CHECK_TYPE_R1 (0x0u << 6)
/* Response type R5 for the response content checker */
#define SRS3_RESPONSE_CHECK_TYPE_R5 (0x1u << 6)
/* multi block DAT line data transfers */
#define SRS3_MULTI_BLOCK_SEL        0x00000020u
/* Data Transfer Direction Select */
/* data transfer direction - write */
#define SRS3_TRANS_DIRECT_WRITE     (0x0u << 4)
/* data transfer direction - read */
#define SRS3_TRANS_DIRECT_READ      (0x1u << 4)
/* Auto CMD Enable */
/* Auto CMD23 enable */
#define SRS3_AUTOCMD23_ENABLE       (0x2u << 2)
/* Auto CMD12 enable */
#define SRS3_AUTOCMD12_ENABLE       (0x1u << 2)
/* Block count enable */
#define SRS3_BLOCK_COUNT_ENABLE     0x00000002u
/* DMA enable */
#define SRS3_DMA_ENABLE             0x00000001u

/*-----------------------------------------------------------------------------
    @name Present state register masks (SRS9) - masks
 ------------------------------------------------------------------------------
*/
/* U2DET - STB.L Detection (UHS-II only)
 * Field used when UHS-II interface is enabled (U2IE=1)
 * and SD clock is supplied (SDCE=1)  */
#define SRS9_STBL_DETECTION         0x80000000u
/* U2SYN Lane Synchronization (UHS-II only)
 * After STB.L Detection set 1,
 * this bit is set when SYN LLS is received on D1 lan*/
#define SRS9_LANE_SYNCHRONIZATION   0x40000000u
/* U2DOR - In Dormant State (UHS-II only) */
#define SRS9_IN_DORMANT_STATE       0x20000000u
/* CMD line signal level */
#define SRS9_CMD_SIGNAL_LEVEL       0x01000000u
/* DAT[3:0] Line Signal Level */
/* DAT3 signal level */
#define SRS9_DAT3_SIGNAL_LEVEL      (0x1u << 23)
/* DAT2 signal level */
#define SRS9_DAT2_SIGNAL_LEVEL      (0x1u << 22)
/* DAT1 signal level */
#define SRS9_DAT1_SIGNAL_LEVEL      (0x1u << 21)
/* DAT0 signal level */
#define SRS9_DAT0_SIGNAL_LEVEL      (0x1u << 20)
/* Write protect switch pin level */
#define SRS9_WP_SWITCH_LEVEL        0x00080000u
/* Card detect pin level */
#define SRS9_CARD_DETECT_LEVEL      0x00040000u
/* Card state stable */
#define SRS9_CARD_STATE_STABLE      0x00020000u
/* Card inserted */
#define SRS9_CARD_INSERTED          0x00010000u
/* Buffer read enable */
#define SRS9_BUFF_READ_EN           0x00000800u
/* Buffer write enable */
#define SRS9_BUFF_WRITE_EN          0x00000400u
/* Read transfer active */
#define SRS9_READ_TRANS_ACTIVE      0x00000200u
/* Write transfer active */
#define SRS9_WRITE_TRANS_ACTIVE     0x00000100u
/* DAT[7:4] Line Signal Level */
/* DAT7 Line Signal Level l (SD Mode only) */
#define SRS9_WRITE_DAT7_PIN_LEVEL   (0x1u << 7)
/* DAT6 Line Signal Level l (SD Mode only) */
#define SRS9_WRITE_DAT6_PIN_LEVEL   (0x1u << 6)
/* DAT5 Line Signal Level l (SD Mode only) */
#define SRS9_WRITE_DAT5_PIN_LEVEL   (0x1u << 5)
/* DAT4 Line Signal Level l (SD Mode only) */
#define SRS9_WRITE_DAT4_PIN_LEVEL   (0x1u << 4)
/* The signal will be used by the SD driver to rerun the
   DAT line active */
#define SRS9_DAT_LINE_ACTIVE        0x00000004u
/* Command Inhibit (DAT) */
#define SRS9_CMD_INHIBIT_DAT        0x00000002u
/* Command Inhibit (CMD) */
#define SRS9_CMD_INHIBIT_CMD        0x00000001u


/*-----------------------------------------------------------------------------
    @name SRS10
 ------------------------------------------------------------------------------
*/
/* SD Bus Voltage Select */
/* SD bus voltage - 3.3V */
#define SRS10_SET_3_3V_BUS_VOLTAGE          (0x7u << 9)
/* SD bus voltage - 3.0V */
#define SRS10_SET_3_0V_BUS_VOLTAGE          (0x6u << 9)
/* SD bus voltage - 1.8V */
#define SRS10_SET_1_8V_BUS_VOLTAGE          (0x5u << 9)
/* SD bus voltage mask */
#define SRS10_BUS_VOLTAGE_MASK              (0x7u << 9)
/* SD bus power. The SD device is powered. */
#define SRS10_SD_BUS_POWER                    0x00000100u
/* select SDMA mode */
#define SRS10_DMA_SELECT_SDMA               (0x0u << 3)
/* select ADMA1 mode */
#define SRS10_DMA_SELECT_ADMA1              (0x1u << 3)
/* select ADMA2 mode */
#define SRS10_DMA_SELECT_ADMA2              (0x2u << 3)
/* DMA mode selection mask */
#define SRS10_DMA_SELECT_MASK               (0x3u << 3)
/* Set 4 bit data transfer width */
#define SRS10_DATA_WIDTH_4BIT                  0x00000002u
/* Extended Data Transfer Width */
#define SRS10_EXTENDED_DATA_TRANSFER_WIDTH     0x00000020u
/* High speed enable. */
#define SRS10_HIGH_SPEED_ENABLE             0x00000004u
/* Turning on the LED.*/
#define SRS10_TURN_ON_LED                   0x00000001u
/*-----------------------------------------------------------------------------
    @name SRS11
 ------------------------------------------------------------------------------
*/
/* Data timeout mask */
#define SRS11_TIMEOUT_MASK                  (0xFu << 16)
/* SD clock enable */
#define SRS11_SD_CLOCK_ENABLE               0x00000004u
/* SDCLK Frequency mask */
#define SRS11_SEL_FREQ_BASE_MASK            (0x0000FF00u | 0x000000C0u)
/*The SDCLK Frequency Divider method can be selected by this field */
#define SRS11_CLOCK_GENERATOR_SELECT        (0x1u << 5)
/* Internal clock stable */
#define SRS11_INT_CLOCK_STABLE              0x00000002u
/* internal clock enable */
#define SRS11_INT_CLOCK_ENABLE              0x00000001u

/*-----------------------------------------------------------------------------
    @name Interrupt status register (SRS12) - masks
 ------------------------------------------------------------------------------
*/
/* ERSP - Response Error (SD Mode only) */
#define SRS12_RESPONSE_ERROR        0x08000000u
/* Tuning error */
/*#define SRS12_TUNING_ERROR          0x04000000u*/
/* ADMA error */
#define SRS12_ADMA_ERROR            0x02000000u
/* Auto CMD (CMD12 or CMD23) error */
#define SRS12_AUTO_CMD_ERROR        0x01000000u
/* Current limit error host controller is not supplying power to SD card
   due some failure. */
#define SRS12_CURRENT_LIMIT_ERROR   0x00800000u
/* Data end bit error */
#define SRS12_DATA_END_BIT_ERROR    0x00400000u
/* Data CRC error */
#define SRS12_DATA_CRC_ERROR        0x00200000u
/* Data timeout error */
#define SRS12_DATA_TIMEOUT_ERROR    0x00100000u
/* Command index error. Index error occurs in the command response. */
#define SRS12_COMMAND_INDEX_ERROR   0x00080000u
/* Command end bit error */
#define SRS12_COMMAND_END_BIT_ERROR 0x00040000u
/* Command CRC error */
#define SRS12_COMMAND_CRC_ERROR     0x00020000u
/* Command timeout error */
#define SRS12_COMMAND_TIMEOUT_ERROR 0x00010000u
/* Error interrupt */
#define SRS12_ERROR_INTERRUPT       0x00008000u
/* Command Queuing - interrupt */
#define SRS12_CMD_QUEUING_INT       0x00004000u

#if 0
/* Re-Tuning Event */
#define SRS12_RETUNING_EVENT        0x00001000u
/* Interrupt on line C */
#define SRS12_INTERRUPT_ON_LINE_C   0x00000800u
/* Interrupt on line B */
#define SRS12_INTERRUPT_ON_LINE_B   0x00000400u
/* Interrupt on line A */
#define SRS12_INTERRUPT_ON_LINE_A   0x00000200u
#endif

/* Card interrupt */
#define SRS12_CARD_INTERRUPT        0x00000100u
/* Card removal */
#define SRS12_CARD_REMOVAL          0x00000080u
/* Card insertion */
#define SRS12_CARD_INSERTION        0x00000040u
/* Buffer read ready. Host is ready to read the buffer. */
#define SRS12_BUFFER_READ_READY     0x00000020u
/* Buffer write ready. Host is ready for writing data to the buffer.*/
#define SRS12_BUFFER_WRITE_READY    0x00000010u
/* DMA interrupt */
#define SRS12_DMA_INTERRUPT         0x00000008u
/* Block gap event */
#define SRS12_BLOCK_GAP_EVENT       0x00000004u
/* Transfer complete */
#define SRS12_TRANSFER_COMPLETE     0x00000002u
/* Command complete */
#define SRS12_COMMAND_COMPLETE      0x00000001u
/* normal interrupt status mask */
#define SRS12_NORMAL_STAUS_MASK     (0xFFFFu)
#define SRS12_ERROR_STATUS_MASK     (0xFFFF8000u)
#define SRS12_ERROR_CMD_LINE        (SRS12_COMMAND_TIMEOUT_ERROR \
                                     | SRS12_COMMAND_CRC_ERROR \
                                     | SRS12_COMMAND_END_BIT_ERROR \
                                     | SRS12_COMMAND_INDEX_ERROR)

/*-----------------------------------------------------------------------------
    @name Interrupt status enable register (SRS13) - masks
 ------------------------------------------------------------------------------
*/
/** ERSP_SE - Response Error Status Enable */
#define SRS13_RESPONSE_ERROR_STAT_EN        0x08000000u
/* Tuning error status enable */
#define SRS13_TUNING_ERROR_STAT_EN          (0x1u << 26)
/* ADMA error status enable */
#define SRS13_ADMA_ERROR_STAT_EN            0x02000000u
/* Auto CMD12 error status enable */
#define SRS13_AUTO_CMD12_ERR_STAT_EN        0x01000000u
/* Current limit error status enable */
#define SRS13_CURRENT_LIMIT_ERR_STAT_EN     0x00800000u
/* Data end bit error status enable */
#define SRS13_DATA_END_BIT_ERR_STAT_EN      0x00400000u
/* Data CRC error status enable */
#define SRS13_DATA_CRC_ERR_STAT_EN          0x00200000u
/* Data timeout error status enable */
#define SRS13_DATA_TIMEOUT_ERR_STAT_EN      0x00100000u
/* Command index error status enable */
#define SRS13_COMMAND_INDEX_ERR_STAT_EN     0x00080000u
/* Command end bit error status enable */
#define SRS13_COMMAND_END_BIT_ERR_STAT_EN   0x00040000u
/* Command CRC error status enable */
#define SRS13_COMMAND_CRC_ERR_STAT_EN       0x00020000u
/* Command timeout error status enable */
#define SRS13_COMMAND_TIMEOUT_ERR_STAT_EN   0x00010000u
/* Command Queuing Status Enable */
#define SRS13_CMD_QUEUING_STAT_EN           0x00004000u
/* Re-Tuning Event status enable */
#define SRS13_RETUNING_EVENT_STAT_EN        (0x1u << 12)
/*Interrupt on line C status enable */
#define SRS13_INTERRUPT_ON_LINE_C_STAT_EN   (0x1u << 11)
/* Interrupt on line B status enable */
#define SRS13_INTERRUPT_ON_LINE_B_STAT_EN   (0x1u << 10)
/* Interrupt on line A status enable */
#define SRS13_INTERRUPT_ON_LINE_A_STAT_EN   (0x1u << 9)
/* Card interrupt status enable */
#define SRS13_CARD_INTERRUPT_STAT_EN        0x00000100u
/* Card removal status enable */
#define SRS13_CARD_REMOVAL_STAT_EN          0x00000080u
/* Card insertion status enable */
#define SRS13_CARD_INERTION_STAT_EN         0x00000040u
/* Buffer read ready status enable */
#define SRS13_BUF_READ_READY_STAT_EN        0x00000020u
/* Buffer write ready status enable */
#define SRS13_BUF_WRITE_READY_STAT_EN       0x00000010u
/* DMA interrupt status enable */
#define SRS13_DMA_INTERRUPT_STAT_EN         0x00000008u
/* Block gap event status enable */
#define SRS13_BLOCK_GAP_EVENT_STAT_EN       0x00000004u
/* Transfer complete status enable */
#define SRS13_TRANSFER_COMPLETE_STAT_EN     0x00000002u
/* Command complete status enable */
#define SRS13_COMMAND_COMPLETE_STAT_EN      0x00000001u

/*-----------------------------------------------------------------------------
    @name Interrupt signal enable register (SRS14) - masks
 ------------------------------------------------------------------------------
*/
/* Response error interrupt signdla enable */
#define SRS14_RESPONSE_ERROR_SIG_EN         0x08000000u
/* Tuning error signal enable */
#define SRS14_TUNING_ERROR_SIG_EN           0x04000000u
/* ADMA error signal enable */
#define SRS14_ADMA_ERROR_SIG_EN             0x02000000u
/* Auto CMD12 error signal enable */
#define SRS14_AUTO_CMD12_ERR_SIG_EN         0x01000000u
/* Current limit error signal enable */
#define SRS14_CURRENT_LIMIT_ERR_SIG_EN      0x00800000u
/* Data end bit error signal enable */
#define SRS14_DATA_END_BIT_ERR_SIG_EN       0x00400000u
/* Data CRC error signal enable */
#define SRS14_DATA_CRC_ERR_SIG_EN           0x00200000u
/* Data timeout error signal enable */
#define SRS14_DATA_TIMEOUT_ERR_SIG_EN       0x00100000u
/* Command index error signal enable */
#define SRS14_COMMAND_INDEX_ERR_SIG_EN      0x00080000u
/* Command end bit error signal enable */
#define SRS14_COMMAND_END_BIT_ERR_SIG_EN    0x00040000u
/* Command CRC error signal enable */
#define SRS14_COMMAND_CRC_ERR_SIG_EN        0x00020000u
/* Command timeout error signal enable */
#define SRS14_COMMAND_TIMEOUT_ERR_SIG_EN    0x00010000u
/* Command Queuing - interrupt enable */
#define SRS14_CMD_QUEUING_SIG_EN            0x00004000u
/* SD4HC__SRS__SRS14__RSVD_0_MASK           0x00003E00u */
/* Re-Tuning Event signal enable */
#define SRS14_RETUNING_EVENT_SIG_EN         (0x1u << 12)
/*Interrupt on line C signal enable */
#define SRS14_INTERRUPT_ON_LINE_C_SIG_EN    (0x1u << 11)
/* Interrupt on line B signal enable */
#define SRS14_INTERRUPT_ON_LINE_B_SIG_EN    (0x1u << 10)
/* Interrupt on line A signal enable */
#define SRS14_INTERRUPT_ON_LINE_A_SIG_EN    (0x1u << 9)
/* Card interrupt signal enable */
#define SRS14_CARD_INTERRUPT_SIG_EN         0x00000100u
/* Card removal signal enable */
#define SRS14_CARD_REMOVAL_SIG_EN           0x00000080u
/* Card insertion signal enable */
#define SRS14_CARD_INERTION_SIG_EN          0x00000040u
/* Buffer read ready signal enable */
#define SRS14_BUFFER_READ_READY_SIG_EN      0x00000020u
/* Buffer write ready signal enable */
#define SRS14_BUFFER_WRITE_READY_SIG_EN     0x00000010u
/* DMA interrupt signal enable */
#define SRS14_DMA_INTERRUPT_SIG_EN          0x00000008u
/* Block gap event signal enable */
#define SRS14_BLOCK_GAP_EVENT_SIG_EN        0x00000004u
/* Transfer complete signal enable */
#define SRS14_TRANSFER_COMPLETE_SIG_EN      0x00000002u
/* Command complete signal enable */
#define SRS14_COMMAND_COMPLETE_SIG_EN       0x00000001u

/*-----------------------------------------------------------------------------
    @name AutoCMD12 Error Status Register/Host Control Register (SRR15) - masks
 ------------------------------------------------------------------------------
*/
/* Preset Value Enable */
#define SRS15_PRESET_VALUE_ENABLE       0x80000000u
/* Asynchronous Interrupt Enable */
#define SRS15_ASYNCHRONOUS_INT_EN       (0x1u << 30)
/* 64-bit Addressing Specifies the addressing mode for DMA ending. */
#define SRS15_64_BIT_ADDRESSING         0x20000000u
/* Host Version 4.00 Enable
* Selects backward (SD Host 3.00 Version) compatibility mode
* or SD Host 4.00 Version mode
*/
#define SRS15_HOST_4_ENABLE             0x10000000u
/* UHS-II Interface Enable */
#define SRS15_UHSII_ENABLE              0x01000000u
/* Sampling Clock Select */
#define SRS15_SAMPLING_CLOCK_SELECT     0x00800000u
/* Execute Tuning */
#define SRS15_EXECUTE_TUNING            0x00400000u

/* Driver Strength Select mask */
#define SRS15_DRIVER_TYPE_MASK          (0x3u << 20)
/* Driver Strength Select type D */
#define SRS15_DRIVER_TYPE_D             (0x3u << 20)
/* Driver Strength Select type C */
#define SRS15_DRIVER_TYPE_C             (0x2u << 20)
/* Driver Strength Select type A */
#define SRS15_DRIVER_TYPE_A             (0x1u << 20)
/* Driver Strength Select type B */
#define SRS15_DRIVER_TYPE_B             (0x0u << 20)
/* This bit is to switch I/O signaling voltage level on */
/* the SD interface between 3.3V and 1.8V */
#define SRS15_18V_ENABLE                0x00080000u
/* UHS mode select mask */
#define SRS15_UHS_MODE_MASK             (0x7u << 16)
/* UHS-II mode select */
#define SRS15_UHS_MODE_UHSII            (0x7u << 16)
/* DDR50 mode select */
#define SRS15_UHS_MODE_DDR50            (0x4u << 16)
/* SDR104 mode select */
#define SRS15_UHS_MODE_SDR104           (0x3u << 16)
/* SDR50 mode select */
#define SRS15_UHS_MODE_SDR50            (0x2u << 16)
/* SDR25 mode select */
#define SRS15_UHS_MODE_SDR25            (0x1u << 16)
/* SDR12 mode select */
#define SRS15_UHS_MODE_SDR12            (0x0u << 16)
/* Command not issued bu auto CMD12 error */
#define SRS15_CMD_NOT_ISSUED_ERR        0x00000080u
/* Auto CMD12 index error */
#define SRS15_AUTO_CMD12_INDEX_ERR      0x00000010u
/* Auto CMD12 end bit error */
#define SRS15_AUTO_CMD12_END_BIT_ERR    0x00000008u
/* Auto CMD12 CRC error */
#define SRS15_AUTO_CMD12_CRC_ERR        0x00000004u
/* Auto CMD12 timeout error */
#define SRS15_AUTO_CMD12_TIMEOUT_ERR    0x00000002u
/* Autp CMD12 not executed */
#define SRS15_AUTO_CMD12_NOT_EXECUTED   0x00000001u

/*-----------------------------------------------------------------------------
    @name SRS16
 ------------------------------------------------------------------------------
*/
#define SRS16_64BIT_SUPPORT                 0x10000000u
/* 512 is the maximum block size that can be written */
/* to the buffer in the Host Controller. */
#define SRS16_MAX_BLOCK_LENGTH_512          (0x0u << 16)
/* 1024 is the maximum block size that can be written */
/* to the buffer in the Host Controller. */
#define SRS16_MAX_BLOCK_LENGTH_1024         (0x1u << 16)
/* 2048 is the maximum block size that can be written to*/
/* the buffer in the Host Controller. */
#define SRS16_MAX_BLOCK_LENGTH_2048         (0x2u << 16)
/* timeout unit clock is MHz*/
#define SRS16_TIMEOUT_CLOCK_UNIT_MHZ        0x00000080u
/* 64-bit System Bus Support */
#define SRS16_64BIT_SUPPORT                 0x10000000u
/* Voltage 1.8V is supported */
#define SRS16_VOLTAGE_1_8V_SUPPORT          0x04000000u
/* Voltage 3.0V is supported */
#define SRS16_VOLTAGE_3_0V_SUPPORT          0x02000000u
/* Voltage 3.3V is supported */
#define SRS16_VOLTAGE_3_3V_SUPPORT          0x01000000u

#define SRS13_STATUS_EN     (SRS13_TUNING_ERROR_STAT_EN \
                                      | SRS13_ADMA_ERROR_STAT_EN  \
                                      | SRS13_AUTO_CMD12_ERR_STAT_EN  \
                                      | SRS13_CURRENT_LIMIT_ERR_STAT_EN  \
                                      | SRS13_DATA_END_BIT_ERR_STAT_EN  \
                                      | SRS13_DATA_CRC_ERR_STAT_EN  \
                                      | SRS13_DATA_TIMEOUT_ERR_STAT_EN \
                                      | SRS13_COMMAND_INDEX_ERR_STAT_EN \
                                      | SRS13_COMMAND_END_BIT_ERR_STAT_EN \
                                      | SRS13_COMMAND_CRC_ERR_STAT_EN \
                                      | SRS13_COMMAND_TIMEOUT_ERR_STAT_EN \
                                      | SRS13_RETUNING_EVENT_STAT_EN \
                                      | SRS13_INTERRUPT_ON_LINE_A_STAT_EN \
                                      | SRS13_INTERRUPT_ON_LINE_B_STAT_EN \
                                      | SRS13_INTERRUPT_ON_LINE_C_STAT_EN \
                                      | SRS13_CARD_REMOVAL_STAT_EN \
                                      | SRS13_CARD_INERTION_STAT_EN \
                                      | SRS13_BUF_READ_READY_STAT_EN \
                                      | SRS13_BUF_WRITE_READY_STAT_EN \
                                      | SRS13_DMA_INTERRUPT_STAT_EN \
                                      | SRS13_BLOCK_GAP_EVENT_STAT_EN \
                                      | SRS13_TRANSFER_COMPLETE_STAT_EN \
                                      | SRS13_COMMAND_COMPLETE_STAT_EN \
                                      | SRS13_RESPONSE_ERROR_STAT_EN \
                                      | SRS13_CMD_QUEUING_STAT_EN)

/*-----------------------------------------------------------------------------
    @name Capabilities Register #2 Fields  (SRS17) - masks
 ------------------------------------------------------------------------------
*/
/* VDD2 Supported */
#define SRS17_VDD2_ENABLED                  0x10000000u
/* Macro gets value of clock multiplier */
static inline uint32_t SRS17_GET_CLOCK_MULTIPLIER(const uint32_t val)
{
    return ((val & 0x00FF0000u) >> 16u);
}
/* Re-Tuning Modes                          SD4HC__SRS__SRS17__RTNGM_MASK */
/* Re-Tuning Modes - mode 3 */
#define SRS17_RETUNING_MODE_3               (0x2u << 14)
/* Re-Tuning Modes - mode 2 */
#define SRS17_RETUNING_MODE_2               (0x1u << 14)
/* Re-Tuning Modes - mode 1 */
#define SRS17_RETUNING_MODE_1               (0x0u << 14)
/*  tuning operation is necessary in SDR50 mode */
#define SRS17_USE_TUNING_SDR50              0x00002000u
/* It gest value of timer Count for Re-Tuning, */
static inline uint32_t SRS17_GET_RETUNING_TIMER_COUNT(const uint32_t val)
{
    /* (1 << (((val >> 8) & 0xF) - 1)) */
    uint32_t result = 0x80000000u; /* 1 << -1 */

    uint32_t shift = ((val & 0x00000F00u) >> 8u);
    if (shift > 0u) {
        shift -= 1u;
        result = (((uint32_t)1u) << shift);
    }
    return (result);
}
/* 1.8V Line Driver Type D Supported */
#define SRS17_1_8V_DRIVER_TYPE_D_SUPPORTED  0x00000040u
/* 1.8V Line Driver Type C Supported */
#define SRS17_1_8V_DRIVER_TYPE_C_SUPPORTED  0x00000020u
/* 1.8V Line Driver Type A Supported */
#define SRS17_1_8V_DRIVER_TYPE_A_SUPPORTED  0x00000010u
/* UHS-II Supported */
#define SRS17_UHSII_SUPPORTED               0x00000008u
/* DDR50 Supported */
#define SRS17_DDR50_SUPPORTED               0x00000004u
/* SDR104 Supported */
#define SRS17_SDR104_SUPPORTED              0x00000002u
/* SDR50 Supported */
#define SRS17_SDR50_SUPPORTED               0x00000001u

/*-----------------------------------------------------------------------------
    @name PHY settings register (HRS4) - masks and macros
 ------------------------------------------------------------------------------
*/
/* PHY request acknowledge */
#define HRS_PHY_ACKNOWLEDGE_REQUEST         0x04000000u
/* make read request */
#define HRS_PHY_READ_REQUEST                0x02000000u
/* make write request */
#define HRS_PHY_WRITE_REQUEST               0x01000000u

/*-----------------------------------------------------------------------------
    @name PHY Delay Value Registers addressing
 ------------------------------------------------------------------------------
*/
/* PHY register addresses using */
#define UIS_ADDR_HIGH_SPEED                 0x00u
#define UIS_ADDR_DEFAULT_SPEED              0x01u
#define UIS_ADDR_UHSI_SDR12                 0x02u
#define UIS_ADDR_UHSI_SDR25                 0x03u
#define UIS_ADDR_UHSI_SDR50                 0x04u
#define UIS_ADDR_UHSI_DDR50                 0x05u
#define UIS_ADDR_MMC_LEGACY                 0x06u
#define UIS_ADDR_MMC_SDR                    0x07u
#define UIS_ADDR_MMC_DDR                    0x08u
#define UIS_ADDR_SDCLK                      0x0Bu
#define UIS_ADDR_HS_SDCLK                   0x0Cu
#define UIS_ADDR_DAT_STROBE                 0x0Du

/*---------------------------------------------------------------------------
    @name OCR register bits definitions of SD memory cards
 ----------------------------------------------------------------------------
*/
#define SDCARD_REG_OCR_2_7_2_8  (1u << 15)
#define SDCARD_REG_OCR_2_8_2_9  (1u << 16)
#define SDCARD_REG_OCR_2_9_3_0  (1u << 17)
#define SDCARD_REG_OCR_3_0_3_1  (1u << 18)
#define SDCARD_REG_OCR_3_1_3_2  (1u << 19)
#define SDCARD_REG_OCR_3_2_3_3  (1u << 20)
#define SDCARD_REG_OCR_3_3_3_4  (1u << 21)
#define SDCARD_REG_OCR_3_4_3_5  (1u << 22)
#define SDCARD_REG_OCR_3_5_3_6  (1u << 23)
/* Switching to 1.8V request */
#define SDCARD_REG_OCR_S18R     (1u<< 24)
/* Switching to 1.8V accepted */
#define SDCARD_REG_OCR_S18A     (1u << 24)
/* SDXC power controll (0 - power saving, 1 - maximum performance) */
/* (this bit is not aviable in the SDIO cards) */
#define SDCARD_REG_OCR_XPC      (1u << 28)
/* card capacity status (this bit is not aviable in the SDIO cards) */
#define SDCARD_REG_OCR_CCS      (1u << 30)
/* card power up busy status (this is not aviable in the SDIO cards) */
#define SDCARD_REG_OCR_READY    (1u << 31)


/*---------------------------------------------------------------------------
    @name SCR register bits defnitions and slot masks
 ----------------------------------------------------------------------------
*/

/* CMD20 (speed class controll) command is supported by card */
#define SDCARD_REG_CMD20                (1u << 0)
/* CMD23 (set block count) command is supported by card */
#define SDCARD_REG_CMD23                (1u << 1)
static inline uint32_t SDCARD_REG_GET_EXT_SECURITY(uint32_t x)
{
    return ((x >> 11) & 0xFU);
}

#define SDCARD_REG_SD_SPEC3_SUPPORT     (1u << 15)

/* SD supported bus width 1 bit */
#define SDCARD_REG_SCR_SBW_1BIT         (1u << 16)
/* SD supported bus width 4 bit */
#define SDCARD_REG_SCR_SBW_4BIT         (4u << 16)
/* SD bus width mask */
#define SDCARD_REG_SCR_SBW_MASK         0x000F0000u
/* SD security mask */
#define SDCARD_REG_SCR_SEC_MASK         0x00700000u
/* SD security - no security */
#define SDCARD_REG_SCR_SEC_NO           0x00000000u
/* SD security version 1.01 */
#define SDCARD_REG_SCR_SEC_VER_101      0x00200000u
/* SD security version 2.00 */
#define SDCARD_REG_SCR_SEC_VER_200      0x00300000u
/* Data state after erase is 1 */
#define SDCARD_REG_SCR_DSAE_1           0x00800000u
/* Physical Layer Specification Version supported by the card mask. */
#define SDCARD_REG_SCR_SPEC_MAS         0x0F000000u
/* Physical Layer Specification Version 1.00 - 1.01 */
#define SDCARD_REG_SCR_SPEC_VER_100     0x00000000u
/* Physical Layer Specification Version 1.10 */
#define SDCARD_REG_SCR_SPEC_VER_110     0x01000000u
/* Physical Layer Specification Version 2.00 */
#define SDCARD_REG_SCR_SPEC_VER_200     0x02000000u
/* Physical Layer Specification mask */
#define SDCARD_REG_SCR_SPEC_VER_MASK    0x0F000000u
/* SCR structure mask */
#define SDCARD_REG_SCR_STRUCTURE_MASK   0xF0000000u
/* SCR version 1.0 */
#define SDCARD_REG_SCR_VER_10           0x00000000u

/*---------------------------------------------------------------------------
    @name Response R4 bit definitions
 ----------------------------------------------------------------------------
*/
/* card ready bit */
#define SDCARD_R4_CARD_READY        (1u << 31)
/* memory present bit */
#define SDCARD_R4_MEMORY_PRESENT    (1u << 27)

/*---------------------------------------------------------------------------
    @name SD card function register bits, masks and macros definitions
 ----------------------------------------------------------------------------
*/
/* Check function mode - is used to query if the card supports a specific
   function or functions. */
#define SDCARD_SWITCH_FUNC_MODE_SWITCH          (0x1u << 31)
/* Set function mode  - is used to switch the functionality of the card.*/
#define SDCARD_SWITCH_FUNC_MODE_CHECK           (0x0u << 31)
/* Card access mode - SDR12 default */
#define SDCARD_SWITCH_ACCESS_MODE_SDR12         0x0u
/* Card access mode - SDR25 highspeed */
#define SDCARD_SWITCH_ACCESS_MODE_SDR25         0x1u
/* Card access mode - SDR50 */
#define SDCARD_SWITCH_ACCESS_MODE_SDR50         0x2u
/* Card access mode - SDR104 */
#define SDCARD_SWITCH_ACCESS_MODE_SDR104        0x3u
/* Card access mode - DDR50 */
#define SDCARD_SWITCH_ACCESS_MODE_DDR50         0x4u

/* Card command system - default */
#define SDCARD_SWITCH_CMD_SYSTEM_DEFAULT        0x0u
/* Card command system - eCommerce command set */
#define SDCARD_SWITCH_CMD_SYSTEM_E_COMMERCE     0x1u
/* Card command system - OTP */
#define SDCARD_SWITCH_CMD_SYSTEM_OTP            0x3u
/* Card command system - ASSD */
#define SDCARD_SWITCH_CMD_SYSTEM_ASSD           0x4u
/* Card command system - vendor specific command set */
#define SDCARD_SWITCH_CMD_SYSTEM_NR_VENDOR      0xEu

/* Card driver strength - Type B default */
#define SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_B    0x0u
/* Card driver strength - Type A */
#define SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_A    0x1u
/* Card driver strength - Type C */
#define SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_C    0x2u
/* Card driver strength - Type D */
#define SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_D    0x3u

#define SDCARD_SWITCH_GROUP_NR_1    1u
#define SDCARD_SWITCH_GROUP_NR_2    2u
#define SDCARD_SWITCH_GROUP_NR_3    3u
#define SDCARD_SWITCH_GROUP_NR_4    4u
#define SDCARD_SWITCH_GROUP_NR_5    5u
#define SDCARD_SWITCH_GROUP_NR_6    6u

/* macro gets one byte from dword */
#define GetByte(dword, byte_nr)     (((dword) >> ((byte_nr) * 8u)) & 0xFFu)

static inline uint8_t GET_BYTE_FROM_BUFFER(const void* buffer, uintptr_t byteNr)
{
    return ((uint8_t)GetByte((*(uint32_t*)((uintptr_t)buffer + (byteNr & ~3u))), (byteNr & 3u)));
}
static inline uint8_t GET_BYTE_FROM_BUFFER2(const void* buffer, uintptr_t bufferSize, uintptr_t byteNr)
{
    return (GET_BYTE_FROM_BUFFER(buffer, bufferSize - 1u - byteNr));
}

/* Macro returns 1 if given function is supported by the card */
static inline uint8_t SDCARD_SWITCH_FUNC_IS_FUNC_SUPPORTED(const uint8_t* val, uint8_t groupNum, uint8_t funcNum)
{
    uint32_t supportStatus = 0u;

    if (funcNum < 32u) {
        const uintptr_t offset = (groupNum - 1u) * 2u;

        supportStatus = (((uint32_t)GET_BYTE_FROM_BUFFER2(val, 64u, 50u - offset)
                          | (uint32_t)(GET_BYTE_FROM_BUFFER2(val, 64u, 51u - offset))) << 8)
                        & (uint32_t)(1u << funcNum);
    }

    return ((supportStatus != 0u) ? 1u : 0u);
}
/* Macro gets function status code from the switch function status data structure */
static inline uint8_t SDCARD_SWICH_FUNC_GET_STAT_CODE(const uint8_t* val, uint8_t groupNum)
{
    uint8_t result = 0u;
    const uint8_t shift = ((groupNum - 1u) % 2u) * 4u;

    const uintptr_t offset = 47u + ((groupNum - 1u) / 2u);

    result = (GET_BYTE_FROM_BUFFER2(val, 64u, offset) >> shift) & 0xFu;

    return (result);
}
/* Macro gets function busy status from the switch function status data structure */
/* Macro returns 1 if function is busy 0 otherwise */
static inline uint8_t SDCARD_SWICH_FUNC_GET_BUSY_STAT(const uint8_t* val, uint8_t groupNum, uint8_t funcNum)
{
    uint32_t busyStatus = 0u;

    if (funcNum < 32u) {
        const uintptr_t offset = (groupNum - 1u) * 2u;

        busyStatus = ((GET_BYTE_FROM_BUFFER2(val, 64u, 34u - offset)
                       | GET_BYTE_FROM_BUFFER2(val, 64u, 35u - offset)) << 8)
                     & (uint32_t)(1u << funcNum);
    }

    return ((busyStatus != 0u) ? 1u : 0u);
}

/*---------------------------------------------------------------------------
    @name Command masks
 ----------------------------------------------------------------------------
*/
/* host hight capacity support -*/
#define SDCARD_ACMD41_HCS   (1u << 30)

/*--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
    @name Response SPI R1 for SD memory cards bits defnitions
 ----------------------------------------------------------------------------
*/
/* The card is in idle state and running the initializing process. */
#define SDCARD_RESP_R1_IDLE                 0x01u
/* An erase sequence was cleared before executing because an out of
  erase sequence command was received. */
#define SDCARD_RESP_R1_ERASE_RESET          0x02u
/* An illegal command code was detected. */
#define SDCARD_RESP_R1_ILLEGAL_CMD_ERR      0x04u
/* The CRC check of the last command failed. */
#define SDCARD_RESP_R1_COM_CRC_ERR          0x08u
/* An error in the sequence of erase commands occurred. */
#define SDCARD_RESP_R1_ERASE_SEQUENCE_ERR   0x10u
/* A misaligned address that did not match the block length was used in the command.*/
#define SDCARD_RESP_R1_ADDRESS_ERR          0x20u
/* The command's argument (e.g. address, block length) was outside the allowed */
#define SDCARD_RESP_R1_PARAM_ERR            0x40u
/* All errors mask */
#define SDCARD_RESP_R1_ALL_ERRORS           (SDCARD_RESP_R1_ILLEGAL_CMD_ERR \
                                             | SDCARD_RESP_R1_COM_CRC_ERR \
                                             | SDCARD_RESP_R1_ERASE_SEQUENCE_ERR \
                                             | SDCARD_RESP_R1_ADDRESS_ERR \
                                             | SDCARD_RESP_R1_PARAM_ERR)


/* No operation go to next descriptor on the list. */
#define ADMA2_DESCRIPTOR_TYPE_NOP   (0x0u << 4)
/* Transfer data from the pointed page and go to next descriptor on the list.*/
#define ADMA2_DESCRIPTOR_TYPE_TRAN  (0x2u << 4)
/* Go to the next descriptor list */
#define ADMA2_DESCRIPTOR_TYPE_LINK  (0x3u << 4)
/* the ADMA interrupt is generated */
/* when the ADMA1 engine finishes processing the descriptor. */
#define ADMA2_DESCRIPTOR_INT        (0x1u << 2)
/* it signals termination of the transfer */
/* and generates Transfer Complete Interrupt */
/* when this transfer is completed */
#define ADMA2_DESCRIPTOR_END        (0x1u << 1)
/* it indicates the valid descriptor on a list */
#define ADMA2_DESCRIPTOR_VAL        (0x1u << 0)

/*-----------------------------------------------------------------------------
    Command Queuing Configuration (CQRS02)- masks
 ------------------------------------------------------------------------------
*/
/* Direct Command (DCMD) Enable */
#define CQRS02_DIRECT_CMD_ENABLE            0x00001000u
/* Task Descriptor Size 128 bits */
#define CQRS02_TASK_DESCRIPTOR_SIZE_128     (1u << 8)
/* Task Descriptor Size 64 bits */
#define CQRS02_TASK_DESCRIPTOR_SIZE_64      (0u << 8)
/* Task Descriptor Size mask */
#define CQRS02_TASK_DESCRIPTOR_SIZE_MASK    0x00000100u
/* Command Queuing Enable */
#define CQRS02_COMMAND_QUEUING_ENABLE       0x00000001u

/*-----------------------------------------------------------------------------
    Command Queuing Interrupt Status (CQRS04)- masks
 ------------------------------------------------------------------------------
*/
/* Task cleared interrupt */
#define CQRS04_TASK_CLEARED_INT             0x00000008u
/* Response Error Detected Interrupt */
#define CQRS04_RESP_ERR_INT                 0x00000004u
/* Task Complete Interrupt */
#define CQRS04_TASK_COMPLETE_INT            0x00000002u
/* Halt Complete Interrupt */
#define CQRS04_HALT_COMPLETE_INT            0x00000001u

/*-----------------------------------------------------------------------------
    Command Queuing Interrupt Status Enable (CQRS05)- masks
 ------------------------------------------------------------------------------
*/
/* Task cleared status enable */
#define CQRS05_TASK_CLEARED_STAT_EN         0x00000008u
/* Response Error Detected status enable */
#define CQRS05_RESP_ERR_STAT_EN             0x00000004u
/* Task Complete status enable */
#define CQRS05_TASK_COMPLETE_STAT_EN        0x00000002u
/* Halt Complete status enable */
#define CQRS05_HALT_COMPLETE_STAT_EN        0x00000001u

/*-----------------------------------------------------------------------------
    Command Queuing Interrupt Signal Enable (CQRS06)- masks
 ------------------------------------------------------------------------------
*/
/* Task cleared interrupt signal enable */
#define CQRS06_TASK_CLEARED_INT_SIG_EN      0x00000008u
/* Response Error Detected Interrupt signal enable */
#define CQRS06_RESP_ERR_INT_SIG_EN          0x00000004u
/* Task Complete Interrupt signal enable */
#define CQRS06_TASK_COMPLETE_INT_SIG_EN     0x00000002u
/* Halt Complete Interrupt signal enable */
#define CQRS06_HALT_COMPLETE_INT_SIG_EN     0x00000001u

/*-----------------------------------------------------------------------------
    Command Queuing Interrupt Coalescing (CQRS07)- masks and macros
 ------------------------------------------------------------------------------
*/
#define CQRS07_INT_COAL_ENABLE                  0x80000000u
/* Interrupt Coalescing Status Bit */
#define CQRS07_INT_COAL_STATUS_BIT              0x00100000u
/* Counter and Timer Reset(ICCTR) */
#define CQRS07_INT_COAL_COUNTER_TIMER_RESET     0x00010000u
/* Interrupt Coalescing Counter Threshold Write Enable */
#define CQRS07_INT_COAL_COUNT_THRESHOLD_WE      0x00008000u

/* task management argument - discard task */
#define  CQ_TASK_MGMT_ARG_TM_DISCARD_TASK      1u
/* task management argument - discard queue */
#define  CQ_TASK_MGMT_ARG_TM_DISCARD_QUEUE     2u

/* number of supported tasks */
#define  CQ_HOST_NUMBER_OF_TASKS   32u
/* direct command task ID */
#define  CQ_DCMD_TASK_ID           31u

/*-----------------------------------------------------------------------------
    @name Task Descriptor Fields
 ------------------------------------------------------------------------------
*/
/* The descriptor is valid */
#define CQ_DESC_VALID                        (1u <<  0)
/* it is the last descriptor */
#define CQ_DESC_END                          (1u <<  1)
/* Hardware shall generate an interrupt upon the task's completion */
#define CQ_DESC_INT                          (1u <<  2)
/* Descriptor type - Task descriptor */
#define CQ_DESC_ACT_TASK                     (5u << 3)
/* Descriptor type - Data Transfer descriptor */
#define CQ_DESC_ACT_TRAN                     (4u << 3)
/* Descriptor type - Link descriptor */
#define CQ_DESC_ACT_LINK                     (6u << 3)
/* Descriptor type - No operation*/
#define CQ_DESC_ACT_NOP                      (0u << 3)
/* enable force programming */
#define CQ_DESC_FORCE_PROG                   (6u << 1)
/* set context ID */
static inline uint16_t CQ_DESC_SET_CONTEXT_ID(uint16_t id)
{
    return ((id & 0xFU) << 7);
}
#define CQ_DESC_TAG_REQUEST                  (1u <<  11)
/* Data read direction */
#define CQ_DESC_DATA_DIR_READ                (1u <<  12)
#define CQ_DESC_DATA_DIR_WRITE               (0u <<  12)
/* High priority task  */
#define CQ_DESC_PRIORITY_HIGH                (1u <<  13)
#define CQ_DESC_QUEUE_BARRIER                (1u <<  14)
#define CQ_DESC_RELIABLE_WRITE               (1u <<  15)
/* set data block count to transfer */
static inline uint32_t CQ_DESC_SET_BLOCK_COUNT(uint32_t count)
{
    return ((count & 0xFFFFu) << 16);
}
/* Length of data buffer in bytes. A value of 0000 means 64 KB */
static inline uint32_t CQ_DESC_SET_DATA_LEN(uint32_t len)
{
    return ((len & 0xFFFFu) << 16);
}

static inline uint32_t CQ_DESC_DCMD_SET_CMD_INDEX(uint32_t idx)
{
    return ((idx & 0x3Fu) << 16);
}
/* Command may be sent to device during data activity or busy time */
#define CQ_DESC_DCMD_CMD_TIMING              (1u << 22)
/* expected reponse on direct command - R1 or R4 or R5*/
#define CQ_DESC_DCMD_RESP_TYPE_R1_R4_R5      (2u << 23)
/* expected reponse on direct command - R1B */
#define CQ_DESC_DCMD_RESP_TYPE_R1B           (3u << 23)
/* no expected reponse on direct command */
#define CQ_DESC_DCMD_RESP_TYPE_NO_RESP       (0u << 23)


/*-----------------------------------------------------------------------------
    @name CCCR transfer direction definitions
 ------------------------------------------------------------------------------
*/
/* Read data from CCCR register */
#define SDIOHOST_CCCR_READ  0u
/* Write data to CCCR register */
#define SDIOHOST_CCCR_WRITE 1u

/*---------------------------------------------------------------------------
    @name Bus interface control register bit definitions
 ----------------------------------------------------------------------------
*/
/* Data 4 bit bus width */
#define SDCARD_BIS_BUS_WIDTH_4BIT   0x02u
/* Data 1 bit bus width */
#define SDCARD_BIS_BUS_WIDTH_1BIT   0x00u
/* Connect[0]/Disconnect[1] the 10K-90K ohm pull-up resistor on CD/DAT[3] */
/* (pin 1) of card. */
#define SDCARD_BIS_CD_DISABLE       0x80u
/* Support contiunous SPI interrupt (irrespective of the state the CS line) */
#define SDCARD_BIS_SCSI             0x40u
/* Enable contiunous SPI interrupt (irrespective of the state the CS line) */
#define SDCARD_BIS_ECSI             0x20u

/*---------------------------------------------------------------------------
    @name Card capability register bit definitions
 ----------------------------------------------------------------------------
*/
/* Card supports direct commands during data trnsfer. (only in SD mode) */
#define SDCARD_CCR_SDC  0x01u
/* Card supports multiblock */
#define SDCARD_CCR_SMB  0x02u
/* Card supports read wait */
#define SDCARD_CCR_SRW  0x04u
/* Card supports Suspend/Resume */
#define SDCARD_CCR_SBS  0x08u
/* Card supports interrupt between blocks of data in 4-bit SD mode. */
#define SDCARD_CCR_S4MI 0x10u
/* Enable interrupt between blocks of data in 4-bit SD mode. */
#define SDCARD_CCR_E4MI 0x20u
/* Card is a low-speed card */
#define SDCARD_CCR_LSC  0x40u
/* 4 bit support for Low-Speed cards */
#define SDCARD_CCR_4BLS 0x80u

/*---------------------------------------------------------------------------
    @name Bus speed register bit definitions
 ----------------------------------------------------------------------------
*/
/* Support high speed. */
#define SDIO_CCCR_13_SHS          0x01u
/* Enable high speed. */
#define SDIO_CCCR_13_EHS          0x02u

#define SDIO_CCCR_13_BSS_MASK     (0x7u << 1)
#define SDIO_CCCR_13_BSS_SDR12    (0x0u << 1)
#define SDIO_CCCR_13_BSS_SDR25    (0x1u << 1)
#define SDIO_CCCR_13_BSS_SDR50    (0x2u << 1)
#define SDIO_CCCR_13_BSS_SDR104   (0x3u << 1)
#define SDIO_CCCR_13_BSS_DDR50    (0x4u << 1)

/*---------------------------------------------------------------------------
    @name USH-I suport bits register 0x14
 ----------------------------------------------------------------------------
*/
#define SDIO_CCCR_14_SSDR50       (0x1u << 0)
#define SDIO_CCCR_14_SSDR104      (0x1u << 1)
#define SDIO_CCCR_14_SDDR50       (0x1u << 2)

/*---------------------------------------------------------------------------
    @name Card status bits and masks definitions for cards (SD SDIO MMC)
 ----------------------------------------------------------------------------
*/
/* Error authentication process */
#define CARD_SATUS_AKE_SEQ_ERROR          (0x1u << 3)
/* The card will expect ACMD, or an indication that the command has been
   interpreted as ACMD */
#define CARD_SATUS_APP_CMD                (0x1u << 5)
/* Card didn't switch to the expected mode as requested by the SWITCH command */
#define CARD_STATUS_SWITCH_ERROR          (0x1u << 7)
/* Corresponds to buffer empty signaling on the bus - buffer is ready */
#define CARD_STATUS_READY_FOR_DATA         (0x1u << 8)
/* The state of the card when receiving the command. Below are definded all 9 satuses.*/
#define CARD_STATUS_CS_MASK               (0xFu << 9)
/* Current status card is in Idle State */
#define CARD_STATUS_CS_IDLE               (0x0u << 9)
/* Current status card is in Ready State */
#define CARD_STATUS_CS_READY              (0x1u << 9)
/* Current status card is Identification State */
#define CARD_STATUS_CS_IDENT              (0x2u << 9)
/* Current status card is in Stand-by State */
#define CARD_STATUS_CS_STBY               (0x3u << 9)
/* Current status card is in Transfer State */
#define CARD_STATUS_CS_TRAN               (0x4u << 9)
/* Current status card is in Sending-data State */
#define CARD_STATUS_CS_DATA               (0x5u << 9)
/* Current status card is in Receive-data State */
#define CARD_STATUS_CS_RCV                (0x6u << 9)
/* Current status card is in Programming State */
#define CARD_STATUS_CS_PRG                (0x7u << 9)
/* Current status card is in Disconnect State */
#define CARD_STATUS_CS_DIS                (0x8u << 9)
/* An erase sequence was cleared before executing because an out of erase
 sequence command was received */
#define CARD_STATUS_ERASE_RESET           (0x1u << 13)
/* The command has been executed without using the internal ECC. */
#define CARD_STATUS_CARD_ECC_DISABLED     (0x1u << 14)
/* Problem with erase part of memory because it is protected */
#define CARD_STATUS_WP_ERASE_SKIP         (0x1u << 15)
/* Can be either one of the following errors: */
/* - The read only section of the CSD does not match the card content. */
/* - An attempt to reverse the copy (set as original) or permanent WP (unprotected) bits was made.*/
#define CARD_STATUS_CSD_OVERWRITE         (0x1u << 16)
/* The card could not sustain data programming in stream write mode */
#define CARD_STATUS_OVERRUN               (0x1u << 17)
/* The card could not sustain data transfer in stream read mode */
#define CARD_STATUS_UNDERRUN              (0x1u << 18)
/* A general or an unknown error occurred during the operation. */
#define CARD_STATUS_ERROR                 (0x1u << 19)
/* Internal card controller error */
#define CARD_STATUS_CC_ERROR              (0x1u << 20)
/* Card internal ECC was applied but failure failed to correct the data.*/
#define CARD_STATUS_CARD_ECC_FAILED       (0x1u << 21)
/* Command not legal for the card state */
#define CARD_STATUS_ILLEGAL_COMMAND       (0x1u << 22)
/* The CRC check of the previous error command failed. */
#define CARD_STATUS_COM_CRC_ERROR         (0x1u << 23)
/* Set when a sequence or password error has been detected in lock/unlock card command.*/
#define CARD_STATUS_LOCK_UNLOCK_FAILED    (0x1u << 24)
/* When set, signals that the card is card locked by the host */
#define CARD_STATUS_CARD_IS_LOCKED        (0x1u << 25)
/* Set when the host attempts to write to a protected block or
 to the temporary or permanent write protected card. */
#define CARD_STATUS_WP_VIOLATION          (0x1u << 26)
/* An invalid selection of write-blocks for erase occurred.*/
#define CARD_STATUS_ERASE_PARAM           (0x1u << 27)
/* An error in the sequence of erase error commands occurred.*/
#define CARD_STATUS_ERASE_SEQ_ERROR       (0x1u << 28)
/* The transferred block length is not allowed for this card, or the number*/
/* of transferred bytes does not match the block length.*/
#define CARD_STATUS_BLOCK_LEN_ERROR       (0x1u << 29)
/* A misaligned address which did not match the block length was used in the command.*/
#define CARD_STATUS_ADDRESS_ERROR         (0x1u << 30)
/* The command's argument was out of the allowed range for this card.*/
#define CARD_STATUS_OUT_OF_RANGE          (0x1u << 31)

/* All errors mask definition */
#define CARD_STATUS_ALL_ERRORS_MASK (   CARD_STATUS_OUT_OF_RANGE        \
                                        | CARD_STATUS_ADDRESS_ERROR       \
                                        | CARD_STATUS_BLOCK_LEN_ERROR     \
                                        | CARD_STATUS_ERASE_SEQ_ERROR     \
                                        | CARD_STATUS_ERASE_PARAM         \
                                        | CARD_STATUS_WP_VIOLATION        \
                                        | CARD_STATUS_LOCK_UNLOCK_FAILED  \
                                        | CARD_STATUS_COM_CRC_ERROR       \
                                        | CARD_STATUS_ILLEGAL_COMMAND     \
                                        | CARD_STATUS_CARD_ECC_FAILED     \
                                        | CARD_STATUS_CC_ERROR            \
                                        | CARD_STATUS_ERROR               \
                                        | CARD_STATUS_UNDERRUN            \
                                        | CARD_STATUS_OVERRUN             \
                                        | CARD_STATUS_WP_ERASE_SKIP       \
                                        | CARD_STATUS_SWITCH_ERROR        \
                                        | CARD_SATUS_AKE_SEQ_ERROR )


/****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* __MSS_MMC_REGS_H_ */
