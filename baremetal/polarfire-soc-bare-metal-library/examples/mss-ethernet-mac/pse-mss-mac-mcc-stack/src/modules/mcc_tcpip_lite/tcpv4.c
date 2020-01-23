/**
    Transmission Control Protocol (TCP) version4 implementation
    
  Company:
    Microchip Technology Inc.

  File Name:
    tcpv4.c

  Summary:
    This is the implementation of for TCP version 4 stack

  Description:
    This header file provides the implementation of the API for the TCP v4 stack.

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

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "ipv4.h"
#include "tcpv4.h"
#include "network.h"
#include "ethernet_driver.h"
#include "tcpip_types.h"
#include "log.h"
#include "tcpip_config.h"
#include "icmp.h"

tcpTCB_t *tcbList;
socklistsize_t tcbListSize;
tcpTCB_t *currentTCB;

static tcpHeader_p_t tcpHeader;
static uint16_t nextAvailablePort;
static uint32_t nextSequenceNumber;

static uint32_t receivedRemoteAddress;
static uint16_t rcvPayloadLen;
static uint8_t *rcvPayloadAddr;
static uint16_t tcpMss = 536;

//jira: CAE_MCU8-6056
static uint16_t tcpDataLength;
static uint16_t bytesToSendForRetransmit = 0;
static uint8_t *txBufferPtrForRetransmit;
static uint32_t localSeqnoForRetransmit;
static uint32_t lastAckNumber;

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest)  
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

static error_msg TCP_FiniteStateMachine(void);  //jira: CAE_MCU8-5647

static error_msg TCP_TimoutRetransmit(void);

error_msg TCP_PayloadSave(uint8_t *packet, uint16_t len);   //jira: CAE_MCU8-5647

/* PMCS: Todo - check size of buffer required */
uint8_t tcp_packet[1536];

/** The function will insert a pointer to the new TCB into the TCB pointer list.
 *
 *  @param ptr
 *      pointer to the user allocated memory for the TCB structure
 *
 * @return
 *      Status of the function
 */
static void TCB_Insert(tcpTCB_t *ptr)
{
    // Insert the new TCB at the head of the list.
    // This prevents a list traversal and saves time.
    if(tcbList != NULL)
    {
        // link this TCB as the previous one for the top of the list
        tcbList->prevTCB = ptr;
    }
    ptr->nextTCB = tcbList; // put the existing list at the end of this tcb...
    tcbList = ptr;           // put this tcb at the head of the list.
    ptr->prevTCB = NULL;     // make sure that the upstream pointer is empty
    tcbListSize++;
}

/** The function will remove a pointer to a TCB from the TCB pointer list
 *  and connect any TCB's before & behind together.
 *
 *  @param ptr
 *      pointer to the user allocated memory for the TCB structure
 *
 * @return
 *      None
 */
static void TCB_Remove(tcpTCB_t *ptr)
{
    if(tcbListSize > 1)
    {
        // check if this is the first in list
        if(ptr->prevTCB == NULL)
        {
            tcbList = ptr->nextTCB;
            ((tcpTCB_t *)(ptr->nextTCB))->prevTCB = NULL;
        } else
        {
            ((tcpTCB_t *)(ptr->prevTCB))->nextTCB = ptr->nextTCB;
            ((tcpTCB_t *)(ptr->nextTCB))->prevTCB = ptr->prevTCB;
        }
        tcbListSize --;
    }
    else if(tcbListSize==1)
    {
        tcbList = NULL;
    }
}

/** Reseting the socket to a known state.
 * 
 * @param tcbPtr 
 *      pointer to socket/TCB structure
 * 
 * @return
 *      None
 */
static void TCB_Reset(tcpTCB_t *tcbPtr)
{
    tcbPtr->destIP = 0;
    tcbPtr->destPort = 0;
    tcbPtr->localSeqno = 0;
    tcbPtr->localLastAck = 0;
    tcbPtr->remoteSeqno = 0;
    tcbPtr->remoteAck = 0;
    tcbPtr->remoteWnd = 0;

    tcbPtr->timeout = 0;
    tcbPtr->timeoutReloadValue = 0;
    tcbPtr->timeoutsCount = 0;
    tcbPtr->flags = 0;
    
    tcbPtr->localPort = 0;
    tcbPtr->bytesSent = 0;
    tcbPtr->payloadSave = false;
    tcbPtr->socketState = SOCKET_CLOSING;
}

/** Check is a pointer to a socket/TCB. If the pointer is in the TCB list
 *  then it is a valid socket.
 * 
 * @param tcbPtr 
 *      pointer to socket/TCB structure
 * 
 * @return
 *      None
 */
static error_msg TCB_Check(tcpTCB_t *ptr)   //jira: CAE_MCU8-5647
{
    tcpTCB_t *tcbPtr;
    error_msg ret = NET_ERROR;    //jira: CAE_MCU8-5647
    socklistsize_t count = 0;
    
    if((tcbList != NULL) && (ptr != NULL))
    {
        // search for ptr into the active TCB/sockets list
        tcbPtr = tcbList;
        while((tcbPtr != NULL) && (count < tcbListSize))
        {
            if (tcbPtr == ptr)
            {
                ret = NET_SUCCESS;   //jira: CAE_MCU8-5647
                break;
            }
            else
            {
                tcbPtr = tcbPtr->nextTCB;
                count++;
            }
        }        
    }
    return ret;
}


/** Internal function of the TCP Stack to send an TCP packet.
 * 
 * @param tcbPtr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      true - The buffer was send successfully
 * @return
 *      false - Send buffer fails.
 */
