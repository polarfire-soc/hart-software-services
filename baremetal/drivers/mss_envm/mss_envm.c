/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eNVM bare metal software driver implementation.
 *
 */
 
#include "drivers/mss_envm/envm.h"
#include "drivers/mss_envm/mss_envm.h"
#include "mpfs_hal/mss_sysreg.h"

static PageLatchData pageLatchData;
static WriteRowData  writeRowData;
static ReadRowData   readRowData;
static EnvmTrims     envmTrims;
static EnvmReadTrims envmReadTrims;
static uint8_t       envm_clock_freq = DEFAULT_CLOCK_FREQ ;
static uint_fast8_t  envm_lock_prog_page = 1u; /* default value is lock active */

/**
* envm_set_clock function which setups the clock and timing for the eNVM
*
* Must be called after envm_init()
*
* \param[in] mss_frequency is the frequency the MSS is running at in MHz
*/
void envm_set_clock(uint32_t mss_frequency)
{
    uint8_t  clock_cfg_div  = (uint8_t)(1u << ((SYSREG->CLOCK_CONFIG_CR >> 4) & 0x3)); /* ahb clock divider */
    uint32_t mss_freq       = mss_frequency / clock_cfg_div;
    uint8_t envm_clock_div = ((mss_freq + (SYSENVMREG_ENVM_CLOCK_FREQ/2)) / SYSENVMREG_ENVM_CLOCK_FREQ) - 1;

    SYSREG->ENVM_CR |= (uint32_t)(((uint32_t)envm_clock_div << SYSENVMREG_ENVM_CR_CLK_PERIOD_SHIFT) & SYSENVMREG_ENVM_CR_CLK_PERIOD);

    /* calculate how many clock ticks there are in 1us */
    envm_clock_freq = (uint8_t)((mss_freq + ((envm_clock_div + 1U)/2u)) / (envm_clock_div + 1U));  /* added (envm_clock_div+1)/2u) for integer rounding */

    /* Set clock value */
    Enter_C_Bus_Mode();
    setClockFrequency((uint8_t)envm_clock_freq);
    Exit_C_Bus_Mode();
}

/**
* envm_load_parameters(void) function which loads the paramters for the eNVM.
*
* Called from envm_init()
*
* \post envmReadTrims & envmTrims is initialized
*
*/
static void envm_load_parameters(const uint8_t * p_envm_params)
{
    uint8_t * envmTrims8 = (uint8_t *) &envmTrims;
    uint8_t * envmReadTrims8 = (uint8_t *) &envmReadTrims;
    uint8_t inc;

    for(inc = 0; inc < sizeof(envmTrims); inc++)
    {
        envmTrims8[inc] = p_envm_params[inc];
    }

    for(inc = 0; inc < sizeof(envmReadTrims); inc++)
    {
        envmReadTrims8[inc] = p_envm_params[inc + 52u];
    }
}

/**
* envm_init function which sets up the programming and read parameters.
*
* Must be called first in order to use eNVM
*
* \pre eNVM powered up by controller
*
* \retval 0 passed
* \retval 1 failed
*
*/
uint8_t envm_init(const uint8_t * p_envm_params)
{
    /* setup global variables */
    pageLatchData.iByteAddr = 0;
    pageLatchData.iLoadSize = (PAGE_SIZE/4u)-1;

    envm_load_parameters(p_envm_params);
    envm_lock_prog_page = 0u;

    envmTrims.PREPGM_NDAC = (envmTrims.PREPGM_NDAC & 0x1f);
    envmTrims.PREPGM_PDAC = (envmTrims.PREPGM_PDAC & 0x1f);
    envmTrims.PGM_NDAC    = (envmTrims.PGM_NDAC & 0x1f);
    envmTrims.PGM_PDAC    = (envmTrims.PGM_PDAC & 0x1f);
    envmTrims.ERS_NDAC    = (envmTrims.ERS_NDAC & 0x1f);
    envmTrims.ERS_PDAC    = (envmTrims.ERS_PDAC & 0x1f);
    envmTrims.NEGMM_NDAC  = (envmTrims.NEGMM_NDAC & 0x1f);
    envmTrims.POSMM_PDAC  = (envmTrims.POSMM_PDAC & 0x1f);

    envmReadTrims.STARTUP_DELAY = (uint8_t)((envmReadTrims.STARTUP_DELAY + 1) * 20); /* 20us steps */

    return((uint8_t)envm_lock_prog_page);
}


