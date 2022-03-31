/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PoalrFire SoC Microprocessor Subsystem RTC bare metal driver implementation.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#include <string.h>
#include "mss_rtc.h"
#include "mss_rtc_regs.h"
#include "mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
 * CONTROL_REG register masks.
 */
#define CONTROL_RUNNING_MASK        0x00000001u
#define CONTROL_RTC_START_MASK      0x00000001u
#define CONTROL_RTC_STOP_MASK       0x00000002u
#define CONTROL_ALARM_ON_MASK       0x00000004u
#define CONTROL_ALARM_OFF_MASK      0x00000008u
#define CONTROL_RESET_MASK          0x00000010u
#define CONTROL_UPLOAD_MASK         0x00000020u
#define CONTROL_WAKEUP_CLR_MASK     0x00000100u
#define CONTROL_UPDATED_MASK        0x00000400u

/*-------------------------------------------------------------------------*//**
 * MODE_REG register masks.
 */
#define MODE_CLK_MODE_MASK          0x00000001u
#define MODE_WAKEUP_EN_MASK         0x00000002u
#define MODE_WAKEUP_RESET_MASK      0x00000004u
#define MODE_WAKEUP_CONTINUE_MASK   0x00000008u

/*-------------------------------------------------------------------------*//**
 * Other masks.
 */
#define MAX_BINARY_HIGHER_COUNT     0x7FFu
#define MASK_32_BIT                 0xFFFFFFFFu
#define MAX_PRESCALAR_COUNT         0x03FFFFFFu
#define CALENDAR_SHIFT              8u

/*-------------------------------------------------------------------------*//**
 * Index into look-up table.
 */
#define SECONDS     0
#define MINUTES     1
#define HOURS       2
#define DAYS        3
#define MONTHS      4
#define YEARS       5
#define WEEKDAYS    6
#define WEEKS       7

/*-------------------------------------------------------------------------*//**
 * A pointer to the RTC_TypeDef structure is used to configure the user selected
 * RTC. This pointer is used by all the mss RTC driver function to carry out the
 * required functionality.
 */
RTC_TypeDef *      mss_rtc;

static uint8_t
get_clock_mode
(
    void
);

static void
set_rtc_mode
(
    uint8_t requested_mode
);

