
#include <string.h>
#include <stdint.h>

#include "interface.h"
#include "netx_io_areas.h"
#include "systime.h"
#include "uart.h"
#include "uprintf.h"
#include "version.h"


/*-------------------------------------------------------------------------*/

#define UART_UNIT 0
static const UART_CONFIGURATION_T tUartCfg =
{
	.us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};

typedef enum BTSC_ENUM
{
	BTSC_SOFT_RESET           = 0x01,
	BTSC_READ_DEV_INFO        = 0x02,
	BTSC_ENABLE_LED_FORCING   = 0x03,
	BTSC_SET_LED_STATE        = 0x04,
	BTSC_SET_BTSTREAM_PARAM   = 0x10,
	BTSC_ENABLE_BTSTREAM      = 0x11,
	BTSC_WRITE_BTSTREAM_DATA  = 0x12,
	BTSC_DTM_TEST_REQ         = 0x1f,
	BTSC_SET_LIGHT_PARAM      = 0x20,
	BTSC_ENABLE_LIGHT         = 0x21,
	BTSC_LIGHT_SELFTEST       = 0x22,
	BTSC_DFU_MODE_REQUEST     = 0x30
} BTSC_T;



typedef enum FRAME_TYPE_ENUM
{
	FRAME_TYPE_Command  = 0,
	FRAME_TYPE_Event    = 1
} FRAME_TYPE_T;



typedef enum BTSC_ERROR_ENUM
{
	BTSC_ERROR_Success                   = 0x00,
	BTSC_ERROR_UnknownCommand            = 0x01,
	BTSC_ERROR_CommandDisallowed         = 0x02,
	BTSC_ERROR_InvalidCommandParameters  = 0x03,
	BTSC_ERROR_HardwareFailure           = 0x04,
	BTSC_ERROR_BluetoothLeStreamTimeout  = 0x10,
	BTSC_ERROR_LightSelfTestFailed       = 0x20,
	BTSC_ERROR_Unspecified               = 0xff
} BTSC_ERROR_T;


