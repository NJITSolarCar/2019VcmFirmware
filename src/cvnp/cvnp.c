/*
 * cvnp.c
 *
 * Core driver of the CVNP api
 *
 *  Created on: Mar 7, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>
#include "cvnp.h"
#include "cvnp_hal.h"
#include "cvnp_config.h"

/**
 * Table of DDEF request handlers. Each entry in this table
 * corresponds to one handler for one ddef.
 */
static void (*g_pfnDdefTable[CVNP_NUM_DDEF])(tCanFrame *frame);

/**
 * Table of current multicast response handlers registered to
 * the system
 */
static tMulticastHandler g_psMulticastTable[CVNP_MULTICAST_BUF_SIZE];

/**
 * Table of current standard query response handlers registered to
 * the system
 */
static tStdHandler g_psStdQueryTable[CVNP_STD_QUERY_BUF_SIZE];

/**
 * Table of current broadcast handlers registered to
 * the system
 */
static tBroadHandler g_psBroadcastTable[CVNP_BROADCAST_BUF_SIZE];

/**
 * Table of current non-compliant frame handlers registered to
 * the system
 */
static tNonCHandler g_psNonCTable[CVNP_NONCOMPLIANT_BUF_SIZE];