static error_msg TCP_Snd(tcpTCB_t *tcbPtr)   //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;
    tcpHeader_p_t txHeader;
    uint16_t payloadLength;
    uint16_t cksm;
    uint8_t *data;
    uint8_t *pkt_ptr = tcp_packet;

    txHeader.sourcePort = htons(tcbPtr->localPort);
    txHeader.destPort = htons(tcbPtr->destPort);

    txHeader.sequenceNumber = htonl(tcbPtr->localSeqno);

    txHeader.ackNumber = htonl(tcbPtr->remoteAck); //ask for next packet

    txHeader.dataOff = (uint8_t)(5U << 4);  /* We don't support options for now. Also clears flags nibble */
    txHeader.windowSize = htons(tcbPtr->localWnd);
    txHeader.checksum = 0;
    txHeader.urgentPtr = 0;

    if ((tcbPtr->flags) & (TCP_SYN_FLAG | TCP_RST_FLAG))
    {
        tcpDataLength = 0; // SYN and RST packets doesn't have any payload
    } 
    else if(tcbPtr->payloadSave == true)
    {
        tcpDataLength = 0;
    }else
    {
        tcpDataLength = tcbPtr->bytesSent;

        if (tcpDataLength != 0)
        {
            if(tcbPtr->remoteWnd == 0)
            {
                tcbPtr->remoteWnd = 1;
            }
            if(tcpDataLength > tcbPtr->remoteWnd)
            {
                tcpDataLength = tcbPtr->remoteWnd;
            }

            if(tcpDataLength > tcbPtr->mss)
            {
                tcpDataLength = tcbPtr->mss;
            }
            data = tcbPtr->txBufferPtr;

            // update the pointer to the next byte that needs to be sent
            tcbPtr->txBufferPtr = tcbPtr->txBufferPtr + tcpDataLength;
            tcbPtr->bytesToSend = (uint16_t)(tcbPtr->bytesSent - tcpDataLength);

            if (tcbPtr->bytesToSend == 0)
            {
                tcbPtr->flags = tcbPtr->flags | TCP_PSH_FLAG;
            }
        }
    }
    //update the TCP Flags
    txHeader.flags = tcbPtr->flags;
    payloadLength = (uint16_t)(sizeof(tcpHeader_t) + tcpDataLength);

    ret = IPv4_Start(tcp_packet, &pkt_ptr, tcbPtr->destIP, TCP_TCPIP);
    if (ret == NET_SUCCESS)
    {
        tcpHeader_p_t *p_txHeader;

        IP_MOVE_BYTES_OUT(pkt_ptr, txHeader, sizeof(tcpHeader_t));   //jira: M8TS-608
        p_txHeader = (tcpHeader_p_t *)pkt_ptr;

        if (tcpDataLength > 0)
        {
            IP_MOVE_BYTES_OUT(pkt_ptr, data, tcpDataLength);   //jira: M8TS-608
        }

        cksm = (uint16_t)(payloadLength + TCP_TCPIP);
        // Calculate the TCP checksum
        p_txHeader->checksum = IPV4_ComputeChecksum(tcp_packet + (OFFSET_TCP - 8), (uint16_t)(payloadLength + 8), cksm);

        ret = IPV4_Send(tcp_packet, payloadLength);
//        tcbPtr->txBufferPtr = tcbPtr->txBufferPtr - tcpDataLength;
    }

    // The packet wasn't transmitted
    // Use the timeout to retry again later
    if ((ret != NET_SUCCESS) && (ret != TX_QUEUED))	// jira: CAE_MCU8-5647, CAE_MCU8-6056
    {
        // make sure we keep the remaining timeouts and skip this send  that failed
        // try at least once
        tcbPtr->timeoutsCount = (uint8_t)(tcbPtr->timeoutsCount - 1u); // CAE_MCU8-5749, CAE_MCU8-5647

        if (tcbPtr->timeout == 0)
        {
            tcbPtr->timeout = TCP_START_TIMEOUT_VAL;
        }
    }
    else
    {
        //if the packet was sent increment the Seqno.
        tcbPtr->localSeqno = tcbPtr->localSeqno + tcpDataLength;
        logMsg("tcp_packet sent",LOG_INFO, LOG_DEST_CONSOLE);
    }

    return ret;
}

/** Internal function of the TCP Stack. Will copy the TCP packet payload to 
 * the socket RX buffer. This function will also send the ACK for
 * the received packet and any ready to be send data.
 * 
 * @param len
 *      length of the payload received
 * 
 * @return
 *      true - The payload was copied to RX buffer successfully
 * @return
 *      false - Copying the payload failed.
 */
error_msg TCP_PayloadSave(uint8_t *packet, uint16_t len)   //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;   //jira: CAE_MCU8-5647
    uint16_t buffer_size;
    uint8_t *pkt_ptr = packet;

    // check if we have a valid buffer
    if (currentTCB->rxBufState == RX_BUFF_IN_USE)
    {
        // make sure we have enough space
        if (currentTCB->localWnd >= len)
        {
            buffer_size = len;
        }else
        {
            buffer_size = currentTCB->localWnd;
        }
        
        IP_MOVE_BYTES_IN(currentTCB->rxBufferPtr, pkt_ptr, buffer_size);
        currentTCB->rxBufferPtr =  currentTCB->rxBufferPtr + buffer_size;

        //update the local window to inform the remote of the available space
        currentTCB->localWnd =  (uint16_t)(currentTCB->localWnd - buffer_size);
        currentTCB->remoteAck = currentTCB->remoteSeqno + buffer_size;

        //prepare to send the ACK and maybe some data if there are any
        currentTCB->flags = TCP_ACK_FLAG;
        currentTCB->payloadSave = true;
        
        TCP_Snd(currentTCB);
        currentTCB->payloadSave = false;
        ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
    }
    return ret;
}


/** This function will read and parse the OPTIONS field in TCP header.
 *  Each TCP header could have the options field.
 *  we will read only the ones that has SYN or SYN + ACK
 *  for the other TCP headers the options will be skipped.
 *
 * @param 
 *      None
 *  
 * @return
 *      true - parsing the options filed was successfully
 * @return
 *      false - parsing the options filed was failed.
 */
