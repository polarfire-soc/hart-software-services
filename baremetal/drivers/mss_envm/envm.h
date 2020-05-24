/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eNVM bare metal software driver public APIs.
 *
 */

#ifndef _ENVM_H
#define _ENVM_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYSENVMREG_ENVM_CLOCK_FREQ              20 /* MHz */
#define SYSENVMREG_ENVM_CLOCK_VALUE             19 /* default divider value */
#define SYSENVMREG_ENVM_CR_TIMER_VALUE          0x00000040u

#define SYSENVMREG_ENVM_CR_CLK_PERIOD           0x0000003Fu
#define SYSENVMREG_ENVM_CR_CLK_PERIOD_SHIFT     0
#define SYSENVMREG_ENVM_CR_CLK_CONTINUOUS       0x00000100u
#define SYSENVMREG_ENVM_CR_CLK_SUPPRESS         0x00000200u
#define SYSENVMREG_ENVM_CR_READ_AHEAD           0x00010000u
#define SYSENVMREG_ENVM_CR_SLOW_READ            0x00020000u
#define SYSENVMREG_ENVM_CR_INT_EN               0x00040000u
#define SYSENVMREG_ENVM_CR_TIMER                0xFF000000u
#define SYSENVMREG_ENVM_CR_TIMER_SHIFT          24

#define SYSENVMREG_ENVM_POWER_CR_RESET          0x00000001u
#define SYSENVMREG_ENVM_POWER_CR_PD1            0x00000002u
#define SYSENVMREG_ENVM_POWER_CR_PD2            0x00000004u
#define SYSENVMREG_ENVM_POWER_CR_PD3            0x00000008u
#define SYSENVMREG_ENVM_POWER_CR_PD4            0x00000010u
#define SYSENVMREG_ENVM_POWER_CR_ISO            0x00000020u
#define SYSENVMREG_ENVM_POWER_CR_SLEEP          0x00000040u

  /* timing parameters in us */
#define ENVM_TIME_T1 10
#define ENVM_TIME_T2 20
#define ENVM_TIME_T3 10
#define ENVM_TIME_T4 40
#define ENVM_TIME_TACTV 10

#define envm_buf    pageLatchData.pageData

/* --- MPFS specific settings --- */
#define PAGE_SIZE           0x100    /* Macro Page Size in bytes */
#define WORD_SIZE           0x020    /* Number of bits per word in the pagelatch */
#define NUM_FM_SECTORS      0x02
#define NUM_SM_SECTORS      0x02
#define BYA_SET_VALUE       0x00        /* 32-bit write/read is default */
#define DEFAULT_CLOCK_FREQ  0x14
#define WAIT_RD_FRM         2


/* --- cy_types --- */
#define REG8(addr)              (*((volatile uint8_t *)(addr)))
#define REG16(addr)             (*((volatile uint16_t *)(addr)))
#define REG32(addr)             (*((volatile uint32_t *)(addr)))
#define SET_REG32(addr, value)  (*((volatile uint32_t *)(addr)) = (uint32)(value))

/* --- reg_default_defines --- */
/* Addresses for FM SYS (REG_IF) regs accessed through C_BUS */

