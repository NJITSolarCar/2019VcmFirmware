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
static bool (*g_pfnDdefTable[CVNP_NUM_DDEF])(tCanFrame *frame, uint32_t *pLen, uint32_t *pData);

/**
 * Table of current multicast response handlers registered to
 * the system
 */
static tQueryHandler g_pMulticastTable[CVNP_MULTICAST_BUF_SIZE];

/**
 * Table of current standard query response handlers registered to
 * the system
 */
static tQueryHandler g_pStdQueryTable[CVNP_STD_QUERY_BUF_SIZE];

/**
 * Table of current broadcast handlers registered to
 * the system
 */
static tBroadHandler g_pBroadcastTable[CVNP_BROADCAST_BUF_SIZE];

/**
 * Table of current non-compliant frame handlers registered to
 * the system
 */
static tNonCHandler g_pNonCTable[CVNP_NONCOMPLIANT_BUF_SIZE];
