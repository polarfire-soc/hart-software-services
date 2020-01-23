/**
  ENC28J60 Ethernet Driver
	
  Company:
    Microchip Technology Inc.

  File Name:
    ENC28J60_driver.c

  Summary:
    This is the Ethernet driver implementation for ENC28J60 family devices.

  Description:
    This file provides the Ethernet driver API implementation for
    the ENC28J60 device.

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


#include <stdint.h>
#include <stddef.h>
#include "ethernet_driver.h"
#include "network.h"

volatile ethernetDriver_t ethData;

#if 0

#define ETH_NCS_HIGH() do{LATC6 = 1;} while(0)  //Use the Ethernet Chip select as per your hardware specification here
#define ETH_NCS_LOW()  do{LATC6 = 0;} while(0)  //Use the Ethernet Chip select as per your hardware specification here
        
#define ETH_SPI_READ8()   spi1_exchangeByte(0)
#define ETH_SPI_WRITE8(a) spi1_exchangeByte(a)

const mac48Address_t *eth_MAC;
static uint16_t nextPacketPointer;
static receiveStatusVector_t rxPacketStatusVector;
sfr_bank_t lastBank;


    // Packet write in progress, not ready for transmit
#define ETH_WRITE_IN_PROGRESS       (0x0001 << 0)
    // Packet complete, in queue for transmit
#define ETH_TX_QUEUED               (0x0001 << 1)
    // Flag for pool management - free or allocated
#define ETH_ALLOCATED               (0x0001 << 2)

// adjust these parameters for the MAC...
#define RAMSIZE (0x1FFF) //8191
#define MAX_TX_PACKET_SIZE  (1518)
#define MIN_TX_PACKET_SIZE  (64)
#define MAX_TX_PACKETS (20)

#define TX_STATUS_VECTOR_SIZE   (7)

#define MIN_TX_PACKET           (MIN_TX_PACKET_SIZE + TX_STATUS_VECTOR_SIZE)
#define TX_BUFFER_SIZE          ((MAX_TX_PACKET_SIZE + TX_STATUS_VECTOR_SIZE) << 1)


// typical memory map for the MAC buffers
#define TXSTART (RAMSIZE - TX_BUFFER_SIZE) //5141
#define TXEND	(RAMSIZE) // 8191
#define RXSTART (0) 
#define RXEND	(TXSTART - 2) //5139

#define TX_BUFFER_MID           ((TXSTART) + ((TX_BUFFER_SIZE) >> 1) )

#define SetBit( bitField, bitMask )     do{ bitField = bitField | bitMask; } while(0)
#define ClearBit( bitField, bitMask )   do{ bitField = bitField & (~bitMask); } while(0)
#define CheckBit( bitField, bitMask )   (bool)(bitField & bitMask)

// Define a temporary register for passing data to inline assembly
// This is to work around the 110110 LSB errata and to control RDPTR WRPTR update counts

uint8_t ethListSize;

static txPacket_t txData[MAX_TX_PACKETS];

static txPacket_t  *pHead;
static txPacket_t  *pTail;

error_msg ETH_SendQueued(void);
error_msg ETH_Shift_Tx_Packets(void);

void ETH_PacketListReset(void);
txPacket_t* ETH_NewPacket(void);
void ETH_RemovePacket(txPacket_t* packetHandle);


static uint8_t ENC28_Rcr8(enc28j60_registers_t);
static uint16_t ENC28_Rcr16(enc28j60_registers_t);

static void ENC28_Wcr8(enc28j60_registers_t, uint8_t);
static void ENC28_Wcr16(enc28j60_registers_t, uint16_t);

static void ENC28_Bfs(enc28j60_registers_t, char);
static void ENC28_Bfc(enc28j60_registers_t, char);

static uint16_t ENC28_PhyRead(enc28j60_phy_registers_t a);
static void ENC28_PhyWrite(enc28j60_phy_registers_t a, uint16_t d);

static void ENC28_BankSel(enc28j60_registers_t);

static uint16_t checksumCalculation(uint16_t, uint16_t, uint16_t);

uint16_t TXPacketSize;

/*******************************************************************************/

/**
 * Bank Select
 * @param r
 */
static void ENC28_BankSel(enc28j60_registers_t r)
{
    uint8_t a = r & BANK_MASK;

    if (a != sfr_common && a != lastBank)
    {
        lastBank = a;
        // clear the bank bits
        ETH_NCS_LOW();
        ETH_SPI_WRITE8(bfc_inst | 0x1F);
        ETH_SPI_WRITE8(0x03);
        ETH_NCS_HIGH();
        NOP();
        NOP();
        // set the needed bits
        ETH_NCS_LOW();
        ETH_SPI_WRITE8(bfs_inst | 0x1F);
        ETH_SPI_WRITE8(a >> 6);
        ETH_NCS_HIGH();
    }
}

/**
 * Check for the link presence
 * @return
 */
