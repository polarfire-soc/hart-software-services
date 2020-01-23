/*
 * seg.h
 *
 *  Created on: Aug 24, 2017
 *      Author: Ken.Irving
 */

#ifndef SEG_H
#define SEG_H


#include <stdint.h>

typedef struct {

	struct {
		volatile int32_t  	offset : 15;
		volatile int32_t  	rsrvd  : 16;
		volatile int32_t  	locked : 1;
	} CFG[6];

	uint32_t fill[64-6];

} seg_t;

#define SEG ((seg_t*) 0x20005d00)

#endif

