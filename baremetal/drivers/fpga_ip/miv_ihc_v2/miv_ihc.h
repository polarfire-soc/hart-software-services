/*******************************************************************************
 * The PolarFire SoC platform is released under the following software license:
 *
 * Copyright 2023 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Bare metal driver for the Mi-V Inter Hart Communication (IHC) subsystem
 *
 */
/*=========================================================================*//**
  @mainpage Mi-V IHC Bare Metal Driver.

  ==============================================================================
  Introduction
  ==============================================================================
  The Bare metal Mi-V IHC driver facilitates the use of the Mi-V IHC
  FPGA-subsystem which allows inter hart/processor communication as part of a
  HW/SW stack.

  ==============================================================================
  Definition of terms
  ==============================================================================
  Terms definition

     - Channel A-B
       This refers to the communications connection between any two Harts or
       contexts. ( Note: A Context refers to a collection of harts. Member
       harts of a context are defined in the MSS Configurator.)
     - Mi-V Inter Hart Communication Channel (IHCC)
       This is the IP core for each channel. It has an A and a B side. Each
       side is configured from the hart associated with each side using the apb
       bus.
     - Mi-V Inter Hart Communication Interrupt Module (IHCIM) IP
       This is the Interrupt Module Core, one for each hart. It takes inputs
       from the Mi-V IHCC's and generates an interrupt to the connected hart
       based on the programmed configuration.
     - Mi-V Inter Hart Communication (IHC) subsystem
       This is the collection of Mi-V IHCC's and Mi-V IHCIM's which form the
       Inter hart Communications subsystem.

  ==============================================================================
  Driver and IP version
  ==============================================================================
  The driver version can be found in the following two places:
      - The constants section of the User Guide
      - In the miv_ihc_version.h file

  IP version
      - Each module used in the Mi-V IHC contains a read only version
        register. This can be read to determine the module version. The register
        is at offset zero from the base of the channel.


 *//*=========================================================================*/

#ifndef __MSS_MIV_IHC_H_
#define __MSS_MIV_IHC_H_ 1

#include "miv_ihc_regs.h"
#include "miv_ihc_types.h"

#include "hal/hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**

  ## Interrupt mapping table

  Choose the interrupt mapping used in you system
  Please see defines in miv_ihc.h for the default defines.
  The default values below are over-written with those in the
  miv_ihc_add_mapping.h file if it this is present in the boards directory.
 */

#ifndef IHC_APP_X_H0_IRQHandler
#define IHC_APP_X_H0_IRQHandler fabric_f2h_63_plic_IRQHandler
#endif

#ifndef IHC_APP_X_H1_IRQHandler
#define IHC_APP_X_H1_IRQHandler fabric_f2h_62_plic_IRQHandler
#endif

#ifndef IHC_APP_X_H2_IRQHandler
#define IHC_APP_X_H2_IRQHandler fabric_f2h_61_plic_IRQHandler
#endif

#ifndef IHC_APP_X_H3_IRQHandler
#define IHC_APP_X_H3_IRQHandler fabric_f2h_60_plic_IRQHandler
#endif

#ifndef IHC_APP_X_H4_IRQHandler
#define IHC_APP_X_H4_IRQHandler fabric_f2h_59_plic_IRQHandler
#endif

#ifndef IHC_CTRL_H0_H1_IRQHandler
#define IHC_CTRL_H0_H1_IRQHandler U54_f2m_31_local_IRQHandler
#endif

#ifndef IHC_CTRL_H0_H2_IRQHandler
#define IHC_CTRL_H0_H2_IRQHandler U54_f2m_30_local_IRQHandler
#endif

#ifndef IHC_CTRL_H0_H3_IRQHandler
#define IHC_CTRL_H0_H3_IRQHandler U54_f2m_29_local_IRQHandler
#endif

#ifndef IHC_CTRL_H0_H4_IRQHandler
#define IHC_CTRL_H0_H4_IRQHandler U54_f2m_28_local_IRQHandler
#endif


