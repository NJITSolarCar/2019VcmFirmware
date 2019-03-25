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
tMotorData g_lMotData, g_rMotData;


/**
 * Parse an incoming A2D batch 1 frame
 */
void _kbl_parseA2DBatch1(tCanFrame *frame, bool isLeft) {
	tMotorData *dat = isLeft ? &g_lMotData : g_rMotData;
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
	tMotorData *dat = isLeft ? &g_lMotData : g_rMotData;
	dat->ia = (float)frame->data[0];
	dat->ib = (float)frame->data[1];
	dat->ic = (float)frame->data[2];
	dat->va = ((float)frame->data[3]) * KBL_VBAT_SCL;
	dat->vb = ((float)frame->data[4]) * KBL_VBAT_SCL;
	dat->vc = ((float)frame->data[5]) * KBL_VBAT_SCL;
}



















