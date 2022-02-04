/*******************************************************************************
 * Copyright 2021-2021 Microchip FPGA Embedded Systems Solutions.
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
 *
 * PolarFire SoC Microprocessor Subsystem bare metal software driver
 * public API.
 *
 */
/*=========================================================================*//**
  @mainpage PolarFire FPGA-IP MiV-IHC Bare Metal Driver.


  ==============================================================================
  Introduction
  ==============================================================================
  The Bare metal MiV-IHC driver facilitates the use of the MiV-IHC FPGA-IP which
  helps inter hart/processor communication as part of a IP/SW stack.

  ==============================================================================
  Definition of terms
  ==============================================================================
  Terms definition

     - Channel A-B  This refers to the communications connection between two
       Harts
     - MiV-IHCC MiV Inter Hart Communications Channel
       This is the IP core for each channel. It has an A and a B side. Each
       side is configured from the hart associated with each side using the apb
       bus.
     - MiV-IHCIA MiV Inter Hart Communication Interrupt Aggregator IP
       This is the Aggregator Core, one for each hart. It takes inputs
       from the MiV-IHCC's and generates an interrupt to the connected hart based
       on the programmed configuration.
     - MiV-IHC MiV Inter Hart Communication IP
       This is the collection of MiV-IHCC's and MiV-ICA's which form the
       Inter hart Communications IP solution.

  ==============================================================================
  Driver and IP version
  ==============================================================================
  The driver version can be found in the following two places:
      - The constants section of the User Guide
      - In the miv_ihc_version.h file

  IP version
      - Each module used in the MiV-IHC contains a read only version
        register. This can be read to determine the module version.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The MiV-IHC IP must be present in your FPGA design and connected. The base
  addresses of the IP must be mapped as designed. If the default addressing is
  not used, the base addresses listed in miv_ihc_add_mapping.h must be edited
  to reflect the base addresses used in the Libero design.
  Likewise it the connected interrupts deviate from default, the mappings must
  be updated in the miv_ihc_config.h file to reflect the Libero design.
  Note the address mapping is divided into five regions to allow ease use of
  PMPs to restrict access to registers from harts that are not required b that
  hart. Each miv_IHCC has a register set associated with its A and B side, each
  of which appears in separate address space associated with the connected core.
  Below is a simplified view of the MiV-IHC.

  ```
        ^               ^               ^               ^               ^
        |               |               |               |               |
+-----------------------------------------------------------------------------------------+
|       | Int to hart0  | Int to hart1  | Int to hart2  | Int to hart3  | Int to hart4    |
|       |               |               |               |               |                 |
|    +--+--------+   +--+--------+   +--+--------+   +--+--------+   +--+--------+        |
|    | miv_IHCIA |  | miv_IHCIA  |   | miv_IHCIA |   | miv_IHCIA |   | miv_IHCIA |        |
|    |  hart0    |   |  hart1    |   |  hart2    |   |  hart3    |   |  hart4    |        |
|    +---+-------+   +^--^-------+   +^--^---^---+   +--------^--+   +---^-^---^-+        |
|        ^            |  |            |  |   |                |          | |   |          |
|        |   +--------+  |  +---------+  |   |                |          | |   |          |
|        |   |           |  |       +----+   |                |          | |   |          |
|      +-+---+-+      +--+--+-+     |  +-----+-+       +------++         | |   |          |
|      |miv_IHCC|     |miv_IHCC|    |  |miv_IHCC|      |miv_IHCC+--------+ |   |          |
|      | 0<->1  |     | 1<->2  |    |  | 2<->4  +----+ | 3<->4  |          |   |          |
|      +--------+     +--------+    |  +--------+    |  +-------+          |   |          |
|                                   |                +---------------------+   |          |
|      +-------+      +--------+    |  +-------+                               |          |
|      |miv_IHCC|     |miv_IHCC|    +--+miv_IHCC+------------------------------+          |
|      | 0<->2  |     | 1<->3  |       | 2<->4 |                                          |
|      +--------+     +--------+       +-------+                                          |
|                                                                                         |
|      +--------+     +--------+       miv_IHC - Inter Hart Communication module          |
|      |miv_IHCC|     |miv_IHCC|       Not all connections are shown for                  |
|      | 0<->3  |     | 1<->4  |       reasons of clarity                                 |
|      +--------+     +--------+                                                          |
|                                                                                         |
|      +--------+                                                                         |
|      |miv_IHCC|                                                                         |
|      | 0<->4  |                                                                         |
|      +--------+                                                                         |
|                                                                                         |
+-----------------------------------------------------------------------------------------+


  ```

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The MiV-IHC facilitates sending messages between independent OS running on
  Separate harts. API functions are provided for the following:
    - Initialization and configuration functions
    - Polled transmit and receive functions
    - Interrupt driven transmit and receive functions

  --------------------------------
  Initialization and Configuration
  --------------------------------
  At start-up, one hart should be designated to Initialize the MiV-IHC and set
  to default values.
  This hart must have access to the full memory range. In the Microchip software
  system, the HSS carries out this function.
  Local contexts Initialize local channels. Examples of initialization are given
  in the functions descriptions.
  local initialization routines
  A drawing of an example start-up showing the order of the functions used is
  shown below:

```
    +----------------+
    |                |
    |  monitor hart  |                           Example start-up
    |                |
    +-------+--------+
            |
   +--------v---------+
   |IHC_global_init()|
   +--------+---------+
            |
+-----------v--------------+   +----------------+---------------------------------+--------------------------------+---------------------------------+
|IHC_local_context_init()  |   |                |                                 |                                |                                 |
+-----------+--------------+   |                |                                 |                                |                                 |
            ^                  |                |                                 |                                |                                 |
+-----------+--------------+   |    +-----------v--------------+    +-------------v------------+     +-------------v------------+     +--------------v-----------+
|IHC_local_remote_config() |   |    |IHC_local_context_init() |    |IHC_local_context_init() |     |IHC_local_context_init() |     |IHC_local_context_init() |
| channel 0 <> 1 config    |   |    +--------------------------+    +-------------+------------+     +-------------+------------+     +--------------+-----------+
+--------------------------+   |                                                  |                                |                                 |
            |                  |    +--------------------------+    +-------------v------------+     +-------------v------------+     +--------------v-----------+
+-----------v--------------+   |    |IHC_local_remote_config()|    |IHC_local_remote_config()|     |IHC_local_remote_config()|     |IHC_local_remote_config()|
|IHC_local_remote_config() |   |    | channel 1 <> 0 config    |    | channel 1 <> 0 config    |     | channel 1 <> 0 config    |     | channel 4 <> 0 config    |
| channel 0 <> 2 config    |   |    +--------------------------+    +-------------+------------+     +-------------+------------+     +--------------+-----------+
+--------------------------+   |                                                  |                                |                                 |
            |                  |    +--------------------------+                  v                                v                  +--------------v-----------+
+-----------v--------------+   |    |IHC_local_remote_config()|                                                                      |IHC_local_remote_config()|
|IHC_local_remote_config() |   |    | channel A <> B config    |                                                                      | channel B <> A config    |
| channel 0 <> 3 config    |   |    +--------------------------+                                                                      +--------------+-----------+
+--------------------------+   |                |                                                                                                    |
            |                  |                v                                                                                                    v
+-----------v--------------+   |
|IHC_local_remote_config() |   |
| channel 0 <> 4 config    |   |
+--------------------------+   |
            |                  |
+-----------v---------------+  |
| Start other harts/contexts+--+
+-----------+---------------+
            |
            v

```

  --------------------------------------
  Polled Transmit and Receive Operations
  --------------------------------------
  See the function description for message_present_poll(void) below.
  Note one side of a channel can be polled and the other side interrupt driven.
  This is the case in the HSS where on the HSS side on hart0 incoming messages
  are polled. The outgoing messages are interrupt driven on the far side where
  the local interrupt is connected from the MiV-IHC and incoming messages are
  received by the registered handler or the particular interrupt.

  ---------------------------
  Interrupt Driven Operation
  ---------------------------
  On start-up the driver uses the function IHC_local_remote_config() to
  register an interrupt handler for each incoming channel being configured.
  Below is a bare metal example of handling an incoming message:

  ```
                  +-------------------------+
                  | registered function for |
                  | incoming channel A<>B   |
                  +-----------------+-------+
     +-------------------------+    ^
     | registered function for |    |
     | incoming channel 0<>4   |    |
     +----+--------------------+    |
          ^                         |
          | Bare metal application  |
+------------------------------------------+
          | Bare metal Driver       |
          |                         |
          |                         |
          |                         |
     +----+-------------------------+--+
     |  miv_ihc_hart4_int()            |
     |                                 |
     |  parses IP and calls registered |
     |  interrupt associated with the  |
     |  channel                        |
     +--------+------------------------+
              ^
              |
              |
              | Interrupt from MiV-IHCIA
              + to hart4
  ```

  ==============================================================================
  Files used in the MiV-IHC
  ==============================================================================
  Below are the names along with a brief description of each file used in the
  MiV-IHC bare metal driver. They are grouped into two sections. The fixed
  files in the driver folder, and configuration files which are edited to
  configure for particular use case and it is recommended to place them under
  the boards directory in the recommended program structure for a bare metal
  project( See the associated example project or HSS).

  --------------------------------
  Driver files
  --------------------------------
  These files are found in

  ```
  src/platform/drivers/fpga-ip/miv_icm/
  ```

  |file name                  | detail                                         |
  |---------------------------| -----------------------------------------------|
  | **miv_ihc_defines.h**     | defines required by configuration files        |
  | **miv_ihc_regs.h**        | defines related to hardware                    |
  | **miv_ihc.h**             | software and API defines                       |
  | **miv_ihc_version.h**     | Contains the version to the software driver    |
  | **miv_ihc.c**             | All code including API functions               |

  --------------------------------
  Configuration files
  --------------------------------
  These files are found in

  ```
  src/boards/your-board/platform-config/drivers-config/fpga-ip/miv_icm/
  ```

  |file name                      | detail                                            |
  |-------------------------------| --------------------------------------------------|
  | **miv_ihc_add_mapping.h**     | address MiV-IHC is instantiated in Libero design  |
  | **miv_ihc_config.h**          | Configures channels used in software design       |

  Please note when using the MiV-IHC, the configuration files must be
  referenced in the mss_sw_config.h file, to include them in the project.
  See the example project.

 *//*=========================================================================*/