bool ETH_CheckLinkUp(void)
{
    uint16_t phstat2;
    
    phstat2 = ENC28_PhyRead(J60_PHSTAT2);
    
    if(phstat2 & 0x0400)
    {
        ethData.up = true;
        return true;
    }
    else
        return false;
}

/**
 * "Allocate" a new packet element and link it into the chained list
 * @param 
 * @return  packet address
 */
txPacket_t* ETH_NewPacket(void)
{
    uint8_t index = 0;

    if( ethListSize == MAX_TX_PACKETS )
    {
        return NULL;
    }

    while( index < MAX_TX_PACKETS )
    {
        if( CheckBit(txData[index].flags, ETH_ALLOCATED) == false )
        {
            txData[index].flags = 0;                        // reset all flags
            SetBit(txData[index].flags, ETH_ALLOCATED);     // allocated = true - mark the handle as allocated

            txData[index].packetEnd = TXEND;

            txData[index].prevPacket = NULL;
            txData[index].nextPacket = pHead;

            if( pHead != NULL )
            {
                pHead->prevPacket = &txData[index];
                txData[index].packetStart = pHead->packetEnd + TX_STATUS_VECTOR_SIZE;
                    
                // Try to keep a 2byte alignment
                if( txData[index].packetStart & 0x0001 )
                {
                    // Make sure the end of the packet is odd, so the beginning of the next one is even
                    ++ txData[index].packetStart;
                }
            }
            else
            {
                txData[index].packetStart = TXSTART;
                pTail = (txPacket_t*)&txData[index];
            }

            pHead = (txPacket_t*)&txData[index];

            ethListSize ++;
            return (&txData[index]);
        }
        index ++;
    }

    return NULL;
}

/**
 * Reset the Ethernet Packet List
 * @param 
 * @return
 */
void ETH_PacketListReset(void)
{
    uint16_t index = 0;
    uint8_t* ptr = (uint8_t*)&txData;
    ethListSize = 0;

    pHead = NULL;
    pTail = NULL;

    while( index < (MAX_TX_PACKETS * sizeof(txPacket_t)) )
    {
        ptr[index] = 0;
        index++;
    }
}

/**
 * Unlink a packet element from the chained list and "deallocate" it
 * @param   packetHandle
 * @return 
 */
void ETH_RemovePacket(txPacket_t* pPacket)
{    
#ifdef VALIDATE_ALLOCATED_PTR
    uint8_t index = 0;
#endif /* VALIDATE_ALLOCATED_PTR */

    if( (pPacket == NULL) || (ethListSize == 0) )
    {
        return;
    }

#ifdef VALIDATE_ALLOCATED_PTR
    while( index < MAX_TX_PACKETS )
    {
        if( (pPacket == &txData[index]) && (txData[index].allocated == true) )
        {
            break;
        }
        index ++;
    }
    if( index == MAX_TX_PACKETS )
    {
        return;
    }
#endif  /* VALIDATE_ALLOCATED_PTR */

    // Unlink from the chained list
    if( pPacket->nextPacket == NULL )
    {
        pTail = pPacket->prevPacket;
        if( pTail != NULL )
        {
            pTail->nextPacket = NULL;
        }
    }

    if( pPacket->prevPacket == NULL )
    {
        pHead = pPacket->nextPacket;
        if( pHead != NULL )
        {
            pHead->prevPacket = NULL;
        }
    }

    // Deallocate
    pPacket->flags = 0;
    pPacket->prevPacket = NULL;
    pPacket->nextPacket = NULL;

    ethListSize --;

    return;
}

/**
 * Release SPI Bus
 */
void ETH_CloseSPI(void)
{
    spi1_close();
}
 
/**
  * Connect SPI Bus
  */
void ETH_OpenSPI(void)
{
    while (!spi_master_open(MAC));
}

/**
 * Ethernet Initialization - Initializes TX/RX Buffer, MAC and PHY
 */