/**
* Top level envm_program_page function which takes in absolute address
* performs a full program and verify cycle of the eNVM
* performs a preprogram - erase - write cycle and then a verify
*
* Erased is 0 and Program is 1
*
* \pre pageLatchData.pageData correctly setup with data to be programmed
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was programmed successfully
* \retval 1 eNVM verify error occurred during programming
* \retval 2 page is outside boundary error
* \retval 3 page latch load error
* \retval 4 startup previously failed
*/
uint_fast8_t envm_program_page_address(uint64_t absolute_address)
{
    envm_addr_t envm_info;

    if (envm_address(absolute_address, &envm_info))
    {
        return 2u;
    }

    return envm_program_page(envm_info.sector, envm_info.page);
}


/**
* Top level envm_erase_sector function which takes in absolute address
* performs a preprogram - erase cycle of a sector
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was erased successfully
* \retval 2 page is outside boundary error
* \retval 4 startup previously failed
*/
uint_fast8_t envm_erase_sector_address(uint64_t absolute_address)
{
    envm_addr_t envm_info;

    if(envm_address(absolute_address, &envm_info))
    {
        return 2u;
    }

    return envm_erase_sector(envm_info.sector);
}


/**
* Top level envm_erase_bulk function
* performs a preprogram - erase cycle of all sectors
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was erased successfully
* \retval 4 startup previously failed
*/
uint_fast8_t envm_erase_bulk(void)
{
    if(envm_lock_prog_page)
    {
        return 4u;
    }

    /*
     * The original code had the sector address 0 sequence first and the sector
     * address 2 sequence second. This works for the Renode model but not for
     * the emulation platform. This order works for both but we will need to
     * check how the real hardware works and possibly update the Renode model
     * on the basis of that so they are consistent.
     */
    writeRowData.iSector = 2u;

    BulkPrePgm();
    RunTimerWait();
    HvCyclesExit();

    writeRowData.iSector = 2u;

    BulkErase();
    RunTimerWait();
    HvCyclesExit();

    writeRowData.iSector = 0u;

    BulkPrePgm();
    RunTimerWait();
    HvCyclesExit();

    writeRowData.iSector = 0u;

    BulkErase();
    RunTimerWait();
    HvCyclesExit();
    return 0u;
}


/**
* performs an erase of a full sector of the eNVM
* performs a preprogram - erase cycle
*
* \param[in] sector specifies one of the four sectors
*
* \retval 0 eNVM was programmed successfully
* \retval 4 startup previously failed
* \retval 5 sector outside bounds error
*/
uint_fast8_t envm_erase_sector(uint_fast8_t sector)
{
    if(envm_lock_prog_page)
    {
        return 4u;
    }

    if(sector > 3)
    {
        return 5u;
    }

    /* sets up sector and page address */
    writeRowData.iSector = (uint8_t)sector;
    writeRowData.iPageAddress = 0u;

    SectorPrePgm();
    RunTimerWait();
    HvCyclesExit();

    SectorErase();
    RunTimerWait();
    HvCyclesExit();

    return 0u;
}


/**
* Returns the eNVM Sector & Page information derived from eNVM absolute address
* Returns 0 if Sector & Page are valid
*
*/
uint_fast8_t envm_address(uint64_t absolute_address, envm_addr_t * envm_address)
{
    uint_fast8_t sector;
    uint_fast8_t page;
    uint_fast8_t max_pages;

    /* determine sector and page address
     * check sector address is valid */
    if(absolute_address < MSS_ENVM_DATA_SECTOR2)
    {
        return 1u;
    }
    else if(absolute_address < MSS_ENVM_DATA_SECTOR0)
    {
        sector    = 2u;
        page      = (uint_fast8_t)((absolute_address - MSS_ENVM_DATA_SECTOR2) /
                    PAGE_SIZE);
        max_pages = NUM_SM_PAGES;
    }
    else if(absolute_address < MSS_ENVM_DATA_SECTOR1)
    {
        sector    = 0u;
        page      = (uint_fast8_t)((absolute_address - MSS_ENVM_DATA_SECTOR0) /
                    PAGE_SIZE);
        max_pages = NUM_FM_PAGES;
    }
    else if(absolute_address < MSS_ENVM_DATA_SECTOR3)
    {
        sector    = 1u;
        page      = (uint_fast8_t)((absolute_address - MSS_ENVM_DATA_SECTOR1) /
                    PAGE_SIZE);
        max_pages = NUM_FM_PAGES;
    }
    else if(absolute_address < (MSS_ENVM_DATA_SECTOR3 + NUM_SM_PAGES*PAGE_SIZE))
    {
        sector    = 3u;
        page      = (uint_fast8_t)((absolute_address - MSS_ENVM_DATA_SECTOR3) /
                    PAGE_SIZE);
        max_pages = NUM_SM_PAGES;
    }
    else
    {
        return 1u;
    }

    /* check page address is valid */
    if(page >= max_pages) {
        return 1u;
    }

    envm_address->sector = (uint8_t)sector;
    envm_address->page   = (uint8_t)page;

    return 0u;
}


