/*
 * util.c
 *
 *  Created on: Apr 6, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/timer.h>

#include "hal/resource.h"
/**
 * Returns the current millisecond timestamp of the system
 */
uint32_t util_msTimestamp() {
	return (uint32_t)(TimerValueGet64(SYS_US_TIMER_BASE) / 1000);
}



/**
 * Returns the current microsecond timestamp of the system
 */
uint64_t util_usTimestamp() {
	return TimerValueGet64(SYS_US_TIMER_BASE);
}
