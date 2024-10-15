/*
	Program	usbmididrv.c
	Date	2012/04/30 .. 2012/05/01	(PSoC1)
	Copyright (C) 2012 by AKIYA
	--- up date ---
*/
#include "usbmididrv.h"
#include "circularbuffer.h"
#include "UART.h"
#include "USBFS.h"

/* --- for USB IN/OUT --- */
#define IN_EP			(2)
#define OUT_EP			(1)
#define EP_DATA_SIZE	(64)
#define EP_DATA_COUNT	(EP_DATA_SIZE/4)

static unsigned long in_buf[EP_DATA_COUNT];
static unsigned char in_buf_wc = 0;
static unsigned long out_buf[EP_DATA_COUNT];

/* --- for UART IN/OUT --- */
#define UART_TX_BUF_SIZE	(128)
#define UART_RX_BUF_SIZE	(128)

static unsigned char tx_buf[UART_TX_BUF_SIZE];
static unsigned char rx_buf[UART_RX_BUF_SIZE];

static SCIRCULARBUFFER sTx = {0,0,UART_TX_BUF_SIZE-1,tx_buf};
static SCIRCULARBUFFER sRx = {0,0,UART_RX_BUF_SIZE-1,rx_buf};

static unsigned char txIntEnable = 0;

/* --- initialize --- */
void usbmididrv_ini(void)
{
	USBFS_Start(0, USB_5V_OPERATION); //Start USBFS Operation using device 0 at 5V
	while(!USBFS_bGetConfiguration()); //Wait for Device to enumerate
	USBFS_EnableOutEP(OUT_EP);

	UART_IntCntl(UART_ENABLE_RX_INT|UART_DISABLE_TX_INT);
	UART_Start(UART_PARITY_NONE);
}

/* --- Transmit To PC --- */
short TransmitUsbMidiIn(unsigned long lData)
{
	static unsigned char toggle = USB_NO_TOGGLE;

	if (in_buf_wc == EP_DATA_COUNT) {
		if (USBFS_bGetEPState(IN_EP) != IN_BUFFER_EMPTY) {
			return -1;
		}
		USBFS_LoadInEP(IN_EP, (unsigned char *)in_buf, EP_DATA_SIZE, toggle);
		toggle = USB_TOGGLE;
		in_buf_wc = 0;
	}

	in_buf[in_buf_wc++] = lData;

	if (USBFS_bGetEPState(IN_EP) == IN_BUFFER_EMPTY) {
		while (in_buf_wc < EP_DATA_COUNT) {
			in_buf[in_buf_wc++] = 0;
		}
		USBFS_LoadInEP(IN_EP, (unsigned char *)in_buf, EP_DATA_SIZE, toggle);
		toggle = USB_TOGGLE;
		in_buf_wc = 0;
	}

	return 0;
}

/* --- Receive from PC --- */
unsigned char ReceiveUsbMidiOut(unsigned long **buf)
{
	unsigned char count;

	if (USBFS_bGetEPState(OUT_EP) == OUT_BUFFER_FULL) {
		count = USBFS_bReadOutEP(OUT_EP, (unsigned char *)out_buf, EP_DATA_SIZE);
		USBFS_EnableOutEP(OUT_EP);
	}
	else {
		count = 0;
	}
	if (buf) {
		*buf = out_buf;
	}
	return count/4;	// /4 for long count
}

/* --- get uart receive data --- */
short get_uart_rx_buf(void)
{
	return getbuf(&sRx);
}

/* --- put uart transmit data --- */
short put_uart_tx_buf(unsigned char data)
{
	short ret = putbuf(&sTx, data);

	if (ret == 0) {
		if (txIntEnable == 0) {
			short d = getbuf(&sTx);

			if (d >= 0) {
				txIntEnable = 1;
				UART_IntCntl(UART_ENABLE_RX_INT|UART_ENABLE_TX_INT);
				UART_SendData(d);
			}
		}
	}
	return ret;
}

/* --- uart rx interrupt routine --- */
void uart_rx_int(void)
{
	unsigned char data;

	data = UART_bReadRxData();
	if (UART_bReadRxStatus() & UART_RX_ERROR) {
		// error !
	}
	else {
		if (putbuf(&sRx, data) != 0) {
			// buffer full !
		}
	}

	return;
}

/* --- uart tx interrupt routine --- */
void uart_tx_int(void)
{
	short data;

	data = getbuf(&sTx);
	if (data >= 0) {
		UART_SendData(data);
	}
	else {
		UART_IntCntl(UART_ENABLE_RX_INT|UART_DISABLE_TX_INT);
		txIntEnable = 0;
	}

	return;
}