/**
* performs a full program and verify cycle of the eNVM
* performs a preprogram - erase - write cycle and then a verify
*
* Erased is 0 and Program is 1
*
* \pre pageLatchData.pageData correctly setup with data to be programmed
*
* \param[in] sector specifies one of the four sectors
* \param[in] page specifies one of the pages within a sector
*
* \retval 0 eNVM was programmed successfully
* \retval 1 eNVM verify error occurred during programming
* \retval 2 page is outside boundary error
* \retval 3 page latch load error
* \retval 4 startup previously failed
* \retval 5 sector outside bounds error
*/
uint_fast8_t envm_program_page(uint_fast8_t sector, uint_fast8_t page)
{
    uint_fast8_t status = 0u;

    if(envm_lock_prog_page)
    {
        return 4u;
    }

    if(sector > 3)
    {
        return 5u;
    }

    /* sets up sector and page address */
    writeRowData.iSector = (uint8_t)sector;
    writeRowData.iPageAddress = (uint8_t)page;

    /* check page is within sector */
    if(((sector < 2)  && (page >= NUM_FM_PAGES)) ||
       ((sector >= 2) && (page >= NUM_SM_PAGES)))
    {
        return 2u;
    }

    /* loads pageLatchData into the latch */
    if(LoadPageLatch())
    {
        return 3u;
    }

    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress; /* 8 bit Row Address is expected */
    REG_AXABA = writeRowData.iSector;  /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    HvCycles(PREPGM_PAGE_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);
    RunTimerWait();

    /*End HV operations */
    HvCyclesExit();

    Enter_C_Bus_Mode();

    /* Perform Erase with HvCycles */
    HvCycles(ERASE_WL_MODE, envmTrims.ERSPW_HI, envmTrims.ERSPW_LO );
    RunTimerWait();

    /* End HV operations */
    HvCyclesExit();

    Enter_C_Bus_Mode();

    /* Perform Program with HvCycles */
    HvCycles(PROGRAM_WL_MODE, envmTrims.PGMPW_HI, envmTrims.PGMPW_LO);
    RunTimerWait();

    /* End HV operations */
    HvCyclesExit();

    /* 0V Verify */
    status |= WriteRowVerify();

    return (status == 0) ? 0u : 1u;
}


/**
* load block of data into pageData
*
*/
void envm_set_page_data(uint32_t *source)
{
    int i;

    for(i = 0; i < 64; i++, source++)
    {
        pageLatchData.pageData[i] = *source;
    }
}


/**
* clears pageData contents to 0
*
*/
void envm_clear_page_data(void)
{
    uint_fast8_t i;
    for(i = 0; i < 64; i++)
    {
        pageLatchData.pageData[i] = 0;
    }
}


/**
*
*
*/
void Enter_C_Bus_Mode(void)
{
    /* Suppress Clock */
    SYSREG->ENVM_CR       |= SYSENVMREG_ENVM_CR_CLK_SUPPRESS;
    REG_FM_SYS_SW2FM_ADDR  = SW2FM_SEQ0;
    REG_FM_SYS_SW2FM_ADDR  = SW2FM_SEQ1;
    REG_FM_SYS_SW2FM_ADDR  = SW2FM_SEQ2;
    SYSREG->ENVM_CR       &= ~SYSENVMREG_ENVM_CR_CLK_SUPPRESS;
}


