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

static tMpptData g_mpptDat[MPPT_NUM_MPPT];


/**
 * Parses an MPPT response frame. Will determine which MPPT this is from
 * based on the offset of the ID from the base. If the number is out of
 * range of the expected values, an error will be flagged.
 */
static void _parseMpptResponse(tCanFrame *frame) {
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
	dat->temp = (float)frame->data[6];

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