#ifndef __MSS_MIV_IHC_H_
#define __MSS_MIV_IHC_H_ 1

#include <stddef.h>
#include <stdint.h>
#include "miv_ihc_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern IHC_TypeDef * IHC[];

/*-------------------------------------------------------------------------*//**

  ## Interrupt mapping table

  Choose the interrupt mapping used in you system
  Please see miv_ihc_regs.h for the defaults
  The default values below are over-written with those in the
  miv_ihc_add_mapping.h file if it this is present in the platform directory.

  |driver interrupt name |default mapped fabric interrupt in the ref. design   |
  |---------------------------------| -----------------------------------------|
  | **IHCIA_hart0_int**              | fabric_f2h_63_plic_IRQHandler            |
  | **IHCIA_hart1_int**              | fabric_f2h_62_plic_IRQHandler            |
  | **IHCIA_hart2_int**              | fabric_f2h_61_plic_IRQHandler            |
  | **IHCIA_hart3_int**              | fabric_f2h_60_plic_IRQHandler            |
  | **IHCIA_hart4_int**              | fabric_f2h_59_plic_IRQHandler            |

 */
#ifndef IHCIA_hart0_IRQHandler
#define IHCIA_hart0_IRQHandler fabric_f2h_63_plic_IRQHandler
#endif
#ifndef IHCIA_hart1_IRQHandler
#define IHCIA_hart1_IRQHandler fabric_f2h_62_plic_IRQHandler
#endif
#ifndef IHCIA_hart2_IRQHandler
#define IHCIA_hart2_IRQHandler fabric_f2h_61_plic_IRQHandler
#endif
#ifndef IHCIA_hart3_IRQHandler
#define IHCIA_hart3_IRQHandler fabric_f2h_60_plic_IRQHandler
#endif
#ifndef IHCIA_hart4_IRQHandler
#define IHCIA_hart4_IRQHandler fabric_f2h_59_plic_IRQHandler
#endif

