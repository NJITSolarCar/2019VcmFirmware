/*
 * vcm.c
 *
 * Primary control algorithm for the VCM. This file contains the
 * main flow control loop of the system, as well as high level
 * scheduling and module control. Basic flow execution of the VCM
 * is as follows:
 *
 * On reset,the following startup sequence will be executed:
 * - Configure MCU peripherals
 * - Bind ISRs
 * - Assert all I/Os into a safe default state
 * - Enable timestamp timer
 * - Perform a Power On Self Test (POST) of the entire vehicle system
 * 		- If any errors, assert relevant comm / POST faults, set indicator, and trap the VCM
 * - Enable timing loops
 * - Start watchdog
 * - Enter Run Mode
 *
 * While in Run Mode:
 * - Idle loop: Do nothing (as of now)
 * - Timing loop: Continuously run periodic tick() routines
 *
 * At any point, certain interrupts can change the regular control flow, and
 * their specific behavior can be detailed at the ISR.
 *
 *  Created on: April 1, 2019
 *      Author: Duemmer
 */

// Common C includes
#include <stdbool.h>
#include <stdint.h>

// General purpose files
#include "src/fault.h"
#include "src/interrupt.h"
#include "src/hal/resource.h"

// Specific modules / devices
#include "src/device/bms.h"
#include "src/device/mppt.h"
#include "src/device/kbl.h"
#include "src/device/ina225.h"
#include "src/device/indicator.h"
#include "src/device/vcm_io.h"

// CVNP
#include "src/cvnp/cvnp.h"

/**
 * main.c
 */
int main(void)
{
	return 0;
}
