/*
 * bms.c
 *
 *	Primary BMS interface drivers for the VCM. These routines are
 *	responsible for all interractions between the BMS and VCM,
 *	including frame transmission, parsing, etc. The goal of this
 *	code is to make all of this information available to
 *	the main VCM driver, which can in turn extract it and communicate
 *	it over the network.
 *
 *  Created on: Mar 22, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>
#include "bms.h"
#include "../fault.h"
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"

// Current bms state data
static tBMSData g_bmsData;

/**
 * Timeout routine to bind to the CVNP reader frames. Will be called
 * when a timeout occurs for any of the CAN frames. This will assert
 * a BMS communication fault
 */
void _bms_cvnpOnTimeout(bool wasKilled) {
	tFaultData data;
	data.ui64 = wasKilled;
	fault_assert(FAULT_BMS_COMM, data);
}


/**
 * Frame 0. Contains:
 *  - Checksum (unused at the moment)
 *  - Pack current (0.1A)
 *  - Pack voltage (0.1V)
 *  - Pack SOC (0.5%)
 *  - Charging supply voltage (0.1V)
 */
void _bms_parseFrame0(tCanFrame *frame) {
	uint16_t rawCurrent = frame->data[2] << 8;
	rawCurrent |= frame->data[3];
	g_bmsData.iBat = ((float) rawCurrent) * 0.1f;
	g_bmsData.vBat = ((float)frame->data[4]) * 0.1f;
	g_bmsData.soc = ((float)frame->data[5]
}




/**
 * Frame 1. Contains:
 *  - Relay State (bit packed byte)
 *  - High cell voltage (mV * 10)
 *  - Highest cell index
 *  - Low cell voltage (mV * 10)
 *  - Lowest cell index
 *  - Average cell voltage (mV * 10)
 */
void _bms_parseFrame1(tCanFrame *frame) {

}





/**
 * Frame 2. Contains:
 *  - Custom flags (bit packed word)
 *  - High temperature thermistor
 *  - Highest thermistor ID
 *  - High temperature thermistor
 *  - Highest thermistor ID
 *  - Average temperature thermistor
 */
void _bms_parseFrame2(tCanFrame *frame) {

}



/**
 * Initializes the BMS driver. Binds all of the BMS handlers to
 * the CVNP api
 */
void bms_init() {
	// Temporary handler structure
	tNonCHandler tmpHdl;

	// Bind handlers. Many of the parameters stay the same
	// TODO: track any failed inserts to assert an error
	// Frame 0
	tmpHdl.id = BMS_ID_BASE;
	tmpHdl.lastRun = 0;
	tmpHdl.timeout = BMS_FRAME0_TIMEOUT;
	tmpHdl.valid = true;
	tmpHdl.pfnOnDeath = _bms_cvnpOnTimeout;
	tmpHdl.pfnProcFrame = _bms_parseFrame0;
	cvnp_registerNonCHandler(&tmpHdl);

	// Frame 1
	tmpHdl.id = BMS_ID_BASE + 1;
	tmpHdl.timeout = BMS_FRAME1_TIMEOUT;
	tmpHdl.pfnProcFrame = _bms_parseFrame1;
	cvnp_registerNonCHandler(&tmpHdl);

	// Frame 2
	tmpHdl.id = BMS_ID_BASE + 2;
	tmpHdl.timeout = BMS_FRAME2_TIMEOUT;
	tmpHdl.pfnProcFrame = _bms_parseFrame2;
	cvnp_registerNonCHandler(&tmpHdl);
}


/**
 * Tick routine to be run every ~10ms. This is where this api can
 * periodically query, check stuff, etc.
 *
 *	At the moment, nothing in here needs to be ticked, but the function
 *	is kept here for compliance with other device classes.
 *
 * \param now the current millisecond timestamp
 */
void bms_tick(uint32_t now) {

}



/**
 * Retrieves the instantaneous data for the BMS.
 */
tBMSData *bms_data() {
	return &g_bmsData;
}