#define FM_SYS_PA_ADDR                 FM_SYS_START_ADDR           /* Page Address Register */
#define FM_SYS_PAGE_WRITE_ADDR         FM_SYS_START_ADDR + 0x04    /* Data to write at page address */
#define FM_SYS_PAGE_WRITE_INC_ADDR     FM_SYS_START_ADDR + 0x08    /* Data to write at page address with auto address increment */
#define FM_SYS_FM_ADDR                 FM_SYS_START_ADDR + 0x0C    /* Selects one of four 32 bit words on a lash read operation */
#define FM_SYS_FM_READ                 FM_SYS_START_ADDR + 0x10    /* Read Flash data from Address in FM_ADDR */
#define FM_SYS_FM_READ_INC             FM_SYS_START_ADDR + 0x14    /* Read flash data from address in FM_ADDR, then increment the address */
#define FM_SYS_FM_CTRL_ADDR            FM_SYS_START_ADDR + 0x18    /* Flash Macro configuration register. Includes FM_MODE, FM_SEQ and DAA MUX Select (DA[5:0]) */
#define FM_SYS_TIMER_CFG_ADDR          FM_SYS_START_ADDR + 0x1C    /* Timer configuration register: includes Period_Low, Period_high, Timer enable, and Sense Amp config (CFG[5:0] */
#define FM_SYS_ANALOG_CFG_ADDR         FM_SYS_START_ADDR + 0x20    /* Analog Configuration Register: includes itim, BDAC, MDAC, PDAC and NDAC */
#define FM_SYS_TM_CFG_ADDR             FM_SYS_START_ADDR + 0x24    /* TM (test mode) Configuration register */
#define FM_SYS_STATUS_ADDR             FM_SYS_START_ADDR + 0x28    /* Status Register */
#define FM_SYS_WAIT_REG_ADDR           FM_SYS_START_ADDR + 0x2C    /* Wait REgister: rests to 2 for page write and 5 for read */
#define FM_SYS_MONITOR_ADDR            FM_SYS_START_ADDR + 0x30    /* Monitor register */
#define FM_SYS_SW2FM_ADDR              FM_SYS_START_ADDR + 0x34    /* c_bus switch flag. Uses codes to switch from r_bus to c_bus */
#define FM_SYS_SCRATCH_ADDR            FM_SYS_START_ADDR + 0x38    /* Scratch pad register for test */
#define FM_SYS_HV_CONFIG_ADDR          FM_SYS_START_ADDR + 0x3C    /* HV Configuration Register. PM clock frequency in MHz in multiples of 1MHZ * value in this register. Resets to 40MHz */
#define FM_SYS_IRQ_MASK_ADDR           FM_SYS_START_ADDR + 0x40    /* Mask register to enable IRQs */
#define FM_SYS_ACLK_GEN_ADDR           FM_SYS_START_ADDR + 0x44    /* Generate Aclk. Writing to this address will generate one Aclk pulse */
#define FM_SYS_FM_READ_DUMMY           FM_SYS_START_ADDR + 0x48    /* Read dummy used in the hv cycles routine */

/* Defines for 8bit registers within 32-bit REG_IF registers */
#define BYTE_SELECT                    FM_SYS_PA_ADDR
#define PA                             FM_SYS_PA_ADDR + 0x01       /* Page Address. [5:0] */
#define BYA                            FM_SYS_FM_ADDR              /* BYA selects down to 8 or 16 bits if required based on c_size. [1:0] */
#define CAWA                           FM_SYS_FM_ADDR + 0x01
#define RA                             FM_SYS_FM_ADDR + 0x02
#define AXABA                          FM_SYS_FM_ADDR + 0x03
#define FM_MODE                        FM_SYS_FM_CTRL_ADDR
#define FM_SEQ                         FM_SYS_FM_CTRL_ADDR + 0x01
#define DAA_MUX                        FM_SYS_FM_CTRL_ADDR + 0x02
#define TIMER_PERIOD_LOW               FM_SYS_TIMER_CFG_ADDR
#define TIMER_PERIOD_HIGH              FM_SYS_TIMER_CFG_ADDR + 0x01
#define TIMER_CONFIG                   FM_SYS_TIMER_CFG_ADDR + 0x02
#define SA_CONFIG                      FM_SYS_TIMER_CFG_ADDR + 0x03
#define ITIM_BDAC                      FM_SYS_ANALOG_CFG_ADDR
#define MDAC                           FM_SYS_ANALOG_CFG_ADDR + 0x01
#define PDAC                           FM_SYS_ANALOG_CFG_ADDR + 0x02
#define NDAC                           FM_SYS_ANALOG_CFG_ADDR + 0x03
#define FM_CONTROL_REG1                FM_SYS_TM_CFG_ADDR
#define FM_CONTROL_REG2                FM_SYS_TM_CFG_ADDR + 0x01
#define FM_CONTROL_REG3                FM_SYS_TM_CFG_ADDR + 0x02
#define FM_CONTROL_REG4                FM_SYS_TM_CFG_ADDR + 0x03
#define HV_TIMER_DO                    FM_SYS_STATUS_ADDR
#define STATUS_REG1                    FM_SYS_STATUS_ADDR  + 0x01
#define STATUS_REG2                    FM_SYS_STATUS_ADDR  + 0x02
#define STATUS_REG3                    FM_SYS_STATUS_ADDR  + 0x03
#define WAIT_REG_ADDR_0                FM_SYS_WAIT_REG_ADDR
#define WAIT_REG_ADDR_1                FM_SYS_WAIT_REG_ADDR + 0x01
#define WAIT_REG_ADDR_2                FM_SYS_WAIT_REG_ADDR + 0x02
#define WAIT_REG_ADDR_3                FM_SYS_WAIT_REG_ADDR + 0x03
#define SCRATCH_PAD_0                  FM_SYS_SCRATCH_ADDR
#define SCRATCH_PAD_1                  FM_SYS_SCRATCH_ADDR + 0x01
#define SCRATCH_PAD_2                  FM_SYS_SCRATCH_ADDR + 0x02
#define SCRATCH_PAD_3                  FM_SYS_SCRATCH_ADDR + 0x03
#define IRQ_MASK1                      FM_SYS_IRQ_MASK_ADDR
#define IRQ_MASK2                      FM_SYS_IRQ_MASK_ADDR + 0x01
#define IRQ_MASK3                      FM_SYS_IRQ_MASK_ADDR + 0x02
#define IRQ_MASK4                      FM_SYS_IRQ_MASK_ADDR + 0x03

