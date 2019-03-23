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
#include "../cvnp/cvnp.h"
#include "../cvnp/cvnp_hal.h"


/**
 * Timeout routine to bind to the CVNP reader frames. Will be called
 * when a timeout occurs for any of the CAN frames. This will assert
 * a BMS communication fault
 */
void _bms_cvnpOnTimeout(bool wasKilled) {

}


/**
 * Frame parse routines
 */
void _bms_parseFrame0(tCanFrame *frame) {

}

void _bms_parseFrame1(tCanFrame *frame) {

}

void _bms_parseFrame2(tCanFrame *frame) {

}



/**
 * Initializes the BMS driver. Binds all of the BMS handlers to
 * the CVNP api
 */
void bms_init() {

}


/**
 * Tick routine to be run every ~10ms. This is where this api can
 * periodically query, check stuff, etc.
 *
 * \param now the current millisecond timestamp
 */
void bms_tick(uint32_t now) {

}



/**
 * Retrieves the instantaneous data for the BMS.
 */
tBMSData *bms_data() {

}