static void add_alarm_cfg_values
(
    uint8_t calendar_item,
    uint32_t * p_calendar_value,
    uint32_t * p_compare_mask
);

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_init
(
    RTC_TypeDef *base_address,
    uint8_t mode,
    uint32_t prescaler
)
{
    ASSERT(prescaler <= MAX_PRESCALAR_COUNT);

    /* Assigning the user selected MSS RTC base address to global RTC structure
     * pointer so that the other driver functions can use it.
     */
    mss_rtc = base_address;

    if (prescaler <= MAX_PRESCALAR_COUNT)
    {
        /* Stop the RTC. */
        MSS_RTC_stop();

        /* Disable alarm. */
        mss_rtc->CONTROL_REG = CONTROL_ALARM_OFF_MASK;

        /* Disable Interrupt */
        MSS_RTC_disable_irq();

        /* Clear RTC wake up interrupt signal */
        MSS_RTC_clear_irq();

        /* Select mode of operation, including the wake configuration. */
        if (MSS_RTC_CALENDAR_MODE == mode)
        {
            mss_rtc->MODE_REG = MODE_CLK_MODE_MASK;
        }
        else
        {
            mss_rtc->MODE_REG = 0u;
        }

        /* Reset the alarm and compare registers to a known value. */
        mss_rtc->ALARM_LOWER_REG = 0u;
        mss_rtc->ALARM_UPPER_REG = 0u;
        mss_rtc->COMPARE_LOWER_REG = 0u;
        mss_rtc->COMPARE_UPPER_REG = 0u;

        /* Reset the calendar counters */
        MSS_RTC_reset_counter();

        /* Set new Prescaler value */
        mss_rtc->PRESCALER_REG = prescaler;
    }
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_set_calendar_count
(
    const mss_rtc_calender_t *new_rtc_value
)
{
    uint8_t error = 0u;
    uint8_t clock_mode;

    const uint8_t g_rtc_max_count_lut[] =
    {
       /* Calendar mode */
        59u,    /* Seconds */
        59u,    /* Minutes */
        23u,    /* Hours   */
        31u,    /* Days    */
        12u,    /* Months  */
        254u,   /* Years   */
        7u,     /* Weekdays*/
        52u     /* Week    */
    };

    const uint8_t g_rtc_min_count_lut[] =
    {
       /* Calendar mode */
        0u, /* Seconds */
        0u, /* Minutes */
        0u, /* Hours   */
        1u, /* Days    */
        1u, /* Months  */
        0u, /* Years   */
        1u, /* Weekdays*/
        1u  /* Week    */
    };

    /* Assert if the values cross the limit */
    ASSERT(new_rtc_value->second >= g_rtc_min_count_lut[SECONDS]);
    ASSERT(new_rtc_value->second <= g_rtc_max_count_lut[SECONDS]);
    ASSERT(new_rtc_value->minute >= g_rtc_min_count_lut[MINUTES]);
    ASSERT(new_rtc_value->minute <= g_rtc_max_count_lut[MINUTES]);
    ASSERT(new_rtc_value->hour >= g_rtc_min_count_lut[HOURS]);
    ASSERT(new_rtc_value->hour <= g_rtc_max_count_lut[HOURS]);
    ASSERT(new_rtc_value->day >= g_rtc_min_count_lut[DAYS]);
    ASSERT(new_rtc_value->day <= g_rtc_max_count_lut[DAYS]);
    ASSERT(new_rtc_value->month >= g_rtc_min_count_lut[MONTHS]);
    ASSERT(new_rtc_value->month <= g_rtc_max_count_lut[MONTHS]);
    ASSERT(new_rtc_value->year >= g_rtc_min_count_lut[YEARS]);
    ASSERT(new_rtc_value->year <= g_rtc_max_count_lut[YEARS]);
    ASSERT(new_rtc_value->weekday >= g_rtc_min_count_lut[WEEKDAYS]);
    ASSERT(new_rtc_value->weekday <= g_rtc_max_count_lut[WEEKDAYS]);
    ASSERT(new_rtc_value->week >= g_rtc_min_count_lut[WEEKS]);
    ASSERT(new_rtc_value->week <= g_rtc_max_count_lut[WEEKS]);

    if (new_rtc_value->second < g_rtc_min_count_lut[SECONDS]) {error = 1u;}
    if (new_rtc_value->second > g_rtc_max_count_lut[SECONDS]) {error = 1u;}
    if (new_rtc_value->minute < g_rtc_min_count_lut[MINUTES]) {error = 1u;}
    if (new_rtc_value->minute > g_rtc_max_count_lut[MINUTES]) {error = 1u;}
    if (new_rtc_value->hour < g_rtc_min_count_lut[HOURS]) {error = 1u;}
    if (new_rtc_value->hour > g_rtc_max_count_lut[HOURS]) {error = 1u;}
    if (new_rtc_value->day < g_rtc_min_count_lut[DAYS]) {error = 1u;}
    if (new_rtc_value->day > g_rtc_max_count_lut[DAYS]) {error = 1u;}
    if (new_rtc_value->month < g_rtc_min_count_lut[MONTHS]) {error = 1u;}
    if (new_rtc_value->month > g_rtc_max_count_lut[MONTHS]) {error = 1u;}
    if (new_rtc_value->year < g_rtc_min_count_lut[YEARS]) {error = 1u;}
    if (new_rtc_value->year > g_rtc_max_count_lut[YEARS]) {error = 1u;}
    if (new_rtc_value->weekday < g_rtc_min_count_lut[WEEKDAYS]) {error = 1u;}
    if (new_rtc_value->weekday > g_rtc_max_count_lut[WEEKDAYS]) {error = 1u;}
    if (new_rtc_value->week < g_rtc_min_count_lut[WEEKS]) {error = 1u;}
    if (new_rtc_value->week > g_rtc_max_count_lut[WEEKS]) {error = 1u;}

    /*
     * This function can only be used when the RTC is configured to operate in
     * calendar counter mode.
     */
    clock_mode = get_clock_mode();
    ASSERT(MSS_RTC_CALENDAR_MODE == clock_mode);

    if ((0u == error) && (MSS_RTC_CALENDAR_MODE == clock_mode))
    {
        uint32_t upload_in_progress;

       /* Write the RTC new value.
        */
        mss_rtc->SECONDS_REG = new_rtc_value->second;
        mss_rtc->MINUTES_REG = new_rtc_value->minute;
        mss_rtc->HOURS_REG = new_rtc_value->hour;
        mss_rtc->DAY_REG = new_rtc_value->day;
        mss_rtc->MONTH_REG = new_rtc_value->month;
        mss_rtc->YEAR_REG = new_rtc_value->year;
        mss_rtc->WEEKDAY_REG = new_rtc_value->weekday;
        mss_rtc->WEEK_REG = new_rtc_value->week;

        /* Data is copied, now issue upload command */
        mss_rtc->CONTROL_REG = CONTROL_UPLOAD_MASK ;

        /* Wait for the upload to complete. */
        do {
            upload_in_progress = mss_rtc->CONTROL_REG & CONTROL_UPLOAD_MASK;
        } while (upload_in_progress);
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_set_binary_count
(
    uint64_t new_rtc_value
)
{
    uint8_t clock_mode;

    /*
     * This function can only be used when the RTC is configured to operate in
     * binary counter mode.
     */
    clock_mode = get_clock_mode();
    ASSERT(MSS_RTC_BINARY_MODE == clock_mode);

    if (MSS_RTC_BINARY_MODE == clock_mode)
    {
        uint32_t rtc_upper_32_bit_value;

        rtc_upper_32_bit_value = (uint32_t)(new_rtc_value >> 32u) & MASK_32_BIT;

        /* Assert if the values cross the limit */
        ASSERT(rtc_upper_32_bit_value <= MAX_BINARY_HIGHER_COUNT);

        if (rtc_upper_32_bit_value <= MAX_BINARY_HIGHER_COUNT)
        {
            uint32_t upload_in_progress;

            /*
             * Write the RTC new value.
             */
            mss_rtc->DATE_TIME_LOWER_REG = (uint32_t)new_rtc_value;
            mss_rtc->DATE_TIME_UPPER_REG =
                    (uint32_t)(( new_rtc_value >> 32u) & MAX_BINARY_HIGHER_COUNT);

            /* Data is copied, now issue upload command */
            mss_rtc->CONTROL_REG = CONTROL_UPLOAD_MASK;

            /* Wait for the upload to complete. */
            do {
                upload_in_progress = mss_rtc->CONTROL_REG & CONTROL_UPLOAD_MASK;
            } while (upload_in_progress);
        }
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_get_calendar_count
(
    mss_rtc_calender_t *p_rtc_calendar
)
{
    uint8_t clock_mode;
    /*
     * This function can only be used when the RTC is configured to operate in
     * calendar counter mode.
     */
    clock_mode = get_clock_mode();
    ASSERT(MSS_RTC_CALENDAR_MODE == clock_mode);

    if (MSS_RTC_CALENDAR_MODE == clock_mode)
    {
        p_rtc_calendar->second = (uint8_t)mss_rtc->SECONDS_REG;
        p_rtc_calendar->minute = (uint8_t)mss_rtc->MINUTES_REG;
        p_rtc_calendar->hour = (uint8_t)mss_rtc->HOURS_REG;
        p_rtc_calendar->day = (uint8_t)mss_rtc->DAY_REG;
        p_rtc_calendar->month = (uint8_t)mss_rtc->MONTH_REG;
        p_rtc_calendar->year = (uint8_t)mss_rtc->YEAR_REG;
        p_rtc_calendar->weekday = (uint8_t)mss_rtc->WEEKDAY_REG;
        p_rtc_calendar->week = (uint8_t)mss_rtc->WEEK_REG;
    }
    else
    {
        /*
         * Set returned calendar count to zero if the RTC is not configured for
         * calendar counter mode. This should make incorrect release application
         * code behave consistently and help application debugging.
         */
        memset(p_rtc_calendar, 0, sizeof(mss_rtc_calender_t));
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
uint64_t
MSS_RTC_get_binary_count
(
    void
)
{
    uint64_t rtc_count;
    uint8_t clock_mode;

    /*
     * This function can only be used when the RTC is configured to operate in
     * binary counter mode.
     */
    clock_mode = get_clock_mode();
    ASSERT(MSS_RTC_BINARY_MODE == clock_mode);

    if (MSS_RTC_BINARY_MODE == clock_mode)
    {
        rtc_count = mss_rtc->DATE_TIME_LOWER_REG;
        rtc_count = rtc_count | ((uint64_t)mss_rtc->DATE_TIME_UPPER_REG << 32u) ;
    }
    else
    {
        /*
         * Set returned binary count to zero if the RTC is not configured for
         * binary counter mode. This should make incorrect release application
         * code behave consistently and help application debugging.
         */
        rtc_count = 0u;
    }

    return rtc_count;
}

/*-------------------------------------------------------------------------*//**
 *
 */
static void add_alarm_cfg_values
(
    uint8_t calendar_item,
    uint32_t * p_calendar_value,
    uint32_t * p_compare_mask
)
{
    if (MSS_RTC_CALENDAR_DONT_CARE == calendar_item)
    {
        *p_calendar_value = (uint32_t)(*p_calendar_value << CALENDAR_SHIFT);
        *p_compare_mask = (uint32_t)(*p_compare_mask << CALENDAR_SHIFT);
    }
    else
    {
        *p_calendar_value = (uint32_t)((*p_calendar_value << CALENDAR_SHIFT) | (uint32_t)calendar_item);
        *p_compare_mask = (uint32_t)((*p_compare_mask << CALENDAR_SHIFT) | (uint32_t)0xFFu);
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void MSS_RTC_set_calendar_count_alarm
(
    const mss_rtc_calender_t * alarm_value
)
{
    uint32_t calendar_value;
    uint32_t compare_mask;
    uint8_t mode;

    mode = (uint8_t)(mss_rtc->MODE_REG & MODE_CLK_MODE_MASK);
    /*
     * This function can only be used with the RTC set to operate in calendar
     * mode.
     */
    ASSERT(MSS_RTC_CALENDAR_MODE == mode);
    if (MSS_RTC_CALENDAR_MODE == mode)
    {
        uint8_t required_mode_reg;

        /* Disable the alarm before updating*/
        mss_rtc->CONTROL_REG = CONTROL_ALARM_OFF_MASK;

        /* Set alarm and compare lower registers. */
        calendar_value = 0u;
        compare_mask = 0u;

        add_alarm_cfg_values(alarm_value->day, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->hour, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->minute, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->second, &calendar_value, &compare_mask);

        mss_rtc->ALARM_LOWER_REG = calendar_value;
        mss_rtc->COMPARE_LOWER_REG = compare_mask;

        /* Set alarm and compare upper registers. */
        calendar_value = 0u;
        compare_mask = 0u;

        add_alarm_cfg_values(alarm_value->week, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->weekday, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->year, &calendar_value, &compare_mask);
        add_alarm_cfg_values(alarm_value->month, &calendar_value, &compare_mask);

        mss_rtc->ALARM_UPPER_REG = calendar_value;
        mss_rtc->COMPARE_UPPER_REG = compare_mask;

        /* Configure the RTC to enable the alarm. */
        required_mode_reg = mode | MODE_WAKEUP_EN_MASK | MODE_WAKEUP_CONTINUE_MASK;
        set_rtc_mode(required_mode_reg);

        /* Enable the alarm */
        mss_rtc->CONTROL_REG = CONTROL_ALARM_ON_MASK ;
    }
}

/*-------------------------------------------------------------------------*//**
  We only write the RTC mode register if really required because the RTC needs
  to be stopped for the mode register to be written. Stopping the RTC every time
  the wakeup alarm configuration is set might induce drift on the RTC time.
  This function is intended to be used when setting alarms.
 */
static void
set_rtc_mode
(
    uint8_t requested_mode
)
{
    if (mss_rtc->MODE_REG != requested_mode)
    {
        uint32_t rtc_running;
        rtc_running = mss_rtc->CONTROL_REG & CONTROL_RUNNING_MASK;
        if (rtc_running)
        {
            /* Stop the RTC in order to change the mode register content.*/
            MSS_RTC_stop();
            mss_rtc->MODE_REG = requested_mode;
            MSS_RTC_start();
        }
        else
        {
            mss_rtc->MODE_REG = requested_mode;
        }
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_set_binary_count_alarm
(
    uint64_t alarm_value,
    mss_rtc_alarm_type_t alarm_type
)
{
    uint8_t mode;

    mode = (uint8_t)(mss_rtc->MODE_REG & MODE_CLK_MODE_MASK);
    /*
     * This function can only be used with the RTC set to operate in binary
     * counter mode.
     */
    ASSERT(MSS_RTC_BINARY_MODE == mode);
    if (MSS_RTC_BINARY_MODE == mode)
    {
        uint8_t required_mode_reg;

        /* Disable the alarm before updating*/
        mss_rtc->CONTROL_REG = CONTROL_ALARM_OFF_MASK;

        /* Set the alarm value. */
        mss_rtc->COMPARE_LOWER_REG = COMPARE_ALL_BITS;
        mss_rtc->COMPARE_UPPER_REG = COMPARE_ALL_BITS;
        mss_rtc->ALARM_LOWER_REG = (uint32_t)alarm_value;
        mss_rtc->ALARM_UPPER_REG = (uint32_t)(alarm_value >> 32u);

        /*
         * Configure the RTC to enable the alarm.
         */
        required_mode_reg = mode | MODE_WAKEUP_EN_MASK | MODE_WAKEUP_CONTINUE_MASK;
        if (MSS_RTC_PERIODIC_ALARM == alarm_type)
        {
            /*
             * The RTC binary counter will be fully reset when the alarm occurs.
             * The counter will continue counting while the wake-up interrupt is
             * active.
             */
            required_mode_reg |= MODE_WAKEUP_RESET_MASK;
        }
        set_rtc_mode(required_mode_reg);

        /* Enable the alarm */
        mss_rtc->CONTROL_REG = CONTROL_ALARM_ON_MASK;

        uint8_t test = get_clock_mode();
    }
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_start
(
    void
)
{
    mss_rtc->CONTROL_REG = CONTROL_RTC_START_MASK;
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_stop
(
    void
)
{
    uint32_t rtc_running;

    /*
     * Send command to stop RTC.
     */
    mss_rtc->CONTROL_REG = CONTROL_RTC_STOP_MASK;

    /*
     * Wait for RTC internal synchronization to take place and RTC to actually
     * stop.
     */
    do {
        rtc_running =  mss_rtc->CONTROL_REG & CONTROL_RUNNING_MASK;
    } while(rtc_running);
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_reset_counter
(
    void
)
{
    uint32_t upload_in_progress;

    mss_rtc->CONTROL_REG = CONTROL_RESET_MASK;

    /* Wait for the upload to complete. */
    do {
        upload_in_progress = mss_rtc->CONTROL_REG & CONTROL_UPLOAD_MASK;
    } while (upload_in_progress);
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
uint32_t
MSS_RTC_get_update_flag
(
    void
)
{
    uint32_t updated;
    updated = mss_rtc->CONTROL_REG & CONTROL_UPDATED_MASK;
    return updated;
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_clear_update_flag
(
    void
)
{
    /* Clear the "updated" control bit. */
    mss_rtc->CONTROL_REG = CONTROL_UPDATED_MASK;
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_enable_irq
(
    void
)
{
    /*
     * Only the PLIC level interrupt enable is performed within this function.
     * The RTC level interrupt enable is performed within the alarm setting
     * functions.
     * This avoid the MODE register being modified whenever  RTC
     * interrupts are enabled/disabled.
     */
    PLIC_EnableIRQ(RTC_WAKEUP_PLIC);
}

/*-------------------------------------------------------------------------*//**
  See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_disable_irq
(
    void
)
{
    /*
     * Only the PLIC level interrupt disable is performed within this function.
     * This avoid the MODE register being modified whenever RTC
     * interrupts are enabled/disabled.
     */
    PLIC_DisableIRQ(RTC_WAKEUP_PLIC);
}

/*-------------------------------------------------------------------------*//**
 * See "mss_rtc.h" for details of how to use this function.
 */
void
MSS_RTC_clear_irq
(
    void
)
{
    volatile uint32_t dummy_read;

    /* Clear wake up interrupt signal */
    mss_rtc->CONTROL_REG = CONTROL_WAKEUP_CLR_MASK;

    /*
    * Ensure that the posted write to the CONTROL_REG register completed before
    * returning from this function. Not doing this may result in the interrupt
    * only being cleared some time after this function returns.
    */
    dummy_read = mss_rtc->CONTROL_REG;

    /* Dummy operation to avoid warning message */
    ++dummy_read;
}

/*-------------------------------------------------------------------------*//**
  The get_clock_mode() function gets the clock mode of RTC hardware.
  Possible clock modes are:
    MSS_RTC_CALENDAR_MODE
    MSS_RTC_BINARY_MODE
 */
static uint8_t
get_clock_mode
(
    void
)
{
    uint8_t clock_mode;

    clock_mode = (uint8_t)(mss_rtc->MODE_REG & MODE_CLK_MODE_MASK);

    return(clock_mode);
}

#ifdef __cplusplus
}
#endif
