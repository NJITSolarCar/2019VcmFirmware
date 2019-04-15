/*
 * can_hal.c
 *
 *	Contains implementation for the cvnp_hal system
 *
 *  Created on: Apr 12, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/can.h>
#include <driverlib/gpio.h>

#include "../util.h"
#include "../fault.h"
#include "../cvnp/cvnp_hal.h"
#include "../cvnp/cvnp_config.h"
#include "../hal/resource.h"

#define CAN_RX_BUF_SIZE			6
#define CAN_MAX_BUF_SIZE		32

// Message buffers
static uint8_t g_rxBuf[8], g_txBuf[8];

static tCANMsgObject g_txMsg;
static tCANMsgObject g_rxMsg;


void _can_rxIntHandler() {

}


void _can_txIntHandler() {

}


void _can_tivaFrameToCVNP(tCanFrame *cvnp, tCANMsgObject *tiva) {
	cvnp->id = tiva->ui32MsgID;
	cvnp->head.dlc = tiva->ui32MsgLen;
	cvnp->head.ide = !!(tiva->ui32Flags & MSG_OBJ_EXTENDED_ID);
	cvnp->head.rtr = 0; // For now, RTR is unused

	for(int i=0; i<8; i++)
		cvnp->data[i] = tiva->pui8MsgData[i];
}

void _can_cvnpFrameToTiva(tCanFrame *cvnp, tCANMsgObject *tiva) {
	tiva->ui32MsgID = cvnp->id;
	tiva->ui32MsgLen = cvnp->head.dlc;
	tiva->ui32Flags = cvnp->head.ide ? MSG_OBJ_EXTENDED_ID : 0;

	for(int i=0; i<8; i++)
		tiva->pui8MsgData[i] = cvnp->data[i];
}


/**
 * Initializes the CAN hardware
 */
bool cvnpHal_init() {
	// Setup the GPIO as CAN
	GPIOPinTypeGPIOOutput(CAN_STB_PORT, CAN_STB_PIN);

	GPIOPinConfigure(CAN_TX_PINCONFIG);
	GPIOPinConfigure(CAN_RX_PINCONFIG);
	GPIOPinTypeCAN(CAN_TX_PORT, CAN_TX_PIN);
	GPIOPinTypeCAN(CAN_RX_PORT, CAN_RX_PIN);

	// Initialize H/W
	CANInit(CAN_RX_IFACE);
	CANBitRateSet(CAN_RX_IFACE, UTIL_CLOCK_SPEED, CVNP_BITRATE);
	CANIntRegister(CAN_RX_IFACE, _can_rxIntHandler);
	CANInit(CAN_TX_IFACE);
	CANBitRateSet(CAN_TX_IFACE, UTIL_CLOCK_SPEED, CVNP_BITRATE);

	// Bind TX data buffer
	g_txMsg.pui8MsgData = g_txBuf;

	g_rxMsg.pui8MsgData = g_rxBuf;
	g_rxMsg.ui32MsgIDMask = 0x0;
	g_rxMsg.ui32MsgLen = 8;
	g_rxMsg.ui32Flags = MSG_OBJ_RX_INT_ENABLE; // TODO: possibly need MSG_OBJ_EXTENDED_ID, but documentation is inconclusive


	for(int i=0; i<CAN_RX_BUF_SIZE; i++) {
		CANMessageSet(CAN_RX_IFACE,
					  i+1,
					  &g_rxMsg,
					  MSG_OBJ_TYPE_RX);
	}

	return true;
}





void cvnpHal_sendFrame(tCanFrame frame) {
	_can_cvnpFrameToTiva(&frame, &g_txMsg);

	// Find highest clear message ID
	uint32_t txStat = CANStatusGet(CAN_TX_IFACE, CAN_STS_TXREQUEST);

	int i=0;
	while(i<32 && ((txStat >> i) & 1))
		i++;

	if(i>= 32) {
		// TODO: Buffer full, possible VCM comm error
	}

	CANMessageSet(CAN_TX_IFACE, i, &g_txMsg, MSG_OBJ_TYPE_TX);
}