#ifndef IHCIA_hart0_INT
#define IHCIA_hart0_INT  FABRIC_F2H_63_PLIC
#endif

#ifndef IHCIA_hart1_INT
#define IHCIA_hart1_INT  FABRIC_F2H_62_PLIC
#endif

#ifndef IHCIA_hart2_INT
#define IHCIA_hart2_INT  FABRIC_F2H_61_PLIC
#endif

#ifndef IHCIA_hart3_INT
#define IHCIA_hart3_INT  FABRIC_F2H_60_PLIC
#endif

#ifndef IHCIA_hart4_INT
#define IHCIA_hart4_INT  FABRIC_F2H_59_PLIC
#endif

/*-------------------------------------------------------------------------*//**

  ## Channel definitions

  Choose the interrupt mapping used in you system
  Please see miv_ihc_regs.h for the defaults
  The default values below are over-written with those in the
  miv_ihc_add_mapping.h file if it this is present in the platform directory.

  |value | channel                      |comment                               |
  |------|------------------------------| -------------------------------------|
  | 0    | **IHC_CHANNEL_TO_HART0**     | channel connected to hart 0          |
  | 1    | **IHC_CHANNEL_TO_HART1**     | channel connected to hart 1          |
  | 2    | **IHC_CHANNEL_TO_HART2**     | channel connected to hart 2          |
  | 3    | **IHC_CHANNEL_TO_HART3**     | channel connected to hart 3          |
  | 4    | **IHC_CHANNEL_TO_HART4**     | channel connected to hart 4          |
  | 5    | **IHC_CHANNEL_TO_CONTEXTA**  | channel context A                    |
  | 6    | **IHC_CHANNEL_TO_CONTEXTB**  | channel context B                    |

 */
