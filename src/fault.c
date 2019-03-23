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
 * Returns a summary of which faults are set in the system, where each bit
 * in the return value corresponds to one fault. If the fault is set, then
 * the bit will be set, and vice versa.
 */
uint64_t fault_getFaultSummary() {
	uint64_t sum = 0;
	for(int i=0; i<FAULT_NUM_FAULTS; i++) {
		sum |= g_faults[i].bSet << i;
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
	fh->bSet = true;
	fh->pfnOnAssert(uData);
	fh->uData = uData;
}




/**
 * Deasserts the fault from the system
 */
void fault_deassert(uint32_t ui32FaultNum) {
	tFaultHook *fh = &g_faults[ui32FaultNum]; // utility copy
	fh->bSet = false;
	fh->pfnOnDeassert();
}













