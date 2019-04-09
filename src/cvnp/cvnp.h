/*
 * cvnp.h
 *
 * Contains the interface for the CVNP protocol. The functions here are what should be called
 * by application code to participate in the protocol.
 *
 *  Created on: Mar 6, 2019
 *      Author: Duemmer
 */

#ifndef CVNP_CVNP_H_
#define CVNP_CVNP_H_

#include "cvnp_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Buffer and table sizes
#define CVNP_NUM_DDEF                           128 // Number of DDEFs in the protocol

// Internal error constant identifiers
#define CVNP_INTERNAL_ERR_UNKNOWN				0
#define CVNP_INTERNAL_ERR_BAD_RX_FRAME			1
#define CVNP_INTERNAL_ERR_NO_NONC_HANDLER		2

// Token positions in a compliant ID word
#define CVNP_NONC_POS							28
#define CVNP_BROAD_POS							27
#define CVNP_SCLS_POS							21
#define CVNP_SINST_POS							17
#define CVNP_RCLS_POS							11
#define CVNP_RINST_POS							7
#define CVNP_DDEF_POS							7

// Token length masks. Assumes the lsb of the field is aligned with bit 0
#define CVNP_CLASS_LEN_MASK						((1 << 6) - 1)
#define CVNP_INST_LEN_MASK						((1 << 4) - 1)
#define CVNP_DDEF_LEN_MASK						((1 << 7) - 1)

// Standard DDEFS
#define CVNP_DDEF_ERROR							0
#define CVNP_DDEF_DEVINFO						1
#define CVNP_DDEF_DEVNAME						2
#define CVNP_DDEF_DEVVER						3
#define CVNP_DDEF_RESET							4

// Error constant byte for the error frame response
#define CVNP_ERROR_CONST                        0xD8

// Magic constant for a reset frame. The data must match this exactly
// in order for that frame to be recognized and a reset to be performed.
// This is to prevent accidental resets.
#define CVNP_RESET_MAGIC						0x06D3BAAE

/**
 * Represents a CVNP compliant frame ID. Note that the ordering
 * of these elements is not defined, so they cannot be directly mapped
 * to the bits of an integer ID.
 */
typedef struct {
    uint32_t broad 	: 1;
    uint32_t nonc 	: 1;
    uint32_t scls 	: 6;
    uint32_t sinst 	: 4;
    uint32_t rcls 	: 6;
    uint32_t rinst 	: 4;
    uint32_t ddef 	: 7;
} tCompliantId;



/*
 * Handler for noncompliant frames on the bus.
 */
typedef struct {
    uint32_t valid : 1; // Set automatically by the CVNP dispatcher. 1 if this is a live handler, 0 otherwise
    uint32_t timeout : 31; // Time in ms to keep this active before it is killed. Set to 0 to disable timeouts.
    uint32_t id;
    uint32_t lastRun;
    void (*pfnProcFrame)(tCanFrame *frame);
    void (*pfnOnDeath)(bool wasKilled);
} tNonCHandler;


/*
 * Handler for standard query responses. These are submitted
 * when a query is executed. These will stay in the buffer
 * until killed, when removed from the buffer by a new query, or when
 * the timeToLive expires.
 */
typedef struct {
	uint32_t valid : 1; // Set automatically by the CVNP dispatcher. 1 if this is a live handler, 0 otherwise
	uint32_t timeToLive : 31;              	// Time in ms to keep this active before it is killed. Set to 0 to disable timeouts.
	tCompliantId id;                        // The Query that was sent with this handler
    uint32_t submittedAt;                 	// Time in ms that the query was executed
    void (*pfnProcFrame)(tCanFrame *frame); // function to be called on a hit
    void (*pfnOnDeath)(bool wasKilled);    	// function to be called when this handler either times out or is kicked from the buffer
} tQueryHandler;


/*
 * Handler for standard query responses. These are submitted
 * when a query is executed. These will stay in the buffer
 * until killed, when removed from the buffer by a new query, or when
 * the timeToLive expires.
 */
typedef struct {
	 uint32_t valid : 1; // Set automatically by the CVNP dispatcher. 1 if this is a live handler, 0 otherwise
     tCompliantId id;                        	// The broadcast to listen on. Only looks at SCLS and DDEF.
     uint32_t timeout : 31;						// minimum time between matching frames that causes the onTimeout function to run. Set to 0 to ignore timeouts.
     uint32_t lastRun;                 			// Time in ms that the broadcast was last received
     void (*pfnProcFrame)(tCanFrame *frame); 	// function to be called on a hit
     void (*pfnOnTimeout)(void);    			// function to be called when this handler either times out or is kicked from the buffer
} tBroadHandler;


/**
 * Datastructure that contains all the relevant information to perform a
 * CVNP query
 */
typedef struct {
    uint32_t rcls : 6;
    uint32_t rinst : 4;
    uint32_t ddef : 7;
    uint32_t timeout : 31;
    uint32_t doesTimeOut : 1;
    void (*pfnProcFrame)(tCanFrame *frame); // function to be called on a hit
    void (*pfnOnDeath)(bool wasKilled);    	// function to be called when this handler either times out or is kicked from the buffer
} tQueryInfo;


/**
 * Sends a compliant query on the bus. This will add the given handlers to the query
 * handler buffer, format a can frame with the requisite ID and the given data, and send it
 * out. When a frame is received that matches the request to this request, the handler will
 * be invoked and passed that data. If a timeout period passes before a response is
 * received or the handler is kicked from the buffer (normally due to insufficient buffer
 * size), the onTimeput handler will be called, with an indication if it was killed or just
 * timed out.
 */
void cvnp_query(tQueryInfo *info, uint32_t len, uint8_t *pData);


/**
 * Main input frame processor for the CVNP system. This should be called
 * during RX interrupts for all new frames
 */
void cvnp_procFrame(tCanFrame *frame);


/**
 * Tick routine that should be called periodically by the HAL. Every
 * 10-50ms is a good time to use.
 */
void cvnp_tick(uint32_t now);


/**
 * Starts the CVNP system on this device. This should be called after setting up
 * broadcast and nonC handlers and initializing the DDEF table. This will clear
 * the multicast and standard query buffer. This will automatically call the
 * HAL initialization, and if successful begin listening and operating.
 * Returns true on success, false otherwise.
 */
bool cvnp_start(uint32_t myClass, uint32_t myInst);



/**
 * Converts an integer ID to an ID structure with the bits broken out.
 */
tCompliantId cvnp_idToStruct(uint32_t id);


/**
 * Converts an ID structure to an integer ID
 */
uint32_t cvnp_structToId(tCompliantId id);



/**
 * Registers new handlers with the system. If another handler
 * with the same effective ID is already registered, it will be replaced.
 */
bool cvnp_registerBroadHandler(tBroadHandler *handler);
bool cvnp_registerNonCHandler(tNonCHandler *handler);
bool cvnp_registerDdefHandler(uint32_t ddef, bool (*pfnHandler)(tCanFrame *frame, uint32_t *pLen, uint8_t pData[8]));

#endif /* CVNP_CVNP_H_ */