static error_msg TCP_ParseTCPOptions(uint8_t *packet, uint8_t **pkt_read)   //jira: CAE_MCU8-5647
{
    uint8_t  opt;
    uint16_t tcpOptionsSize;
    error_msg ret;     //jira: CAE_MCU8-5647
    uint8_t *pkt_ptr = *pkt_read;

    (void)packet;
    ret = NET_ERROR;      //jira: CAE_MCU8-5647
    // Check for the option fields in TCP header
    tcpOptionsSize = (uint16_t)(((tcpHeader.dataOff & 0xF0) >> 2u) - (uint16_t)sizeof(tcpHeader_t));   //jira: CAE_MCU8-5647

    if (tcpOptionsSize > 0)
    {
        // RFC 1122, page 85, Section 4.2.2.6  Maximum Segment Size Option: RFC-793 Section 3.1
        // more explanations in RFC-6691
        tcpMss = 536;
        // parse the option only for SYN segments
        if(tcpHeader.flags & TCP_FIN_FLAG)
        {
            // Parse for the TCP MSS option, if present.
            while(tcpOptionsSize--)
            {
                IP_READ_BYTE_IN(opt, pkt_ptr);
                switch(opt)
                {
                    case TCP_EOP:
                        // End of options.
                        if(tcpOptionsSize)
                        {
                            // dump remaining unused bytes
                            pkt_ptr += tcpOptionsSize;
                            tcpOptionsSize = 0;
                        }
                        ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
                        break;
                    case TCP_NOP:
                        // NOP option.
                        break;
                    case TCP_MSS:
                        if(tcpOptionsSize >= 3) // at least 3 more bytes
                        {
                            IP_READ_BYTE_IN(opt, pkt_ptr);
                            if(opt == 0x04)
                            {
                                // An MSS option with the right option length.
                                IP_READ_WORD_IN(tcpMss, pkt_ptr); // value returned in host endianess
                                // Advance to the next option
                                tcpOptionsSize = (uint16_t)(tcpOptionsSize - 3);

                                // Limit the mss to the configured TCP_MAX_SEG_SIZE
                                if (tcpMss > TCP_MAX_SEG_SIZE)
                                {
                                    tcpMss = TCP_MAX_SEG_SIZE;
                                }
                                // so far so good
                                ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
                            }
                            else
                            {
                                // Bad option size length
                                logMsg("tcp_parseopt: bad option size length",LOG_INFO, LOG_DEST_CONSOLE);
                                // unexpected error
                                tcpOptionsSize = 0;
                                ret = NET_ERROR;     //jira: CAE_MCU8-5647
                            }
                        }
                        else
                        {
                            // unexpected error
                            tcpOptionsSize = 0;
                            ret = NET_ERROR;     //jira: CAE_MCU8-5647
                        }
                        break;
                    default:
                        logMsg("tcp_parseopt: other",LOG_INFO, LOG_DEST_CONSOLE);
                        IP_READ_BYTE_IN(opt, pkt_ptr);
                        tcpOptionsSize--;

                        if (opt > 1) // this should be at least 2 to be valid
                        {
                            // adjust for the remaining bytes for the current option
                            opt = (uint8_t)(opt - 2u);    //jira: CAE_MCU8-5647
                            if (opt <= tcpOptionsSize)
                            {
                                // All other options have a length field, so that we easily can skip them.
                                pkt_ptr += opt;
                                tcpOptionsSize = (uint16_t)(tcpOptionsSize - opt);
                                ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
                            }
                            else
                            {
                                logMsg("tcp_parseopt: bad option length",LOG_INFO, LOG_DEST_CONSOLE);
                                // the options are malformed and we don't process them further.
                                tcpOptionsSize = 0;
                                ret = NET_ERROR;     //jira: CAE_MCU8-5647
                            }
                        }
                        else
                        {
                            logMsg("tcp_parseopt: bad length",LOG_INFO, LOG_DEST_CONSOLE);
                            // If the length field is zero, the options are malformed
                            // and we don't process them further.
                            tcpOptionsSize = 0;
                            ret = NET_ERROR;     //jira: CAE_MCU8-5647
                        }
                        break;
                }
            }
        }
        else // jump over the Options from TCP header
        {
            pkt_ptr += tcpOptionsSize;
            ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
        }
    }
    else
    {
        ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
    }

    *pkt_read = pkt_ptr;

    return ret;
}

/** This function will be called by the IP layer for each received TCP packet.
 * It will identify the destination socket and also parse the TCP header.
 * 
 * @param remoteAddress
 *      Source IP address for the received TCP packet
 * 
 * @param length
 *      Length of the TCP payload
 * 
 * @return
 *      None
 */
