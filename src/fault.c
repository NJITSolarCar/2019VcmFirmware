/*
 * fault.c
 *
 *  Created on: Mar 22, 2019
 *      Author: Duemmer
 */
#include <stdint.h>
#include <stdbool.h>
#include "fault.h"

// List of all faults, handlers, and their data in the system
static tFaultHook g_faults[FAULT_NUM_FAULTS];


/**
 * Default function to be called on a fault assert , if none is set. This
 * will lock up the system and set the indicator light.
 */
static void _fault_defaultOnAsert(tFaultData data) {
	// TODO: add a call to put the system into a safe state

//	for(;;); // Trap system
}




/**
 * Default function to be called for a fault deassert. Does nothing.
 */
static void _fault_defaultOnDeasert() {

}




/**
 * Initializes the fault control module. Will populate the
 * fault hook table with default handlers, which will lock
 * the system while in a debug build. If not in a debug build,
 * the fault will be ignored.
 */
void fault_init() {
	// bind default hooks
	for(int i=0; i<FAULT_NUM_FAULTS; i++) {
		g_faults[i].pfnOnAssert = _fault_defaultOnAsert;
		g_faults[i].pfnOnDeassert = _fault_defaultOnDeasert;
		g_faults[i].bSet = 0;
		g_faults[i].ui32TSet = 0;
	}
}



/**
 * Returns a summary of which faults are set in the system, where each bit
 * in the return value corresponds to one fault. If the fault is set, then
 * the bit will be set, and vice versa.
 */
uint64_t fault_getFaultSummary() {
	uint64_t sum = 0;
	for(int i=0; i<FAULT_NUM_FAULTS; i++) {
		sum |= ((uint64_t)g_faults[i].bSet) << i;
	}

	return sum;
}




/**
 * Asserts the fault, with the specified data
 * TODO: Add a call to get the timestamp for the lastAsserted
 * time in the handler
 */
void fault_assert(uint32_t ui32FaultNum, tFaultData uData) {
	tFaultHook *fh = &g_faults[ui32FaultNum]; // utility copy
	if(!fh->bSet) {
		fh->bSet = true;
		fh->pfnOnAssert(uData);
		fh->uData = uData;
	}
}




/**
 * Deasserts the fault from the system
 */
void fault_deassert(uint32_t ui32FaultNum) {
	tFaultHook *fh = &g_faults[ui32FaultNum]; // utility copy
	fh->bSet = false;
	fh->pfnOnDeassert();
}



/**
 * Fetches the data for the specified fault
 */
tFaultData fault_getFaultData(uint32_t fault) {
	return g_faults[fault].uData;
}



/**
 * Fetches the time asserted for the specified fault
 */
uint32_t fault_getFaultTime(uint32_t fault) {
	return g_faults[fault].ui32TSet;
}



/**
 * Registers a set of handler functions for a certain fault number.
 */
void fault_regHook(uint32_t faultNum, void (*pfnOnAssert)(tFaultData), void (*pfnOnDeassert)(void)) {
	tFaultHook *fh = &g_faults[faultNum]; // utility copy
	fh->pfnOnAssert = pfnOnAssert;
	fh->pfnOnDeassert = pfnOnDeassert;
}










