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
 * Searches the broadcast handler table for a match to this ID. If
 * one is found, its handler is executed and timer reset. This only
 * looks at the scls and ddef portions of id.
 */
static bool _cvnp_handleBroadcast(tCanFrame *frame, tCompliantId id, uint32_t now) {
	// For now this uses a linear search, as there probably won't
	// be enough broadcast handlers to justify the effort of a
	// binary search. In the future this may change, or different
	// implementations be selectable via preprocessor commands.
	bool hit = false;
	tBroadHandler *pTmpHandler;
	for(int i=0; i<CVNP_BROADCAST_BUF_SIZE; i++) {
		pTmpHandler = &g_pBroadcastTable[i];

		// hit if valid, with matching scls and ddef
		hit = pTmpHandler->valid &&
				pTmpHandler->id.scls == id.scls &&
				pTmpHandler->id.ddef == id.ddef;
		if(hit) {
			pTmpHandler->lastRun = now;
			pTmpHandler->pfnProcFrame(frame);
			break;
		}
	}

	return hit;
}



/**
 * Searches the standard handler table for a handler matching
 * this ID. If one is found, its handler is executed and timer reset.
 * This only looks at the scls, sinst and ddef portions of id.
 */
static bool _cvnp_handleStdResp(tCanFrame *frame, tCompliantId id) {
	bool hit = false;
	tQueryHandler *pTmpHandler;
	for(int i=0; i<CVNP_STD_QUERY_BUF_SIZE; i++) {
		pTmpHandler = &g_pStdQueryTable[i];

		// hit if valid with matching scls, sinst, and ddef
		hit = pTmpHandler->valid &&
				pTmpHandler->id.scls == id.scls &&
				pTmpHandler->id.sinst == id.sinst &&
				pTmpHandler->id.ddef == id.ddef;
		if(hit) {
			// Free and execute the handler; its job is done
			pTmpHandler->valid = 0;
			pTmpHandler->pfnProcFrame(frame);
			break;
		}
	}

	return hit;
}




/**
 * Searches the multicast handler table for a handler matching
 * this ID. If one is found, its handler is executed.
 * This only looks at the scls, sinst and ddef portions of id.
 */
static bool _cvnp_handleMulticast(tCanFrame *frame, tCompliantId id) {
	bool hit = false;
	tQueryHandler *pTmpHandler;
	for(int i=0; i<CVNP_MULTICAST_BUF_SIZE; i++) {
		pTmpHandler = &g_pMulticastTable[i];

		// hit if valid with matching scls, sinst, and ddef
		hit = pTmpHandler->valid &&
				pTmpHandler->id.scls == id.scls &&
				pTmpHandler->id.sinst == id.sinst &&
				pTmpHandler->id.ddef == id.ddef;
		if(hit) {
			// Execute but don't free the handler; it can be reused
			pTmpHandler->pfnProcFrame(frame);
			break;
		}
	}

	return hit;
}



/**
 * Searches the noncompliant handler table for a handler matching
 * this ID. If one is found, its handler is executed and timer reset.
 * This checks for equality of IDs directly.
 */
static bool _cvnp_handleNonC(tCanFrame *frame, uint32_t now) {
	bool hit = false;
	for(int i=0; i<CVNP_NONCOMPLIANT_BUF_SIZE; i++) {
		hit = g_pNonCTable[i].valid && frame->id == g_pNonCTable[i].id;
		if(hit) {
			// Reset timer, call the handler
			g_pNonCTable[i].lastRun = now;
			g_pNonCTable[i].pfnProcFrame(frame);
		}
	}

	return hit;
}





/**
 * Processes an incoming frame for use in the CVNP system. This function
 * is responsible for dispatching and updating the different handlers and
 * buffers in order to reflect the new frame.
 */
void cvnp_procFrame(tCanFrame *frame) {
	tCompliantId id = cvnp_idToStruct(frame->id);
	uint32_t now = cvnpHal_now();
	bool hit = false;

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
		else if(id.broad)
			_cvnp_handleBroadcast(frame, id, now);

		// If none of the above, then it's probably a response to our own request
		else {
			// Check for a standard then multicast if a standard handler isn't found
			hit = _cvnp_handleStdResp(frame, id);
			if(!hit)
				hit = _cvnp_handleMulticast(frame, id);

			// If there still wasn't a hit, there must be an error. This
			// shouldn't happen in the protocol. In all likelihood, this
			// will occur if either an unsolicited frame is sent here, there
			// is another device with the same class and instance, or a
			// response was sent after a timeout period.

			// TODO: handle this error condition
		}
	}

	// Is a non-compliant frame
	else if(!_cvnp_handleNonC(frame, now)) {
		// TODO: the frame wasn't found, assert an error
	}
}



/**
 * Internal CVNP periodic tick routine. This is to be used primarily for
 * checking for timeouts on differnt handler routines. If any are found,
 * this will run the onTimeout routine and kick it from the buffer if
 * necessary.
 */
void cvnp_tick(uint32_t now) {
	tBroadHandler *tmpBroad;
	tQueryHandler *tmpQuery;
	tNonCHandler *tmpNonC;
	bool timeout;

	// Check for any valid handlers that are supposed to time
	// out that have timed out. If found, reset the
	// run timer and run the onTimeout function
	for(int i=0; i<CVNP_BROADCAST_BUF_SIZE; i++) {
		tmpBroad = &g_pBroadcastTable[i];
		timeout = tmpBroad->valid && tmpBroad->timeout &&
				now - tmpBroad->lastRun > tmpBroad->timeout;
		if(timeout) {
			tmpBroad->lastRun = now;
			tmpBroad->pfnOnTimeout();
		}
	}

	// Same thing for std query buffer, but remove from the buffer and
	// don't bother resetting the timer
	for(int i=0; i<CVNP_STD_QUERY_BUF_SIZE; i++) {
		tmpQuery = &g_pStdQueryTable[i];
		timeout = tmpQuery->valid && tmpQuery->timeToLive &&
				now - tmpQuery->submittedAt > tmpQuery->timeToLive;
		if(timeout) {
			tmpQuery->valid = false;
			tmpQuery->pfnOnDeath(false);
		}
	}

	// Same as std query buffer for multicast response buffer
	for(int i=0; i<CVNP_MULTICAST_BUF_SIZE; i++) {
		tmpQuery = &g_pMulticastTable[i];
		timeout = tmpQuery->valid && tmpQuery->timeToLive &&
				now - tmpQuery->submittedAt > tmpQuery->timeToLive;
		if(timeout) {
			tmpQuery->valid = false;
			tmpQuery->pfnOnDeath(false);
		}
	}

	// Same as for broadcast buffer
	for(int i=0; i<CVNP_NONCOMPLIANT_BUF_SIZE; i++) {
		tmpNonC = &g_pNonCTable[i];
		timeout = tmpNonC->valid && tmpNonC->timeout &&
				now - tmpNonC->lastRun > tmpNonC->timeout;
		if(timeout) {
			tmpNonC->lastRun = now;
			tmpNonC->pfnOnDeath(false);
		}
	}
}