void TCP_Recv(uint8_t *packet, uint8_t **pkt_read, uint32_t remoteAddress, uint16_t length)
{
    tcpTCB_t *tcbPtr;
    socklistsize_t count = 0;
    uint8_t *pkt_ptr = *pkt_read;

    tcbPtr = NULL;
    //make sure we will not reuse old values
    receivedRemoteAddress = 0;
    rcvPayloadLen = 0;

    IP_MOVE_BYTES_IN(tcpHeader, pkt_ptr, sizeof(tcpHeader_p_t));

    currentTCB = NULL;

    // quick check on destination port
    if ((tcpHeader.destPort != 0) && (tcpHeader.sourcePort != 0))
    {
        tcpHeader.sourcePort = ntohs(tcpHeader.sourcePort);
        tcpHeader.destPort = ntohs(tcpHeader.destPort);
        
        // search for active TCB
        tcbPtr = tcbList;
        while((tcbPtr != NULL) && (count < tcbListSize))
        {
            if (tcpHeader.destPort == tcbPtr->localPort)
            {
                currentTCB = tcbPtr;
                break;
            }
            else
            {
                tcbPtr = tcbPtr->nextTCB;
                count ++;
            }
        }

        if (currentTCB != NULL)
        {
            if((tcpHeader.sourcePort == currentTCB->destPort) ||
               (currentTCB->destIP == 0))
            {
                // we will need this if the port is in listen mode
                // or to check for the correct TCB
                receivedRemoteAddress = remoteAddress;
                rcvPayloadLen = (uint16_t)(length - (uint16_t)((tcpHeader.dataOff & 0xF0) >> 2));
                rcvPayloadAddr = pkt_ptr;

                // check/skip the TCP header options
                if (TCP_ParseTCPOptions(packet, &pkt_ptr) == NET_SUCCESS)
                {
                    // we got a packet
                    // sort out the events
                    if(tcpHeader.flags & TCP_SYN_FLAG)
                    {
                        if(tcpHeader.flags & TCP_ACK_FLAG)
                        {
                            logMsg("found syn&ack",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_SYNACK;
                        } else
                        {
                            logMsg("found syn",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_SYN;
                        }
                    } else if(tcpHeader.flags & TCP_FIN_FLAG)
                    {
                        if(tcpHeader.flags & TCP_ACK_FLAG)
                        {
                            logMsg("found fin&ack",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_FINACK;
                        } else
                        {
                            logMsg("found fin",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_FIN;
                        }
                    } else if(tcpHeader.flags & TCP_RST_FLAG)
                    {
                        if(tcpHeader.flags & TCP_ACK_FLAG)
                        {
                            logMsg("found rst&ack",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_RSTACK;
                        } else
                        {
                            logMsg("found rst",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->connectionEvent = RCV_RST;
                        }
                    } else if(tcpHeader.flags & TCP_ACK_FLAG)
                    {
                        logMsg("found ack",LOG_INFO, LOG_DEST_CONSOLE);
                        currentTCB->connectionEvent = RCV_ACK;
                    }
                    else
                    {
                        logMsg("confused",LOG_INFO, LOG_DEST_CONSOLE);
                    }
                    // convert it here to save some cycles later
                    tcpHeader.ackNumber = ntohl(tcpHeader.ackNumber);
                    tcpHeader.sequenceNumber = ntohl(tcpHeader.sequenceNumber);

                    TCP_FiniteStateMachine();
                }else
                {
                    logMsg("pkt dropped: bad options",LOG_INFO, LOG_DEST_CONSOLE);
                }
            } // we will not send a reset message for PORT not open
        }
    }
}


/** This function is TCP stack state machine.
 * 
 *  The function is called each time an event (opening/closing a connection,
 *  receiving TCP packet, etc) occurs.
 * 
 * @param 
 *      None
 * 
 * @return
 *      None
 */
static error_msg TCP_FiniteStateMachine(void)  //jira: CAE_MCU8-5647
{
    uint16_t notAckBytes;
    error_msg ret = NET_ERROR;  //jira: CAE_MCU8-5647

    tcp_fsm_states_t nextState = currentTCB->fsmState; // default don't change states
    tcpEvent_t event = currentTCB->connectionEvent;
    if(isPortUnreachable(currentTCB->localPort))
    {
        event = RCV_RST;
        resetPortUnreachable();
    }
    switch (currentTCB->fsmState)
    {
        case LISTEN:
            switch (event)
            {
                case RCV_SYN:
                    logMsg("LISTEN: rx_syn",LOG_INFO, LOG_DEST_CONSOLE);
                    // Start the connection on the TCB

                    currentTCB->destIP = receivedRemoteAddress;
                    currentTCB->destPort = tcpHeader.sourcePort;

                    // TO DO modify the start seq no to be "random"
                    currentTCB->localLastAck = 0;

                    currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;
                    currentTCB->remoteAck = currentTCB->remoteSeqno + 1; // ask for next packet

                    // save data from TCP header
                    currentTCB->remoteWnd = ntohs(tcpHeader.windowSize);
                    currentTCB->mss = tcpMss;

                    // create and send a SYN+ACK packet
                    currentTCB->flags =   TCP_SYN_FLAG | TCP_ACK_FLAG;
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = TCP_MAX_SYN_RETRIES;

                    TCP_Snd(currentTCB);
                    nextState = SYN_RECEIVED;
                    break;
                case CLOSE:
                    logMsg("LISTEN: close",LOG_INFO, LOG_DEST_CONSOLE);
                    nextState = CLOSED;
                    TCB_Reset(currentTCB);
                    break;
                default:
                    // for all other cases the packet is invalid and will be discarded
                    break;
            }
            break;
        case SYN_SENT:
            switch (event)
            {
                case RCV_SYN:
                    logMsg("SYN_SENT: rx_syn",LOG_INFO, LOG_DEST_CONSOLE);
                    // Simultaneous open
                    currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;
                    currentTCB->remoteAck = tcpHeader.sequenceNumber + 1; //ask for next packet

                    // save data from TCP header
                    currentTCB->remoteWnd = ntohs(tcpHeader.windowSize);
                    currentTCB->mss = tcpMss;

                    // create and send a ACK packet
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = TCP_MAX_SYN_RETRIES;
                    currentTCB->flags = TCP_SYN_FLAG | TCP_ACK_FLAG;

                    TCP_Snd(currentTCB);

                    //Go to SYN_RECEIVED and waiting for the ack
                    nextState = SYN_RECEIVED;
                    break;
                case RCV_SYNACK:
                    logMsg("SYN_SENT: rx_synack",LOG_INFO, LOG_DEST_CONSOLE);

                    currentTCB->timeout = 0;

                    if ((currentTCB->localSeqno + 1) == tcpHeader.ackNumber)
                    {
                        // create and send a ACK packet
                        currentTCB->localSeqno = currentTCB->localSeqno + 1;
                        currentTCB->flags =  TCP_ACK_FLAG;

                        // save data from TCP header
                        currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;
                        // ask for next packet
                        currentTCB->remoteAck = tcpHeader.sequenceNumber + 1;

                        currentTCB->remoteWnd = ntohs(tcpHeader.windowSize);
                        currentTCB->mss = tcpMss;

                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = ESTABLISHED;
                            currentTCB->socketState = SOCKET_CONNECTED;
                        }
                    }
                    else
                    {
                        //send reset
                        currentTCB->localSeqno = tcpHeader.ackNumber;
                        currentTCB->flags =  TCP_RST_FLAG | TCP_ACK_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))  //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break;
                case RCV_ACK:
                    logMsg("SYN_SENT: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);

                    currentTCB->timeout = 0;

                    if ((currentTCB->localSeqno + 1) == tcpHeader.ackNumber)
                    {
                        // create and send a ACK packet
                        currentTCB->localSeqno = currentTCB->localSeqno + 1;
                        currentTCB->flags =  TCP_ACK_FLAG;

                        // save data from TCP header
                        currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;
                        currentTCB->remoteAck = tcpHeader.sequenceNumber + 1; // ask for next packet

                        currentTCB->remoteWnd = ntohs(tcpHeader.windowSize);
                        currentTCB->mss = tcpMss;

                        nextState = ESTABLISHED;
                        currentTCB->socketState = SOCKET_CONNECTED;
                    }
                    else
                    {
                        //send reset
                        currentTCB->localSeqno = tcpHeader.ackNumber;
                        currentTCB->flags =  TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))    //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break;
                case CLOSE:
                    logMsg("SYN_SENT: close",LOG_INFO, LOG_DEST_CONSOLE);
                    //go to CLOSED state
                    nextState = CLOSED;
                    TCB_Reset(currentTCB);
                    break;
                case TIMEOUT:
                    logMsg("SYN_SENT: timeout",LOG_INFO, LOG_DEST_CONSOLE);
                    // looks like the the packet was lost
                    // check inside the packet to see where to jump next
                    if (currentTCB->timeoutsCount)
                    {
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            if (currentTCB->flags & TCP_RST_FLAG)
                            {
                                nextState = CLOSED;
                                TCB_Reset(currentTCB);
                            }else
                            if(currentTCB->flags & TCP_ACK_FLAG)
                            {
                                nextState = ESTABLISHED;
                                currentTCB->socketState = SOCKET_CONNECTED;
                            }                        
                        }
                    }else
                    {
                        // just reset the connection if there is no reply
                        currentTCB->flags = TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))  //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    } 
                    break;
                case RCV_RST:
                case RCV_RSTACK:
                    // port seems not to be opened
                    nextState = CLOSED;
                    TCB_Reset(currentTCB);
                    break;
                default:
                    // for all other cases the packet is invalid and will be discarded
                    break;
            }
            break;
        case SYN_RECEIVED:
            switch (event)
            {
                case RCV_SYNACK:
                    logMsg("SYN_RECEIVED: rx_synack",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->localPort == tcpHeader.destPort)
                    {
                        // stop the current timeout
                        currentTCB->timeout = 0;

                        // This is part of simultaneous open
                        // TO DO: Check if the received packet is the one that we expect
                        if ((currentTCB->destIP == receivedRemoteAddress) && (currentTCB->destPort == tcpHeader.sourcePort))
                        if ((currentTCB->localSeqno + 1) == tcpHeader.ackNumber)

                        nextState = ESTABLISHED;
                        currentTCB->socketState = SOCKET_CONNECTED;
                    }
                    break;
                case RCV_ACK:
                    logMsg("SYN_RECEIVED: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);

                    // check if the packet is for the curent TCB
                    // we need to check the remote IP adress and remote port
                    if ((currentTCB->destIP == receivedRemoteAddress) && (currentTCB->destPort == tcpHeader.sourcePort))
                    {
                        // check the sequence numbers
                        // is it the packet that I've ask for?
                        if (currentTCB->remoteAck == tcpHeader.sequenceNumber)
                        {
                            // is ACK OK?
                            if ((currentTCB->localSeqno + 1) == tcpHeader.ackNumber)
                            {
                                currentTCB->localSeqno = currentTCB->localSeqno + 1;
                                // stop the current timeout
                                currentTCB->timeout = 0;
                                
                                nextState = ESTABLISHED;
                                currentTCB->socketState = SOCKET_CONNECTED;
                            }
                        }
                    }
                    break;
                case CLOSE:
                    logMsg("SYN_RECEIVED: close",LOG_INFO, LOG_DEST_CONSOLE);
                    // stop the current timeout
                    currentTCB->timeout = 0;
                    // Need to send FIN and go to the FIN_WAIT_1
                    currentTCB->flags = TCP_FIN_FLAG;
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = TCP_MAX_RETRIES;
                    
                    nextState = FIN_WAIT_1;
                    TCP_Snd(currentTCB);
                    break;
                case RCV_RSTACK:
                case RCV_RST:
                    // Reset the connection
                    logMsg("SYN_RECEIVED:  rx_rst",LOG_INFO, LOG_DEST_CONSOLE);
                    //check if the local port match; else drop the pachet
                    if (currentTCB->localPort == tcpHeader.destPort)
                    {
                        if (currentTCB->remoteAck ==  tcpHeader.sequenceNumber)
                        {
                            logMsg("rst seq OK",LOG_INFO, LOG_DEST_CONSOLE);
                            currentTCB->destIP = 0;
                            currentTCB->destPort = 0;
                            currentTCB->localSeqno = 0;
                            currentTCB->localLastAck = 0;
                            currentTCB->remoteSeqno = 0;
                            currentTCB->remoteAck = 0;
                            currentTCB->remoteWnd = 0;
                            //TCP_MAX_SEG_SIZE instead of 0
                            currentTCB->mss = TCP_MAX_SEG_SIZE;
                            
                            nextState = LISTEN;
                        }
                    }
                    break;
                case TIMEOUT:
                    logMsg("SYN_RECEIVED:  timeout",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->timeoutsCount)
                    {
                        TCP_Snd(currentTCB);
                    }
                    else
                    {
                        //reseting the connection if there is no reply
                        currentTCB->flags =   TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))  //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            currentTCB->destIP = 0;
                            currentTCB->destPort = 0;
                            currentTCB->localSeqno = 0;
                            currentTCB->localLastAck = 0;
                            currentTCB->remoteSeqno = 0;
                            currentTCB->remoteAck = 0;
                            currentTCB->remoteWnd = 0;
                            //TCP_MAX_SEG_SIZE instead of 0
                            currentTCB->mss = TCP_MAX_SEG_SIZE;
                            nextState = LISTEN;
                        }
                    }
                    break;
                default:
                    // invalid packet so drop it
                    break;
            }
            break;
        case ESTABLISHED:
            switch (event)
            {
                case RCV_ACK:
                    logMsg("ESTABLISHED: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->destIP == receivedRemoteAddress)
                    {
                        // is sequence number OK?
                        // remote ACK should be equal to header sequence number
                        // we don't accept out of order packet (not enough memory)
                        if (currentTCB->remoteAck == tcpHeader.sequenceNumber)
                        {
                            // This is a ACK packet only
                            // check the ACK sequence
                            // check if this is on already received Ack
                            if (currentTCB->localLastAck < tcpHeader.ackNumber)
                            {
                                // check how many bytes sent was acknowledged
                                if ((currentTCB->localSeqno + 1) >= tcpHeader.ackNumber)
                                {
                                    notAckBytes = (uint16_t)(currentTCB->localSeqno - tcpHeader.ackNumber);

                                    // update the pointer for next TX
                                    currentTCB->txBufferPtr = currentTCB->txBufferPtr - notAckBytes;
                                    currentTCB->bytesToSend = (uint16_t)(currentTCB->bytesToSend  + notAckBytes);
                                    
                                    currentTCB->localLastAck = tcpHeader.ackNumber - 1;
                                    currentTCB->localSeqno = tcpHeader.ackNumber;
                                    if(bytesToSendForRetransmit == 0)
                                    {
                                        localSeqnoForRetransmit = currentTCB->localSeqno;
                                    }
                                    // Check if all TX buffer/data was acknowledged
                                    if(currentTCB->bytesToSend == 0) 
                                    {
                                        if (currentTCB->txBufState == TX_BUFF_IN_USE)
                                        {
                                            currentTCB->txBufState = NO_BUFF;
                                            //stop timeout
                                            currentTCB->timeout = 0;
                                        }
                                    }                                    
                                    else
                                    {       
                                        if(bytesToSendForRetransmit)	//jira: CAE_MCU8-6056
                                        {
                                            currentTCB->txBufferPtr = txBufferPtrForRetransmit;
                                            currentTCB->bytesSent = bytesToSendForRetransmit;
                                            currentTCB->localSeqno = localSeqnoForRetransmit;
                                        }
                                        else
                                        {
                                            currentTCB->bytesSent = currentTCB->bytesToSend;
                                        }
                                        currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                                        currentTCB->timeoutsCount = TCP_MAX_RETRIES;
                                        TCP_Snd(currentTCB);
                                        if( (bytesToSendForRetransmit>0) && (lastAckNumber != tcpHeader.ackNumber) )	//jira: CAE_MCU8-6056
                                        {
                                            bytesToSendForRetransmit =0;
                                        }
                                    }

                                    
                                    // check if the packet has payload
                                    if(rcvPayloadLen > 0)
                                    {
                                        currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;

                                        // copy the payload to the local buffer
                                        TCP_PayloadSave(rcvPayloadAddr, rcvPayloadLen);
                                    }
                                }else
                                {
                                    // this is a wrong Ack
                                    // ACK a packet that wasn't transmitted
                                }
                            }
                        }
                    }
                    break;
                case CLOSE:
                    logMsg("ESTABLISHED: close",LOG_INFO, LOG_DEST_CONSOLE);
                    currentTCB->flags = TCP_FIN_FLAG | TCP_ACK_FLAG ;	//jira: M8TS-514, M8TS-538, M8TS-463
                    nextState = FIN_WAIT_1;
                    currentTCB->timeout = 0;
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = TCP_MAX_RETRIES;
                    TCP_Snd(currentTCB);
                    break;
                case RCV_FIN:
                    logMsg("ESTABLISHED: rx_fin",LOG_INFO, LOG_DEST_CONSOLE);
                    break;
                case RCV_FINACK:
                    if (currentTCB->destIP == receivedRemoteAddress)        //jira: CAE_MCU8-5830
                    {
                        // is sequence number OK?
                        // remote ACK should be equal to header sequence number
                        // we don't accept out of order packet (not enough memory)
                        if (currentTCB->remoteAck == tcpHeader.sequenceNumber)    //jira: CAE_MCU8-5830
                        {
                            currentTCB->bytesSent = 0;                   
                            // ACK the current packet
                            // TO DO  check if it's a valid packet
                            currentTCB->localSeqno = tcpHeader.ackNumber;
                            currentTCB->remoteAck = currentTCB->remoteAck + 1;

                            // check if the packet has payload added for MCCV3xx-6762
                            if(rcvPayloadLen > 0)	 //jira: CAE_MCU8-6056
                            {
                                currentTCB->remoteSeqno =  tcpHeader.sequenceNumber;

                                // copy the payload to the local buffer
                                TCP_PayloadSave(rcvPayloadAddr, rcvPayloadLen);
                            }

                            currentTCB->socketState = SOCKET_CLOSING;
                            currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                            currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                            currentTCB->timeoutsCount = TCP_MAX_RETRIES;
                            // JUMP over CLOSE_WAIT state and send one packet with FIN + ACK
                            currentTCB->flags =  TCP_FIN_FLAG | TCP_ACK_FLAG;

                            nextState = LAST_ACK;
                            TCP_Snd(currentTCB);
                        }
                    }
                    break;
                case RCV_RST:
                case RCV_RSTACK:                     
                    currentTCB->flags = TCP_RST_FLAG;
                    TCP_Snd(currentTCB);
                    nextState = CLOSED;
                    TCB_Reset(currentTCB);
                    break;
                case TIMEOUT:
                    logMsg("ESTABLISHED:  timeout",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->timeoutsCount)
                    {
                        TCP_TimoutRetransmit();	//jira: CAE_MCU8-6056
                    }else
                    {
                        // reset the connection if there is no reply
                        currentTCB->flags =   TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))    //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case FIN_WAIT_1:
            switch (event)
            {
                case RCV_FIN:
                    currentTCB->flags =  TCP_ACK_FLAG;
                    if (currentTCB->remoteAck == tcpHeader.sequenceNumber)	//jira: M8TS-514, M8TS-538, M8TS-463	
                    {
                        currentTCB->bytesSent = 0;                   
                        currentTCB->localSeqno = currentTCB->localSeqno + 1;
                        currentTCB->remoteAck = currentTCB->remoteAck + 1;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647
                       {
                           nextState = CLOSING;
                       }
                    }     
                    break;
                case RCV_ACK:
                    logMsg("FIN_WAIT_1: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);
                    // stop the current timeout
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = 1;
                    nextState = FIN_WAIT_2;
                    break;
                case RCV_FINACK:
                    logMsg("FIN_WAIT_1: rx_finack",LOG_INFO, LOG_DEST_CONSOLE);
                    currentTCB->flags =  TCP_ACK_FLAG;
                    if (currentTCB->remoteAck == tcpHeader.sequenceNumber)	//jira: M8TS-514, M8TS-538, M8TS-463
                    {
                        currentTCB->bytesSent = 0;                   
                        currentTCB->localSeqno = currentTCB->localSeqno + 1;
                        currentTCB->remoteAck = currentTCB->remoteAck + 1;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647
                        {
                            nextState = TIME_WAIT;
                        }
                    }
                    break;
                case TIMEOUT:
                    logMsg("FIN_WAIT_1:  timeout",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->timeoutsCount)
                    {
                        TCP_Snd(currentTCB);
                    }else
                    {
                        // just reset the connection if there is no reply
                        currentTCB->flags =   TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))  //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break;                    
                default:
                    break;
            }
            break;
        case FIN_WAIT_2:
            switch (event)
            {
                case RCV_FINACK:					 		//jira: M8TS-514, M8TS-538, M8TS-463                   
                case RCV_FIN:
                    logMsg("FIN_WAIT_2: rx_fin/rx_finack",LOG_INFO, LOG_DEST_CONSOLE);
                    currentTCB->flags =  TCP_ACK_FLAG;					//jira: M8TS-514, M8TS-538, M8TS-463
                    if (currentTCB->remoteAck == tcpHeader.sequenceNumber)
                    {
                        currentTCB->bytesSent = 0;                   
                        currentTCB->localSeqno = currentTCB->localSeqno + 1;
                        currentTCB->remoteAck = currentTCB->remoteAck + 1;     
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647
                        {
                            nextState = TIME_WAIT;
                        }   
                    }
                                 
                    break;
                case TIMEOUT:
                    logMsg("FIN_WAIT_2:  timeout",LOG_INFO, LOG_DEST_CONSOLE);
                    if (currentTCB->timeoutsCount)
                    {
                        TCP_Snd(currentTCB);
                    }else
                    {
                        // just reset the connection if there is no reply
                        currentTCB->flags =   TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))   //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break; 
                default:
                    break;
            }
            break;
        case CLOSE_WAIT:
            // This state is defined in RFC, but is not used in the application
            break;
        case CLOSING:
            switch (event)
            {
                case RCV_ACK:
                    logMsg("CLOSING: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);
                    nextState = TIME_WAIT;
                    break;
                default:
                    break;
            }
            break;
        case LAST_ACK:
            // check if the packet belongs to the curent TCB
            switch (event)
            {
                case RCV_FINACK:
                case RCV_ACK:
                    if ((currentTCB->destIP == receivedRemoteAddress) &&
                        (currentTCB->destPort == tcpHeader.sourcePort))
                    {
                        logMsg("LAST_ACK: rx_ack",LOG_INFO, LOG_DEST_CONSOLE);
                        nextState = CLOSED;
                        TCB_Reset(currentTCB);
                    }
                    break;
                case TIMEOUT:
                    if (currentTCB->timeoutsCount)
                    {
                        TCP_Snd(currentTCB);
                    }
                    else
                    {
                        // just reset the connection if there is no reply
                        currentTCB->flags =   TCP_RST_FLAG;
                        if(TCP_Snd(currentTCB) == (TX_QUEUED | NET_SUCCESS))    //jira: CAE_MCU8-5647, CAE_MCU8-6056
                        {
                            nextState = CLOSED;
                            TCB_Reset(currentTCB);
                        }
                    }
                    break;

                default:
                    break;
            }
            break;
        case TIME_WAIT:
            logMsg("Time Wait",LOG_INFO, LOG_DEST_CONSOLE);
            nextState = CLOSED;
            TCB_Reset(currentTCB);
            break;
        case CLOSED:
            switch (event)
            {
                case ACTIVE_OPEN:
                    logMsg("CLOSED: active_open",LOG_INFO, LOG_DEST_CONSOLE);
                    // create and send a SYN packet
                    currentTCB->timeout = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                    currentTCB->timeoutsCount = TCP_MAX_SYN_RETRIES;
                    currentTCB->flags = TCP_SYN_FLAG;
                    TCP_Snd(currentTCB);
                    nextState = SYN_SENT;
                    ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
                    break;
                case PASIVE_OPEN:
                    logMsg("CLOSED: passive_open",LOG_INFO, LOG_DEST_CONSOLE);
                    currentTCB->destIP = 0;
                    currentTCB->destPort = 0;
                    nextState = LISTEN;
                    ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    currentTCB->connectionEvent = NOP; // we are handling the event...
    currentTCB->fsmState = nextState;
    return ret;
}


void TCP_Init(void)
{
    tcbList = NULL;
    tcbListSize = 0;
    nextAvailablePort = LOCAL_TCP_PORT_START_NUMBER;
    nextSequenceNumber = 0;
}

error_msg TCP_SocketInit(tcpTCB_t *tcbPtr)   //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;     //jira: CAE_MCU8-5647
    
    // verify that this socket is not in the list
    if(TCB_Check(tcbPtr) == NET_ERROR)    //jira: CAE_MCU8-5647
    {
        TCB_Reset(tcbPtr);

        tcbPtr->localWnd = 0; // here we should put the RX buffer size
        tcbPtr->mss = TCP_MAX_SEG_SIZE;
        tcbPtr->fsmState = CLOSED;
        tcbPtr->connectionEvent = NOP;
        tcbPtr->rxBufferStart = NULL;
        tcbPtr->rxBufState = NO_BUFF;
        tcbPtr->txBufferStart = NULL;
        tcbPtr->txBufferPtr = NULL;
        tcbPtr->bytesToSend = 0;
        tcbPtr->bytesSent = 0;
        tcbPtr->payloadSave = false;
        tcbPtr->txBufState = NO_BUFF;
        tcbPtr->socketState = SOCKET_CLOSED;

        TCB_Insert(tcbPtr);
        ret = NET_SUCCESS;   //jira: CAE_MCU8-5647
    }
    return ret;
}

error_msg TCP_SocketRemove(tcpTCB_t *tcbPtr)    //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;   //jira: CAE_MCU8-5647
    
    // verify that this socket is in the Closed State
    if(TCP_SocketPoll(tcbPtr) == SOCKET_CLOSING)
    {
        TCB_Remove(tcbPtr);
        ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
    }
    return ret;
}

socketState_t TCP_SocketPoll(tcpTCB_t *socket_ptr)
{
    socketState_t tmpSocketState;
   
    tmpSocketState = NOT_A_SOCKET;
    
    if (TCB_Check(socket_ptr) == NET_SUCCESS)    //jira: CAE_MCU8-5647
    {
        tmpSocketState = socket_ptr->socketState;
    }

    return tmpSocketState;
}


error_msg TCP_Bind(tcpTCB_t *tcbPtr, uint16_t port)    //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;     //jira: CAE_MCU8-5647

    logMsg("tcp_bind",LOG_INFO, LOG_DEST_CONSOLE);

    if (TCB_Check(tcbPtr) == NET_SUCCESS)    //jira: CAE_MCU8-5647
    {
        tcbPtr->localPort = port;
        ret = NET_SUCCESS;   //jira: CAE_MCU8-5647
    }
    return ret;
}