typedef enum IHC_CHANNEL_
{
    IHC_CHANNEL_TO_HART0              = 0x00,   /*!< your hart to hart 0 */
    IHC_CHANNEL_TO_HART1              = 0x01,   /*!< your hart to hart 1 */
    IHC_CHANNEL_TO_HART2              = 0x02,   /*!< your hart to hart 2 */
    IHC_CHANNEL_TO_HART3              = 0x03,   /*!< your hart to hart 3 */
    IHC_CHANNEL_TO_HART4              = 0x04,   /*!< your hart to hart 4 */
    IHC_CHANNEL_TO_CONTEXTA           = 0x05,   /*!< your hart to context A */
    IHC_CHANNEL_TO_CONTEXTB           = 0x06,   /*!< your hart to context B */
}   IHC_CHANNEL;

/*-------------------------------------------------------------------------*//**
  These hold pointers to IP associated with each hart
  Generally the mapping is fixed between Libero designs to keep thing obvious
  but it can change. The base addresses are located in the mapping header file.
 */
extern IHC_TypeDef                  IHC_H0_IP_GROUP;
extern IHC_TypeDef                  IHC_H1_IP_GROUP;
extern IHC_TypeDef                  IHC_H2_IP_GROUP;
extern IHC_TypeDef                  IHC_H3_IP_GROUP;
extern IHC_TypeDef                  IHC_H4_IP_GROUP;