/******************************************************************************************
* Function Name: Hv Cycles
******************************************************************************************
* Summary:  This subroutine loads the REG_IF parameters into their respective
  registers and performs the HV Cycle Sequencing with required delays.
*           ***
*
* Parameters: 

  @param cMode Holds the high voltage mode, and three control bits for vctat ,
  pwall mode, and non-blocking
  mode = {[7] [6] [5] [4] [3:0]}
  7: 1 for all Program Modes, 0 for all Erase Modes
  6: 1 for non-blocking commands, 0 for all other commands.
  5: 1 to set PWALL mode, 0 all other cases.
  4: Not used
  [3:0]: Flash Mode that is written to FM_MODE reg.

  @param timerHiByte Holds the 8 bits to be programmed into the TIMER_PERIOD_HIGH register
    in REG_IF during sequence 2 of the High voltage operation

  @param timerLoByte Holds the 8 bits to be programmed into the TIMER_PERIOD_LOW register
    in REG_IF during sequence 2 of the High voltage operation
*
* Return Values: NONE
  
******************************************************************************************/
void HvCycles(uint8_t cMode, uint8_t timerHiByte, uint8_t timerLoByte)
{
    volatile uint32_t dummyData;

    /* check if one of prepgm sector or page mode */
    if(cMode & PREPGM_MODE_MASK)
    {
        REG_NDAC = envmTrims.PREPGM_NDAC;
        REG_PDAC = envmTrims.PREPGM_PDAC;
        /* Set BDAC trim (leave ITIM alone) */
        REG_ITIM_BDAC  = (uint8_t)((REG_ITIM_BDAC & 0xF0) |
                         (envmTrims.PREPGM_BDAC & 0x0F));
      
        REG_FM_MODE = PREPRG_MODE;
        REG_FM_SEQ  = SEQ_0;
        StrobeAclk();
    }

    /* Set Timer and DAC values if this is a single WL operation */
    switch (cMode)
    {
        case PREPGM_PAGE_MODE:
            cMode = PROGRAM_WL_MODE;
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;

        case PREPGM_SECTOR_CHK_MODE:
            cMode = PROGRAM_SC_MODE;
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;

        case PREPGM_SECTOR_MODE:
            cMode = PROGRAM_SC_ALL_MODE;
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;

        case PREPGM_BULK_MODE:
            cMode = PROGRAM_BULK_MODE;
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;

        case PREPGM_BULK_ALL_MODE:
            cMode = PROGRAM_BULK_ALL_MODE;
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;

        case ERASE_WL_MODE:
            REG_NDAC = envmTrims.ERS_NDAC;
            REG_PDAC = envmTrims.ERS_PDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_ERS & 0x07));
            break;

        case ERASE_SC_MODE:
        case ERASE_SUB_SC_MODE:
        case ERASE_BULK_MODE:
            REG_PDAC = envmTrims.ERS_PDAC;
            REG_NDAC = envmTrims.ERS_NDAC;
            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_ERS & 0x07));
            break;

        case PROGRAM_WL_MODE:
        case PROGRAM_SC_MODE:
        case PROGRAM_SC_ALL_MODE:
        case PROGRAM_BULK_MODE:
        case PROGRAM_BULK_ALL_MODE:
            REG_MDAC = envmTrims.PGM_MDAC;
            REG_NDAC = envmTrims.PGM_NDAC;
            REG_PDAC = envmTrims.PGM_PDAC;
            /* Set BDAC trim (leave ITIM alone) */
            REG_ITIM_BDAC  = (uint8_t)((REG_ITIM_BDAC & 0xF0) |
                                       (envmTrims.PGM_BDAC & 0x0F));

            REG_WAIT_REG_ADDR_3 = (uint8_t)((REG_WAIT_REG_ADDR_3 & 0xF8) |
                                           (envmTrims.CTAT_PGM & 0x07));
            break;
    }

    /* Bulk and sector operations assume DAC and Timer setting set previous
    * to HvPulse operation */
    REG_FM_MODE = (cMode & 0x0F); /* Set Flash mode for HV operation */

    /* Run SEQ1 */
    REG_FM_SEQ = SEQ_1;
    REG_TIMER_PERIOD_LOW = envmTrims.SEQ1PW_LO; /* Set SEQ1 timer */
    REG_TIMER_PERIOD_HIGH = envmTrims.SEQ1PW_HI;
    AclkRunTimer(); /* Enables Ackl Timer and waits until timer time's out */

    /* Run FM control to SEQ2 pre-pulse */
    REG_FM_SEQ = SEQ_2;
    REG_TIMER_PERIOD_LOW = envmTrims.SEQ2PREPW_LO; /* Set SEQ2 pre pw timer */
    REG_TIMER_PERIOD_HIGH = envmTrims.SEQ2PREPW_HI;
    AclkRunTimer(); /* Enables Ackl Timer and waits until timer time's out */

    dummyData = REG_FM_SYS_STATUS_ADDR;
    (void) dummyData; //Use to avoid compiler warning

    /* Do HV Pulse */
    REG_TIMER_PERIOD_LOW = timerLoByte; /* Timing parameters offset by input mode
                                           (Prepgm, Erase, or Program) */
    REG_TIMER_PERIOD_HIGH = timerHiByte;
    REG_TIMER_CONFIG |= PE_EN;
    RunTimer(); /* Enables Timer and waits until timer time's out */
    REG_FM_SYS_IRQ_MASK_ADDR = HV_IRQ_MASK; /* Unmask the timer interrupt */
}


