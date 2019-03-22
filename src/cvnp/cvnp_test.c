/*
 * cvnp_test.c
 *
 *	Unit testing for the core, platform independent CVNP library.
 *	This is intended to be run on a fully-featured C system, not
 *	on the embedded target. This will create a child process for simulating
 *	the CAN bus, which will send and receive various messages in order
 *	to test the system
 *
 *  Created on: Mar 22, 2019
 *      Author: Duemmer
 */


/**
 * Debug enable. If this constant is enabled, the CVNP debug routines
 * will be included in the build. This includes a cvnp_hal implementation,
 * test routines, and a main() function to run it. This is designed to
 * run on a test bench setup, so printf, fprintf, fork, etc. are used.
 * This constant will be set by cvnp_debug.make
 */
#ifdef CVNP_DEBUG_ENABLE

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "cvnp_config.h"
#include "cvnp_hal.h"
#include "cvnp.h"

// Pipes for communication between the CAN child and parent
int p1[2], p2[2];

// cvnp_hal implementations. Stubs for now.
bool cvnpHal_init() {

}

void cvnpHal_sendFrame(tCanFrame frame) {

}

uint32_t cvnpHal_now() {

}

void cvnpHal_handleError(uint32_t errNum) {

}

void cvnpHal_resetSystem() {

}

int main() {

}










#endif