/* Status Register Flags */
#define STATUS_SUCCESS                  0x00  /* Return status for successful operation */
#define STATUS_INVALID_CHIP_PROT_LEVEL  0x01  /* Returns status for attempt to move part into an invalid target protection mode */
#define STATUS_INVALID_NVL_ADDR         0x02  /* Returns status when an invalid nvl address is given */
#define STATUS_INVALID_PL_ADDRESS       0x03  /* Return status when load_size+byte_addr in load_flash_bytes command fails */
#define STATUS_INVALID_ROW_ID           0x04  /* Return status when check is made on row id for bounds check: within max flash, wounded, privileged flash */
#define STATUS_ROW_PROTECTED            0x05  /* Return status for failed check of row protection */
#define STATUS_SRAM_ADDR_INVALID        0x06  /* Return statsu when check fails on start and stop address for parameters in SRAM */
#define STATUS_PL_CHECK_FAILED          0x07  /* Return status write to the page latch fails */
#define STATUS_WRITE_CHECK_FAILED       0x08  /* Return status when a command that is not resume is attempted when a command needs to be resumed */
#define STATUS_COMMAND_IN_PROGRESS      0x09  /* Return status for attempted RESUME command outside of SPC interrupt, Command still in progress. Timer still enabled. */
#define STATUS_INIT_READ_FAILED         0x0A  /* Return status for a failed check for a zero checksum after erase */
#define STATUS_INVALID_OPCODE           0x0B  /* The opcode given is not valid */
#define STATUS_KEY_OPCODE_MISMATCH      0x0C  /* The opcode given does not match the keys given */
#define STATUS_NO_SFLASH_READS_IN_OPEN  0x0D  /* Return status for attempted SFLASH read in open mode during a margin read command */
#define STATUS_INVALID_ST_ADDR          0x0E

/* Flash Specifications */

#define FLASH_NUM_PAGES         0x00000040                   /* 64 Pages of 2048 bits each (256 bytes) */
#define FLASH_PAGE_SIZE_BYTES   0x00000100                   /* 256 Bytes in a Flash Page */
#define FLASH_PAGE_SIZE_BITS    (FLASH_PAGE_SIZE_BYTES << 3) /* Number of bits in a Flash Page */
#define FLASH_NUM_PRIV_PAGES    0x00000000

/* OP CODES and OP CODE MASK */
#define OPC_SIL_ID    0x00
#define OPC_WR_ROW    0x05
#define OPC_PGM_ROW   0x06
#define OPC_NB_WR     0x07
#define OPC_NB_PGM    0x08
#define OPC_RESUME    0x09
#define OPC_ER_ALL    0x0A
#define OPC_CHECKSUM  0x0B
#define OPC_MM_READ   0x0C
#define OPC_WR_PROT   0x0D
#define OPC_MM_RESET  0x12
#define OPC_CALIBRATE 0x13
#define OPCODE_MASK   0x0000FFFF;