/*****************************************************************************
* Function Name: Hv Cycles Exit
******************************************************************************
* Summary: Called after pulse is done or from handler routine
*         
* Called By: None
*
* Calls: HvCycles
*
*
*
*****************************************************************************/
void HvCyclesExit(void)
{
    volatile uint32_t dummyData;

    dummyData = REG_FM_SYS_STATUS_ADDR; /* clear c_irq */

    REG_TIMER_CONFIG &= ~PE_EN;

    /* Mask Timer Interrupt */
    REG_FM_SYS_IRQ_MASK_ADDR = 0x00;

    /* Run FM control to SEQ2 post-pulse */
    REG_TIMER_PERIOD_LOW = envmTrims.SEQ2POSTPW_LO; /* Set SEQ2 post pw timer */
    REG_TIMER_PERIOD_HIGH = envmTrims.SEQ2POSTPW_HI;
    RunTimer();
    RunTimerWait();

    /* Run SEQ3 */
    REG_FM_SEQ = SEQ_3;
    REG_TIMER_PERIOD_LOW = envmTrims.SEQ3PW_LO; /* Set SEQ3 pw timer */
    REG_TIMER_PERIOD_HIGH = envmTrims.SEQ3PW_HI;
    AclkRunTimer(); /* Enables Ackl Timer and waits until timer time's out */

    REG_TIMER_CONFIG = 0x00;

    /* Set FM control to mode 0 (read) and seq 0 */
    REG_FM_MODE = READ_MODE;
    REG_FM_SEQ = SEQ_0;

    /* Do two dummy reads to generate 2 Aclks */

    dummyData = REG_FM_SYS_FM_READ_DUMMY;
    dummyData = REG_FM_SYS_FM_READ_DUMMY;
    (void) dummyData; // use to avoid compiler warning

    REG_FM_SYS_FM_CTRL_ADDR = 0x00000000; /* Set Flash control register back to zero */
    REG_PA = 0x00;

    Exit_C_Bus_Mode();
}


/*****************************************************************************
* Function Name: Load Page Latch
******************************************************************************
* Summary: Loads the page latch buffer with data to be programmed into a row
*          of flash. Load sizes are in granularity  of 32-bits to a row size of
*          256 bytes. Data is programmed into
*          the page latch buffer starting at the location specified by the
*          byte address input parameter. Data programmed into the latch buffer
*          will remain until a program is performed, which will clear the page
*          latch contents.
*
* Called By: None
*
* Calls: None
*
*
* Return Values: None
*
* Return Status: STATUS_SUCCESS
*                STATUS_INVALID_PL_ADDRESS - The start address and size are
*                invalid.
*****************************************************************************/
uint8_t LoadPageLatch(void)
{ 
    uint8_t  iByteAddr     = pageLatchData.iByteAddr; /* PL Byte Address */
    uint8_t  iLoadSize     = pageLatchData.iLoadSize; /* PL Load Size */
    uint8_t  iFinalAddress = (uint8_t)(iByteAddr + iLoadSize);
    uint32_t lRowSize      = (uint32_t)PAGE_SIZE/((uint32_t)WORD_SIZE/8); /* lRowSize */

    Enter_C_Bus_Mode();

    /* Check to make sure the load does not overflow the page latch */
    if(iFinalAddress < lRowSize)
    {
        REG_PA = pageLatchData.iByteAddr; /* 2048 bits in a row / 32 bits at a time. 64 possible addresses in REG_PA[5:0] */
    
        /* No longer need lByteAddr param - use as interation variable.
         * for a loop of start=0 to lLoadSize (lLoadSize=0 for 1 byte) */
        for(iByteAddr = pageLatchData.iByteAddr; iByteAddr <= iFinalAddress; iByteAddr++)
        {
            /* load page latch data register with user data */
            REG_FM_SYS_PAGE_WRITE_INC_ADDR = pageLatchData.pageData[iByteAddr];
        }
    } /* end if page load size + lByteAddr check */
    else
    {
        Exit_C_Bus_Mode();
        return (uint8_t)STATUS_INVALID_PL_ADDRESS;
    }   /* end else page load size + lByteAddr check */

    /* Clear control register */
    REG_FM_SYS_FM_CTRL_ADDR = 0x00000000;

    Exit_C_Bus_Mode();
    return (uint8_t)STATUS_SUCCESS;
} /* end LoadFlashBytes command */


