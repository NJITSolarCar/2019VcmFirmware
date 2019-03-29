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