/*========================================================================================
; FM MODE Parameters
//---------------------------------------------------------------------------------------*/
#define READ_MODE               0x00   /* 4'b0000 */
#define PREPRG_MODE             0x01   /* 4'b0001 */
#define PAGE_WRALL_MODE         0x02   /* 4'b0010 */
#define HV_RESET_MODE           0x03   /* 4'b0011 */
#define CLEAR_MODE              0x04   /* 4'b0100 */
#define ERASE_WL_MODE           0x07   /* 4'b0111 */
#define ERASE_SUB_SC_MODE       0x08   /* 4'b1000 */
#define ERASE_SC_MODE           0x09   /* 4'b1001 */
#define ERASE_BULK_MODE         0x0A   /* 4'b1010 */
#define PROGRAM_WL_MODE         0x0B   /* 4'b1011 */
#define PROGRAM_SC_MODE         0x0C   /* 4'b1100 */
#define PROGRAM_SC_ALL_MODE     0x0D   /* 4'b1101 */
#define PROGRAM_BULK_MODE       0x0E   /* 4'b1110 */
#define PROGRAM_BULK_ALL_MODE   0x0F   /* 4'b1111 */

/* added to streamline code */
#define PREPGM_PAGE_MODE        0x10
#define PREPGM_SECTOR_MODE      0x11
#define PREPGM_BULK_MODE        0x12
#define PREPGM_BULK_ALL_MODE    0x13
#define PREPGM_SECTOR_CHK_MODE  0x14
#define PREPGM_MODE_MASK        0x10

/* TIMER CONFIG Register Paramaters */
/* Values to be stored in the 3rd byte of the TIMER_CFG_ADDR register (REG_TIMER_CONFIG) */
#define ACLK_EN                 0x08
#define PE_EN                   0x04
#define SCALE_HUNDREDS_MICRO    0x02
#define TIMER_EN                0x01

/* FM_SEQ Values */
#define SEQ_0                   0x00
#define SEQ_1                   0x01
#define SEQ_2                   0x02
#define SEQ_3                   0x03

/* Mask to enable IRQs */
#define HV_IRQ_MASK             0x01

/* Verify Mode values */
#define VERIFY_VMAR_POS         0x01
#define VERIFY_VMAR_NEG         0x02

/* --- end reg_default_defines --- */

/* --- flash_regs --- */
/* Addresses for FM SYS (REG_IF) regs  */

#define REG_FM_SYS_PA_ADDR                 REG32(FM_SYS_PA_ADDR)             /* Page Address Register */
#define REG_FM_SYS_PAGE_WRITE_ADDR         REG32(FM_SYS_PAGE_WRITE_ADDR)     /* Data to write at page address */
#define REG_FM_SYS_PAGE_WRITE_INC_ADDR     REG32(FM_SYS_PAGE_WRITE_INC_ADDR) /* Data to write at page address with auto address increment */
#define REG_FM_SYS_FM_ADDR                 REG32(FM_SYS_FM_ADDR)             /* Selects one of four 32 bit words on a lash read operation */
#define REG_FM_SYS_FM_READ                 REG32(FM_SYS_FM_READ)             /* Read Flash data from Address in FM_ADDR */
#define REG_FM_SYS_FM_READ_INC             REG32(FM_SYS_FM_READ_INC)         /* Read flash data from address in FM_ADDR, then increment the address */
#define REG_FM_SYS_FM_CTRL_ADDR            REG32(FM_SYS_FM_CTRL_ADDR)        /* Flash Macro configuration register. Includes FM_MODE, FM_SEQ and DAA MUX Select (DA[5:0]) */
#define REG_FM_SYS_TIMER_CFG_ADDR          REG32(FM_SYS_TIMER_CFG_ADDR)      /* Timer configuration register: includes Period_Low, Period_high, Timer enable, and Sense Amp config (CFG[5:0] */
#define REG_FM_SYS_ANALOG_CFG_ADDR         REG32(FM_SYS_ANALOG_CFG_ADDR)     /* Analog Configuration Register: includes itim, BDAC, MDAC, PDAC and NDAC */
#define REG_FM_SYS_TM_CFG_ADDR             REG32(FM_SYS_TM_CFG_ADDR)         /* TM (test mode) Configuration register */
#define REG_FM_SYS_STATUS_ADDR             REG32(FM_SYS_STATUS_ADDR)         /* Status Register */
#define REG_FM_SYS_WAIT_REG_ADDR           REG32(FM_SYS_WAIT_REG_ADDR)       /* Wait REgister: rests to 2 for page write and 5 for read */
#define REG_FM_SYS_MONITOR_ADDR            REG32(FM_SYS_MONITOR_ADDR)        /* Monitor register */
#define REG_FM_SYS_SW2FM_ADDR              REG32(FM_SYS_SW2FM_ADDR)          /* c_bus switch flag. Uses codes to switch from r_bus to c_bus */
#define REG_FM_SYS_SCRATCH_ADDR            REG32(FM_SYS_SCRATCH_ADDR)        /* Scratch pad register for test */
#define REG_FM_SYS_HV_CONFIG_ADDR          REG32(FM_SYS_HV_CONFIG_ADDR)      /* HV Configuration Register. PM clock frequency in MHz in multiples of 1MHZ * value in this register. Resets to 40MHz */
#define REG_FM_SYS_IRQ_MASK_ADDR           REG32(FM_SYS_IRQ_MASK_ADDR)       /* Mask register to enable IRQs */
#define REG_FM_SYS_ACLK_GEN_ADDR           REG32(FM_SYS_ACLK_GEN_ADDR)       /* Generate Aclk. Writing to this address will generate one Aclk pulse */
#define REG_FM_SYS_FM_READ_DUMMY           REG32(FM_SYS_FM_READ_DUMMY)       /* Read dummy used in the hv cycles routine */