/*****************************************************************************
* Function Name: Sector Pre-Program
******************************************************************************
* Summary: PrePrograms the sector selected by the WriteRowPaData Struct sector variable.
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: 
*
* Return Values: STATUS
*
*****************************************************************************/
void SectorPrePgm(void)
{
    Enter_C_Bus_Mode();

    /* Set Sector to work on */
    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector;

    HvCycles(PREPGM_SECTOR_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);
}


/*****************************************************************************
* Function Name: Sector Erase
******************************************************************************
* Summary: Erase all data in a sector. Sector should be selected before this call
*         with the SetWriteRowParams() function
*          ***
* Called By: None
*
* Calls: 
*
* Parameters: 
*
* Return Values: None
*
* Return Status: 
*
*****************************************************************************/
void SubSectorErase(void)
{ 
    Enter_C_Bus_Mode();

    /* Set the Sector to work on */
    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector;

    HvCycles(ERASE_SUB_SC_MODE, envmTrims.ERSPW_HI,envmTrims.ERSPW_LO);
} /* end SubSectorErase command */


/*****************************************************************************
* Function Name: Sector Erase
******************************************************************************
* Summary: Erase all data in a sector. Sector should be selected before this call
*         with the SetWriteRowParams() function
*          ***
* Called By: None
*
* Calls: 
*
* Parameters: 
*
* Return Values: None
*
* Return Status: 
*
*****************************************************************************/
void SectorErase(void)
{
  Enter_C_Bus_Mode();

    /* Set the Sector to work on: SetWriteRowParams should be called before this function */
    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector;

    HvCycles(ERASE_SC_MODE, envmTrims.ERSPW_HI,envmTrims.ERSPW_LO);
} /* end SectorErase command */


/*****************************************************************************
* Function Name: Bulk Erase
******************************************************************************
* Summary: Erase all data in Two sectors depending on MSB bit of the 2-bit sector address
*          ***
* Called By: None
*
* Calls:
*
* Parameters:
*
* Return Values: None
*
* Return Status:
*
*****************************************************************************/
void BulkErase(void)
{
    Enter_C_Bus_Mode();

    /* Set the Sector to work on: SetWriteRowParams should be called before this function */
    REG_RA    = 0u;
    REG_AXABA = writeRowData.iSector;  /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    HvCycles(ERASE_BULK_MODE, envmTrims.ERSPW_HI,envmTrims.ERSPW_LO);
} /* end SectorErase command */


/*****************************************************************************
* Function Name: Sector Program 
******************************************************************************
* Summary: Programs all of the rows in a sector
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: NONE
*
* Return Values: STATUS
*
*****************************************************************************/
void SectorPgm(void)
{
    Enter_C_Bus_Mode();

    /* Set Sector to work on */
    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector; /* Select sector */

    HvCycles(PROGRAM_SC_ALL_MODE, envmTrims.PGMPW_HI,envmTrims.PGMPW_LO);
}


/*****************************************************************************
* Function Name: Write Row
******************************************************************************
* Summary: Write Row: Erases then programs the addressed row of non-privileged
*          with data in the page latch buffer.
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: Row Id: (Size: 2 bytes) (Location: 2 + value in sysarg reg)
*             Number of row to write or program. (I.E.0x00 - row 0)
*
*
*
*****************************************************************************/
void WriteRow(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress; /* 8 bit Row Address is expected */
    REG_AXABA = writeRowData.iSector; /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    /* Perform Pre- Program with HvCycles
     * PRE_PRG_BASE is start address of PRE_PGM PW/DACS/CTAT_SLOPE
     * PGM_BASE is start address of PGM PW/DACS/CTAT_SLOPE */

    HvCycles(PREPGM_PAGE_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);
    RunTimerWait();

    /* End HV operations */
    HvCyclesExit();

    Enter_C_Bus_Mode();

    /* Perform Erase with HvCycles */
    HvCycles(ERASE_WL_MODE, envmTrims.ERSPW_HI, envmTrims.ERSPW_LO );
    RunTimerWait();

    /* End HV operations */
    HvCyclesExit();

    Enter_C_Bus_Mode();

    /* Perform Program with HvCycles */
    HvCycles(PROGRAM_WL_MODE, envmTrims.PGMPW_HI, envmTrims.PGMPW_LO);
    RunTimerWait();

    /* End HV operations*/
    HvCyclesExit();
} /* end WriteRow command */