void ETH_Init(void)
{
    ETH_OpenSPI();

    Control_Byte = 0x00;
   // initialize the driver variables
    ethData.error = false; // no error
    ethData.up = false; // no link
    ethData.linkChange = false;
    
    ETH_PacketListReset();

    ethData.saveRDPT = 0;

    lastBank = sfr_bank0;    

    __delay_us(100);

    ETH_SendSystemReset(); // software reset
    __delay_ms(10);

    // Wait for the OST
    while (!(ENC28_Rcr8(J60_ESTAT) & 0x01)); // wait for CLKRDY to go high

    // Initialize RX tracking variables and other control state flags
    nextPacketPointer = RXSTART;

    ENC28_Bfs (J60_ECON2, 0x80); // enable AUTOINC

    // Set up TX/RX buffer addresses
    ENC28_Wcr16(J60_ETXSTL, TXSTART);
    ENC28_Wcr16(J60_ETXNDL, TXEND);
    ENC28_Wcr16(J60_ERXSTL, RXSTART);
    ENC28_Wcr16(J60_ERXNDL, RXEND);
    ENC28_Wcr16(J60_ERDPTL,nextPacketPointer);

    ENC28_Wcr16(J60_ERDPTL, RXSTART);
    ENC28_Wcr16(J60_EWRPTL, TXSTART);

    // Configure the receive filter
    ENC28_Wcr8(J60_ERXFCON, 0b10101001); //UCEN,OR,CRCEN,MPEN,BCEN (unicast,crc,magic packet,broadcast)

    // what is my MAC address?
    eth_MAC =  MAC_getAddress();

    // Initialize the MAC
    ENC28_Wcr8(J60_MACON1, 0x0D); // TXPAUS, RXPAUS, MARXEN
    ENC28_Wcr8(J60_MACON3, 0x23); // Pad < 60 bytes, Enable CRC, Frame Check, Full Duplex	jira: MCU8CC-3708
    ENC28_Wcr8(J60_MACON4, 0x40); // DEFER set
    ENC28_Wcr16(J60_MAIPGL, 0x0c12);
    ENC28_Wcr8(J60_MABBIPG, 0x12);
    ENC28_Wcr16(J60_MAMXFLL, MAX_TX_PACKET_SIZE);
    ENC28_Wcr8(J60_MAADR1, eth_MAC->mac_array[0]);  NOP();
    ENC28_Wcr8(J60_MAADR2, eth_MAC->mac_array[1]);  NOP();
    ENC28_Wcr8(J60_MAADR3, eth_MAC->mac_array[2]);  NOP();
    ENC28_Wcr8(J60_MAADR4, eth_MAC->mac_array[3]);  NOP();
    ENC28_Wcr8(J60_MAADR5, eth_MAC->mac_array[4]);  NOP();
    ENC28_Wcr8(J60_MAADR6, eth_MAC->mac_array[5]);  NOP();

    ENC28_Wcr8(J60_ECON1, 0x04); // RXEN enabled    

    // Initialize the PHY
    ENC28_PhyWrite(J60_PHCON1, 0x0100); //PDPXMD - PHY Ful-Duplex mode enabled	jira: MCU8CC-3708
    ENC28_PhyWrite(J60_PHCON2, 0x0100); // Do not transmit loopback
    ENC28_PhyWrite(J60_PHLCON, 0x0472); // LED control - LEDA = Link, LEDB = TX/RX, Stretched LED
    // LEDB is grounded so default is Half Duplex

    // Configure the IRQ's
    ENC28_Wcr8(J60_EIE, 0xDB); // Enable PKTIE,INTIE,LINKIE,TXIE,TXERIE,RXERIE
    ENC28_Wcr16(((enc28j60_registers_t)J60_PHIE), 0x12); //Enable PLNKIE and PGEIE  

    // check for a preexisting link
    ETH_CheckLinkUp();
    
}

/**
 * Poll Ethernet Controller for new events
 */
void ETH_EventHandler(void)
{
    eir_t eir_val;
    phstat2_t phstat2_val; 
     
    // MAC is sending an interrupt
	// what is the interrupt
	eir_val.val = ENC28_Rcr8(J60_EIR);
	phstat2_val.val = ENC28_Rcr16((enc28j60_registers_t)J60_PHSTAT2);
	
	if (eir_val.LINKIF) // something about the link changed.... update the link parameters
	{
        ENC28_Bfc(J60_EIR,0x01);//CLEAR LINKIF
		ethData.linkChange = true;
		ethData.up = false;
		if(ETH_CheckLinkUp())
		{
		}
		if (phstat2_val.DPXSTAT) // Update MAC duplex settings to match PHY duplex setting
		{
			ENC28_Wcr16(J60_MABBIPG, 0x15); // Switching to full duplex
			ENC28_Bfs(((enc28j60_registers_t)J60_PHSTAT2),0x01);
		}
        else
		{
			ENC28_Wcr16(J60_MABBIPG, 0x12); // Switching to half duplex
            ENC28_Bfc(((enc28j60_registers_t)J60_PHSTAT2),0x01);
        }
    }
	if(eir_val.TXIF) // finished sending a packet
	{
		ENC28_Bfc(J60_EIR,0x08);//CLear TXIF
		ETH_RemovePacket(pTail);
		if( ethListSize > 0 )
		{
			if( ENC28_Rcr16(J60_EWRPTL) > TX_BUFFER_MID )
			{
				// Shift all the queued packets to the start of the TX Buffer
                ETH_Shift_Tx_Packets();
            }
                
            // Send the next queued packet
            ETH_SendQueued();
        }
    }
    if (eir_val.PKTIF || ENC28_Rcr8(J60_EPKTCNT)) // Packet receive buffer has at least 1 unprocessed packet
    {
		ethData.pktReady = true;
        ENC28_Bfc(J60_EIR,0x04);//clear PKTIF
    }
    
}

/**
 * Retrieve information about last received packet and the address of the next ones
 */