/* Defines for 8bit registers within 32-bit REG_IF registers */
#define REG_BYTE_SELECT                    REG8(BYTE_SELECT)
#define REG_PA                             REG8(PA)
#define REG_BYA                            REG8(BYA)
#define REG_CAWA                           REG8(CAWA)
#define REG_RA                             REG8(RA)
#define REG_AXABA                          REG8(AXABA)
#define REG_FM_MODE                        REG8(FM_MODE)
#define REG_FM_SEQ                         REG8(FM_SEQ)
#define REG_DAA_MUX                        REG8(DAA_MUX)
#define REG_TIMER_PERIOD_LOW               REG8(TIMER_PERIOD_LOW)
#define REG_TIMER_PERIOD_HIGH              REG8(TIMER_PERIOD_HIGH)
#define REG_TIMER_CONFIG                   REG8(TIMER_CONFIG)
#define REG_SA_CONFIG                      REG8(SA_CONFIG)
#define REG_ITIM_BDAC                      REG8(ITIM_BDAC)
#define REG_MDAC                           REG8(MDAC)
#define REG_PDAC                           REG8(PDAC)
#define REG_NDAC                           REG8(NDAC)
#define REG_FM_CONTROL_REG1                REG8(FM_CONTROL_REG1)
#define REG_FM_CONTROL_REG2                REG8(FM_CONTROL_REG2)
#define REG_FM_CONTROL_REG3                REG8(FM_CONTROL_REG3)
#define REG_FM_CONTROL_REG4                REG8(FM_CONTROL_REG4)
#define REG_HV_TIMER_DO                    REG8(HV_TIMER_DO)
#define REG_STATUS_REG1                    REG8(STATUS_REG1)
#define REG_STATUS_REG2                    REG8(STATUS_REG2)
#define REG_STATUS_REG3                    REG8(STATUS_REG3)
#define REG_WAIT_REG_ADDR_0                REG8(WAIT_REG_ADDR_0)
#define REG_WAIT_REG_ADDR_1                REG8(WAIT_REG_ADDR_1)
#define REG_WAIT_REG_ADDR_2                REG8(WAIT_REG_ADDR_2)
#define REG_WAIT_REG_ADDR_3                REG8(WAIT_REG_ADDR_3)
#define REG_SCRATCH_PAD_0                  REG8(SCRATCH_PAD_0)
#define REG_SCRATCH_PAD_1                  REG8(SCRATCH_PAD_1)
#define REG_SCRATCH_PAD_2                  REG8(SCRATCH_PAD_2)
#define REG_SCRATCH_PAD_3                  REG8(SCRATCH_PAD_3)
#define REG_IRQ_MASK1                      REG8(IRQ_MASK1)
#define REG_IRQ_MASK2                      REG8(IRQ_MASK2)
#define REG_IRQ_MASK3                      REG8(IRQ_MASK3)
#define REG_IRQ_MASK4                      REG8(IRQ_MASK4)

/* --- end flash_regs --- */

#define SW2FM_READ                         0x00
#define SW2FM_SEQ0                         0xC3
#define SW2FM_SEQ1                         0xB2
#define SW2FM_SEQ2                         0x99

