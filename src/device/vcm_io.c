/*
 * vcm_io.c
 *
 * Primary set of drivers for the VCM in terms of I/O. This provides
 * the external API of the VCM itself, particularly over CAN. This is not
 * so much the place for program flow control, structure, and algorithm. That
 * should be resident in vcm.c.
 *
 *  Created on: Apr 3, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>
#include <machine/endian.h>

#include "bms.h"
#include "ina225.h"
#include "indicator.h"
#include "kbl.h"
#include "mppt.h"
#include "vcm_io.h"

#include "../fault.h"
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"

#include "../util.h"

/**
 * DDEF handler 16. This transmits a fault summary of the VCM.
 */
bool _vcmio_cvnp_ddef16(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	uint64_t fSum = fault_getFaultSummary();
	*pLen = 8;
	for(int i=0; i<8; i++) {
		// Right shift fSum by 8*i, and mask the last byte
		pData[i] = (fSum >> (i<<3)) & 0xFF;
	}

	return true;
}


/**
 * DDEF handler 17. Given a fault index, returns the time it was last
 * asserted
 */
bool _vcmio_cvnp_ddef17(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	if(frame->head.dlc != 4) {
		return cvnp_errorFrameHandler(frame, pLen, pData);
	}

	// Extract the fault's ID
	uint32_t faultId;
	UTIL_BYTEARR_TO_INT(frame->data, faultId)

	// Extract the timestamp for that fault
	uint64_t fTime = fault_getFaultTime(faultId);
	UTIL_INT_TO_BYTEARR(pData, fTime);

	*pLen = 8;
	return true;

}



/**
 * DDEF handler 18. Given a fault index, returns the data from the fault
 */
bool _vcmio_cvnp_ddef18(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	if(frame->head.dlc != 4) {
		return cvnp_errorFrameHandler(frame, pLen, pData);
	}

	// Extract the fault's ID
	uint32_t faultId;
	UTIL_BYTEARR_TO_INT(frame->data, faultId);

	// Extract the timestamp for that fault
	uint64_t fDat = fault_getFaultData(faultId).ui64;
	UTIL_INT_TO_BYTEARR(pData, fDat);

	*pLen = 8;
	return true;

}



/**
 * DDEF 41	PACK_PWR_SUM
 * Request: There are no specific requirements to request this frame.
 * Response: A set of values containing summary statistics about power distribution and flow inside the battery pack.
 *
 */
bool _vcmio_cvnp_ddef41(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {

	tBMSData *bmsDat = bms_data();
	uint16_t tmp;

	pData[0] = (uint8_t)bmsDat->soc;

	// Copy pack current
	tmp = (uint16_t)bmsDat->iBat;
	UTIL_INT_TO_BYTEARR(pData+1, tmp);

	// Copy pack voltage
	tmp = (uint16_t)bmsDat->iBat;
	UTIL_INT_TO_BYTEARR(pData+1, tmp);
	return true;

}



/**
 * Initializes the vcm I/O system. This means just setting up internal
 * communications and CAN infrastructure. This doesn't include CVNP
 * initialization, but it should include binding different handlers
 * and other implementation specific details.
 */
void vcmio_init() {
	// Initialize CVNP
}
