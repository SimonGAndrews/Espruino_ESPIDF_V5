/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * CC3000 WiFi Interface
 * ----------------------------------------------------------------------------
 */

#include "hci.h"
#include "spi.h"

#include "jshardware.h"
#include "board_spi.h"

#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5)

#define SPI_HEADER_SIZE         (5)

#define     eSPI_STATE_POWERUP               (0)
#define     eSPI_STATE_INITIALIZED           (1)
#define     eSPI_STATE_IDLE                  (2)
#define     eSPI_STATE_WRITE_IRQ             (3)
#define     eSPI_STATE_WRITE_FIRST_PORTION   (4)
#define     eSPI_STATE_WRITE_EOT             (5)
#define     eSPI_STATE_READ_IRQ              (6)
#define     eSPI_STATE_READ_FIRST_PORTION    (7)
#define     eSPI_STATE_READ_EOT              (8)

typedef struct
{
    gcSpiHandleRx  SPIRxHandler;
    unsigned short usTxPacketLength;
    unsigned short usRxPacketLength;
    unsigned long  ulSpiState;
    unsigned char *pTxPacket;
    unsigned char *pRxPacket;

}tSpiInformation;


tSpiInformation sSpiInformation;


// buffer for 5 bytes of SPI HEADER
unsigned char tSpiReadHeader[] = {READ, 0, 0, 0, 0};


void SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
void SpiWriteAsync(const unsigned char *data, unsigned short size);
void SpiPauseSpi(void);
void SpiResumeSpi(void);
void SSIContReadOperation(void);
#define ASSERT_CS()          jshPinSetValue(WLAN_CS_PIN, 0)
#define DEASSERT_CS()        jshPinSetValue(WLAN_CS_PIN, 1)


// The magic number that resides at the end of the TX/RX buffer (1 byte after
// the allocated size) for the purpose of detection of the overrun. The location
// of the memory where the magic number resides shall never be written. In case
// it is written - the overrun occurred and either receive function or send
// function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

char spi_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];


void  SpiInit(void)
{
  // SPI config
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.pinSCK =  WLAN_CLK_PIN;
  inf.pinMISO = WLAN_MISO_PIN;
  inf.pinMOSI = WLAN_MOSI_PIN;
  jshSPISetup(WLAN_SPI, &inf);

  // WLAN CS, EN and WALN IRQ Configuration
  jshSetPinStateIsManual(WLAN_CS_PIN, false);
  jshPinOutput(WLAN_CS_PIN, 1);
  jshSetPinStateIsManual(WLAN_EN_PIN, false);
  jshPinOutput(WLAN_EN_PIN, 0);
  jshSetPinStateIsManual(WLAN_IRQ_PIN, false);
  jshPinSetState(WLAN_IRQ_PIN, JSHPINSTATE_GPIO_IN_PULLUP); // flip into read mode with pullup
}

void SpiClose(void)
{
    if (sSpiInformation.pRxPacket)
        sSpiInformation.pRxPacket = 0;
    //  Disable Interrupt
    WlanInterruptDisable();
}

void SpiOpen(gcSpiHandleRx pfRxHandler)
{
	sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;
	sSpiInformation.SPIRxHandler = pfRxHandler;
    sSpiInformation.usTxPacketLength = 0;
    sSpiInformation.pTxPacket = NULL;
    sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
    sSpiInformation.usRxPacketLength = 0;
    spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
    wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

	// Enable interrupt
    tSLInformation.WlanInterruptEnable();
}

long
SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
    // workaround for first transaction
    ASSERT_CS();

    // 50 microsecond delay
    jshDelayMicroseconds(50);

    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    jshDelayMicroseconds(50);

    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    DEASSERT_CS();

    return(0);
}