/*****************************************************************************
* Function Name: Pre Program Row
******************************************************************************
* Summary: Pre-programs a row without following through with erase or program
*          operations. Expects Row data to be set in SRAM using the
*          SetWriteRowParams() function
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: Row Id: (Size: 2 bytes) (Location: 2 + value in sysarg reg)
*             Number of row to write or program. (I.E.0x00 - row 0)
*
*****************************************************************************/
void PrePgmRow(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress; /* 8 bit Row Address is expected */
    REG_AXABA = writeRowData.iSector;  /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    /* Perform Pre- Program with HvCycles
     * PRE_PRG_BASE is start address of PRE_PGM PW/DACS/CTAT_SLOPE
     * PGM_BASE is start address of PGM PW/DACS/CTAT_SLOPE */

    HvCycles(PREPGM_PAGE_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);

}


/*****************************************************************************
* Function Name: Bulk Row
******************************************************************************
* Summary: Pre-programs all sectors without falling through with erase or program
*            operations.
*          ***
* Called By: None
*
* Calls: HvCycles
*
*
*
*
*****************************************************************************/
void BulkPrePgm(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = 0u;
    REG_AXABA = writeRowData.iSector; /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    /* Perform Pre- Program with HvCycles
     * PRE_PRG_BASE is start address of PRE_PGM PW/DACS/CTAT_SLOPE
     * PGM_BASE is start address of PGM PW/DACS/CTAT_SLOPE */

    HvCycles(PREPGM_BULK_ALL_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);
} /* end PrePgmRow command */


/*****************************************************************************
* Function Name: Erase Row
******************************************************************************
* Summary: Erase data in a row. Expects SetWriteRowParams() to have already been 
            called to identify which row to erase. 
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: Row Id: (Size: 2 bytes) (Location: 2 + value in sysarg reg)
*             Number of row to write or program. (I.E.0x00 - row 0)
*
*
*
*****************************************************************************/
void EraseRow(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress; /* 8 bit Row Address is expected */
    REG_AXABA = writeRowData.iSector; /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    /* Perform Erase with HvCycles */
    HvCycles(ERASE_WL_MODE, envmTrims.ERSPW_HI, envmTrims.ERSPW_LO );
} /* end EraseRow command */


/*****************************************************************************
* Function Name: Program Row
******************************************************************************
* Summary: Programs a row without folling through with erase or program 
            operations. Expects Row data to be set in SRAM using the SetWriteRowParams()
            function
*          ***
* Called By: None
*
* Calls: HvCycles
*
* Parameters: Row Id: (Size: 2 bytes) (Location: 2 + value in sysarg reg)
*             Number of row to write or program. (I.E.0x00 - row 0)
*
*
*
*****************************************************************************/
void ProgramRow(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress; /* 8 bit Row Address is expected */
    REG_AXABA = writeRowData.iSector;  /* AXA bit REG_AXABA[1], BA bit REG_AXABA[0] */

    /* Perform Program with HvCycles */
    HvCycles(PROGRAM_WL_MODE, envmTrims.PGMPW_HI, envmTrims.PGMPW_LO);
} /* end ProgramRow command */


/*****************************************************************************
* Function Name: Write Row Verify
******************************************************************************
* Summary:  Verifies that the data from the page latch was written to the addressed
            row as expected using the cmpr test mode.The cmpr test mode checks 64-bits
            at a time, so to check an entire row, the column address is swept. 
*
* Paramaters: 

* Called By: None
*
* Calls: None
*
* Return Values: None
*
* Return Status: STATUS_SUCCESS
*                STATUS_WRITE_CHECK_FAILED- The page latch data did not write to the row correctly
*****************************************************************************/
uint8_t WriteRowVerify(void)
{
    volatile uint32_t dummyData;
    uint8_t i;  /* Generic loop counter */
    uint8_t status = STATUS_SUCCESS;

    Enter_C_Bus_Mode();

    /* Set Sector */
    REG_AXABA = writeRowData.iSector;

    /* Set Row address */
    REG_RA = writeRowData.iPageAddress;

    /* latch in addressing */
    REG_FM_SYS_ACLK_GEN_ADDR = 0x00;

    /* Set test mode to compare row to page latch */
    REG_FM_SYS_TM_CFG_ADDR |= 0x00008000;

    /* set CTAT to 0 during Verify */
    REG_WAIT_REG_ADDR_3 = (REG_WAIT_REG_ADDR_3 & 0xF8);

    /* Latch in read mode */
    REG_FM_MODE = READ_MODE;
    REG_FM_SEQ = SEQ_0;
    REG_FM_SYS_ACLK_GEN_ADDR = 0x00;

    /* Sweep columns and check cmprx bit */
    for(i = 0; i < 32; i++)
    {
        REG_CAWA = (uint8_t)(i << 1); /* Shift left 1 to ignore WA bit. */
        dummyData = REG_FM_SYS_FM_READ_DUMMY; /* Apparently you have to read the dummy register to increment the page latch counter */
        (void) dummyData; // use to avoid compiler warning
        REG_FM_SYS_ACLK_GEN_ADDR = 0x00;  /* Latch in the column address change */

        /* Check the cmprx bit in the FM_SYS_MONITOR register */
        if(!(REG_FM_SYS_MONITOR_ADDR & 0x00000004))
        {
            status = STATUS_WRITE_CHECK_FAILED;
            break;
        }
    }

    REG_FM_SYS_TM_CFG_ADDR &= 0xFCFF7FF0; /* Turn off the test modes */
    REG_MDAC = 0u;

    REG_FM_MODE = READ_MODE;
    REG_FM_SEQ = SEQ_0;
    REG_FM_SYS_ACLK_GEN_ADDR = 0x00;

    REG_MDAC = envmTrims.PGM_MDAC; /* set MDAC to PGM condition */

    Exit_C_Bus_Mode();
    return status;
}


