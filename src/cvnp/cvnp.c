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
static bool (*g_pfnDdefTable[CVNP_NUM_DDEF])(tCanFrame *frame, uint32_t *pLen, uint8_t (*pData)[8]);

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


// This device's class and instance number
static uint8_t g_myClass;
static uint8_t g_myInst;

/**
 * Dispatches a DDEF handler, while providing the simple calling interface
 * offered to the handler.
 */
static inline void _cvnp_runDdefhandler(tCanFrame *frame, tCompliantId id) {
	tCanFrame newFrame;

	// No handler is set for this DDEF, so respond with the error handler instead
	if(!g_pfnDdefTable[id.ddef])
		id.ddef = CVNP_DDEF_ERROR;

	// Need to use specific len variable because you can't take a pointer
	// to a bitfield
	uint32_t len;
	if((*g_pfnDdefTable[id.ddef])(frame, &len, &newFrame.data)) {
		newFrame.head.dlc = len;

		// move around the ID so it points back to whoever sent this request
		id.rcls = id.scls;
		id.rinst = id.sinst;
		id.scls = g_myClass;
		id.sinst = g_myInst;
		newFrame.id = cvnp_structToId(&id);
		newFrame.head.rtr = 0;
		newFrame.head.ide = 1;
		cvnpHal_sendFrame(newFrame);
	}
}



/**
 * Processes an incoming frame for use in the CVNP system. This function
 * is responsible for dispatching and updating the different handlers and
 * buffers in order to reflect the new frame.
 */
void cvnp_procFrame(tCanFrame *frame) {
	tCompliantId id = cvnp_idToStruct(frame->id);
	uint32_t now = cvnpHal_now();

	// True if the noncompliant bit is set or the frame uses an 11-bit (standard) id
	bool isCompliant = !id.nonc || !frame->head.ide;

	// True if the frame is specifically directed at this device, or is
	// a multicast that includes this device
	bool isForMe =
			(!id.rcls || id.rcls==g_myClass ) &&
			(!id.rinst || id.rinst==g_myInst);

	if(isCompliant && isForMe) {
		if(frame->head.rtr)
			_cvnp_runDdefhandler(frame, id);
		else if(id.broad) {
			// For now this uses a linear search, as there probably won't
			// be enough broadcast handlers to justify the effort of a
			// binary search. In the future this may change, or different
			// implementations be selectable via preprocessor commands.
			bool hit = false;
			for(int i=0; i<CVNP_BROADCAST_BUF_SIZE; i++) {
				// hit if valid, with matching scls and ddef
				hit = g_pBroadcastTable[i].valid &&
						g_pBroadcastTable[i].id.scls == id.scls &&
						g_pBroadcastTable[i].id.ddef == id.ddef;
				if(hit)
					g_pBroadcastTable[i].lastRun = cvnpHal_now();
					g_pBroadcastTable[i].pfnProcFrame(frame);
			}
		}
	}
}