void ETH_NextPacketUpdate(void)
{
    // Set the RX Read Pointer to the beginning of the next unprocessed packet
    //Errata 14 inclusion
    if (nextPacketPointer == RXSTART)
        ENC28_Wcr16(J60_ERXRDPTL, RXEND);
    else
        ENC28_Wcr16(J60_ERXRDPTL, nextPacketPointer-1);
    
    ENC28_Wcr16(J60_ERDPTL, nextPacketPointer);
    
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rbm_inst);
    ((char *) &nextPacketPointer)[0] = ETH_SPI_READ8();
    ((char *) &nextPacketPointer)[1] = ETH_SPI_READ8();
    ((char *) &rxPacketStatusVector)[0] = ETH_SPI_READ8();
    ((char *) &rxPacketStatusVector)[1] = ETH_SPI_READ8();
    ((char *) &rxPacketStatusVector)[2] = ETH_SPI_READ8();
    ((char *) &rxPacketStatusVector)[3] = ETH_SPI_READ8();
    ETH_NCS_HIGH();
    rxPacketStatusVector.byteCount -= 4; // I don't care about the frame checksum at the end.
    // the checksum is 4 bytes.. so my payload is the byte count less 4.
}

void ETH_ResetReceiver(void)
{
    uint8_t econ1;
    econ1 = ENC28_Rcr8(J60_ECON1);
    ENC28_Wcr8(J60_ECON1,(econ1 | 0x40));
}


/**
 * Read 1 byte from SFRs
 * @param a
 * @return
 */
static uint8_t ENC28_Rcr8(enc28j60_registers_t a)
{
    uint8_t v;

    ENC28_BankSel(a);
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rcr_inst | (a & SFR_MASK));
    v = ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return v;
}

/**
 * Read 2 bytes from SFRs
 * @param a
 * @return
 */
static uint16_t ENC28_Rcr16(enc28j60_registers_t a)
{
    uint16_t v;

    ENC28_BankSel(a);
    a &= SFR_MASK;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rcr_inst | (a));
    ((char *) &v)[0] = ETH_SPI_READ8();
    ETH_NCS_HIGH();
    NOP();
    NOP();
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rcr_inst | (a + 1));
    ((char *) &v)[1] = ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return v;
}

/**
 * Write 1 byte to SFRs
 * @param a
 * @param v
 */
static void ENC28_Wcr8(enc28j60_registers_t a, uint8_t v)
{
    ENC28_BankSel(a);
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wcr_inst | (a & SFR_MASK));
    ETH_SPI_WRITE8(v);
    ETH_NCS_HIGH();
}

/**
 * Write 2 bytes to SFRs
 * @param a
 * @param v
 */
static void ENC28_Wcr16(enc28j60_registers_t a, uint16_t v)
{
    ENC28_BankSel(a);
    a &= SFR_MASK;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wcr_inst | (a));
    ETH_SPI_WRITE8(((char *) &v)[0]);
    ETH_NCS_HIGH();
    NOP();
    NOP();
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wcr_inst | (a + 1));
    ETH_SPI_WRITE8(((char *) &v)[1]);
    ETH_NCS_HIGH();
}

/**
 * SFR Bit Field Set
 * @param a
 * @param bits
 */
static void ENC28_Bfs(enc28j60_registers_t a, char bits) // can only be used for ETH Control Registers
{
    ENC28_BankSel(a);
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(bfs_inst | (a & SFR_MASK));
    ETH_SPI_WRITE8(bits);
    ETH_NCS_HIGH();
}

/**
 * SFR Bit Field Clear
 * @param a
 * @param bits
 */
static void ENC28_Bfc(enc28j60_registers_t a, char bits)
{
    ENC28_BankSel(a);
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(bfc_inst | (a & SFR_MASK));
    ETH_SPI_WRITE8(bits);
    ETH_NCS_HIGH();
}

/**
 * Write PHY  register
 * @param a
 * @param d
 */
static void ENC28_PhyWrite(enc28j60_phy_registers_t a, uint16_t d)
{
    uint8_t v=1;

    ENC28_Wcr8(J60_MIREGADR, a);
    ENC28_Wcr16(J60_MIWRL, d);
    while(v & 0x01)
    {
        v = ENC28_Rcr8(J60_MISTAT);
    }
}

/**
 * Read PHY register
 * @param a
 * @return
 */
static uint16_t ENC28_PhyRead(enc28j60_phy_registers_t a)
{
    ENC28_Wcr8(J60_MIREGADR, a);
    ENC28_Bfs(J60_MICMD, 0x01); // set the read flag
    while (ENC28_Rcr8(J60_MISTAT) & 0x01); // wait for the busy flag to clear
    ENC28_Bfc(J60_MICMD, 0x01); // clear the read flag

    return ENC28_Rcr16(J60_MIRDL);
}

/**
 * System Software Reset
 */
void ETH_SendSystemReset(void)
{
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(src_inst);
    ETH_NCS_HIGH();
}


/**
 * Read 1 byte of data from the RX Buffer
 * @return
 */

uint8_t ETH_Read8(void)
{
    uint8_t b;

    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rbm_inst);
    b = ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return b;
}

/**
 * Read 2 bytes of data from the RX Buffer
 * @return
 */