error_msg TCP_Listen(tcpTCB_t *tcbPtr)    //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;    //jira: CAE_MCU8-5647

    logMsg("tcp_listen",LOG_INFO, LOG_DEST_CONSOLE);

    if (TCB_Check(tcbPtr) == NET_SUCCESS)    //jira: CAE_MCU8-5647
    {
        tcbPtr->connectionEvent = PASIVE_OPEN;
        tcbPtr->socketState = SOCKET_IN_PROGRESS;
        tcbPtr->localSeqno = nextSequenceNumber;
        currentTCB = tcbPtr;
        if (tcbPtr->localPort == 0)
        {
            tcbPtr->localPort = nextAvailablePort++;
        }
        ret = TCP_FiniteStateMachine();
    }
    return ret;
}


error_msg TCP_Connect(tcpTCB_t *tcbPtr, sockaddr_in4_t *srvaddr)   //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;     //jira: CAE_MCU8-5647

    if (TCP_SocketPoll(tcbPtr) == SOCKET_CLOSED)
    {
        tcbPtr->destIP = srvaddr->addr.s_addr;
        tcbPtr->destPort = srvaddr->port;
        if(tcbPtr->localPort == 0)
        {
            // use a "random" port for the local one
            tcbPtr->localPort = nextAvailablePort++;
        }

        tcbPtr->fsmState = CLOSED;
        tcbPtr->socketState = SOCKET_IN_PROGRESS;
        tcbPtr->localSeqno = nextSequenceNumber;
        tcbPtr->connectionEvent = ACTIVE_OPEN;

        currentTCB = tcbPtr;
        ret = TCP_FiniteStateMachine();
    }

    return ret;
}


