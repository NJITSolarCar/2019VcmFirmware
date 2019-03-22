/*
 * cvnp_test.c
 *
 *	Unit testing for the core, platform independent CVNP library.
 *	This is intended to be run on a fully-featured C system, not
 *	on the embedded target.
 *
 *  Created on: Mar 22, 2019
 *      Author: Duemmer
 */

#include "cvnp_config.h"

/**
 * Debug enable. If this constant is enabled, the CVNP debug routines
 * will be included in the build. This includes a cvnp_hal implementation,
 * test routines, and a main() function to run it. This is designed to
 * run on a test bench setup, so printf, fprintf, fork, etc. are used.
 * This constant will be set by cvnp_debug.make
 */
#ifdef CVNP_DEBUG_ENABLE

#include "cvnp.h"
#include "cvnp_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#endif
