/*******************************************************************************
 * (c) Copyright 2013 Microsemi SoC Products Group.  All rights reserved.
 * 
 * 
 *
 * SVN $Revision: 7009 $
 * SVN $Date: 2014-11-11 00:37:32 +0530 (Tue, 11 Nov 2014) $
 */
 
    .syntax unified
    .cpu cortex-m3
    .thumb
    
    .global lwip_cortem_chksum
    .type lwip_cortem_chksum, function

/*==============================================================================
 *
 */

lwip_cortem_chksum:
    PUSH   {R3-R11}
    MOV      R11, #0
    MOVS   R2, #0
    STRH.W R2, [SP]
    MOVS   R3, #0
    ANDS.W R2, R0, #1
    BEQ.N  lwip_cortem_chksum_0
          
    CMP    R1, #1
    BLT.N  lwip_cortem_chksum_0
          
    LDRB.W R4, [R0], #0x1
    STRB.W R4, [SP, #0x1]
    SUBS   R1, R1, #1
    
lwip_cortem_chksum_0:
    ANDS.W   R4, R0, #3
    BEQ.N    lwip_cortem_chksum_1

    CMP      R1, #2
    ITT      GE
    LDRHGE.W R3, [R0], #0x2
    SUBGE    R1, R1, #2
    
lwip_cortem_chksum_1:
    CMP      R1, #16
    BLT.N    lwip_cortem_chksum_2

lwip_cortem_chksum_3:
    LDMIA    R0!, {R4, R5, R6, R7}
    
    ADDS     R3, R3, R4
    ADCS     R3, R3, R5
    ADCS     R3, R3, R6
    ADCS     R3, R3, R7
    ADC      R3, R3, R11
    
    SUBS     R1, R1, #16
    CMP      R1, #16
    BGE.N    lwip_cortem_chksum_3

lwip_cortem_chksum_2:
    CMP      R1, #8
    BLT.N    lwip_cortem_chksum_21
    
    LDMIA    R0!, {R4, R5}
    ADDS     R3, R3, R4
    ADCS     R3, R3, R5
    ADC      R3, R3, R11
    SUBS     R1, R1, #8
    
lwip_cortem_chksum_21:
    CMP      R1, #4
    BLT.N    lwip_cortem_chksum_22
    
    LDMIA    R0!, {R4}
    ADDS     R3, R3, R4
    ADC      R3, R3, R11
    SUBS     R1, R1, #4
    
lwip_cortem_chksum_22:
    CMP      R1, #2
    BLT.N    lwip_cortem_chksum_4

    LDRH.W   R4, [R0], #0x2
    ADDS     R3, R4, R3
    ADC      R3, R3, R11
    SUBS     R1, R1, #2

lwip_cortem_chksum_4:
    CMP      R1, #1
    ITT      GE
    LDRBGE   R0, [R0]
    STRBGE.W R0, [SP]
    LDRH.W   R0, [SP]
    ADDS     R0, R0, R3
    
    UXTH     R1, R0
    ADD.W    R0, R1, R0, LSR #16
    UXTH     R1, R0
    ADD.W    R0, R1, R0, LSR #16
    CBZ      R2, lwip_cortem_chksum_6
    REV16    R0, R0
    
lwip_cortem_chksum_6:
    POP      {R1, R4-R11}
    BX       LR

.end