typedef struct
{
    uint8_t iByteAddr;
    uint8_t iLoadSize;
    uint32_t pageData[64];
} PageLatchData;

typedef struct
{
    uint8_t iPageAddress;
    uint8_t iSector; /* b'XXXXXX[AXA][BA] (X is don't care) */
} WriteRowData;

typedef struct
{
    uint8_t iPageAddress;
    uint8_t iSector; /* b'XXXXXX[AXA][BA] */
    uint8_t iCA;
    uint8_t iWA;
} ReadRowData;

typedef struct
{
    uint32_t STARTUP_KEY_RAM;
    uint32_t STARTUP_KEY_RAM_BAR;
    uint8_t PREPGMPW_LO;
    uint8_t PREPGMPW_HI;
    uint8_t PREPGM_NDAC;
    uint8_t PREPGM_PDAC;
    uint8_t PREPGM_BDAC;
    uint8_t ERSPW_LO;
    uint8_t ERSPW_HI;
    uint8_t ERS_NDAC;
    uint8_t ERS_PDAC;
    uint8_t PGMPW_LO;
    uint8_t PGMPW_HI;
    uint8_t PGM_NDAC;
    uint8_t PGM_PDAC;
    uint8_t PGM_BDAC;
    uint8_t SEQ1PW_LO;
    uint8_t SEQ1PW_HI;
    uint8_t SEQ2PREPW_LO;
    uint8_t SEQ2PREPW_HI;
    uint8_t SEQ2POSTPW_LO;
    uint8_t SEQ2POSTPW_HI;
    uint8_t SEQ3PW_LO;
    uint8_t SEQ3PW_HI;
    uint8_t CTAT_PGM;
    uint8_t CTAT_ERS;
    uint8_t WAIT_WR_HVPL;
    uint8_t VMAR0;
    uint8_t VMAR1;
    uint8_t NEGMM_NDAC;
    uint8_t POSMM_PDAC;
    uint8_t VFY_RAMP_LO;
    uint8_t VFY_DISCH_LO;
    uint8_t MMDELAY_LO;
    uint8_t MMDELAY_HI;
    uint8_t PGM_MDAC;
    uint8_t RESERVED_2;
    uint8_t RESERVED_3;
    uint8_t RESERVED_4;
    uint8_t RESERVED_5;
    uint8_t RESERVED_6;
    uint8_t RESERVED_7;
    uint8_t RESERVED_8;
} EnvmTrims;

typedef struct
{
    uint8_t SDAC;
    uint8_t IDAC;
    uint8_t ITIM;
    uint8_t VREF_VLEVEL;
    uint8_t VREF_ILEVEL_N;
    uint8_t VBG_CURVE;
    uint8_t VBG_ILEVEL;
    uint8_t STARTUP_DELAY;
} EnvmReadTrims;


/* APIs (to be built) */
uint8_t LoadPageLatch(void);
void     WriteRow(void);
void     PrePgmRow(void);
void     EraseRow(void);
void     ProgramRow(void);
uint8_t  WriteRowVerify(void);
void     SectorPgm(void);
void     SectorErase(void);
void     BulkErase(void);
void     BulkPrePgm(void);
void     SubSectorErase(void);
void     SectorPrePgm(void);
void     LoadTrimValues(void);
void     SectorChkPrePgm(void);
void     SectorChkPgm(void);

uint8_t  envm_system_service_read_parameters(void);

/* Larger Subroutines */
void     HvCycles(uint8_t cMode, uint8_t timerHiByte, uint8_t timerLoByte);
void     HvCyclesExit(void);

/* Function to set parameters used by Flash routines  */
void     SetWriteRowParams(uint8_t iPageAddress, uint8_t iAXA, uint8_t iBA);
void     SetReadRowParams(uint8_t iPageAddress, uint8_t iAXA, uint8_t iBA, uint8_t iCA, uint8_t iWA);
void     Enter_C_Bus_Mode(void);
void     RunTimer (void);
void     RunTimerWait (void);
void     StrobeAclk(void);
void     AclkRunTimer(void);
void     setClockFrequency(uint8_t clock_f);
void     ClearPageLatch(void);
void     Exit_C_Bus_Mode(void);

#ifdef __cplusplus
}
#endif

#endif /* _ENVM_H */