error_msg TCP_Close(tcpTCB_t *tcbPtr)   //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;    //jira: CAE_MCU8-5647

    logMsg("tcp_close",LOG_INFO, LOG_DEST_CONSOLE);

    if (TCB_Check(tcbPtr) == NET_SUCCESS)    //jira: CAE_MCU8-5647
    {
        tcbPtr->connectionEvent = CLOSE;
        

        tcbPtr->txBufState = NO_BUFF;
        tcbPtr->rxBufState = NO_BUFF;
        tcbPtr->txBufferPtr = NULL;
        tcbPtr->txBufferStart = NULL;
        tcbPtr->rxBufferPtr = NULL;
        tcbPtr->rxBufferStart = NULL;
        tcbPtr->bytesToSend = 0;
        tcbPtr->bytesSent = 0;
        tcbPtr->payloadSave = false;

        // likely to change this to a needs TX time queue
        currentTCB = tcbPtr;
        ret = TCP_FiniteStateMachine();
    }
    return ret;
}


error_msg TCP_Send(tcpTCB_t *tcbPtr, uint8_t *data, uint16_t dataLen)    //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;    //jira: CAE_MCU8-5647

    if (TCP_SocketPoll(tcbPtr) == SOCKET_CONNECTED)
    {
        if (tcbPtr->txBufState == NO_BUFF)
        {
            if (data != NULL)
            {
                tcbPtr->txBufferStart = data;
                tcbPtr->txBufferPtr = tcbPtr->txBufferStart;
                tcbPtr->bytesToSend = dataLen;
                tcbPtr->txBufState = TX_BUFF_IN_USE;
                tcbPtr->bytesSent = dataLen;

                tcbPtr->timeout = TCP_START_TIMEOUT_VAL; 
                tcbPtr->timeoutReloadValue = TCP_START_TIMEOUT_VAL;
                tcbPtr->timeoutsCount = TCP_MAX_RETRIES;

                tcbPtr->flags = TCP_ACK_FLAG;

                TCP_Snd(tcbPtr);
                ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
            }
        }
    }
    return ret;
}


