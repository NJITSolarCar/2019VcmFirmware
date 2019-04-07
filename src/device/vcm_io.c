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
 *
 * Request: There are no specific requirements to request this frame.
 *
 * Response: A set of values containing summary statistics about power
 *  distribution and flow inside the battery pack.
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
 * 3.2.42	DDEF 42	PACK_TEMP_SUM
 *
 * Request: There are no specific requirements to request this frame.
 *
 * Response: A set of values detailing the temperature information of the battery
 * pack and the BMS. This only provides summary information;
 * queries for specific cell temperature should query the CELL_TEMP_n frames
 */
bool _vcmio_cvnp_ddef42(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {

	tBMSData *bmsDat = bms_data();
	*pLen = 7;

	pData[0] = bmsDat->tMinIdx;
	pData[1] = bmsDat->tMin;
	pData[2] = bmsDat->tMaxIdx;
	pData[3] = bmsDat->tMax;
	pData[4] = 0; // TODO: Add BMS internal temperature reading
	int16_t tAvg = bmsDat->tAvg * 100.0f;
	UTIL_INT_TO_BYTEARR(pData+5, tAvg);

	return true;

}



/**
 * 3.2.43	DDEF 43	CELL_VOLT_SUM
 *
 * Request: There are no specific requirements to request this frame.
 *
 * Response: A set of values summarizing the more extreme cell voltages
 * in the pack. Note that this does not include detailed information
 * about each cell; query for CELL_VOLT_n to see this information.
 */
bool _vcmio_cvnp_ddef43(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {

	tBMSData *bmsDat = bms_data();
	*pLen = 6;
	uint16_t tmp;

	// Low cell voltage
	pData[0] = bmsDat->vMinIdx;
	tmp = (uint16_t)(1.0E4f * bmsDat->cellData[bmsDat->vMinIdx].voltage);
	UTIL_INT_TO_BYTEARR(pData+1, tmp);

	// High cell voltage
	pData[3] = bmsDat->vMaxIdx;
	tmp = (uint16_t)(1.0E4f * bmsDat->cellData[bmsDat->vMaxIdx].voltage);
	UTIL_INT_TO_BYTEARR(pData+4, tmp);

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
