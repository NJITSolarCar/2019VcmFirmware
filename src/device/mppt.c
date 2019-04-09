/*
 * mppt.c
 *
 * Base driver for MPPT communication
 *
 *  Created on: Mar 29, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"
#include "mppt.h"
#include "../fault.h"

static tMpptData g_mpptDat[MPPT_NUM_MPPT];

void _mppt_getResponseFaults(uint32_t mpptNum, tMpptData* dat)
{
	tFaultData fDat;
	fDat.pui32[0] = mpptNum;

	// Check battery charged
	if (dat->bvlr)
	{
		fDat.pfloat[1] = dat->vBat;
		fault_assert(FAULT_MPPT_BATT_CHARGED, fDat);
	}
	// Check battery charged
	if (dat->bvlr)
	{
		fDat.pfloat[1] = dat->vBat;
		fault_assert(FAULT_MPPT_BATT_CHARGED, fDat);
	}
	// Check solar voltage
	if (dat->undv)
	{
		fDat.pfloat[1] = dat->vSolar;
		fault_assert(FAULT_MPPT_LOW_SOLAR_VOLTS, fDat);
	}
	// Check battery connected
	if (dat->noc)
	{
		fault_assert(FAULT_MPPT_NO_BATT, fDat);
	}
	// Assert either a warning, fault, or none for temperature
	if (dat->ovt)
	{
		fDat.pfloat[1] = dat->temp;
		fault_assert(FAULT_MPPT_TEMP, fDat);
	}
	else if (dat->temp > MPPT_TEMP_WARN)
	{
		fDat.pfloat[1] = dat->temp;
		fault_assert(FAULT_MPPT_TEMP_WARN, fDat);
	}
}

/**
 * Parses an MPPT response frame. Will determine which MPPT this is from
 * based on the offset of the ID from the base. If the number is out of
 * range of the expected values, an error will be flagged.
 */
static void _mppt_parseResponse(tCanFrame *frame) {
	if(frame->id & ~0xF != MPPT_BASE_RESPONSE_ID) {
		// TODO: Assert an error, base ID was bad
		return;
	}

	uint32_t mpptNum = (frame->id & 0xF) - 1;
	if(mpptNum > MPPT_NUM_MPPT) {
		// TODO: Assert an error, offset was bad
		return;
	}

	tMpptData *dat = & g_mpptDat[mpptNum]; // utility reference
	uint32_t tmp; // Temporary variable for extracting 12 bit values

	// Extract flags
	dat->bvlr = (frame->data[0] >> MPPT_FLALG_BVLR) & 1;
	dat->ovt = (frame->data[0] >> MPPT_FLALG_OVT) & 1;
	dat->noc = (frame->data[0] >> MPPT_FLALG_NOC) & 1;
	dat->undv = (frame->data[0] >> MPPT_FLALG_UNDV) & 1;

	// Uin - solar voltage
	tmp = ((uint16_t)frame->data[0] & 0x3) << 8;
	tmp |= frame->data[1];
	dat->vSolar = (float)tmp;

	// Iin - solar current
	tmp = ((uint16_t)frame->data[2] & 0x3) << 8;
	tmp |= frame->data[3];
	dat->iSolar = (float)tmp;

	// Uout - battery voltage
	tmp = ((uint16_t)frame->data[4] & 0x3) << 8;
	tmp |= frame->data[5];
	dat->vBat = (float)tmp;

	// Temperature
	dat->temp = (float) (frame->data[6]);

	// Check faults
	_mppt_getResponseFaults(mpptNum, dat, frame);
}


/**
 * Master MPPT timeout routine. Will assert an MPPT communication
 * fault if it was not killed, or a CVNP fault if it was.
 */
static void _mppt_onMpptTimeout(bool wasKilled, uint32_t mpptNum) {
	tFaultData dat;
	dat.pui32[0] = wasKilled;
	dat.pui32[1] = mpptNum;
	fault_assert(FAULT_MPPT_COMM, dat);
}



/**
 * Function to be called by cvnp when a specific MPPT frame hasn't been
 * received in the timeout period. Will delegate to _mppt_onMpptTimeout.
 * For now only 3 of these are declared, but more can be added later if
 * needed.
 */
static void _mppt_onMpptTimeout0(bool wasKilled) { _mppt_onMpptTimeout(wasKilled, 0); }
static void _mppt_onMpptTimeout1(bool wasKilled) { _mppt_onMpptTimeout(wasKilled, 1); }
static void _mppt_onMpptTimeout2(bool wasKilled) { _mppt_onMpptTimeout(wasKilled, 2); }




/**
 * Initialize routine for the MPPTs. Will bind all necessary
 * cvnp handlers for the MPPT module
 */
void mppt_init() {
	tNonCHandler hdl;
	hdl.id = MPPT_BASE_RESPONSE_ID + 1;
	hdl.timeout = MPPT_RX_TIMEOUT;
	hdl.pfnProcFrame = _mppt_parseResponse;

	// For now just bind the first 3 error handlers
	hdl.pfnOnDeath = _mppt_onMpptTimeout0;
	cvnp_registerNonCHandler(&hdl);

	hdl.id++;
	hdl.pfnOnDeath = _mppt_onMpptTimeout1;
	cvnp_registerNonCHandler(&hdl);

	hdl.id++;
	hdl.pfnOnDeath = _mppt_onMpptTimeout2;
	cvnp_registerNonCHandler(&hdl);

}





/**
 * Called periodically to run the mppt queries. Will cycle through all
 * MPPTs
 */
void mppt_tick() {
	static uint32_t count = 0; // count of times that this function was called
	static uint32_t lastMppt = 0; // last MPPT index that was queried
	if(!(count++ % MPPT_NUM_TICKS)) {
		if(++lastMppt >= MPPT_NUM_MPPT) // update which mppt to query
			lastMppt = 0;

		// Frame to send
		tCanFrame frame;
		frame.id = MPPT_BASE_REQUEST_ID + lastMppt + 1;
		frame.head.dlc = 0;
		frame.head.rtr = 0;
		frame.head.ide = 0;

		cvnpHal_sendFrame(frame);
	}
}