uint16_t ETH_Read16(void)
{
    uint16_t b;

    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rbm_inst);
    b = ETH_SPI_READ8()<< 8;
    b |= ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return b;
}

/**
 * Read 4 bytes of data from the RX Buffer
 * @return
 */
uint32_t ETH_Read32(void)
{
    uint32_t b;

    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rbm_inst);
    ((char *) &b)[3] = ETH_SPI_READ8();
    ((char *) &b)[2] = ETH_SPI_READ8();
    ((char *) &b)[1] = ETH_SPI_READ8();
    ((char *) &b)[0] = ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return b;
}

/**
 * Read block of data from the RX Buffer
 * @param buffer
 * @param length
 * @return
 */
uint16_t ETH_ReadBlock(void *buffer, uint16_t length)
{
    uint16_t readCount = length;
    char *p = buffer;

    if (rxPacketStatusVector.byteCount < length)
        readCount = rxPacketStatusVector.byteCount;
    length = readCount;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(rbm_inst);
    while (length--) *p++ = ETH_SPI_READ8();
    ETH_NCS_HIGH();

    return readCount;
}

/**
 * Write 1 byte of data to TX Buffer
 * @param data
 */
void ETH_Write8(uint8_t data)
{
    TXPacketSize += 1;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    ETH_SPI_WRITE8(data);
    ETH_NCS_HIGH();
}

/**
 * Write 2 bytes of data to TX Buffer
 * @param data
 */
void ETH_Write16(uint16_t data)
{
    TXPacketSize += 2;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    ETH_SPI_WRITE8(data >> 8);
    ETH_SPI_WRITE8(data);
    ETH_NCS_HIGH();
}

/**
 * Write 4 bytes of data to TX Buffer
 * @param data
 */
void ETH_Write32(uint32_t data)
{
    TXPacketSize += 4;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    ETH_SPI_WRITE8(data >> 24);
    ETH_SPI_WRITE8(data >> 16);
    ETH_SPI_WRITE8(data >> 8);
    ETH_SPI_WRITE8(data);
    ETH_NCS_HIGH();
}

uint16_t ETH_WriteString(const char *string)
{
    uint16_t length = 0;
    
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    while(*string)
    {
        ETH_SPI_WRITE8(*string++);
        length ++;
    }    
    ETH_NCS_HIGH();
    TXPacketSize += length;
    
    return length;
}

/**
 * Write a block of data to TX Buffer
 * @param data
 * @param length
 */
uint16_t ETH_WriteBlock(const char* data, uint16_t length)  // jira:M8TS-608
{
    const char *p = data;
    TXPacketSize += length;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    while (length--) {
        ETH_SPI_WRITE8(*p++);
    }
    ETH_NCS_HIGH();

    return length;
}

/**
 * Returns the available space size in the Ethernet TX Buffer
 * @param 
 * @return available space left in the TX buffer
 */
uint16_t ETH_GetFreeTxBufferSize(void)
{
    return (uint16_t)(TXEND - (ENC28_Rcr16(J60_EWRPTL)));
}

/**
 * start a packet.
 * If the Ethernet transmitter is idle, then start a packet.  Return is SUCCESS if the packet was started.
 * @param dest_mac
 * @param type
 * @return SUCCESS if packet started.  BUFFER_BUSY or TX_LOGIC_NOT_IDLE if buffer or transmitter is busy respectively
 */
error_msg ETH_WriteStart(const mac48Address_t *dest_mac, uint16_t type)
{
    txPacket_t* ethPacket = NULL;
    
    if( ENC28_Rcr16(J60_EWRPTL) > TX_BUFFER_MID )
    {
        // Need to shift all the queued packets to the start of the TX Buffer

        // Check for TX in progress
        if((ENC28_Rcr8(J60_ECON1) & 0x08))
        {
            return TX_LOGIC_NOT_IDLE;
        }
        
        // Try to move the queued packets
        ETH_Shift_Tx_Packets();
        
        // Verify if shift
        if( ENC28_Rcr16(J60_EWRPTL) > TX_BUFFER_MID )
        {
            return BUFFER_BUSY;
        }
    }

    // Create new packet and queue it in the TX Buffer
    
    // Initialize a new packet handler. It is automatically placed in the queue
    ethPacket = ETH_NewPacket();

    if( ethPacket == NULL )
    {
        // No more available packets
        return BUFFER_BUSY;
    }

    SetBit(ethPacket->flags, ETH_WRITE_IN_PROGRESS);    // writeInProgress = true;
    
    ENC28_Wcr16(J60_EWRPTL, ethPacket->packetStart);

    ETH_ResetByteCount(); 

    TXPacketSize = 0;
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(wbm_inst);
    ETH_SPI_WRITE8(Control_Byte);
    ETH_SPI_WRITE8(dest_mac->mac_array[0]);
    ETH_SPI_WRITE8(dest_mac->mac_array[1]);
    ETH_SPI_WRITE8(dest_mac->mac_array[2]);
    ETH_SPI_WRITE8(dest_mac->mac_array[3]);
    ETH_SPI_WRITE8(dest_mac->mac_array[4]);
    ETH_SPI_WRITE8(dest_mac->mac_array[5]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[0]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[1]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[2]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[3]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[4]);
    ETH_SPI_WRITE8(eth_MAC->mac_array[5]);
    ETH_SPI_WRITE8(type >> 8);
    ETH_SPI_WRITE8(type & 0x0FF);
    ETH_NCS_HIGH();
    TXPacketSize += 15;

    return SUCCESS;
}