/* Macros used across the driver */

#define IHC_MSG_INVALID         -1
#define IHC_MSG_SUCCESS          0
#define IHC_MSG_BUSY            -2
#define IHC_MSG_NT_CLR          -3
#define IHC_MSG_CLR              0
#define IHC_MSG_RECEIVED         0
#define IHC_MSG_PRESENT          6
#define IHC_MSG_CONSUMED         7
#define IHC_NO_MSG              -4
#define IHC_CALLBACK_NOT_CONFIG -5

extern IHC_TypeDef g_ihc;

/* Need confirmation on channels/Processors supported*/
typedef enum IHC_CHANNEL_
{
    MIV_IHC_CH_H0_H1 = 0,
    MIV_IHC_CH_H0_H2,
    MIV_IHC_CH_H0_H3,
    MIV_IHC_CH_H0_H4,
    MIV_IHC_CH_H0_H5,
    MIV_IHC_CH_H1_H0,
    MIV_IHC_CH_H1_H2,
    MIV_IHC_CH_H1_H3,
    MIV_IHC_CH_H1_H4,
    MIV_IHC_CH_H1_H5,
    MIV_IHC_CH_H2_H0,
    MIV_IHC_CH_H2_H1,
    MIV_IHC_CH_H2_H3,
    MIV_IHC_CH_H2_H4,
    MIV_IHC_CH_H2_H5,
    MIV_IHC_CH_H3_H0,
    MIV_IHC_CH_H3_H1,
    MIV_IHC_CH_H3_H2,
    MIV_IHC_CH_H3_H4,
    MIV_IHC_CH_H3_H5,
    MIV_IHC_CH_H4_H0,
    MIV_IHC_CH_H4_H1,
    MIV_IHC_CH_H4_H2,
    MIV_IHC_CH_H4_H3,
    MIV_IHC_CH_H4_H5,
    MIV_IHC_CH_H5_H0,
    MIV_IHC_CH_H5_H1,
    MIV_IHC_CH_H5_H2,
    MIV_IHC_CH_H5_H3,
    MIV_IHC_CH_H5_H4
} IHC_CHANNEL;

typedef enum IHC_MODULE_NUM_
{
    MIV_IHCIM_H0 = 0,
    MIV_IHCIM_H1,
    MIV_IHCIM_H2,
    MIV_IHCIM_H3,
    MIV_IHCIM_H4,
    MIV_IHCIM_H5
} IHC_MODULE_NUM;

typedef enum IHCIM_CONFIG_STATUS_
{
    IHCIM_NT_CONFIGURED = 0,
    IHCIM_CONFIGURED
} IHCIM_CONFIG_STATUS;

