/*********************************************************************
* Software License Agreement:
*
* The software supplied herewith by Microchip Technology Incorporated
* (the "Company") for its PICmicro® Microcontroller is intended and
* supplied to you, the Company's customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*********************************************************************/
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include "rtcc.h"
//#include "../mcc.h"

volatile time_t deviceTime;
volatile bool dirtyTime;
extern volatile uint64_t g_tick_counter;

volatile uint16_t seconds_counter;
/******************************************************************************

  Summary:
    Initialize the clock calendar driver.

  Description:
    This function configured the basics of a software driven RTCC peripheral.
    It relies upon a periodic TMR1 event to provide time keeping.
    The Timer 1 clock provides a steady 100kHz time base.
    0x8000 counts in the TMR1 register will provide a 1HZ tick.
    CLOCKS_PER_SEC is configured for 1 and all is well.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    The C stdlib functions understand UNIX time but they need a UNIX time counter.
    UNIX time is the number of seconds elapsed since Midnight Jan 1, 1970
  ***************************************************************************/
void rtcc_init(void)
{
#if 0
    deviceTime = 1293861600;
    TMR1_SetInterruptHandler(rtcc_handler);
#else
    deviceTime = 1293861600;
    g_tick_counter = deviceTime * 1000;
#endif
}

/****************************************************************************
  Function:
    void rtcc_handler(void) (TMR1 version)

  Summary:
    maintain deviceTime (seconds) using the LCDIF flag/interrupt.

  Description:
    This function decrements seconds_counter until 0 and then increments deviceTime.
    seconds_counter reloads with CLOCK_PER_SEC.
    This version of the function uses Timer 1 as the time base.
    Timer 1 is reloaded with 0x8000 to cause TMR1IF to overflow every second.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    TMR1IF is not looked at or modified because there could be other tasks running
    on this interrupt.
  ***************************************************************************/

void rtcc_handler(void)
{
	deviceTime++;
}


/****************************************************************************
  Function:
    void rtcc_set(time_t time)

  Summary:
    set the device time to the passed unix time value.

  Description:
    Update the device time with the passed unix time.
    Interrupts are disabled for the copy.
    Interrupt state is restored to the original state on exit.
    
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/

void rtcc_set(time_t *t)
{
#if 0
    bool gie_val;
    gie_val = (bool)GIE;
    INTERRUPT_GlobalInterruptDisable();
    deviceTime = *t;
    GIE = gie_val;
#else
    deviceTime = *t;
#endif
}

/****************************************************************************
  Function:
    time_t time(time_t *t)

  Summary:
    return the current device time.

  Description:
    This function retrieves the device time as either a return value or 
    filling in a variable passed by reference.  Interrupts are disabled
    during the copy and restored on exit.
 
  Precondition:
    None

  Parameters:
    time_t *t : A time_t pointer for the current time.

  Returns:
    time_t value of the current time.

  Remarks:
    This function is a prerequisit to supporting the standard C time libraries.
  ***************************************************************************/

/* implement the time(&t) function for the standard libraries */

/* time.h does not implment time as it is application dependent */
time_t time(time_t *t)
{
#if 0
    bool   gie_val;
    time_t  the_time;
    
    gie_val = (bool)GIE;  //jira: CAE_MCU8-5647
    INTERRUPT_GlobalInterruptDisable();
    the_time = deviceTime;
    GIE = gie_val;
#else
    time_t  the_time;
    deviceTime = g_tick_counter / 1000;

    the_time = deviceTime;
#endif
    if(t)
    {
        *t = the_time;
    }

    return(the_time);
}