error_msg TCP_SendDone(tcpTCB_t *tcbPtr)    //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;      //jira: CAE_MCU8-5647

    if(TCB_Check(tcbPtr) == NET_SUCCESS)     //jira: CAE_MCU8-5647
    {
        if (tcbPtr->txBufState == NO_BUFF)
        {
            ret = NET_SUCCESS;   //jira: CAE_MCU8-5647
        }
    }
    return ret;
}

error_msg TCP_InsertRxBuffer(tcpTCB_t *tcbPtr, uint8_t *data, uint16_t data_len)     //jira: CAE_MCU8-5647
{
    error_msg ret = NET_ERROR;     //jira: CAE_MCU8-5647

    if (TCB_Check(tcbPtr) == NET_SUCCESS)     //jira: CAE_MCU8-5647
    {
        if (tcbPtr->rxBufState == NO_BUFF)
        {
            if (data != NULL)
            {
                tcbPtr->rxBufferStart = data;
                tcbPtr->rxBufferPtr = tcbPtr->rxBufferStart;
                tcbPtr->localWnd = data_len;  // update the available receive windows
                tcbPtr->rxBufState = RX_BUFF_IN_USE;
                ret = NET_SUCCESS;    //jira: CAE_MCU8-5647
            }
        }
    }
    return ret;
}