/*******************************************************************************
 * The IHC_init() function initializes the individual processor. It is called
 * from each processor using the IHC once on start-up.
 *
 * @param channel
 *  The channel to be configured
 *
 * @return
 *   This function does not return a value.
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *    int main(void)
 *    {
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_init(uint8_t channel);

/*******************************************************************************
 * The IHC_config_mp_callback_handler() function configures the message present
 * callback handler. It is called by respective processor once initialization
 * is completed or when want to change configuration each time prior to
 * communication.
 *
 * @param channel
 *   channel to be configured
 *
 * @param handler
 *   callback handler called once msg is present
 *
 * @return
 *   This function does not return a value.
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_config_mp_callback_handler(uint8_t channel, QUEUE_IHC_INCOMING handler);

/*******************************************************************************
 * The IHC_config_mc_callback_handler() function configures the message consumed
 * callback handler. It is called by respective processor once initialization
 * is completed or when want to change configuration each time prior to
 * communication.
 *
 * @param channel
 *   channel to be configured
 *
 * @param handler
 *   callback handler called once msg is consumed
 *
 * @return
 *   This function does not return a value.
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_config_mc_callback_handler(uint8_t channel, QUEUE_IHC_INCOMING handler);

/*******************************************************************************
 * The IHC_enable_mp_interrupt() function enables the message present interrupt.
 * It is called by respective processor once initialization & configured.
 *
 * @param channel
 *   channel for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_enable_mp_interrupt(uint8_t channel);

/*******************************************************************************
 * The IHC_enable_mc_interrupt() function enables the message consumed
 * interrupt. It is called by respective processor once initialization &
 * configured.
 *
 * @param channel
 *   channel for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      IHC_enable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_enable_mc_interrupt(uint8_t channel);

/*******************************************************************************
 * The IHC_disable_mp_interrupt() function disables the message present
 * interrupt. It is called by respective processor once initialization &
 * configured.
 *
 * @param channel
 *   channel for which interrupt has be disabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      // further code follows
 *
 *      IHC_disable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_disable_mp_interrupt(uint8_t channel);

/*******************************************************************************
 * The IHC_disable_mc_interrupt() function disables the message consumed
 * interrupt. It is called by respective processor once initialization &
 * configured.
 *
 * @param channel
 *   channel for which interrupt has be disabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      IHC_enable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      // further code follows
 *
 *      IHC_disable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_disable_mc_interrupt(uint8_t channel);

/*******************************************************************************
 * The IHC_enable_mp_interrupt_module() function enables message present
 * interrupt module. Its is AND with MP interrupts hence individual channel
 * interrupt must be enabled before. It is called by respective processor once
 * initialization, configured, interrupt enabled.
 *
 * @param module
 *   module for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      IHC_enable_mp_interrupt_module(MIV_IHCIM_H0);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_enable_mp_interrupt_module(uint8_t channel);

/*******************************************************************************
 * The IHC_disable_mp_interrupt_module() function disables message present
 * interrupt module. Its is AND with MP interrupts hence individual channel
 * interrupt need not to be disable before. It is called by respective processor
 * once initialization & configured.
 *
 * @param module
 *   module for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      IHC_enable_mp_interrupt_module(MIV_IHCIM_H0);
 *
 *      // further code follows
 *
 *      IHC_disable_mp_interrupt_module(MIV_IHCIM_H0);
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_disable_mp_interrupt_module(uint8_t module);

/*******************************************************************************
 * The IHC_enable_mp_interrupt_module() function enables message consumed
 * interrupt module. Its is AND with MC interrupts hence individual channel
 * interrupt must be enabled before. It is called by respective processor once
 * initialization, configured, interrupt enabled.
 *
 * @param module
 *   module for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      IHC_enable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      IHC_enable_mc_interrupt_module(MIV_IHCIM_H0);
 *
 *      // further code follows
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_enable_cp_interrupt_module(uint8_t channel);

/*******************************************************************************
 * The IHC_disable_cp_interrupt_module() function disables message consumed
 * interrupt module. Its is AND with MC interrupts hence individual channel
 * interrupt need not to be disable before. It is called by respective processor
 * once initialization & configured.
 *
 * @param module
 *   module for which interrupt has be enabled
 *
 * @return
 *   This function does not return a value
 *
 * @code
 *    // Initialization code
 *    #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *    {
 *      // expected functionality in callback
 *      return 0;
 *    }
 *    int main(void)
 *    {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      IHC_enable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      IHC_enable_mc_interrupt_module(MIV_IHCIM_H0);
 *
 *      // further code follows
 *
 *      IHC_disable_mc_interrupt_module(MIV_IHCIM_H0);
 *
 *      return (0u);
 *    }
 *  @endcode
 */
void IHC_disable_cp_interrupt_module(uint8_t module);

/*******************************************************************************
 * The IHC_tx_message() is used to send a message from one processor to another
 * processor.
 *
 * @param channel
 *   The channel we want the remote hart ID of.
 *
 * @param message
 *   Pointer to message being sent
 *
 * @param msg_size
 *   size of msg being sent in words
 *
 * @return
 *   message status
 *
 * @code
 *   // example code showing use
 *   if ( IHC_MSG_SUCCESS == IHC_tx_message(MIV_IHC_CH_H0_H1, message_data, 1))
 *   {
 *      // message has been sent
 *   }
 *   else
 *   {
 *      // you can try again...
 *   }
 * @endcode
 */
