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

#define CAN_MSG_BUF_SIZE		32
#define CAN_TX_BUF_SIZE			6

static tCANMsgObject g_txMsgBuf[CAN_TX_BUF_SIZE];
static tCANMsgObject g_rxMsgBuf[CAN_MSG_BUF_SIZE - CAN_TX_BUF_SIZE];


void _can_rxIntHandler() {

}


void _can_txIntHandler() {

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
	CANIntRegister(CAN_TX_IFACE, _can_txIntHandler);

	// Bind message objects
	for(int i=0; i<CAN_TX_BUF_SIZE; i++) {
		CANMessageSet(CAN_TX_IFACE, 1, &g_txMsgBuf[i], eMsgType);
	}
}







