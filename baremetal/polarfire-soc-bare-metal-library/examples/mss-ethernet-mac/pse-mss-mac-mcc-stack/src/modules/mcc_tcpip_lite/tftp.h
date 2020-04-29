/**
  TFTP Client Protocol header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    tftp.h

  Summary:
    This is the header file for the tftp.c

  Description:
    This header file provides the APIs for TFTP implementation.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
and any derivatives exclusively with Microchip products. 
  
THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF 
NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR ITS 
INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE 
IN ANY APPLICATION. 

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL 
OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED 
TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY 
OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S 
TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED 
THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS. 

*/

#ifndef TFTP_H
#define	TFTP_H

#include <stdbool.h>

#define DESTPORT_TFTP       69

typedef enum
{
    tftp_read = 1,
    tftp_write,
    tftp_data,
    tftp_ack,
    tftp_error,
    tftp_optack
}tftp_client_state;

typedef enum
{
    TFTP_READ_REQUEST=0x0001,
    TFTP_WRITE_REQUEST,
    TFTP_DATA,
    TFTP_ACK,
    TFTP_ERROR,
    TFTP_OPTACK
}tftp_opcode;

typedef struct
{
    uint32_t tftp_server_address;  
    char tftp_block_size[5]; 
    char tftp_filename[32];
}tftp_t;
extern tftp_t tftp_msg;
extern uint32_t tftp_last_address;
extern volatile bool last_block;

/*TFTP Acknowledgement.
 * The function sends an acknowledgement on receiving a packet from a TFTP Server.
 * 
 * @param opcode
 *      Packet Opcode
 *
 * @param return
 *      Nothing
 * 
 */
void        TFTP_Ack(uint8_t opcode);

/*TFTP Send Read/ Write Request.
 * The function sends a read or write request to the TFTP server depending on the opcode.
 * 
 * @param opcode
 *      Packet Opcode
 *
 * @param return
 *      Status of the read/ write request
 * 
 */
error_msg   TFTP_Read_Write_Request(uint8_t opcode);

/*Handler for TFTP process.
 * The function process a TFTP message as per the opcode in the packet.
 * 
 * @param length
 *      Packet Length
 *
 * @param return
 *      Nothing
 * 
 */
void        TFTP_Handler(uint8_t *packet, int16_t length);   // jira:M8TS-608

/*Processing TFTP data packets.
 * The function process the TFTP packets with 'data' opcode.
 * 
 * @param address
 *      Address where the data has to be stored (can be EEPROM address, Flash address, etc.)
 *
 * @param data
 *      Actual data in the packet
 *
 * @param length
 *      Packet Length
 *
 * @param return
 *      Nothing
 * 
 */
uint8_t     Process_TFTP_Data(uint32_t address, char *data, uint16_t length);

/*Configuration for TFTP.
 * The function allows the user to configure the options for TFTP server.
 * 
 * @param tftp_server_address
 *      TFTP server address
 *
 * @param tftp_filename
 *      Name of the file to be read from the TFTP server
 *
 * @param tftp_block_size
 *      Size of each packet in bytes sent by the TFTP server
 *
 * @param return
 *      Nothing
 * 
 */
void        TFTP_Configure(uint32_t tftp_server_address, char *tftp_filename, char *tftp_block_size);


#endif	/* TFTP_H */