int8_t IHC_tx_message(uint8_t channel, const uint32_t *message, uint16_t msg_size);

/*******************************************************************************
 * The IHC_poll_message_present() is called to poll message present and
 * processes the message with message present handler previous registered
 * using the IHC_config_mp_callback_handler() function
 *
 * @param channel
 *   The channel for message present to poll
 *
 * @return
 *   status for received message
 *
 * @code
 *   // Initialization code
 *   #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *   {
 *     // expected functionality in callback
 *     return 0;
 *   }
 *
 *   int main(void)
 *   {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_disable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *     //polling data
 *     while(condition)
 *     {
 *         IHC_poll_message_present(MIV_IHC_CH_H0_H1);
 *     }
 *
 *     return (0u);
 *   }
 */
int8_t IHC_poll_msg_present(uint8_t channel);

/*******************************************************************************
 * The IHC_poll_msg_consumed() is called to poll message consumed and jump to
 * callback handler previous registered using IHC_config_mp_callback_handler()
 * function.
 *
 * @param channel
 *   The channel for message present to poll
 *
 * @return
 *   status for received message
 *
 * @code
 *   // Initialization code
 *   #include "mss_ihc.h"
 *
 *    static uint32_t queue_ihc_mc_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *   {
 *     // expected functionality in callback
 *     return 0;
 *   }
 *
 *   int main(void)
 *   {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mc_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mc_isr_callback);
 *
 *      IHC_disable_mc_interrupt(MIV_IHC_CH_H0_H1);
 *
 *     //polling data
 *     while(condition)
 *     {
 *         IHC_poll_msg_consumed(MIV_IHC_CH_H0_H1);
 *     }
 *
 *     return (0u);
 *   }
 */
int8_t IHC_poll_msg_consumed(uint8_t channel);

/*******************************************************************************
 * The IHC_app_irq_handler() is called by the interrupt handler
 *
 * @param module_num
 *   The module for application interrupt occurred
 *
 * @return
 *   none
 *
 * @code
 *   // Initialization code
 *   #include "mss_ihc.h"
 *
 *  uint8_t IHC_APP_X_H5_IRQHandler(void)
 *  {
 *      IHC_app_irq_handler(MIV_IHCIM_H5);
 *      return (EXT_IRQ_KEEP_ENABLED);
 *  }
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *   {
 *     // expected functionality in callback
 *     return 0;
 *   }
 *
 *   int main(void)
 *   {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      return (0u);
 *   }
 */
void IHC_app_irq_handler(uint8_t module_num);

/*******************************************************************************
 * The IHC_ctlr_irqhandler() is called by the interrupt handler
 *
 * @param module_num
 *   The module for application interrupt occurred
 *
 * @return
 *   none
 *
 * @code
 *   // Initialization code
 *   #include "mss_ihc.h"
 *
 *  uint8_t IHC_APP_X_H5_IRQHandler(void)
 *  {
 *      IHC_ctlr_irqhandler(MIV_IHCIM_H5);
 *      return (EXT_IRQ_KEEP_ENABLED);
 *  }
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *   {
 *     // expected functionality in callback
 *     return 0;
 *   }
 *
 *   int main(void)
 *   {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      return (0u);
 *   }
 */
void IHC_ctlr_irqhandler(uint8_t module_num);

