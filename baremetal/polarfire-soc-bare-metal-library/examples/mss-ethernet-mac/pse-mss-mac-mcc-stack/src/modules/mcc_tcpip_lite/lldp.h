/* 
 * File:   lldp.h
 * Author: c14210
 *
 * Created on March 4, 2015, 10:49 AM
 */

#ifndef LLDP_H
#define	LLDP_H

#include "tcpip_types.h"
#include "mac_address.h"

#define MAX_TXBUFF_SIZE 1500
//Define the Varaibles used by LLDP, the values can be changed here if needed. All set to default value
#define txCreditMax     5/**< IEEE 802.1AB 9.2.5.17*/ //default 5
#define msgTxHold       4/**< IEEE 802.1AB 10.5.1  */ //default 4
#define msgTxInterval   30u/**< IEEE 802.1AB 10.5.1  */ //default 30s  //jira: CAE_MCU8-5647
#define msgFastTx       1u/**< IEEE 802.1AB 9.2.5.5 */ //defualt 1 if fast transmot is  needed  //jira: CAE_MCU8-5647
#define txFastInit      4/**< IEEE 802.1AB 9.2.5.19*/ //default 4
#define reinitDelay     2/**< IEEE 802.1AB 9.2.5.10*/ //default 2s
#define MulticastMacAddr mac48Address_t
#define PORTID mac48Address_t
#define setLLDPTick()       do{lldp_tx_timers.txTick=true;}while(0)
#define clrLLDPTick()       do{lldp_tx_timers.txTick=false;}while(0)
#define error

/*************************LLDP TX TIMER****************************/
typedef struct {
    uint16_t txDelay;       /**< IEEE 802.1AB 10.5.3  */ 
    uint16_t txTTR;         /**< IEEE 802.1AB 9.2.2.3 - transmit on expire. */
    uint16_t txShutdownWhile;/**< IEEE 802.1AB 9.2.2.3*/
    uint16_t txDelayWhile;
    bool     txTick;         /**< IEEE 802.1AB 9.2.5.21*/
}lldp_tx_timers_t ;
lldp_tx_timers_t lldp_tx_timers;

/*Mandatory & Optional TLV types from IEEE 802.1AB*/
typedef enum
{
    END_LLDPDU_TLV=0,                   //Mandatory
    CHASSIS_ID_TLV,                     //Mandatory
    PORT_ID_TLV,                        //Mandatory
    TIME_TO_LIVE_TLV,                   //Mandatory
    PORT_DESCRIPTION_TLV,               //OPTIONAL
    SYSTEM_NAME_TLV,                    //OPTIONAL
    SYSTEM_DESCRIPTION_TLV,             //OPTIONAL
    SYSTEM_CAPABILITIES_TLV,            //OPTIONAL
    MANAGEMENT_ADDRESS_TLV,             //OPTIONAL
    /* 9 - 126 are reserved */
    ORG_SPECIFIC_TLV=127                //OPTIONAL

} TLV_TYPES;

/* TLV Subtypes from section 9 of IEEE 802.1AB */
typedef enum
{
  CHASSIS_COMPONENT=1,
  CHASSIS_INTERFACE_ALIAS,
  CHASSIS_PORT_COMPONENT,
  CHASSIS_MAC_ADDRESS,
  CHASSIS_NETWORK_ADDRESS,
  CHASSIS_INTERFACE_NAME,
  CHASSIS_LOCALLY_ASSIGNED
  /* 8-255 - reserved */
} CHASSIS_SUBTYPE;

/* TLV Subtypes from section 9 of IEEE 802.1AB */
typedef enum
{
  /* 0 - reserved */
  reserved,
  PORT_INTERFACE_ALIAS=1,
  PORT_COMPONENT,
  PORT_MAC_ADDRESS,
  PORT_NETWORK_ADDRESS,
  PORT_INTERFACE_NAME,
  PORT_AGENT_CIRCUIT_ID,
  PORT_LOCALLY_ASSIGNED
  /* 8-255 - reserved */
} PORT_SUBTYPE;

