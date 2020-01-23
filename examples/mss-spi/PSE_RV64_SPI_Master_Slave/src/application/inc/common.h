/*
 * common.h
 *
 *  Created on: Jun 12, 2018
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

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
