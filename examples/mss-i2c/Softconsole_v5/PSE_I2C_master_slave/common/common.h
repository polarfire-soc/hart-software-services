/*******************************************************************************
 * (c) Copyright 20018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 * 
 * Stubs for Newlib system calls.
 *  
 * SVN $Revision: 10597 $
 * SVN $Date: 2018-11-23 15:48:29 +0000 (Fri, 23 Nov 2018) $
 */

#ifndef COMMON_H_
#define COMMON_H_


typedef enum COMMAND_TYPE_
{
	CLEAR_COMMANDS        				= 0x00,       	/*!< 0 default behavior              		*/
	START_HART1_U_MODE        			= 0x01,       	/*!< 1 u mode              					*/
	START_HART2_S_MODE        			= 0x02,       	/*!< 2 s mode              					*/
} COMMAND_TYPE;


/**
 * extern variables
 */
extern uint32_t menu_command;
extern uint64_t uart_lock;

/**
 * functions
 */
void mss_init_mutex(uint64_t address);
void mss_take_mutex(uint64_t address);
void mss_release_mutex(uint64_t address);
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);

#endif /* COMMON_H_ */
