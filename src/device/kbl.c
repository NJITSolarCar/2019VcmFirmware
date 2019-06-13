/*
 * kbl.c
 * Main CAN and interface drivers for the motor controller,
 * model KBL96151.
 *  Created on: Mar 25, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>
#include "kbl.h"
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"

// Data for left and right motors
static tMotorData g_lMotData, g_rMotData;

// frame query types for latest transmission
static uint8_t g_lMotQuery = 0, g_rMotQuery = 0;


/**
 * Parse an incoming A2D batch 1 frame
 */
void _kbl_parseA2DBatch1(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	dat->brake = ((float)frame->data[0]) * KBL_THROTTLE_SCL;
	dat->throttle = ((float)frame->data[1]) * KBL_THROTTLE_SCL;
	dat->vOperating = ((float)frame->data[2]) * KBL_VBAT_SCL;
	dat->vs = KBL_VS_TO_VOLTS(frame->data[3]);
	dat->vBat = ((float)frame->data[4]) * KBL_VBAT_SCL;
}




/**
 * Parse an incoming A2D batch 2 frame
 */
void _kbl_parseA2DBatch2(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	dat->ia = (float)frame->data[0] ;
	dat->ib = (float)frame->data[1];
	dat->ic = (float)frame->data[2];
	dat->va = ((float)frame->data[3]) * KBL_VBAT_SCL;
	dat->vb = ((float)frame->data[4]) * KBL_VBAT_SCL;
	dat->vc = ((float)frame->data[5]) * KBL_VBAT_SCL;
}




/**
 * Parse an incoming CCP Monitor 1
 */
void _kbl_parseCCP1(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;

	// If this byte is 0xFF, the sensor is not connected, so record 0 instead
	uint8_t tMotByte = frame->data[2];
	dat->tMot = tMotByte == 0xFF ? 0.0f : (float)tMotByte;

	// Average the 3 internal temperatures
	dat->tController = (float)frame->data[3];
	dat->tController += (float)frame->data[4];
	dat->tController += (float)frame->data[5];
	dat->tController *= 0.3333333333;
}



/**
 * Parse an incoming CCP Monitor 2
 */
void _kbl_parseCCP2(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	uint16_t rpmWord = frame->data[0];
	rpmWord <<= 8;
	rpmWord |= frame->data[1];
	dat->mechRPM = KBL_NUM_POLES_INV * ((float)rpmWord);
	dat->iMot = KBL_RATED_CURRENT * ((float)frame->data[2]) * 0.01f;
}




/**
 * Parse an incoming COM_SW_ACC frame
 */
void _kbl_parseComSwAcc(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	dat->swThrottle = !!frame->data[0];
}



/**
 * Parse an incoming COM_SW_BRK frame
 */
void _kbl_parseComSwBrk(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	dat->swBrake = !!frame->data[0];
}




/**
 * Parse an incoming COM_SW_REV frame
 */
void _kbl_parseComSwRev(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : &g_rMotData;
	dat->swRev = !!frame->data[0];
}



/**
 * Master parse routine for incoming frames. Will decide which
 * motor controller has sent the frame based on the ID. The type of
 * request last sent for each motor controller is recorded by the
 * query routines, and based on that will delegate to the correct
 * sub-parse routine.
 */
void _kbl_parseFrameGen(tCanFrame *frame) {
	bool isLeft = frame->id == KBL_ID_RX_LEFT;
	uint8_t query = isLeft ? g_lMotQuery : g_rMotQuery;

	switch(query) {
	case KBL_FRAME_A2D_BATCH_1: {
		_kbl_parseA2DBatch1(frame, isLeft);
		break;
	}
	case KBL_FRAME_A2D_BATCH_2: {
		_kbl_parseA2DBatch2(frame, isLeft);
		break;
	}
	case KBL_FRAME_CCP1: {
		_kbl_parseCCP1(frame, isLeft);
		break;
	}
	case KBL_FRAME_CCP2: {
		_kbl_parseCCP2(frame, isLeft);
		break;
	}
	case KBL_FRAME_COM_SW_ACC: {
		_kbl_parseComSwAcc(frame, isLeft);
		break;
	}
	case KBL_FRAME_COM_SW_REV: {
		_kbl_parseComSwRev(frame, isLeft);
		break;
	}
	case KBL_FRAME_COM_SW_BRK: {
		_kbl_parseComSwAcc(frame, isLeft);
		break;
	}
	default : {
		// TODO: Assert an error, illegal frame query byte
	}
	}
}



void _kbl_onFrameTimeout(bool wasKilled) {

}




/**
 * Initializes the motor controller driver. Binds the two frame handlers for
 * the CVNP API
 */
void kbl_init() {
	tNonCHandler kblHdl;
	kblHdl.id = KBL_ID_RX_LEFT;
	kblHdl.lastRun = 0;
	kblHdl.timeout = KBL_CAN_TIMEOUT;
	kblHdl.valid = true;
	kblHdl.pfnOnDeath = _kbl_onFrameTimeout;
	kblHdl.pfnProcFrame = _kbl_parseFrameGen;

	// Register the two handlers for left and right
	cvnp_registerNonCHandler(&kblHdl);

	kblHdl.id = KBL_ID_RX_RIGHT;
	cvnp_registerNonCHandler(&kblHdl);
}

/**
 * Tick routine that runs every 10 ms.
 */
void kbl_tick() {
	static uint8_t state = 0;
	static uint32_t callCount = 0;

	// increment each time
	callCount++;
	if(callCount % KBL_TICK_UPDATE_PERIOD == 0) { // send a query
		bool isLeft = state % 2; // Alternating left / right states
		uint8_t queryType;
		switch(state >> 1) { // State / 2. Need it to compensate for alternating between left and right
		case 0: {
			queryType = KBL_FRAME_A2D_BATCH_1;
			state++;
			break;
		}
		case 1: {
			queryType = KBL_FRAME_A2D_BATCH_2;
			state++;
			break;
		}
		case 2: {
			queryType = KBL_FRAME_CCP1;
			state++;
			break;
		}
		case 3: {
			queryType = KBL_FRAME_CCP2;
			state++;
			break;
		}
		case 4: {
			queryType = KBL_FRAME_COM_SW_ACC;
			state++;
			break;
		}
		case 5: {
			queryType = KBL_FRAME_COM_SW_REV;
			state++;
			break;
		}
		case 6: {
			queryType = KBL_FRAME_COM_SW_BRK;
			state++;
			break;
		}
		default: {
			state = 0;
		}
		}

		// Save query byte
		if(isLeft)
			g_lMotQuery = queryType;
		else
			g_rMotQuery = queryType;

		// transmit the query
		tCanFrame txFrame;
		txFrame.id = isLeft ? KBL_ID_TX_LEFT : KBL_ID_TX_RIGHT;
		txFrame.head.dlc = 1;
		txFrame.head.rtr = 0;
		txFrame.head.ide = 0;
		txFrame.data[0] = queryType;
		cvnpHal_sendFrame(txFrame);
	}
}


tMotorData *kbl_leftMotData() {
	return &g_lMotData;
}


tMotorData *kbl_rightMotData() {
	return &g_rMotData;
}