int16_t TCP_GetReceivedData(tcpTCB_t *tcbPtr)
{
    int16_t ret = 0;

    if (TCB_Check(tcbPtr) == NET_SUCCESS)     //jira: CAE_MCU8-5647
    {
        if (tcbPtr->rxBufState == RX_BUFF_IN_USE)
        {
            ret = (int16_t)(tcbPtr->rxBufferPtr - tcbPtr->rxBufferStart);

            if (ret != 0)
            {
                tcbPtr->localWnd = 0;
                tcbPtr->rxBufState = NO_BUFF;
            }
        }
    }
    return ret;
}

int16_t TCP_GetRxLength(tcpTCB_t *tcbPtr)
{
    int16_t ret = 0;

    if (TCB_Check(tcbPtr) == NET_SUCCESS)     //jira: CAE_MCU8-5647
    {
        if (tcbPtr->rxBufState == RX_BUFF_IN_USE)
        {
            ret = (int16_t)(tcbPtr->rxBufferPtr - tcbPtr->rxBufferStart);
        }
    }
    return ret;
}

void TCP_Update(void)
{
    tcpTCB_t *tcbPtr;
    tcbPtr = NULL;
    int count = 0;

    // update sequence number and local port number in order to be different
    // for each new connection
    nextSequenceNumber++;

    // keep local port number in the general port range
    nextAvailablePort = (uint16_t)(nextAvailablePort + 1);
    if (nextAvailablePort < LOCAL_TCP_PORT_START_NUMBER)
    {
        nextAvailablePort = LOCAL_TCP_PORT_START_NUMBER;
    }
    //TO DO also local seq number should be "random"

    tcbPtr = tcbList;
    while((tcbPtr != NULL) && (count < tcbListSize))
    {
        if (tcbPtr->timeout > 0)
        {
            logMsg("tcp timeout",LOG_INFO, LOG_DEST_CONSOLE);
            tcbPtr->timeout = (uint16_t)(tcbPtr->timeout - 1);

            if (tcbPtr->timeout == 0)
            {
                // MAKE sure we don't overwrite anything else
                if (tcbPtr->connectionEvent == NOP)
                {
                    int retries = (int)TCP_MAX_RETRIES - (int)tcbPtr->timeoutsCount; // Jira: CAE_MCU8-5772
                    if(retries < 0)
                    {
                        retries = 0;
                    }
                    tcbPtr->timeout = (uint16_t)(tcbPtr->timeoutReloadValue << retries);
                    //if not zero
                    if (tcbPtr->timeoutsCount != 0)
                    {
                        tcbPtr->timeoutsCount = (uint8_t)(tcbPtr->timeoutsCount - 1u);  //jira: CAE_MCU8-5647
                    }
                    tcbPtr->connectionEvent = TIMEOUT;
                    currentTCB = tcbPtr;
                    TCP_FiniteStateMachine();
                }
            }
        }
        tcbPtr = tcbPtr->nextTCB;
        count ++;
    }
}

static error_msg TCP_TimoutRetransmit(void)	//jira: CAE_MCU8-6056
{
    currentTCB->txBufferPtr -= tcpDataLength;
    txBufferPtrForRetransmit = currentTCB->txBufferPtr;
    bytesToSendForRetransmit = tcpDataLength;
    currentTCB->localSeqno = localSeqnoForRetransmit;
    lastAckNumber = tcpHeader.ackNumber;
    return TCP_Snd(currentTCB);
}
