/* 
 * File:   lldp_tlv_handler_table.h
 * Author: c14210
 *
 * Created on March 4, 2015, 4:35 PM
 */

#ifndef LLDP_TLV_HANDLER_TABLE_H
#define	LLDP_TLV_HANDLER_TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include "lldp.h"

#define CISCO_OUI                   0x000142
#define CISCO_ALT_OUI               0x00005E											
#define IEEE802_3_OUI               0x00120F
#define TIA_OUI                     0x0012BB
#define BASIC_TLVS_TABLE_SIZE        7
#define PROCESSING_TLVS_TABLE_SIZE   2

extern const uint8_t AddressType;

extern bool independentSparedArchitectur;										 
	
extern const orgSpecificTLVs_t lldporgProcessingTlvTable[PROCESSING_TLVS_TABLE_SIZE];
extern const createOrgTLV_t lldpCallOrgSpecTlvTable[];
extern const createBasicTLV_t lldpCallFixedTlvTable[BASIC_TLVS_TABLE_SIZE];
uint8_t get_org_tlvs_table_size (void);
//LLDP APIs that user might need to touch

void LLDP_Run(void);
void LLDP_InitRx(void);
void LLDP_InitTx(void);
void LLDP_InitRxTx(void);
void LLDP_DecTTR(void);

void LLDP_SetDesiredPower(uint16_t);
uint16_t LLDP_GetAllocatedPower(void);

void   LLDP_setPortDescription(const char* val);
char * LLDP_getInfo(char*Name);

void LLDP_setAssetID(const char* val);
char* LLDP_getAssetID(void);

void LLDP_setModelName(const char* val);
char* LLDP_getModelName(void);

void LLDP_setManufacturer(const char* val);
char* LLDP_getManufacturer(void);

void LLDP_setSerialNumber(const char* val);
char* LLDP_getSerialNumber(void);

void LLDP_setSoftwareRevision(const char* val);
char* LLDP_getSoftwareRevision(void);

void LLDP_setHardwareRevision(const char* val);
char* LLDP_getHardwareRevision(void);

void LLDP_setFirmwareRevision(const char* val);
char* LLDP_getFirmwareRevision(void);

void LLDP_setMUDInfo(const char* val);

MulticastMacAddr LLDP_SetDestAddress(void);

error_msg LLDP_createCiscoMDI              (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createIeeeMDI               (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTiaMED_cap            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTiaMED                (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createIeeeConfig            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_Net_pol           (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_PWR_MDI           (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_HW_Rev            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_FW_Rev            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_SW_Rev            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_Srl_Num           (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_Manufacturer_Name (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_Model_Name        (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createTia_Asset_ID          (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createCisco_MUD_Ext         (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createCiscoOUI              (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createCiscoClass            (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);
error_msg LLDP_createCiscoProtocol         (uint8_t *, uint8_t **, uint8_t,uint16_t,uint32_t);

error_msg createChassisTLV  (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createPortTLV     (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createTTLTLV      (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createPortDesc    (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createSysCap      (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createSystemDesc  (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createSysCap      (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);
error_msg createMgmtAddrTLV (uint8_t *, uint8_t **, uint8_t,uint8_t,uint16_t);

error_msg processCiscoPowerTlv(uint8_t *, uint8_t **, uint8_t, uint32_t, uint8_t);
error_msg processIEEE3PowerTlv(uint8_t *, uint8_t **, uint8_t, uint32_t, uint8_t);

#endif	/* LLDP_TLV_HANDLER_TABLE_H */

