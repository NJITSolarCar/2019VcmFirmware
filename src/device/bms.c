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
 * COnverts a BMS flag 0 byte into a tBmsFlag0 structure
 */
static tBmsFlag0 _bms_byteToFlag0(uint8_t b) {
	tBmsFlag0 f;
	f.rlyChgFault = b & 1;
	f.rlyDisFault = (b >> 1) & 1;
	f.cellOVFault = (b >> 2) & 1;
	f.cellUVFault = (b >> 3) & 1;
	f.thermistorFault = (b >> 4) & 1;
	f.overTempFault = (b >> 5) & 1;
	// Skip bit 6, we don't need heatsink thermistor fault
	f.cellOpenWiringFault = (b >> 7) & 1;

	return f;
}




/**
 * COnverts a BMS flag 4 byte into a tBmsFlag0 structure
 */
static tBmsFlag4 _bms_byteToFlag4(uint8_t b) {
	tBmsFlag4 f;
	f.heatsinkThermistorFault = b & 1;
	f.weakCellFault = (b >> 1) & 1;
	f.currentSenseFault = (b >> 2) & 1;
	f.isolationFault = (b >> 3) & 1;
	f.internalCellCommFault = (b >> 4) & 1;
	f.internalLogicFault = (b >> 5) & 1;
	f.internalHardwareFault = (b >> 6) & 1;

	return f;
}




/**
 * Timeout routine to bind to the CVNP reader frames. Will be called
 * when a timeout occurs for any of the CAN frames. This will assert
 * a BMS communication fault
 */
static void _bms_cvnpOnTimeout(bool wasKilled) {
	tFaultData data;
	data.ui64 = wasKilled;
	fault_assert(FAULT_BMS_COMM, data);
}


/**
 * Frame 0. Contains:
 *  - Flag 0
 *  - Pack voltage (0.1V)
 *  - Relay state byte
 *  - Pack current (0.1A)
 *  - Average Cell Voltage (0.1mV)
 */
static void _bms_parseFrame0(tCanFrame *frame) {
	g_bmsData.flag0 = _bms_byteToFlag0(frame->data[0]);

	// Get pack voltage
	uint16_t tmp = ((uint16_t)frame->data[1]) << 8;
	tmp |= frame->data[2];
	g_bmsData.vBat = ((float) tmp) * 0.1f;

	// Extract the relay information we care about.
	// Is contained in a custom flag with the first 2 bits
	// set to the discharge and charge relay states, respectively
	g_bmsData.rlyDis = frame->data[3] & 1;
	g_bmsData.rlyChg = (frame->data[3] >> 1) & 1;

	// Get current
	tmp = ((uint16_t)frame->data[4]) << 8;
	tmp |= frame->data[5];
	g_bmsData.iBat = ((float) tmp) * 0.1f;

	// Get Avg. cell voltage
	tmp = ((uint16_t)frame->data[6]) << 8;
	tmp |= frame->data[7];
	g_bmsData.iBat = ((float) tmp) * 1E-4f;
}




/**
 * Frame 1. Contains:
 *  - Lowest Temperature
 *  - Lowest Temperature thermistor index
 *  - Highest Temperature
 *  - Highest Temperature thermistor index
 *  - Custom Flag 4
 *  - Pack SoC
 *  - Low Cell Voltage Index
 *  - High Cell Voltage Index
 */
static void _bms_parseFrame1(tCanFrame *frame) {
	g_bmsData.tMinIdx = frame->data[1];
	g_bmsData.tMaxIdx = frame->data[3];
	g_bmsData.tMin = frame->data[0];
	g_bmsData.tMax = frame->data[2];
	g_bmsData.flag4 = _bms_byteToFlag4(frame->data[4]);
	g_bmsData.soc = ((float) frame->data[5]) * 0.5f;
	g_bmsData.vMinIdx = frame->data[6];
	g_bmsData.vMaxIdx = frame->data[7];
}





/**
 * Cell Broadcast. Contains:
 *  - Cell Index
 *  - Cell Instantaneous Voltage (0.1mV)
 *  - Cell Resistance (0.01mOhm)
 *  - Cell Open Voltage (0.1mV)
 */
static void _bms_parseCellBroadcast(tCanFrame *frame) {
	tCellData *dat = &g_bmsData.cellData[frame->data[0]];

	// Instantaneous voltage
	uint16_t tmp = ((uint16_t)frame->data[1]) << 8;
	tmp |= frame->data[2];
	dat->voltage = ((float)tmp) * 1E-4f;

	// Resistance
	tmp = ((uint16_t)frame->data[3] & 0x7F) << 8; // Mask the MSB, used to indicate a different condition
	tmp |= frame->data[4];
	dat->voltage = ((float)tmp) * 1E-5f;
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

	// Cell Broadcast
	tmpHdl.id = BMS_ID_BASE + 2;
	tmpHdl.timeout = BMS_CELL_BROAD_TIMEOUT;
	tmpHdl.pfnProcFrame = _bms_parseCellBroadcast;
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







