/**
  Transmission Control Protocol (TCP) v4 Header File
	
  @Company:
    Microchip Technology Inc.

  @File Name:
    tcpv4.h

  @Summary:
    Header file for the TCPv4.c

  @Description:
    This header file provides the API for the TCPv4 protocol.

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

#ifndef TCPV4_H
#define	TCPV4_H

/**
  Section: Included Files
*/
#include <stdbool.h>
#include "tcpip_types.h"

#define TCP_FIN_FLAG 0x01U
#define TCP_SYN_FLAG 0x02U
#define TCP_RST_FLAG 0x04U
#define TCP_PSH_FLAG 0x08U
#define TCP_ACK_FLAG 0x10U
#define TCP_URG_FLAG 0x20U
#define TCP_ECE_FLAG 0x40U
#define TCP_CWR_FLAG 0x80U

/**
  Section: Enumeration Definition
*/

typedef enum
{
     CLOSED = 0,    // make it easy for initialization
     LISTEN, SYN_SENT, SYN_RECEIVED, ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT, CLOSING, LAST_ACK, TIME_WAIT,
}tcp_fsm_states_t;

typedef enum
{
     CLOSE = 0,    // make it easy for initialization
     ACTIVE_OPEN, PASIVE_OPEN, RCV_SYN, RCV_ACK, RCV_SYNACK, RCV_FIN, RCV_FINACK, RX_PACKET, RCV_RST, RCV_RSTACK,
     RCV_PSHACK, TIMEOUT,
     NOP // dummy state, I will remove it later
}tcpEvent_t;

// add socket states to be used in the application
typedef enum
{
    NOT_A_SOCKET = 0,           // This is not a socket
    SOCKET_CLOSED,              // Socket closed
    SOCKET_IN_PROGRESS,         // The TCP listen or initiate a connection
    SOCKET_CONNECTED,           // The TCP is in established state user can send/receive data
    SOCKET_CLOSING              // The user initiate the closing procedure for this socket
} socketState_t;

typedef struct
{
    uint32_t localIP;
    uint16_t localPort;
    uint32_t remoteIP;
    uint16_t remotePort;
}tcpSocket_t;

typedef enum
{
    NO_BUFF = 0,
    RX_BUFF_IN_USE,
    TX_BUFF_IN_USE
}tcpBufferState_t;

typedef struct
{
    uint16_t localPort;             // this is the local port

    uint32_t destIP;
    uint16_t destPort;

    uint32_t remoteSeqno;
    uint32_t remoteAck;             // last ack packet sent to remote

    uint32_t localSeqno;
    uint32_t localLastAck;          // last ack number received

    uint16_t remoteWnd;             // sender window
    uint16_t localWnd;              // receiver window
    
    uint16_t mss;

    uint8_t *rxBufferStart;
    uint8_t *rxBufferPtr;           // pointer to write inside the rx buffer
    tcpBufferState_t rxBufState;

    uint8_t *txBufferStart;
    uint8_t *txBufferPtr;
    uint16_t bytesToSend;
    tcpBufferState_t txBufState;
    uint16_t bytesSent;
    bool payloadSave;

    tcp_fsm_states_t fsmState;      // connection state
    tcpEvent_t connectionEvent;

    // Linked List Pointers
    void *nextTCB;                  // downstream list pointer
    void *prevTCB;                  // upstream list pointer

    uint16_t timeout;               // retransmission time-out in seconds
    uint16_t timeoutReloadValue;
    uint8_t timeoutsCount;          // number of retransmissions
    uint8_t flags;                  // save the flags to be used for timeouts

    socketState_t socketState;     // socket state to be easy
}tcpTCB_t;