/**
 * Enqueue the latest written packet and start the transmission of a queued packet
 * @return
 */
error_msg ETH_SendQueued(void)
{
    uint8_t econ1;
    
    if( pHead->flags & ETH_TX_QUEUED )
    {
            // "Close" the latest written packet and enqueue it
        ClearBit( pHead->flags, ETH_TX_QUEUED);         // txQueued = false

            // Start transmitting from the tails - the packet first written 
        ENC28_Wcr16(J60_ETXSTL, pTail->packetStart);
        ENC28_Wcr16(J60_ETXNDL, pTail->packetEnd);

        NOP(); NOP();
        econ1 = ENC28_Rcr8(J60_ECON1);
        ENC28_Wcr8(J60_ECON1,(econ1 | 0x08));// start sending

        putch('-');
        return SUCCESS;
    }
    else
    {
        return BUFFER_BUSY;
    }
}

/**
 * Start the Transmission
 * @return
 */
error_msg ETH_Send(void)
{
    uint16_t packetEnd = (ENC28_Rcr16(J60_EWRPTL)) - 1;
    
    ENC28_Wcr16(J60_ETXNDL,TXSTART+TXPacketSize);
    if (!ethData.up)
    {
        return LINK_NOT_FOUND;
    }

    if( ethListSize == 0 )
    {
        return BUFFER_BUSY;
    }
    
    ClearBit( pHead->flags, ETH_WRITE_IN_PROGRESS);     // writeInProgress = false
    pHead->packetEnd = packetEnd;
    SetBit( pHead->flags, ETH_TX_QUEUED);               // txQueued = true
    // The packet is prepared to be sent / queued at this time

    if( (ENC28_Rcr16(J60_ECON1) & 0x08)|| (ethListSize > 1) )
    {
        return TX_QUEUED;
    }
    return ETH_SendQueued();
}


/**
 * Clears number of bytes (length) from the RX buffer
 * @param length
 */
void ETH_Dump(uint16_t length)
{
    uint16_t newRXTail;

    length = (rxPacketStatusVector.byteCount <= length) ? rxPacketStatusVector.byteCount : length;
    if (length) {;
        newRXTail = ENC28_Rcr16(J60_ERDPTL);
        newRXTail += length;
        //Write new RX tail
        ENC28_Wcr16(J60_ERDPTL, newRXTail);

        rxPacketStatusVector.byteCount -= length;
    }
}

/**
 * Clears all bytes from the RX buffer
 */
void ETH_Flush(void)
{
    ethData.pktReady = false;
    if (nextPacketPointer == RXSTART)ENC28_Wcr16(J60_ERXRDPTL, RXEND);
            else ENC28_Wcr16(J60_ERXRDPTL,nextPacketPointer-1);
    ENC28_Wcr16(J60_ERDPTL, nextPacketPointer);
        //Packet decrement
    ENC28_Bfs(J60_ECON2, 0x40);
}

/**
 * Insert data in between of the TX Buffer
 * @param data
 * @param len
 * @param offset
 */
void ETH_Insert(char *data, uint16_t len, uint16_t offset)  // jira:M8TS-608
{
    uint16_t current_tx_pointer = 0;    
    offset+=sizeof(Control_Byte);

    current_tx_pointer = ENC28_Rcr16(J60_EWRPTL);
    ENC28_Wcr16(J60_EWRPTL, pHead->packetStart+offset);
    while (len--)
    {
         ETH_NCS_LOW();
         ETH_SPI_WRITE8(wbm_inst); //WBM command
         ETH_SPI_WRITE8(*data++);
         ETH_NCS_HIGH();
    }
    ENC28_Wcr16(J60_EWRPTL, current_tx_pointer);
}

/**
 * Copy the data from RX Buffer to the TX Buffer using DMA setup
 * This is used for ICMP ECHO to eliminate the need to extract the arbitrary payload
 * @param len
 */
