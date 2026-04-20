/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SNVM_BM2_LOADER_H_
#define SNVM_BM2_LOADER_H_

/*
 * Main entry point - called by the E51 CRT after BSS is zeroed.
 * Never returns; halts with ebreak on error, jumps to L2LIM on success.
 */
void bm2_main(void);

#endif /* SNVM_BM2_LOADER_H_ */
