/*
 * cvnp.h
 *
 * Contains the interface for the CVNP protocol.
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

/**
 * Represents a CVNP compliant frame ID. Note that the ordering
 * of these elements is not defined, so they cannot be directly mapped
 * to the bits of an integer ID.
 */
typedef struct {
    uint32_t broad : 1;
    uint32_t nonc : 1;
    uint32_t scls : 6;
    uint32_t sinst : 4;
    uint32_t rcls : 6;
    uint32_t rinst : 4;
    uint32_t ddef : 7;
} tCompliantId;



/*
 * Handler for noncompliant frames on the bus.
 */
typedef struct {
    uint32_t ui32Id;
    uint32_t ui32LastRun;
    uint32_t ui32Timeout : 31;
    uint32_t bHasTimeout : 1;
    void (*pfnProcFrame)(tCanFrame *frame);
    void (*pfnOnDeath)(bool bWasKilled);
} tNonCHandler;


/*
 * Handler for standard query responses. These are submitted
 * when a query is executed. These will stay in the buffer
 * until killed, when removed from the buffer by a new query, or when
 * the timeToLive expires.
 */
typedef struct {
    tCompliantId id;                        // The Query that was sent with this handler
    uint32_t ui32TimeToLive;                // Time in ms to keep this active before it is killed
    uint32_t ui32Submitted;                 // Time in ms that the query was executed
    void (*pfnProcFrame)(tCanFrame *frame); // function to be called on a hit
    void (*pfnOnDeath)(bool bWasKilled);    // function to be called when this handler either times out or is kicked from the buffer
} tQueryHandler;


/*
 * Handler for standard query responses. These are submitted
 * when a query is executed. These will stay in the buffer
 * until killed, when removed from the buffer by a new query, or when
 * the timeToLive expires.
 */
typedef struct {
     tCompliantId id;                        // The broadcast to listen on. Only looks at SCLS and DDEF.
     uint32_t ui32TimeToLive;                // Time in ms to keep this active before it is killed
     uint32_t ui32Submitted;                 // Time in ms that the query was executed
     void (*pfnProcFrame)(tCanFrame *frame); // function to be called on a hit
     void (*pfnOnDeath)(bool bWasKilled);    // function to be called when this handler either times out or is kicked from the buffer
} tBroadHandler;


/**
 * Datastructure that contains all the relevant information to perform a
 * CVNP query
 */
typedef struct {
    uint32_t rcls : 6;
    uint32_t rinst : 4;
    uint32_t ddef : 7;
    uint32_t ui32Timeout : 31;
    uint32_t bDoesTimeOut : 1;
    void (*pfnProcFrame)(tCanFrame *frame); // function to be called on a hit
    void (*pfnOnDeath)(bool bWasKilled);    // function to be called when this handler either times out or is kicked from the buffer
} tQueryInfo;


/**
 * Sends a query on the bus
 */
void cvnp_query(tQueryInfo *info, uint32_t ui32Len, uint8_t data[8]);


/**
 * Main input frame processor for the CVNP system. This should be called
 * during RX interrupts for all new frames
 */
void cvnp_procFrame(tCanFrame *frame);

/**
 * Starts the CVNP system on this device. This will automatically call the
 * HAL initialization. Returns a zero value on success, nonzero otherwise.
 */
uint32_t cvnp_start(uint32_t ui32MyClass, uint32_t ui32MyInst);



/**
 * Converts an integer ID to an ID structure with the bits broken out.
 */
inline tCompliantId cvnp_idToStruct(uint32_t ui32Id);


/**
 * Converts an ID structure to an integer ID
 */
inline uint32_t cvnp_structToId(tCompliantId *id);


/**
 * Registers new handlers with the system. If another handler
 * with the same effective ID is already registered, it will be replaced.
 */
void cvnp_registerBroadHandler(tBroadHandler *handler);
void cvnp_registerNonCHandler(tNonCHandler *handler);
void cvnp_registerDdefHandler(uint32_t ui32Ddef, void (*pfnHandler)(tCanFrame *frame));

#endif /* CVNP_CVNP_H_ */







