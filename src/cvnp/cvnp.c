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
 * corresponds to one handler for one ddef. Each function
 * returns true if the data in pData should be sent out as a response
 */
static bool (*g_pfnDdefTable[CVNP_NUM_DDEF])(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]);

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
static uint32_t g_myClass;
static uint32_t g_myInst;



// =================== STANDARD DDEF HANDLERS ==========================

/**
 * DDEF handler for an error frame response. See the CVNP spec for
 * details on this handler's behavior. Will always return true.
 */
static bool _cvnp_errorFrameHandler(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	tCompliantId id = cvnp_idToStruct(frame->id);

	// pLen is the min of 8 and the input frame dlc + 3
	if(frame->head.dlc < 5)
		*pLen = frame->head.dlc + 3;
	else
		*pLen = 8;

	pData[0] = CVNP_ERROR_CONST;
	pData[1] = id.ddef;
	pData[2] = frame->head.dlc;
	pData[3] = frame->data[0];
	pData[4] = frame->data[1];
	pData[5] = frame->data[2];
	pData[6] = frame->data[3];
	pData[7] = frame->data[4];
	pData[8] = frame->data[5];

	return true;
}


/**
 * DDEF handler for device info response. Responds with device class and info
 * in bytes 0 and 1 respectively.
 */
static bool _cvnp_devinfoFrameHandler(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	*pLen = 2;
	pData[0] = g_myClass;
	pData[1] = g_myInst;

	return true;
}



/**
 * DDEF handler for a reset frame. If the magic constant is correct, this will trigger
 * a device reset, via the cvnpHal_resetSystem() function. Else, it will do nothing.
 */
