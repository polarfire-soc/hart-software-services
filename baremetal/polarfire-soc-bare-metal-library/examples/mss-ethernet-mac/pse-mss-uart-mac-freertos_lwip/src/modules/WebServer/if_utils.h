/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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
 * Declarations for utility functions in main.c for netif access.
 *
 * SVN $Revision: 7941 $
 * SVN $Date: 2015-10-07 16:55:21 +0100 (Wed, 07 Oct 2015) $
 */

/*
 * if_utils.h
 *
 *  Created on: Apr 24, 2016
 *      Author: peter.mcshane
 */

#ifndef IF_UTILS_H_
#define IF_UTILS_H_

#ifdef _ZL303XX_CORE_TSE
#include "core_tse.h"
#else
#include "drivers/mss_mac/mss_ethernet_mac.h"
#endif
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define IP4_IP_ADDR 0
#define IP4_IP_MASK 1
#define IP4_IP_GW   2


typedef enum __ntm_dhcp_mode_t
{
    NTM_DHCP_FIXED    = 0,
    NTM_DHCP_PURE     = 1,
    NTM_DHCP_FALLBACK = 2
} ntm_dhcp_mode_t;

extern ntm_dhcp_mode_t g_network_addr_mode; /* Current network addressing mode */
extern ip4_addr_t      g_ip_address; /* Configured fixed/default address */
extern ip4_addr_t      g_ip_mask;    /* Configured fixed/default mask */
extern ip4_addr_t      g_ip_gateway; /* Configured fixed/default gateway address */

uint32_t get_ipv4_address(int32_t select);
void     set_ipv4_address(int32_t select, ip4_addr_t *address, uint32_t commit);

void     set_ipv4_address_mode(int32_t mode);

void     get_mac_address(uint8_t * mac_addr);
void     set_mac_address(uint8_t * mac_addr);

void prvEthernetConfigureInterface(void * param);

#if LWIP_IPV6
ip6_addr_t get_ipv6_address(int32_t select);
#endif

#ifdef _ZL303XX_CORE_TSE
extern tse_cfg_t g_mac_config;
#else
extern mss_mac_cfg_t g_mac_config;
#endif
extern TaskHandle_t thandle_uart;
extern TaskHandle_t thandle_link;
extern TaskHandle_t thandle_web;

#endif /* IF_UTILS_H_ */