typedef enum
{
TCP_EOP = 0u,        // length = 0   End of Option List,[RFC793]
TCP_NOP = 1u,        // length = 0   No-Operation,[RFC793]
TCP_MSS = 2u,        // length = 4   Maximum Segment Size,[RFC793]

// this options are not implemented
#ifdef ALL_TCP_HEADER_OPTIONS
TCP_WIN_SCALE = 3,  // length = 3   Window Scale,[RFC7323]
TCP_SACK = 4,       // length = 2   SACK Permitted,[RFC2018]

5                   // length = N   SACK,[RFC2018]
6                   // length = 6   Echo (obsoleted by option 8),[RFC1072][RFC6247]
7                   // length = 6   Echo Reply (obsoleted by option 8),[RFC1072][RFC6247]
8                   // length = 10  Timestamps,[RFC7323]
9                   // length = 2   Partial Order Connection Permitted (obsolete),[RFC1693][RFC6247]
10                  // length = 3   Partial Order Service Profile (obsolete),[RFC1693][RFC6247]
11                  // length = 0   CC (obsolete),[RFC1644][RFC6247]
12                  // length = 0   CC.NEW (obsolete),[RFC1644][RFC6247]
13                  // length = 0   CC.ECHO (obsolete),[RFC1644][RFC6247]
14                  // length = 3   TCP Alternate Checksum Request (obsolete),[RFC1146][RFC6247]
15                  // length = N   TCP Alternate Checksum Data (obsolete),[RFC1146][RFC6247]
16                  // length = 0   Skeeter,[Stev_Knowles]
17                  // length = 0   Bubba,[Stev_Knowles]
18                  // length = 3   Trailer Checksum Option,[Subbu_Subramaniam][Monroe_Bridges]
19                  // length = 18  MD5 Signature Option (obsoleted by option 29),[RFC2385]
20                  // length = 0   SCPS Capabilities,[Keith_Scott]
21                  // length = 0   Selective Negative Acknowledgements,[Keith_Scott]
22                  // length = 0   Record Boundaries,[Keith_Scott]
23                  // length = 0   Corruption experienced,[Keith_Scott]
24                  // length = 0   SNAP,[Vladimir_Sukonnik]
25                  // length = 0   Unassigned (released 2000-12-18),
26                  // length = 0   TCP Compression Filter,[Steve_Bellovin]
27                  // length = 8   Quick-Start Response,[RFC4782]
28                  // length = 4   "User Timeout Option (also, other known unauthorized use) [***][1]",[RFC5482]
29                  // length = 0   TCP Authentication Option (TCP-AO),[RFC5925]
30                  // length = N   Multipath TCP (MPTCP),[RFC6824]
31                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
32                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
33                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
34-75               // length = 0   Reserved,
69                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
70                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
71-75               // length = 0   Reserved,
76                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
77                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
78                  // length = 0   Reserved (known unauthorized use without proper IANA assignment) [**],
79-252              // length = 0   Reserved,
253                 // length = N   "RFC3692-style Experiment 1 (also improperly used for shipping products) [*]",[RFC4727]
254                 // length = N   "RFC3692-style Experiment 2 (also improperly used for shipping products) [*]",[RFC4727]"
#endif
}tcp_options_t;


/**
  Section: TCP Public Interface Functions
 */


/*Initialize TCP structures
 * This functions initializes some internal TCP stack variables
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void TCP_Init(void);


/** Initialize the TCB and add it to the list of TCB pointers.
 * Put the socket in the CLOSED state.
 *
 * The user is responsible to manage allocation and releasing of the memory.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      -1 if there was any problems
 * @return
 *       0 if the insertion was successful.
 */
error_msg TCP_SocketInit(tcpTCB_t *tcb_ptr);   //jira: CAE_MCU8-5647


/** Remove the socket pointer from the TCP Stack, actually the pointer will be
 * removed from the list of TCB pointers. After calling this function the user
 * can reuse the memory.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      -1 if there was any problems
 * @return
 *       0 if the removing was successful.
 */
error_msg TCP_SocketRemove(tcpTCB_t *tcb_ptr);


/** The function will provide an interface to read the status of the socket.
 *  This function will also check if the pointer is already into the TCB list 
 *  (this means that the socket is "in use"). If the socket is into the TCB list
 * it will return the status for the socket.
 * 
 * @param tcb_ptr 
 *      pointer to socket/TCB structure
 *
 * @return
 *      the socket state
 */