long
SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
    unsigned char ucPad = 0;

    // Figure out the total length of the packet in order to figure out if there
    // is padding or not
    if(!(usLength & 0x0001))
    {
        ucPad++;
    }

    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

    usLength += (SPI_HEADER_SIZE + ucPad);

    // The magic number that resides at the end of the TX/RX buffer (1 byte after
    // the allocated size) for the purpose of detection of the overrun. If the
    // magic number is overwritten - buffer overrun occurred - and we will stuck
    // here forever!
    if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
    {
        while (1)
            ;
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
    {
        while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED)
            ;
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED)
    {
        // This is time for first TX/RX transactions over SPI: the IRQ is down -
        // so need to send read buffer size command
        SpiFirstWrite(pUserBuffer, usLength);
    }
    else
    {
        // We need to prevent here race that can occur in case 2 back to back
        // packets are sent to the  device, so the state will move to IDLE and once
        //again to not IDLE due to IRQ
        tSLInformation.WlanInterruptDisable();

        while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE)
        {
            ;
        }

        sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
        sSpiInformation.pTxPacket = pUserBuffer;
        sSpiInformation.usTxPacketLength = usLength;

        // Assert the CS line and wait till SSI IRQ line is active and then
        // initialize write operation
        ASSERT_CS();

        // Re-enable IRQ - if it was not disabled - this is not a problem...
        tSLInformation.WlanInterruptEnable();

        // check for a missing interrupt between the CS assertion and enabling back the interrupts
        if (tSLInformation.ReadWlanInterruptPin() == 0)
        {
            SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

            sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

            DEASSERT_CS();
        }
    }

    // Due to the fact that we are currently implementing a blocking situation
    // here we will wait till end of transaction
    while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState)
        ;

    return(0);
}

void
SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
  int bSend = 0, bRecv = 0;
  while (bSend<size && bRecv<size) {
    int r = jshSPISend(WLAN_SPI, (bSend<size)?data[bSend]:-1);
    bSend++;
    if (r>=0) bRecv++;
  }
}


void
SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
  int bSend = 0, bRecv = 0;
  while (bSend<size && bRecv<size) {
    int r = jshSPISend(WLAN_SPI, (bSend<size)?READ:-1);
    bSend++;
    if (r>=0) data[bRecv++] = r;
  }
}

void
SpiReadHeader(void)
{
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

long SpiReadDataCont(void) {
    long data_to_recv;
    unsigned char *evnt_buff, type;

    //determine what type of packet we have
    evnt_buff =  sSpiInformation.pRxPacket;
    data_to_recv = 0;
    STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET,
                                    type);

    switch(type)
    {
    case HCI_TYPE_DATA:
        {
            // We need to read the rest of data..
            STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
                                             HCI_DATA_LENGTH_OFFSET, data_to_recv);
            if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1))
            {
                data_to_recv++;
            }

            if (data_to_recv)
            {
                SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
            }
            break;
        }
    case HCI_TYPE_EVNT:
        {
            // Calculate the rest length of the data
            STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
                                            HCI_EVENT_LENGTH_OFFSET, data_to_recv);
            data_to_recv -= 1;

            // Add padding byte if needed
            if ((HEADERS_SIZE_EVNT + data_to_recv) & 1)
            {

                data_to_recv++;
            }

            if (data_to_recv)
            {
                SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
            }

            sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
            break;
        }
    }

    return (0);
}


void
SpiPauseSpi(void)
{
    // FIXME
}

void
SpiResumeSpi(void)
{
  // FIXME
}

void
SpiTriggerRxProcessing(void)
{

    // Trigger Rx processing
    SpiPauseSpi();
    DEASSERT_CS();

    // The magic number that resides at the end of the TX/RX buffer (1 byte after
    // the allocated size) for the purpose of detection of the overrun. If the
    // magic number is overwritten - buffer overrun occurred - and we will stuck
    // here forever!
    if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
    {
        while (1)
            ;
    }

    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
    sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}

void SpiIntGPIOHandler(void)
{
        if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
        {
            //This means IRQ line was low call a callback of HCI Layer to inform
            //on event
            sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
        }
        else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE)
        {
            sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;

            /* IRQ line goes down - we are start reception */
            ASSERT_CS();

            // Wait for TX/RX Compete which will come as DMA interrupt
            SpiReadHeader();

            sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;

            SSIContReadOperation();
        }
        else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ)
        {
            SpiWriteDataSynchronous(sSpiInformation.pTxPacket,
                                                            sSpiInformation.usTxPacketLength);

            sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

            DEASSERT_CS();
        }
}

void
SSIContReadOperation(void)
{
    // The header was read - continue with  the payload read
    if (!SpiReadDataCont())
    {
        // All the data was read - finalize handling by switching to the task
        //  and calling from task Event Handler
        SpiTriggerRxProcessing();
    }
}

long ReadWlanInterruptPin(void)
{
    return jshPinGetValue(WLAN_IRQ_PIN);
}

void WlanInterruptEnable(void) {
  jshPinWatch(WLAN_IRQ_PIN, true);
}

void WlanInterruptDisable(void) {
  jshPinWatch(WLAN_IRQ_PIN, false);
}