/*--------------------------------Public APIs---------------------------------*/

/*-------------------------------------------------------------------------*//**
  The IHC_global_init() function initializes the IP. It is the first function
  called that accesses the MiV-IHC.
  It must be called from the monitor hart before other harts try and access the
  MiV-IHC. It assumes access to the full memory map. It initializes regs to
  default values which will later be updated using a local init function called
  from the Hart using the particular channels related to it hartid in the
  MiV-IHC.

  @param
    No parameters

  @return
    This function does not return a value.

  @code
      // Initialization code
      #include "mss_ihc.h"
      int main(void)
      {

          // The IHC_global_init() function initializes the IP. It is the first
          // function called that accesses the MiV-IHC. it must be called from
          // the monitor hart before other harts try and access the MiV-IHC.
          // It assumes access to the full memory map.

        IHC_global_init();

        uint32_t local_hartid = HSS_HART_ID;
        IHC_local_context_init((uint32_t)local_hartid);

        {
            uint32_t remote_hart_id = HART1_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART2_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART3_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART4_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }
        return (0u);
      }
  @endcode
 */
void IHC_global_init
(
    void
);

/*-------------------------------------------------------------------------*//**
  The IHC_local_context_init() function initializes the IP. It is called from
  each hart using the inter hart comms once on start-up.
  @param
    No parameters

  @return
    This function does not return a value.

  @code
      // Initialization code
      #include "mss_ihc.h"
      int main(void)
      {

          // The IHC_global_init() function initializes the IP. It is the first
          // function called that accesses the MiV-IHC. it must be called from
          // the monitor hart before other harts try and access the MiV-IHC.
          // It assumes access to the full memory map.

        IHC_global_init();

        uint32_t local_hartid = HSS_HART_ID;
        IHC_local_context_init((uint32_t)local_hartid);

        {
            uint32_t remote_hart_id = HART1_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART2_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART3_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART4_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }
        return (0u);
      }
  @endcode
 */
uint8_t IHC_local_context_init(uint32_t hart_to_configure);

/*-------------------------------------------------------------------------*//**
  The IIHC_local_remote_config() function initializes the IP. It is called from
  each hart using the inter hart comms once for every hart ot context the local
  hart is communicating with.
  @param
    No parameters

  @return
    This function does not return a value.

  @code
      // Initialization code
      #include "mss_ihc.h"
      int main(void)
      {

          // The IHC_global_init() function initializes the IP. It is the first
          // function called that accesses the MiV-IHC. it must be called from
          // the monitor hart before other harts try and access the MiV-IHC.
          // It assumes access to the full memory map.

        IHC_global_init();

        uint32_t local_hartid = HSS_HART_ID;
        IHC_local_context_init((uint32_t)local_hartid);

        {
            uint32_t remote_hart_id = HART1_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART2_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART3_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }

        {
            uint32_t remote_hart_id = HART4_ID;
            bool set_mpie_en = true;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, queue_incoming_hss_main, set_mpie_en, set_ack_en);
        }
        return (0u);
      }
  @endcode
 */
uint8_t IHC_local_remote_config(uint32_t hart_to_configure, uint32_t remote_hart_id, QUEUE_IHC_INCOMING  handler, bool set_mpie_en, bool set_ack_en );


/*-------------------------------------------------------------------------*//**
  The IHC_context_to_local_hart_id() Returns the lowest hart ID of the context
  the local hart is in.

  @param channel
    The channel we want the local hart ID of.

  @return
    hartID

  @code
        // example code showing use
        uint32_t my_hart_id = context_to_local_hart_id(channel);
        uint32_t remote_hart_id = context_to_remote_hart_id(channel);
        uint32_t message_size = IHC[my_hart_id]->HART_IHC[remote_hart_id]->size_msg;
  @endcode
 */