/******************************************************************************************
 *
 ******************************************************************************************/
void SetWriteRowParams(uint8_t iPageAddress, uint8_t iAXA, uint8_t iBA)
{
    writeRowData.iPageAddress = iPageAddress;
    writeRowData.iSector      = (uint8_t)((iAXA << 1) | iBA);
}


/******************************************************************************************
 *
 ******************************************************************************************/
void SetReadRowParams(uint8_t iPageAddress, uint8_t iAXA, uint8_t iBA, uint8_t iCA, uint8_t iWA)
{
    readRowData.iPageAddress = iPageAddress;
    readRowData.iSector      = (uint8_t)((iAXA << 1) | iBA);
    readRowData.iCA          = iCA;
    readRowData.iWA          = iWA;
}


/******************************************************************************************
 *
 ******************************************************************************************/
void SectorChkPrePgm(void)
{ 
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector;

    HvCycles(PREPGM_SECTOR_CHK_MODE, envmTrims.PREPGMPW_HI, envmTrims.PREPGMPW_LO);
}


/******************************************************************************************
 *
 ******************************************************************************************/
void SectorChkPgm(void)
{
    Enter_C_Bus_Mode();

    REG_RA    = writeRowData.iPageAddress;
    REG_AXABA = writeRowData.iSector;

    HvCycles(PROGRAM_SC_MODE,envmTrims.PGMPW_HI, envmTrims.PGMPW_LO);
}


/******************************************************************************************
 *
 ******************************************************************************************/
/* Start the timer */
void RunTimer (void)
{
    REG_TIMER_CONFIG |= TIMER_EN; /* Start and then poll for timer to expire */
}


/******************************************************************************************
 *
 ******************************************************************************************/
/* Wait for the timer to complete */
void RunTimerWait (void)
{
    while(REG_TIMER_CONFIG & TIMER_EN)
        ;
}


/******************************************************************************************
 *
 ******************************************************************************************/
void StrobeAclk(void)
{
    REG_FM_SYS_ACLK_GEN_ADDR = 0x00; /* Register specifically for generating aclk */
}


/******************************************************************************************
 * Strobe Aclk, then start timer & wait until completed
 ******************************************************************************************/
void AclkRunTimer(void)
{
    StrobeAclk();
    RunTimer();
    RunTimerWait();
}


/******************************************************************************************
 * Function Name: Set Clock Frequency
 * Summary: Whenever the input clock frequency is changed, this function needs to be called
 *          to set the clock register in REG_IF.
 ******************************************************************************************/
void setClockFrequency(uint8_t clock_f)
{
    REG_FM_SYS_HV_CONFIG_ADDR = clock_f;
}


/****************************************************************************
 * Function Name: ClearPageLatch
 * Summary: Activastes the test mode that will clear the
 * Page latch in one clock cycle
 ****************************************************************************/
void ClearPageLatch(void)
{
    REG_FM_MODE = CLEAR_MODE;
    REG_FM_SEQ  = SEQ_0;
    StrobeAclk();
}


/******************************************************************************************
 *
 ******************************************************************************************/
void Exit_C_Bus_Mode(void)
{
    REG_FM_SYS_SW2FM_ADDR = SW2FM_READ;
}