static const unsigned char aucCrc8[256] =
{
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
	0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static unsigned char crc8ccitt(const unsigned char *pucData, unsigned int sizData)
{
	unsigned char ucVal;
	const unsigned char *pucCnt;
	const unsigned char *pucEnd;


	ucVal = 0x00U;
	pucCnt = pucData;
	pucEnd = pucCnt + sizData;
	while(pucCnt<pucEnd)
	{
		ucVal = aucCrc8[ucVal ^ *pucCnt];
		pucCnt++;
	}

	return ucVal;
}


static unsigned char aucBuffer[1024];


static void send_command(unsigned char ucCommand)
{
	unsigned long ulValue;
	unsigned int sizPayload;
	unsigned int uiCnt;
	unsigned char aucHeader[8];


	/* Create a new command. */
	sizPayload = 0U;

	/*
	 * Create a header.
	 */
	/* Write the SYNC words. This is 0xDE, 0x51, 0x4E 0x90. */
	aucHeader[0] = 0xDE;
	aucHeader[1] = 0x51;
	aucHeader[2] = 0x4E;
	aucHeader[3] = 0x90;

	/* Write the frame control field. */
	ulValue  = sizPayload;
	ulValue |= FRAME_TYPE_Command << 14U;
	aucHeader[4] = (unsigned char)( ulValue         & 0xffU);
	aucHeader[5] = (unsigned char)((ulValue >>  8U) & 0xffU);

	/* Set the frame identifier. This is the opcode. */
	aucHeader[6] = ucCommand;

	/* Generate the CCITT CRC over the frame control and the frame identifier. */
	aucHeader[7] = crc8ccitt(aucHeader+4U, 3U);

	/* Send all data. */
	for(uiCnt=0; uiCnt<8; ++uiCnt)
	{
		uart_put(UART_UNIT, aucHeader[uiCnt]);
	}
}



static int receive_device_info(unsigned long ulTimeoutinMs)
{
	HOSTDEF(ptUartArea);
	int iResult;
	unsigned long ulValue;
	unsigned int uiCnt;
	unsigned char ucCrc;
	int iIsElapsed;
	TIMER_HANDLE_T tTimer;


	/* Be pessimistic. */
	iResult = -1;

	/* Receive as much as possible in the given timeout. */
	systime_handle_start_ms(&tTimer, ulTimeoutinMs);
	uiCnt = 0;
	iIsElapsed = 0;
	do
	{
		/* Is data in the FIFO? */
		ulValue = ptUartArea->ulUartfr;
		ulValue &= HOSTMSK(uartfr_RXFE);
		if( ulValue==0U )
		{
			aucBuffer[uiCnt] = (unsigned char)((ptUartArea->ulUartdr) & 0xffU);
			++uiCnt;
			if( uiCnt>=sizeof(aucBuffer) )
			{
				break;
			}
		}
		else
		{
			/* Timer elapsed? */
			iIsElapsed = systime_handle_is_elapsed(&tTimer);
			if( iIsElapsed!=0 )
			{
				break;
			}
		}
	} while( uiCnt<sizeof(aucBuffer) );

	/* Check the SYNC words. */
	if( uiCnt>8 && aucBuffer[0]==0xDE && aucBuffer[1]==0x51 && aucBuffer[2]==0x4E && aucBuffer[3]==0x90 )
	{
		/* Check the CRC8. */
		ucCrc = crc8ccitt(aucBuffer+4U, 3U);
		if( ucCrc==aucBuffer[7] )
		{
			/* The response must be a "Command Complete" event. */
			if( aucBuffer[5]==0x40 && aucBuffer[6]==0x00 )
			{
				/* The size of the response must be 0x3a. */
				if( aucBuffer[4]==0x3a )
				{
					/* Is the opcode correct? */
					if( aucBuffer[8]==BTSC_READ_DEV_INFO )
					{
						if( aucBuffer[9]==0x00 )
						{
							iResult = 0;
						}
						else
						{
							uprintf("The status of the response is not OK.\n");
						}
					}
					else
					{
						uprintf("This is no response to the READ_DEV_INFO command.\n");
					}
				}
				else
				{
					uprintf("The payload size is not 0x3a.\n");
					hexdump(aucBuffer, 8);
				}
			}
			else
			{
				uprintf("The response is no 'command complete' event.\n");
				hexdump(aucBuffer, 8);
			}
		}
		else
		{
			uprintf("The CRC of the received header does not match.\n");
			hexdump(aucBuffer, 8);
		}
		}
	else
	{
		uprintf("No SYNC words found.\n");
	}

	return iResult;
}



/*----------------------------------------------------------------------------*/
/*! Poll for data from UART until timeout.
 * @return Amount of bytes received. */
static unsigned int prvUartReceive(uint8_t* pucBuf, unsigned int sizBuf, unsigned long ulTimeout)
{
	HOSTDEF(ptUartArea);
	TIMER_HANDLE_T tTimer;
	int iIsElapsed;
	unsigned long ulValue;
	unsigned int  uiCnt;


	iIsElapsed = 0;
	uiCnt = 0;

	systime_handle_start_ms(&tTimer, ulTimeout);
	do
	{
		/* Is data in the FIFO? */
		ulValue = ptUartArea->ulUartfr;
		ulValue &= HOSTMSK(uartfr_RXFE);
		if( ulValue==0U )
		{
			pucBuf[uiCnt] = (unsigned char)((ptUartArea->ulUartdr) & 0xffU);
			++uiCnt;
		}
		else
		{
			/* Timer elapsed? */
			iIsElapsed = systime_handle_is_elapsed(&tTimer);
			if( iIsElapsed!=0 )
			{
				break;
			}
		}
	} while( uiCnt<sizBuf );

	return uiCnt;
}



/*! Transmit data over UART.
 * \return Amount of bytes transmitted. */
static void prvUartTransmit(const uint8_t* pucBuf, unsigned int sizBuf)
{
	unsigned int sizCnt;


	for(sizCnt=0; sizCnt<sizBuf; ++sizCnt)
	{
		uart_put(UART_UNIT, pucBuf[sizCnt]);
	}
}



/* Second stage bootloader command opcodes. */
#define BTSC_DFU_ACK                    ( 0x79 )
#define BTSC_DFU_NACK                   ( 0x1F )
#define BTSC_DFU_OPCODE_ERASE           ( 0x43 )
#define BTSC_DFU_OPCODE_WRITE           ( 0x31 )
#define BTSC_DFU_OPCODE_GO              ( 0x21 )

#define BTSC_DFU_DEVINFO_PAGE_IDX       ( 115 ) // 116-th page
#define BTSC_DFU_DEVINFO_PAGE_OFFSET    (   8 )
#define BTSC_DFU_DEVINFO_ADDR           ( 0x1007D800 )



/* \return 0 if command is acknowledged; otherwise -1. */
static void prvSendDfuCmdOpcode(uint8_t opcode)
{
	uint8_t obuf[2];


	obuf[0] =  opcode;
	obuf[1] = (uint8_t)(~opcode);

	/* send DFU command opcode. */
	prvUartTransmit(obuf, sizeof(obuf));
}



static void prvSendDfuCmdParameters(const uint8_t* pucParam, unsigned int sizParam)
{
	uint8_t ucChecksum;
	unsigned int sizCnt;


	ucChecksum = 0;
	for(sizCnt = 0; sizCnt<sizParam; ++sizCnt)
	{
		ucChecksum ^= pucParam[sizCnt];
	}

	prvUartTransmit(pucParam, sizParam);
	prvUartTransmit(&ucChecksum, 1);
}



static int prvWaitForAck(void)
{
	int iResult;
	uint8_t ibuf[1] = {0};
	unsigned int ilen;


	iResult = -1;

	/* Wait for ACK. */
	ilen = prvUartReceive(ibuf, sizeof(ibuf), 50);
	if( ilen==1 && ibuf[0]==BTSC_DFU_ACK )
	{
		iResult = 0;
	}

	return iResult;
}



static TEST_RESULT_T readDeviceinfo(unsigned char *pucBuffer)
{
	TEST_RESULT_T tResult;
	int iResult;


	/* Be pessimistic. */
	tResult = TEST_RESULT_ERROR;

	send_command(BTSC_READ_DEV_INFO);
	iResult = receive_device_info(50);
	if( iResult!=0 )
	{
		uprintf("Failed to receive the device info.\n");
	}
	else
	{
		hexdump(aucBuffer, 0x42);
		/* The device info starts at offset 0x0a. Before it there is
		 * the header of the "BTSC_COMMAND_COMPLETE" event, which is...
		 *   8 bytes header
		 *   1 byte opcode triggering this event
		 *   1 byte BTSC error code
		 * ---------------------------------------
		 *  10 bytes
		 * The complete header was already evaluated in the "receive_device_info" function.
		 *
		 * The data after the header is...
		 *  16 bytes firmware version string
		 *  16 bytes hardware version string
		 *  16 bytes bluetooth LE serial number
		 *      0- 3: chip identifier
		 *      4-11: unique device serial number
		 *   8 bytes: Bluetooth public MAC address
		 * ---------------------------------------
		 *  56 bytes (0x38)
		 *
		 * All information was found here: https://kb.hilscher.com/x/7KF_Bg
		 */
		memcpy(pucBuffer, aucBuffer+0x0a, 0x38);
		tResult = TEST_RESULT_OK;
	}

	return tResult;
}



/* TODO: Write new MAC address if required.
 * \note length of the given MAC address must be 6 octets.  */
static TEST_RESULT_T prvUpdateMacAddress(const unsigned char *pucMac)
{
	unsigned int sizReceived;
	int iResult;
	TEST_RESULT_T tResult;
	unsigned int uiParamCount;
	unsigned char aucParam[16];

	/* Be pessimistic. */
	tResult = TEST_RESULT_ERROR;

	/* Read the device info and receive the response.
	 * We expect about 66 bytes which should be transfered at 115.2k in
	 * about 4.6ms . Provide some more time for the device to react.
	 */
	send_command(BTSC_READ_DEV_INFO);
	iResult = receive_device_info(50);
	if( iResult!=0 )
	{
		uprintf("Failed to receive the device info.\n");
	}
	else
	{
		/* Is the MAC in the device equal to the one to set?
		 * memcmp returns 0 if the MAC is the same.
		 */
		uprintf("MAC in device:\n");
		hexdump(aucBuffer+0x3a, 6);

		iResult = memcmp(aucBuffer+0x3a, pucMac, 6);
		if( iResult==0 )
		{
			uprintf("No need to update the device.\n");
			tResult = TEST_RESULT_OK;
		}
		else
		{
			uprintf("Enter second stage bootloader\n");
			send_command(BTSC_DFU_MODE_REQUEST);
			/* NOTE: do not use prvWaitForAck here. There is a 0x00 byte before the ACK.
			 *       But the last byte must be an ACK.
			 */
			sizReceived = prvUartReceive(aucBuffer, sizeof(aucBuffer), 100);
			if( sizReceived==0 )
			{
				uprintf("Nothing received after enter 2nd stage bootloader.\n");
			}
			else if( aucBuffer[sizReceived-1]!=BTSC_DFU_ACK )
			{
				uprintf("The last byte in the buffer is no ACK.\n");
				uprintf("Received:\n");
				hexdump(aucBuffer, sizReceived);
			}
			else
			{
				/*------- Erase configuration page. ----------------*/
				uprintf("Erase configuration page\n");

				/* Send command opcode. */
				prvSendDfuCmdOpcode(BTSC_DFU_OPCODE_ERASE);
				iResult = prvWaitForAck();
				if( iResult!=0 )
				{
					uprintf("No ACK received for erase opcode.\n");
				}
				else
				{
					/* Send command parameter. */
					uiParamCount = 0;
					aucParam[uiParamCount++] = 1;  // number of flash pages to erase
					aucParam[uiParamCount++] = BTSC_DFU_DEVINFO_PAGE_OFFSET + BTSC_DFU_DEVINFO_PAGE_IDX; /* index of configuration page. */

					prvSendDfuCmdParameters(aucParam, uiParamCount);
					iResult = prvWaitForAck();
					if( iResult!=0 )
					{
						uprintf("No ACK received for erase parameter.\n");
					}
					else
					{
						/*------- Write MAC address. ----------------*/
						uprintf("Write MAC address\n");
						/* Send command opcode. */
						prvSendDfuCmdOpcode(BTSC_DFU_OPCODE_WRITE);
						iResult = prvWaitForAck();
						if( iResult!=0 )
						{
							uprintf("No ACK received for write opcode.\n");
						}
						else
						{
							/* Send command parameter: address to write */
							uint32_t ulDevInfoAddress = BTSC_DFU_DEVINFO_ADDR;

							uiParamCount = 0;
							aucParam[uiParamCount++] = (uint8_t)(ulDevInfoAddress >> 24U);
							aucParam[uiParamCount++] = (uint8_t)(ulDevInfoAddress >> 16U);
							aucParam[uiParamCount++] = (uint8_t)(ulDevInfoAddress >>  8U);
							aucParam[uiParamCount++] = (uint8_t)ulDevInfoAddress;

							prvSendDfuCmdParameters(aucParam, uiParamCount);
							iResult = prvWaitForAck();
							if( iResult!=0 )
							{
								uprintf("No ACK received for write parameter.\n");
							}
							else
							{
								/* Write MAC address. */
								uiParamCount = 0;
								aucParam[uiParamCount++] = 8; /* Amount of following data. It must be multiple of 4. */
								aucParam[uiParamCount++] = pucMac[0];
								aucParam[uiParamCount++] = pucMac[1];
								aucParam[uiParamCount++] = pucMac[2];
								aucParam[uiParamCount++] = pucMac[3];
								aucParam[uiParamCount++] = pucMac[4];
								aucParam[uiParamCount++] = pucMac[5];
								aucParam[uiParamCount++] = 0xFF;
								aucParam[uiParamCount++] = 0xFF;

								prvSendDfuCmdParameters(aucParam, uiParamCount);
								iResult = prvWaitForAck();
								if( iResult!=0 )
								{
									uprintf("No ACK received for write data.\n");
								}
								else
								{
									/*------- Send command opcode. ----------------*/
									prvSendDfuCmdOpcode(BTSC_DFU_OPCODE_GO);
									iResult = prvWaitForAck();
									if( iResult!=0 )
									{
										uprintf("No ACK received for go opcode.\n");
									}
									else
									{
										/* Address of application entry function: 0x10044104 */
										uint32_t ulAppEntryAddress = 0x10044104;

										uiParamCount = 0;
										aucParam[uiParamCount++] = (uint8_t)(ulAppEntryAddress >> 24U);
										aucParam[uiParamCount++] = (uint8_t)(ulAppEntryAddress >> 16U);
										aucParam[uiParamCount++] = (uint8_t)(ulAppEntryAddress >>  8U);
										aucParam[uiParamCount++] = (uint8_t) ulAppEntryAddress;

										prvSendDfuCmdParameters(aucParam, uiParamCount);
										iResult = prvWaitForAck();
										if( iResult!=0 )
										{
											uprintf("No ACK received for go parameter.\n");
										}
										else
										{
											uprintf("App init\n");
											sizReceived = prvUartReceive(aucBuffer, sizeof(aucBuffer), 100);
											if( sizReceived!=0 )
											{
												hexdump(aucBuffer, sizReceived);
											}

											/* Read current MAC. */
											uprintf("Read device info\n");
											send_command(BTSC_READ_DEV_INFO);
											iResult = receive_device_info(50);
											if( iResult!=0 )
											{
												uprintf("Failed to receive the device info.\n");
											}
											else
											{
												/* Is the MAC in the device equal to the one to set?
												 * memcmp returns 0 if the MAC is the same.
												 */
												uprintf("Updated MAC in device:\n");
												hexdump(aucBuffer+0x3a, 6);

												iResult = memcmp(aucBuffer+0x3a, pucMac, 6);
												if( iResult!=0 )
												{
													uprintf("The MAC update failed!\n");
												}
												else
												{
													tResult = TEST_RESULT_OK;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return tResult;
}



TEST_RESULT_T test(BT_PARAMETER_T *ptParameter)
{
	TEST_RESULT_T tResult;
	BT_COMMAND_T tCommand;


	systime_init();

	if( ptParameter==NULL )
	{
		uprintf("Bluetooth ID " VERSION_ALL "\n");
		tResult = TEST_RESULT_OK;
	}
	else
	{
		tCommand = ptParameter->tCommand;

		tResult = TEST_RESULT_ERROR;
		switch( tCommand )
		{
		case BT_COMMAND_ReadDeviceInfo:
		case BT_COMMAND_UpdateMAC:
			tResult = TEST_RESULT_OK;
			break;
		}
		if( tResult!=TEST_RESULT_OK )
		{
			uprintf("Invalid command: 0x%08x\n", tCommand);
		}
		else
		{
			uart_init(UART_UNIT, &tUartCfg);

			switch( tCommand )
			{
			case BT_COMMAND_ReadDeviceInfo:
				tResult = readDeviceinfo(ptParameter->uData.tReadDeviceInfo.aucData);
				break;

			case BT_COMMAND_UpdateMAC:
				tResult = prvUpdateMacAddress(ptParameter->uData.tUpdateMac.aucMAC);
				break;
			}
		}
	}

	return tResult;
}
