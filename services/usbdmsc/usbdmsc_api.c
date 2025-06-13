/**************************************************************************//**
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#undef ROUNDUP
#undef ROUNDDOWN
#include "mss_hal.h"
#include "mss_assert.h"
#include "flash_drive_app.h"
#include "mss_plic.h"
#include "mss_l2_cache.h"
#include "uart_helper.h"
#include "usbdmsc_service.h"
#include "hss_init.h"
#include "hss_trigger.h"

/**************************************************************************//**
 */
static bool mpu_blocks_access = false;

extern const uint64_t _hss_start;
extern const uint64_t _hss_end;
const uintptr_t p_hss_start = (uintptr_t)&_hss_start;
const uintptr_t p_hss_end = (uintptr_t)&_hss_end;

static void check_mpu_(const mss_mpu_mport_t master_port, const char * master_port_name, const uint64_t region_base, const uint64_t region_size)
{
    mss_mpu_pmp_region_t pmp_region = 0;
    uint64_t base, size;
    uint8_t permission, lock_en;
    mss_mpu_addrm_t matching_mode;
    int retval;

    while ((retval = MSS_MPU_get_config(master_port, pmp_region, &base, &size, &permission,
        &matching_mode, &lock_en)) == 0) {

        if ((base <= region_base) && ((base + size) >= (region_base + region_size))) {
            if (permission & (MPU_MODE_READ_ACCESS|MPU_MODE_WRITE_ACCESS|MPU_MODE_EXEC_ACCESS)) {
                break;
            }
        }

        ++pmp_region;
    }

    if (retval) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, ">>%s<< is not granted access in Design XML...\n", master_port_name);
        mpu_blocks_access = true;
    }
}

static void reset_usb_block_(void)
{
    SYSREG->SUBBLK_CLOCK_CR &= ~SUBBLK_CLOCK_CR_USB_MASK;
    SYSREG->SUBBLK_CLOCK_CR |= SUBBLK_CLOCK_CR_USB_MASK;
    SYSREG->SOFT_RESET_CR &= ~SOFT_RESET_CR_USB_MASK;
    SYSREG->SOFT_RESET_CR |= SOFT_RESET_CR_USB_MASK;

    HSS_USBInit();
}

bool USBDMSC_Init(void)
{
    bool result = true;

    reset_usb_block_();

    PLIC_init();

    PLIC_SetPriority(PLIC_USB_DMA_INT_OFFSET, 3);
    PLIC_SetPriority(PLIC_USB_MC_INT_OFFSET, 3);

    PLIC_EnableIRQ(PLIC_USB_DMA_INT_OFFSET);
    PLIC_EnableIRQ(PLIC_USB_MC_INT_OFFSET);

#if IS_ENABLED(CONFIG_SERVICE_MMC)
    PLIC_SetPriority(MMC_main_PLIC, 2u);
    PLIC_SetPriority(MMC_wakeup_PLIC, 2u);
    PLIC_EnableIRQ(MMC_main_PLIC);
    PLIC_EnableIRQ(MMC_wakeup_PLIC);
#endif


    // we'll check the MPU configuration and ensure that we have sufficient
    // privileges to implement USBDMSC (and optionally MMC access) and if not we'll
    // complain...
    if (result && !mpu_blocks_access) {
        const struct {
            uintptr_t base;
            ptrdiff_t size;
        } regions[] = {
            { p_hss_start, (ptrdiff_t)p_hss_end - (ptrdiff_t)p_hss_start },
        };

        for (int i = 0; i < ARRAY_SIZE(regions); i++) {
            check_mpu_(MSS_MPU_USB, "MSS_MPU_USB", regions[i].base, regions[i].size);
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            check_mpu_(MSS_MPU_MMC, "MSS_MPU_MMC", regions[i].base, regions[i].size);
#endif
            if (mpu_blocks_access) { break; }
        }

        result = result && !mpu_blocks_access;
    }

    if (result) {
        if (!FLASH_DRIVE_init()) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "FLASH_DRIVE_init() returned false...\n");
            result = false;
        }
    }

    return result;
}

HSSTicks_t last_poll_time = 0u;

bool USBDMSC_Poll(void)
{
    bool idle = mpu_blocks_access;

    if (!HSS_Trigger_IsNotified(EVENT_USBDMSC_REQUESTED)) {
        idle = true;
    }

    if (!idle) {
        //poll PLIC
        uint32_t source = PLIC_ClaimIRQ();

        switch (source) {
#if IS_ENABLED(CONFIG_SERVICE_MMC)
        case MMC_main_PLIC: // MMC interrupt
            PLIC_mmc_main_IRQHandler(); // interrupt 88
            break;
#endif

        case PLIC_USB_MC_INT_OFFSET: // main USB interrupt
            PLIC_usb_mc_IRQHandler(); // interrupt 87
            break;

        case PLIC_USB_DMA_INT_OFFSET: // DMA USB interrupt
            PLIC_usb_dma_IRQHandler(); // interrupt 86
            break;

        default:
            break;
        }

        if (source != INVALID_IRQn) {
            PLIC_CompleteIRQ(source);
        }

        if (HSS_Timer_IsElapsed(last_poll_time, 5*TICKS_PER_SEC)) {
            FLASH_DRIVE_dump_xfer_status();
            last_poll_time = HSS_GetTime();
        }
    }

    return idle;
}

void USBDMSC_Shutdown(void)
{
    reset_usb_block_();

    // clear any residual MMC / main USB / DMA USB interrupts
    PLIC_ClearPendingIRQ();
}