static bool _cvnp_resetFrameHandler(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]) {
	if(*((uint64_t *)(frame->data)) == CVNP_RESET_MAGIC)
		cvnpHal_resetSystem();
	return false;
}




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
	if((*g_pfnDdefTable[id.ddef])(frame, &len, newFrame.data)) {
		newFrame.head.dlc = len;

		// move around the ID so it points back to whoever sent this request
		id.rcls = id.scls;
		id.rinst = id.sinst;
		id.scls = g_myClass;
		id.sinst = g_myInst;
		newFrame.id = cvnp_structToId(id);
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
 * Comparison function for broadcast handlers. This is a c-style
 * compare() function, so it will return 0 if the handlers respond to
 * the same IDs, > 0 if 'a' responds to a higher id, < 0 if 'b'
 * responds to a higher id. Note that only the SCLS and DDEF portions
 * of the ID are considered.
 */
static int _cvnp_compareBroadHandler(const void *a, const void *b) {
	tBroadHandler *ha = (tBroadHandler *)a;
	tBroadHandler *hb = (tBroadHandler *)b;

	// Extract only the DDEF and SCLS from each
	uint32_t id_a = ha->id.ddef;
	id_a |= ha->id.scls << CVNP_SCLS_POS;
	uint32_t id_b = hb->id.ddef;
	id_b |= hb->id.scls << CVNP_SCLS_POS;

	return id_a - id_b;
}




/**
 * Comparison function for non-compliant handlers. This is a c-style
 * compare() function, so it will return 0 if the handlers respond to
 * the same IDs, > 0 if 'a' responds to a higher id, < 0 if 'b'
 * responds to a higher id.
 */
static int _cvnp_compareNonCHandler(const void *a, const void *b) {
	tNonCHandler *ha = (tNonCHandler *)a;
	tNonCHandler *hb = (tNonCHandler *)b;

	return ha->id - hb->id;
}




///////////////////////////////////////////////////////////////////////////////
////////////////////////// MAIN  INTERFACE FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 * Main init routine. Saves our class / instance info, binds common handlers,
 * resets the multicast / standard query buffers, binds common ddefs, and
 * initializes HAL.
 */
bool cvnp_start(uint32_t myClass, uint32_t myInst) {
	g_myClass = myClass;
	g_myInst = myInst;

	// Fill the DDEF buffer with nulls by default
	for(int i=0; i<CVNP_NUM_DDEF; i++)
		g_pfnDdefTable[i] = 0;

	// Common frame binding
	g_pfnDdefTable[CVNP_DDEF_ERROR] = &_cvnp_errorFrameHandler;
	g_pfnDdefTable[CVNP_DDEF_RESET] = &_cvnp_resetFrameHandler;
	g_pfnDdefTable[CVNP_DDEF_DEVINFO] = &_cvnp_devinfoFrameHandler;

	// clear buffers
	for(int i=0; i<CVNP_MULTICAST_BUF_SIZE; i++)
		g_pMulticastTable[i].valid = false;

	for(int i=0; i<CVNP_STD_QUERY_BUF_SIZE; i++)
		g_pStdQueryTable[i].valid = false;

	return cvnpHal_init();
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

			cvnpHal_handleError(CVNP_INTERNAL_ERR_BAD_RX_FRAME);
		}
	}

	// Is a non-compliant frame
	else if(!_cvnp_handleNonC(frame, now)) {
		cvnpHal_handleError(CVNP_INTERNAL_ERR_NO_NONC_HANDLER);
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




/**
 * Converts a numeric ID to a formatted ID structure. This has to be done
 * manually versus bitfields as the order and position of bitfields is not
 * guaranteed.
 */
inline tCompliantId cvnp_idToStruct(uint32_t id) {
	tCompliantId ret;
	ret.nonc = (id >> CVNP_NONC_POS) & 0x1;
	ret.broad = (id >> CVNP_BROAD_POS) & 0x1;
	ret.scls = (id >> CVNP_SCLS_POS) & CVNP_CLASS_LEN_MASK;
	ret.sinst = (id >> CVNP_SINST_POS) & CVNP_INST_LEN_MASK;
	ret.rcls = (id >> CVNP_RCLS_POS) & CVNP_CLASS_LEN_MASK;
	ret.rinst = (id >> CVNP_RINST_POS) & CVNP_INST_LEN_MASK;
	ret.ddef = (id >> CVNP_DDEF_POS) & CVNP_DDEF_LEN_MASK;

	return ret;
}



/**
 * Converts a formatted ID structure to a numeric id. This has to be done
 * manually versus bitfields as the order and position of bitfields is not
 * guaranteed.
 */
inline uint32_t cvnp_structToId(tCompliantId id) {
	uint32_t ret = id.ddef; // Zero everything but the lowest token
	ret |= id.broad << CVNP_BROAD_POS;
	ret |= id.nonc << CVNP_NONC_POS;
	ret |= id.scls << CVNP_SCLS_POS;
	ret |= id.sinst << CVNP_SINST_POS;

	ret |= id.rcls << CVNP_RCLS_POS;
	ret |= id.rinst << CVNP_RINST_POS;

	return ret;
}




/**
 * Registers a new broadcast handler with the system. If another handler
 * with the same effective ID is already registered, it will be replaced. If
 * an identical handler is not found, this will be added to the end of the
 * buffer. If the buffer is full, the handler will not be added.
 */
bool cvnp_registerBroadHandler(tBroadHandler *handler) {

	// Check for a valid frame that matches the one to add
	bool alreadyExists;
	for(int i=0; i<CVNP_BROADCAST_BUF_SIZE; i++) {
		alreadyExists = g_pBroadcastTable[i].valid &&
				_cvnp_compareBroadHandler(handler, &g_pBroadcastTable[i]);
		if(alreadyExists) { // Replace the handler and return
			g_pBroadcastTable[i] = *handler;
			return true;
		}
	}

	// Check for the first invalid handler slot in the buffer to add to
	for(int i=0; i<CVNP_BROADCAST_BUF_SIZE; i++) {
		if(g_pBroadcastTable[i].valid) {
			g_pBroadcastTable[i] = *handler;
			return true;
		}
	}

	// There are no slots in the buffer to insert this frame, so return false
	return false;
}





/**
 * Registers a new broadcast handler with the system. If another handler
 * with the same effective ID is already registered, it will be replaced. If
 * an identical handler is not found, this will be added to the end of the
 * buffer. If the buffer is full, the handler will not be added.
 */
bool cvnp_registerNonCHandler(tNonCHandler *handler) {

	// Check for a valid frame that matches the one to add
	bool alreadyExists;
	for(int i=0; i<CVNP_NONCOMPLIANT_BUF_SIZE; i++) {
		alreadyExists = g_pNonCTable[i].valid &&
				_cvnp_compareNonCHandler(handler, &g_pNonCTable[i]);
		if(alreadyExists) { // Replace the handler and return
			g_pNonCTable[i] = *handler;
			return true;
		}
	}

	// Check for the first invalid handler slot in the buffer to add to
	for(int i=0; i<CVNP_NONCOMPLIANT_BUF_SIZE; i++) {
		if(g_pNonCTable[i].valid) {
			g_pNonCTable[i] = *handler;
			return true;
		}
	}

	// There are no slots in the buffer to insert this frame, so return false
	return false;
}



/**
 * Adds a new DDEF handler to the system. If this ID is already allocated, this will
 * replace it. Will return true if a handler was replaced, false if added new
 */
bool cvnp_registerDdefHandler(uint32_t ddef, bool (*pfnHandler)(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8])) {
	bool isAllocated = g_pfnDdefTable[ddef] != 0;
	g_pfnDdefTable[ddef] = pfnHandler;
	return isAllocated;
}