/* organizationally Specific TLVS IEEE 802.3 sec 79.3*/
typedef enum
{
  /* 0 - reserved */
  res,
  IEEE_3_MAC_PHY_CONFIG=1,
  IEEE_3_POWER_VIA_MDI,
  IEEE_3_LINK_AGGREGATION,//Deprecated
  IEEE_3_MAX_FRAME_SIZE,
  IEEE_3_ENERGY_EFFICIENT_ETHERNET
  /* 6-255 - reserved */
} ORG_IEEE_3_SUBTYPE;

/* organizationally Specific TLVS ANSI-TIA_1057 sec 10*/
typedef enum
{
  /* 0 - reserved */
  res1,
  TIA_LLDP_MED=1,
  TIA_NETWORK_POLICY,
  TIA_LOCATION_ID,
  TIA_EXTENDED_POWER_VIA_MDI,
  TIA_INVENT_HW_REV,
  TIA_INVENT_FW_REV,
  TIA_INVENT_SW_REV,
  TIA_INVENT_SERIAL_NUM,
  TIA_INVENT_MANUFACT_NAME,
  TIA_INVENT_MODEL_NAME,
  TIA_INVENT_ASSET_ID
  /* 6-255 - reserved */
} ORG_TIA_SUBTYPE;



/* organizationally Specific TLVS IEEE 802.3 sec 79.3*/
typedef enum
{
  /* 0 - reserved */
  res2=0,
  CISCO_POWER_VIA_MDI=1,
  CISCO_OUI_TLV,
  CISCO_CLASS_TLV,
  CISCO_PROTOCOL_TLV
} ORG_CISCO_SUBTYPE;

/* organizationally Specific TLVS IEEE 802.3 sec 79.3*/
typedef enum
{
  /* 0 - reserved */
  res3=0,
  CISCO_MUD_TLV=1
} ORG_MUD_CISCO_SUBTYPE;			
/*************************LLDP TLVs*********************/


typedef error_msg(*createBasicFuncPtr)(uint8_t *, uint8_t **pkt_write, uint8_t,uint8_t,uint16_t);

typedef struct{
    uint8_t tlvOrder;
    uint8_t type;
    uint8_t subtype;
    uint16_t buffLength;
    createBasicFuncPtr callTlvMaker;
} createBasicTLV_t;

typedef error_msg(*createOrgFuncPtr)(uint8_t *, uint8_t **pkt_write, uint8_t,uint16_t,uint32_t);

typedef struct{
    uint8_t tlvOrder;
    uint8_t type;
    uint16_t buffLength;
    uint32_t OUI; //organizationally Unique Identifier
    createOrgFuncPtr callTlvMaker;
} createOrgTLV_t;

typedef error_msg (*OrgFuncPtr)(uint8_t *, uint8_t **pkt_read, uint8_t , uint32_t, uint8_t);

typedef struct
{
    uint32_t oui;
    uint8_t subtype;
    OrgFuncPtr processOrgTlvs;
} orgSpecificTLVs_t;


typedef union{
    uint8_t val;
    struct{
        unsigned fixTlvSize                 :1;
        unsigned poeEnabledPair             :1;
        unsigned uPoeEnabledPower           :1;
        unsigned poePlusEnabledPower        :1;
        unsigned poeEnabledMinPower         :1;
        unsigned is4WireSupported           :1;//Switch capabilities...it is RO for all practical purposes
        unsigned PD_requestSparePairOn      :1;//only if the desired power is more than 25.5w and 4-wire supported
        unsigned                            :1;								   
    };
}orgProcessFlags;

void LLDP_Packet(uint8_t *packet, uint8_t **pkt_read, uint32_t packet_length);

#endif	/* LLDP_H */