/*******************************************************************************
 * The IHC_indirect_irq_handler() is called by the interrupt handler
 *
 * @param module_num
 *   The module for application interrupt occurred
 *
 * @param msg_buffer
 *   The msg buffer to store received msg data
 *
 * @return
 *   error code
 *
 * @code
 *   // Initialization code
 *   #include "mss_ihc.h"
 *
 *  uint32_t *msg_buffer;
 *
 *  uint8_t IHC_APP_X_H5_IRQHandler(void)
 *  {
 *      uint8 error_code = IHC_indirect_irq_handler(MIV_IHCIM_H5, msg_buffer);
 *      return (EXT_IRQ_KEEP_ENABLED);
 *  }
 *
 *    static uint32_t queue_ihc_mp_isr_callback(uint8_t channel,
 *                         const uint32_t *message,
 *                         uint32_t message_size)
 *   {
 *     // expected functionality in callback
 *     return 0;
 *   }
 *
 *   int main(void)
 *   {
 *
 *      IHC_init(MIV_IHC_CH_H0_H1);
 *
 *      IHC_config_mp_callback_handler(MIV_IHC_CH_H0_H1, queue_ihc_mp_isr_callback);
 *
 *      IHC_enable_mp_interrupt(MIV_IHC_CH_H0_H1);
 *
 *      return (0u);
 *   }
 */
int8_t IHC_indirect_irq_handler(uint8_t module_num, uint32_t *msg_buffer);
/*******************************************************************************
 * The IHC_get_channel_status() returns the status register details for
 * respective channel
 *
 * @param channel
 *   The channel number to get status
 *
 * @return
 *   Status register
 *
 */
static inline uint32_t IHC_get_ip_version(void)
{
    return *(g_ihc.ip_version);
}

/*******************************************************************************
 * The IHC_get_debug_id() returns the DEBUG register details for respective
 * channel
 *
 * @param channel
 *   The channel number to get debug id for
 *
 * @return
 *   DEBUG register
 *
 */
static inline uint32_t IHC_get_debug_id(uint8_t channel)
{
    return g_ihc.IHC_Channels[channel].HART_IHCC->DEBUG_ID;
}

/*******************************************************************************
 * The IHC_get_mp_intr_status() returns the interrupt status for message
 * present.
 *
 * @param channel
 *   The channel to get interrupt details
 *
 * @return
 *   enable/disable status
 *
 */
static inline uint32_t IHC_get_mp_intr_status(uint8_t channel)
{
    uint32_t module = (uint32_t) channel / CH_PER_MODULE;
    uint32_t module_index = (uint32_t) channel % CH_PER_MODULE;

    if(module_index >= module)
    {
        module_index++;
    }

    return ((g_ihc.HART_IHCIM[module]->IRQ_MASK & (1U << (module_index << 1U))) &&
            (g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG & MIV_IHC_REGS_CH_CTRL_MPIE_MASK));
}

/*******************************************************************************
 * The IHC_get_mc_intr_status() returns the interrupt status for message
 * consumed.
 *
 * @param channel
 *   The channel to get interrupt details
 *
 * @return
 *   enable/disable status
 *
 */
static inline uint32_t IHC_get_mc_intr_status(uint8_t channel)
{
    uint32_t module = (uint32_t)channel / CH_PER_MODULE;
    uint32_t module_index = (uint32_t)channel % CH_PER_MODULE;

    if (module_index >= module)
    {
        module_index++;
    }

    return ((g_ihc.HART_IHCIM[module]->IRQ_MASK & (1U << ((module_index << 1U) + 1U))) &&
            (g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG & MIV_IHC_REGS_CH_CTRL_ACKIE_MASK));
}

/*******************************************************************************
 * The IHC_get_module_intr_status() returns the interrupt status for module.
 *
 * @param channel
 *   The channel to get interrupt details
 *
 * @return
 *   enable/disable status
 *
 */
static inline uint32_t IHC_get_module_intr_status(uint8_t module_num)
{
    return (g_ihc.HART_IHCIM[module_num]->IRQ_STATUS & MIV_IHC_REGS_IRQ_STATUS_NS_MASK);
}

#ifdef __cplusplus
}
#endif

#endif /* __MSS_MIV_IHC_H_ */