/**
 * Sends a compliant query on the bus. This will add the given handlers to the query
 * handler buffer, format a can frame with the requisite ID and the given data, and send it
 * out. When a frame is received that matches the request to this request, the handler will
 * be invoked and passed that data. If a timeout period passes before a response is
 * received or the handler is kicked from the buffer (normally due to insufficient buffer
 * size), the onTimeput handler will be called, with an indication if it was killed or just
 * timed out.
 */
void cvnp_query(tQueryInfo *info, uint32_t len, uint8_t *pData){
	// Build the handler. We have to swap the sender / receiver data for this
	tQueryHandler hdl;
	hdl.valid = true;
	hdl.submittedAt = cvnpHal_now();
	hdl.timeToLive = info->doesTimeOut ? info->timeout : 0;
	hdl.id.scls = info->rcls;
	hdl.id.sinst = info->rinst;
	hdl.id.ddef = info->ddef;
	hdl.pfnOnDeath = info->pfnOnDeath;
	hdl.pfnProcFrame = info->pfnProcFrame;

	// Build the id to use on the transmission
	tCompliantId id;
	id.broad = false;
	id.nonc = false;
	id.scls = g_myClass;
	id.sinst = g_myInst;
	id.rcls = info->rcls;
	id.rinst = info->rinst;
	id.ddef = info->ddef;

	// Build the frame
	tCanFrame frame;
	frame.id = cvnp_structToId(id);
	frame.head.dlc = len;
	frame.head.ide = true;
	frame.head.rtr = true;
	for(int i=0; i<len; i++)
		frame.data[i] = pData[i];


	// Select the correct buffer and limit
	bool isMulti = !info->rcls || !info->rinst; // true if either are 0
	tQueryHandler *bufBase = isMulti ? g_pMulticastTable : g_pStdQueryTable;
	uint32_t bufLimit = isMulti ? CVNP_MULTICAST_BUF_SIZE : CVNP_STD_QUERY_BUF_SIZE;

	// Find a spot in the correct buffer to add this handler
	bool added = false; // if true, was added to a buffer already
	for(int i=0; i<bufLimit; i++) {
		if(!bufBase[i].valid) {// empty slot, just add
			bufBase[i] = hdl;
			added = true;
		}
	}
	if (!added) { // No open slots, kick out the oldest handler
		uint32_t oldIdx = 0;
		for(int i=0; i<bufLimit; i++) {
			if(bufBase[i].submittedAt < bufBase[oldIdx].submittedAt)
				oldIdx = i;
		}
		bufBase[oldIdx].pfnOnDeath(true);
		bufBase[oldIdx] = hdl;
	}

	// Handlers should be in order, so transmit the frame
	cvnpHal_sendFrame(frame);
}



























