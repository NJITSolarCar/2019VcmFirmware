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
#include <driverlib/interrupt.h>

#include <inc/hw_ints.h>

#include "../util.h"
#include "../fault.h"
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"
#include "../cvnp/cvnp_config.h"
#include "../hal/resource.h"

#define CAN_RX_BUF_SIZE			6
#define CAN_MAX_BUF_SIZE		8

// Message buffers
static uint8_t g_rxBuf[8], g_txBuf[8];

static tCANMsgObject g_txMsg;
static tCANMsgObject g_rxMsg;



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
 * Returns a singular interrupt bit, or 0 if none
 * are set. Will select the lowest bit set
 */
uint32_t _can_getLowestInt(uint32_t intStat) {
	for(int i=0; i<32; i++) {
		if(intStat & 0x1)
			return 1 << i;
		intStat >>= 1;
	}
	return 0;
}



void _can_rxIntHandler() {
	// Get the cause
	uint32_t intStat = CANIntStatus(CAN_RX_IFACE, CAN_INT_STS_CAUSE);

	if(intStat > 32) { // For status / error / tx done interrupts
		// Clear the error interrupts
		intStat = CANStatusGet(CAN_RX_IFACE, CAN_STS_CONTROL);
		if(intStat == CAN_STATUS_TXOK) {
			// Do nothing, we don't care about TX interrupts
		} else {
			// TODO: handle a CAN bus error
		}
	} else {
		// Process the frame
		tCanFrame frame;

		// Select a single frame interrupt to handle
		intStat = _can_getLowestInt(intStat);
		CANMessageGet(CAN_RX_IFACE, intStat, &g_rxMsg, true);
		CANIntClear(CAN_RX_IFACE, intStat);

		// Process the frame
		_can_tivaFrameToCVNP(&frame, &g_rxMsg);
		cvnp_procFrame(&frame);
	}
}



void _can_txIntHandler() {
	// The only interrupts here should be errors, so flag a fault
	// TODO: flag a fault
	uint32_t intStat = CANIntStatus(CAN_TX_IFACE, CAN_INT_STS_CAUSE);
	CANIntClear(CAN_TX_IFACE, intStat);
}





/**
 * Initializes the CAN hardware
 */
bool cvnpHal_init() {
	// Setup the GPIO as CAN, and turn off standby
	GPIOPinTypeGPIOOutput(CAN_STB_PORT, CAN_STB_PIN);
	GPIOPinWrite(CAN_STB_PORT, CAN_STB_PIN, 0x0);

	GPIOPinConfigure(CAN_TX_PINCONFIG);
	GPIOPinConfigure(CAN_RX_PINCONFIG);
	GPIOPinTypeCAN(CAN_TX_PORT, CAN_TX_PIN);
	GPIOPinTypeCAN(CAN_RX_PORT, CAN_RX_PIN);

	// Initialize H/W
	CANInit(CAN_RX_IFACE);
	CANBitRateSet(CAN_RX_IFACE, UTIL_CLOCK_SPEED, CVNP_BITRATE);
	CANIntRegister(CAN_RX_IFACE, _can_rxIntHandler);

	// Bind RX data buffer
	g_txMsg.pui8MsgData = g_txBuf;
	g_txMsg.ui32Flags = MSG_OBJ_NO_FLAGS;
	g_txMsg.ui32MsgID = 0;
	g_txMsg.ui32MsgIDMask = 0;

	g_rxMsg.pui8MsgData = g_rxBuf;
	g_rxMsg.ui32MsgIDMask = 0x0;
	g_rxMsg.ui32MsgLen = 8;
	g_rxMsg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;

	// Use a singular mailbox for RX for each standard and extended ID for now
	CANMessageSet(CAN_RX_IFACE, 1, &g_rxMsg, MSG_OBJ_TYPE_RX);

	g_rxMsg.ui32Flags |= MSG_OBJ_EXTENDED_ID;
	CANMessageSet(CAN_RX_IFACE, 2, &g_rxMsg, MSG_OBJ_TYPE_RX);


	// Prepare the RX buffer
	/*for(int i=0; i<CAN_MAX_BUF_SIZE; i++) {
		CANMessageSet(CAN_RX_IFACE,
					  i+1,
					  &g_rxMsg,
					  MSG_OBJ_TYPE_RX);

		// Make half of the messages take extended IDs, with the other half
		// regular
		if(i == 8)
			g_rxMsg.ui32Flags |= MSG_OBJ_EXTENDED_ID;
	}*/

	// Prepare the TX buffer. Sits in the second half of the mailbox
	for(int i=CAN_MAX_BUF_SIZE; i<2*CAN_MAX_BUF_SIZE; i++) {
			CANMessageSet(CAN_RX_IFACE,
						  i+1,
						  &g_txMsg,
						  MSG_OBJ_TYPE_TX);
		}

	// Enable the CAN bus
	CANIntEnable(CAN_RX_IFACE, CAN_INT_ERROR | CAN_INT_MASTER | CAN_INT_STATUS);
	IntEnable(INT_CAN0);
	CANEnable(CAN_RX_IFACE);

	return true;
}





void cvnpHal_sendFrame(tCanFrame frame) {
	_can_cvnpFrameToTiva(&frame, &g_txMsg);

	// Find highest clear message ID
	uint32_t txStat = CANStatusGet(CAN_RX_IFACE, CAN_STS_TXREQUEST);

	int i=CAN_MAX_BUF_SIZE+1;
	while(i<=2*CAN_MAX_BUF_SIZE && ((txStat >> i) & 1))
		i++;

	if(i>= 2*CAN_MAX_BUF_SIZE) {
		tFaultData dat;
		// TODO: Add proper data for this fault
		dat.ui64 = 0;
		fault_assert(FAULT_VCM_COMM, dat);
	}

	// CAN Message objects are 1 indexed, not 0 indexed
	CANMessageSet(CAN_RX_IFACE, i, &g_txMsg, MSG_OBJ_TYPE_TX);
}





uint32_t cvnpHal_now() {
	return util_msTimestamp();
}




void cvnpHal_handleError(uint32_t errNum) {
	// TODO: assert fault
}




void cvnpHal_resetSystem() {
	// TODO: find a safe way to reset the system
}