socketState_t TCP_SocketPoll(tcpTCB_t *tcbPtr);


/** Assign a port number to the specified socket.
 * This is used for configure the local port of a socket.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @param port
 *      port number to be used as a local port. 
 * 
 * @return
 *      true - The port was initialized successfully
 * @return
 *      false - The initialization fails 
 */
error_msg TCP_Bind(tcpTCB_t *tcbPtr, uint16_t port);    //jira: CAE_MCU8-5647


/** Listen for connections on a socket.
 *  It marks the socket as a passive socket, that is, as a socket that 
 *  will be used to accept incoming connection requests.
 *  
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @param port
 *      port number used as a local port to listen on. 
 * 
 * @return
 *      true - The server was started successfully
 * @return
 *      false - The starting of the server fails
 */
error_msg TCP_Listen(tcpTCB_t *tcbPtr);    //jira: CAE_MCU8-5647


/** Start the client for a particular socket.
 *  
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @param port
 *      port number used as a local port to listen on. 
 * 
 * @return
 *      true - The server was started successfully
 * @return
 *      false - The starting of the server fails
 */
error_msg TCP_Connect(tcpTCB_t *tcbPtr, sockaddr_in4_t *srvaddr);    //jira: CAE_MCU8-5647


/** Close the TCP connection.
 * This will initiate the Closing sequence for the TCP connection.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      true - The Close procedure was started successfully
 * @return
 *      false - The Close procedure fails
 */
error_msg TCP_Close(tcpTCB_t *tcbPtr);     //jira: CAE_MCU8-5647


/** Send a buffer to a remote machine using a TCP connection.
 *  The function will add the buffer to the socket and the payload will be
 *  send as soon as possible.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      true - The buffer was added to the socket/TCB successfully
 * @return
 *      false - Adding the buffer to the socket fails
 */
error_msg TCP_Send(tcpTCB_t *tcbPtr, uint8_t *data, uint16_t dataLen);    //jira: CAE_MCU8-5647


/** Check if the TX buffer was send.
 *  This means that the data was received correctly by the remote machine and 
 *  the TX buffer memory can be reused by the application.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 * 
 * @return
 *      true - The buffer was transmitted successfully
 * @return
 *      false - Buffer transmission fails or wasn't sent yet.
 */
error_msg TCP_SendDone(tcpTCB_t *tcbPtr);    //jira: CAE_MCU8-5647


/** Will add the RX buffer to the socket.
 *
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 *
 * @param data
 *      Pointer to the data buffer
 *
 * @param data_len
 *      Size of the buffer
 *
 * @return
 *      true - The buffer was  passed to socket successfully
 * @return
 *      false - passing of the buffer to the socket failed.
 */
error_msg TCP_InsertRxBuffer(tcpTCB_t *tcbPtr, uint8_t *data, uint16_t dataLen);    //jira: CAE_MCU8-5647


/** This function will read the available data from the socket.
 *  The function will provide to the user also the start address of the 
 *  received buffer.
 * 
 * @param tcb_ptr
 *      pointer to the socket/TCB structure 
 * 
 * @return
 *      Number of received bytes in the buffer
 *      
 */
int16_t TCP_GetReceivedData(tcpTCB_t *tcbPtr);


/** This function will check and return the number of available bytes received
 *  on a socket.
 *
 * @param tcb_ptr
 *      pointer to the socket/TCB structure
 *
 * @return
 *      Number of received bytes in the buffer
 *
 */
int16_t TCP_GetRxLength(tcpTCB_t *tcbPtr);


/** This function needs to be called periodically in order to handle the
 *  TCP stack timeouts for each available socket.
 *
 * @param
 *      None
 *
 * @return
 *      None
 */
void TCP_Update(void);

/*
 *  Callback to TCP protocol to deliver the TCP packets
 *
 *  PMCS: todo find non public home for this as it is really an internal function...
 */
void TCP_Recv(uint8_t *packet, uint8_t **pkt_read, uint32_t, uint16_t);

#endif  /* TCPV4_H */

