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
#include "../hal/resource.h"

void _can_intHandler() {

}


/**
 * Initializes the CAN hardware
 */
bool cvnpHal_init() {
	// Setup the GPIO as CAN
}