error_msg ETH_Copy(uint16_t len)
{
    uint16_t tx_buffer_address;
    uint16_t timer;
    uint16_t temp_len;

    timer = 2 * len;
    // Wait until module is idle
    while ((ENC28_Rcr8(J60_ECON1) & 0x20) != 0 && --timer) NOP(); // sit here until the DMAST bit is clear

    if((ENC28_Rcr8(J60_ECON1) & 0x20) == 0)
    {
        tx_buffer_address = ENC28_Rcr16(J60_EWRPTL); // Current TX Write Pointer

        ENC28_Wcr16(J60_EDMADSTL, tx_buffer_address);        
        ENC28_Wcr16(J60_EDMASTL, ethData.saveRDPT);

        tx_buffer_address += len;
        temp_len = ethData.saveRDPT + len;

        if(temp_len > RXEND)
        {
            temp_len = temp_len - (RXEND) + RXSTART;
            ENC28_Wcr16(J60_EDMANDL, temp_len);
        }else
        {
            ENC28_Wcr16(J60_EDMANDL, temp_len);
        }

        // Clear CSUMEN to select a copy operation
        ENC28_Bfc(J60_ECON1, 0x10);
        // start the DMA
        ENC28_Bfs(J60_ECON1, 0x20);
        timer = 40 * len;
        while ((ENC28_Rcr8(J60_ECON1) & 0x20)!=0 && --timer) NOP();// sit here until the DMAST bit is clear
        if((ENC28_Rcr8(J60_ECON1) & 0x20)==0)
        {
            // clean up the source and destination window pointers
            ENC28_Wcr16(J60_EWRPTL, tx_buffer_address);

            TXPacketSize += len; // fix the packet length
            return SUCCESS;
        }
    }
    RESET(); // reboot for now
    return DMA_TIMEOUT;
}


/**
 * Copy all the queued packets to the TX Buffer start address using DMA setup
 * @param 
 * @return
 */
error_msg ETH_Shift_Tx_Packets(void)
{
    uint8_t econ1;
    uint16_t timer;
    uint16_t len = pHead->packetEnd - pTail->packetStart;

    timer = 2 * len;
    while((ENC28_Rcr8(J60_ECON1) & 0x20)!=0 && --timer) NOP(); // sit here until DMA is free
    if((ENC28_Rcr8(J60_ECON1) & 0x20)==0) //ECON1bits.DMAST==0
    {
        ENC28_Wcr16(J60_EDMADSTL,TXSTART);// setup the destination start pointer
        
        ENC28_Wcr16(J60_EDMASTL,pTail->packetStart); // setup the source pointer from the current read pointer
        ENC28_Wcr16(J60_EDMANDL,pTail->packetStart);

        econ1 = ENC28_Rcr8(J60_ECON1);
        ENC28_Wcr8(J60_ECON1,(econ1 | 0x10)); // copy mode ECON1bits.CSUMEN = 0
        econ1 = ENC28_Rcr8(J60_ECON1);
        ENC28_Wcr8(J60_ECON1,(econ1 | 0x20)); // start dma ECON1bits.DMAST  = 1
        
        /* sometimes it takes longer to complete if there is heavy network traffic */
        timer = 40 * len;
        while((ENC28_Rcr8(J60_ECON1) & 0x20)!=0 && --timer) NOP(); // sit here until DMA is free
        if((ENC28_Rcr8(J60_ECON1) & 0x20)== 0)
        {
            // Update the start and end addresses of each packet
            txPacket_t  *pElem = pHead;
            uint16_t shiftOffset = pTail->packetStart;

            while( pElem != NULL )
            {
                pElem->packetStart = pElem->packetStart - shiftOffset;
                pElem->packetEnd = pElem->packetEnd - shiftOffset;
                pElem = pElem->nextPacket;
            }

            // Update the EWRPT
            ENC28_Wcr16(J60_EWRPTL,TXSTART + len);
            return SUCCESS;
        }
    }

    // if we are here. the DMA timed out.
    RESET(); // reboot for now
    return DMA_TIMEOUT;
}

static uint16_t ETH_ComputeChecksum(uint16_t len, uint16_t seed)
{
    uint32_t cksm;
    uint16_t v;

    cksm = seed;

    while(len > 1)
    {
        v = 0;
        ((char *)&v)[1] = ETH_Read8();
        ((char *)&v)[0] = ETH_Read8();
        cksm += v;
        len -= 2;
    }

    if(len)
    {
        v = 0;
        ((char *)&v)[1] = ETH_Read8();
        ((char *)&v)[0] = 0;
        cksm += v;
    }

    // wrap the checksum
    while(cksm >> 16)
    {
        cksm = (cksm & 0x0FFFF) + (cksm>>16);
    }

    // invert the number.
    cksm = ~cksm;

    // Return the resulting checksum
    return (uint16_t)cksm;
}

/**
 * Calculate the Checksum - Hardware Checksum
 * @param position
 * @param length
 * @return
 */
