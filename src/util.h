/*
 * util.h
 *
 *  Created on: Apr 6, 2019
 *      Author: Duemmer
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <stdint.h>
#include <stdbool.h>

// CLock speed 80MHz
#define UTIL_CLOCK_SPEED					80000000
#define UTIL_CYCLE_PER_MS					(UTIL_CLOCK_SPEED / 1000)

/**
 * Copies the data in a byte array to an integer of variable
 * size
 */
#define UTIL_BYTEARR_TO_INT(ARR, DAT)													\
		(DAT) = 0;																		\
		for(int util_bArr2Dat=sizeof(DAT)-1; util_bArr2Dat>=0; util_bArr2Dat--) {		\
			(DAT) <<= 8;																	\
			(DAT) |= (ARR)[util_bArr2Dat];													\
		}

/**
 * Copies the data in a variable length integer into a byte array
 */
#define UTIL_INT_TO_BYTEARR(ARR, DAT)													\
		for(int util_dat2bArr=0; util_dat2bArr < sizeof(DAT); util_dat2bArr++) 			\
			(ARR)[util_dat2bArr] = ((DAT) >> ((sizeof(DAT)-util_dat2bArr) << 3)) & 0xFF; // Arr[i] = DAT >>(8*(sizeof(DAT)-i)) & 0xFF


/**
 * Returns the current millisecond timestamp of the system
 */
uint32_t util_msTimestamp();



/**
 * Returns the current microsecond timestamp of the system
 */
uint64_t util_usTimestamp();




#endif /* SRC_UTIL_H_ */
