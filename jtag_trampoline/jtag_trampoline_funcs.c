#include <stdbool.h>
#include <stdint.h>
#include "mpfs_hal/mss_hal.h"

bool HSS_Setup_Clocks(void);

bool HSS_Setup_Clocks(void)
{
    static const uint32_t hss_subblk_clock_Config = 0xFFFFFFFFu;
    const uint32_t hss_soft_reset_Config = SYSREG->SOFT_RESET_CR &
        ~(
           SOFT_RESET_CR_SGMII_MASK   |
           SOFT_RESET_CR_CFM_MASK     |
           SOFT_RESET_CR_ATHENA_MASK  |
           SOFT_RESET_CR_FIC3_MASK    |
           SOFT_RESET_CR_FIC2_MASK    |
           SOFT_RESET_CR_FIC1_MASK    |
           SOFT_RESET_CR_FIC0_MASK    |
           SOFT_RESET_CR_DDRC_MASK    |
           SOFT_RESET_CR_GPIO2_MASK   |
           SOFT_RESET_CR_GPIO1_MASK   |
           SOFT_RESET_CR_GPIO0_MASK   |
           SOFT_RESET_CR_QSPI_MASK    |
           SOFT_RESET_CR_RTC_MASK     |
           SOFT_RESET_CR_FPGA_MASK    |
           SOFT_RESET_CR_USB_MASK     |
           SOFT_RESET_CR_CAN1_MASK    |
           SOFT_RESET_CR_CAN0_MASK    |
           SOFT_RESET_CR_I2C1_MASK    |
           SOFT_RESET_CR_I2C0_MASK    |
           SOFT_RESET_CR_SPI1_MASK    |
           SOFT_RESET_CR_SPI0_MASK    |
           SOFT_RESET_CR_MMUART4_MASK |
           SOFT_RESET_CR_MMUART3_MASK |
           SOFT_RESET_CR_MMUART2_MASK |
           SOFT_RESET_CR_MMUART1_MASK |
           SOFT_RESET_CR_MMUART0_MASK |
           SOFT_RESET_CR_TIMER_MASK   |
           SOFT_RESET_CR_MMC_MASK     |
           SOFT_RESET_CR_MAC1_MASK    |
           SOFT_RESET_CR_MAC0_MASK    |
           SOFT_RESET_CR_ENVM_MASK);

    SYSREG->SOFT_RESET_CR = 0x3FFFFFFEu; // everything but ENVM
    SYSREG->SOFT_RESET_CR = hss_soft_reset_Config;
    SYSREG->SUBBLK_CLOCK_CR = hss_subblk_clock_Config;

    SYSREG->FABRIC_RESET_CR = FABRIC_RESET_CR_ENABLE_MASK;

    return true;
}