uint16_t ETH_TxComputeChecksum(uint16_t position, uint16_t length, uint16_t seed)
{
    uint32_t cksm;

//    cksm = seed;
    position+= sizeof(Control_Byte);

    while ((ENC28_Rcr8(J60_ECON1) & 0x20) != 0); // sit here until the DMAST bit is clear

    ENC28_Wcr16(J60_EDMASTL, (pHead->packetStart + position));
    ENC28_Wcr16(J60_EDMANDL, pHead->packetStart + position + (length-1));

    if (!(ENC28_Rcr8(J60_ECON1) & 0x10)) //Make sure CSUMEN is not set already
    {
        // Set CSUMEN and DMAST to select and start a checksum operation
        ENC28_Bfs(J60_ECON1, 0x30);
        while ((ENC28_Rcr8(J60_ECON1) & 0x20) != 0); // sit here until the DMAST bit is clear
        ENC28_Bfc(J60_ECON1,0x10);

        cksm = ENC28_Rcr16(J60_EDMACSL);
        if (seed)
        {
            seed=~(seed);
            cksm+=seed;
            while(cksm >> 16)
            {
                cksm = (cksm & 0x0FFFF) + (cksm>>16);
            }          
        }
        cksm = htons(cksm);
    }
    return (uint16_t)cksm;
}

/**
 * Calculate RX checksum - Software checksum
 * @param len
 * @param seed
 * @return
 */
uint16_t ETH_RxComputeChecksum(uint16_t len, uint16_t seed)
{
    uint16_t rxptr;
    uint32_t cksm;

    // Save the read pointer starting address
    rxptr = ENC28_Rcr16(J60_ERDPTL);;

    cksm = ETH_ComputeChecksum( len, seed);

    // Restore old read pointer location
     ENC28_Wcr16(J60_ERDPTL,rxptr);

    // Return the resulting checksum
    return (uint16_t)((cksm & 0xFF00) >> 8) | ((cksm & 0x00FF) << 8);
}

/**
 * To get the MAC address
 * @param mac
 */
void ETH_GetMAC(uint8_t *macAddr)
{
   *macAddr++ = ENC28_Rcr8(J60_MAADR1);
    *macAddr++ = ENC28_Rcr8(J60_MAADR2);
    *macAddr++ = ENC28_Rcr8(J60_MAADR3);
    *macAddr++ = ENC28_Rcr8(J60_MAADR4);
    *macAddr++ = ENC28_Rcr8(J60_MAADR5);
    *macAddr++ = ENC28_Rcr8(J60_MAADR6);
}

/**
 * To set the MAC address
 * @param mac
 */
void ETH_SetMAC(uint8_t *macAddr)
{
    ENC28_Wcr8(J60_MAADR1, *macAddr++ );
    ENC28_Wcr8(J60_MAADR2, *macAddr++ );
    ENC28_Wcr8(J60_MAADR3, *macAddr++ );
    ENC28_Wcr8(J60_MAADR4, *macAddr++ );
    ENC28_Wcr8(J60_MAADR5, *macAddr++ );
    ENC28_Wcr8(J60_MAADR6, *macAddr++ );
}

void ETH_SaveRDPT(void)
{
    ethData.saveRDPT = ENC28_Rcr16(J60_ERDPTL);
}

uint16_t ETH_GetReadPtr(void)
{
    return ENC28_Rcr16(J60_ERDPTL);
}

void ETH_SetReadPtr(uint16_t rdptr)
{
   ENC28_Wcr16(J60_ERDPTL, rdptr);
}

void ETH_MoveBackReadPtr(uint16_t offset)
{
    uint16_t rdptr;
    
    rdptr = ENC28_Rcr16(J60_ERDPTL);
    ENC28_Wcr16(J60_ERDPTL, rdptr-offset);   
    ETH_SetRxByteCount(offset);  
        
}

void ETH_ResetReadPtr()
{
    ENC28_Wcr16(J60_ERDPTL, RXSTART);    
}

uint16_t ETH_GetWritePtr()
{
    return ENC28_Rcr16(J60_EWRPTL);
}

uint16_t ETH_GetRxByteCount()
{
    return (rxPacketStatusVector.byteCount);
}

void ETH_SetRxByteCount(uint16_t count)
{
    rxPacketStatusVector.byteCount += count;
}

void ETH_ResetByteCount(void)
{
    ethData.saveWRPT = ENC28_Rcr16(J60_EWRPTL);
}

uint16_t ETH_GetByteCount(void)
{
     uint16_t wptr;

    wptr = ENC28_Rcr16(J60_EWRPTL);

    return (wptr - ethData.saveWRPT);
}

void ETH_SaveWRPT(void)
{
    ethData.saveWRPT = ENC28_Rcr16(J60_EWRPTL);
}


uint16_t ETH_ReadSavedWRPT(void)
{
    return ethData.saveWRPT;
}

uint16_t ETH_GetStatusVectorByteCount(void)
{
    return(rxPacketStatusVector.byteCount);
}

void ETH_SetStatusVectorByteCount(uint16_t bc)
{
    rxPacketStatusVector.byteCount=bc;
}


void ETH_TxReset(void)
{
    uint8_t econ1;
    econ1 = ENC28_Rcr8(J60_ECON1);
    
    ENC28_Wcr8(J60_ECON1,(econ1 | 0x80));
    
    ETH_ResetByteCount();    
    
    ENC28_Wcr16(J60_ETXSTL, TXSTART); 
    ENC28_Wcr16(J60_EWRPTL, TXSTART);   
   
    ETH_PacketListReset();
}
#endif