uint32_t IHC_context_to_local_hart_id(IHC_CHANNEL channel);

/*-------------------------------------------------------------------------*//**
  The IHC_context_to_remote_hart_id() Returns the lowest hart ID of the context
  the remote hart is in.

  @param channel
    The channel we want the remote hart ID of.

  @return
    hartID

  @code
        // example code showing use
        uint32_t my_hart_id = context_to_local_hart_id(channel);
        uint32_t remote_hart_id = context_to_remote_hart_id(channel);
        uint32_t message_size = IHC[my_hart_id]->HART_IHC[remote_hart_id]->size_msg;
  @endcode
 */
uint32_t IHC_context_to_remote_hart_id(IHC_CHANNEL channel);


/*-------------------------------------------------------------------------*//**
  The IHC_tx_message()
  Is used to send a message

  @param channel
    The channel we want the remote hart ID of.

  @param message
    Pointer to message being sent

  @return MP_BUSY / MESSAGE_SENT

  @code
        // example code showing use
        if ( MESSAGE_SENT == IHC_tx_message(IHC_CHANNEL_TO_HART0, (uint32_t *)&tx_message_buffer[0]))
        {
            // message has been sent
        }
        else
        {
            // you can try again...
  @endcode
 */
extern uint32_t IHC_tx_message(IHC_CHANNEL channel, uint32_t *message);

/*-------------------------------------------------------------------------*//**
  The IHC_message_present_poll()
  When calls parse to see if message present and processes the message with
  message present handler previous registered using the
  IHC_local_remote_config() function

  @param none

  @return none

  @code
        // example code showing use
        uint32_t local_hartid = HSS_HART_ID;
        IHC_local_context_init((uint32_t)local_hartid);
        {
            uint32_t remote_hart_id = HART1_ID;
            bool set_mpie_en = false;
            bool set_ack_en = false;
            IHC_local_remote_config((uint32_t)local_hartid, remote_hart_id, our_message_handler, set_mpie_en, set_ack_en);
        }
        while(1)
        {
            // poll for incoming messages
            // if message present it is handled using our_message_handler()
            // register earlier, and written by you based on one of the examples
            // in the example programs bundled with this driver
            message_present_poll();
            // ..
  @endcode
 */
void  IHC_message_present_poll(void);

/*-------------------------------------------------------------------------*//**
  The IHC_message_present_indirect_isr()
  When called parse to see if message present and processes the message with
  message present handler previous registered using the
  IHC_local_remote_config() function. When we call this function we already
  have worked out our hart ID and have our message storage pointer.
  One use case is when openSBI handler when called by an upper layer.
  Below is a simplified flow diagram:

  ```
       +--------------------------+
       |RP message linux handler  |
       |         s-mode           |
       +------+------------+----+-+
              ^            |    ^
              |            |    |
              |            |    |
              |            |    |
              |            v    |
+-------------+----+    +--+----+------------+
|MiV-IHCIA         |    |ecall               |
|fabric interrupt  |    |openSBI_int_handler |
|openSBI_int_handler    |m-mode              |
|m-mode            |    |                    |
+---------+--------+    +--+----+------------+
          ^                |    ^
          |        +-------v----+------------------+
    MP interrupt   | message_present_indirect_isr()|
          |        +-------------------------------+
          +

  ```

  @param my_hart_id
      hart ID of local MiV-IHCIA

  @param remote_channel
      remote channel ID

  @param message_storage_ptr
      where we want to store the incoming message

  @return none

  @code
        // to add example code showing use
  @endcode
 */
void  IHC_message_present_indirect_isr(uint32_t my_hart_id, uint32_t remote_channel, uint32_t * message_storage_ptr);


#ifdef __cplusplus
}
#endif

#endif /* __MSS_MIV_IHC_H_ */
