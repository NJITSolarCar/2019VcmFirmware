/*
 * mppt.h
 * Driver for the MPPTs. The functions listed should be enough
 * for the external interface, but many more functions will need to be
 * written to be able to get information from the mppts.
 *
 * Assumes each MPPT's CAN ID is indexed sequentially starting from the base
 * address + 1 (offset 1)
 *
 *  Created on: Mar 9, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_MPPT_H_
#define SRC_DEVICE_MPPT_H_

#include <stdint.h>

#define MPPT_NUM_MPPT        	3 // Number of MPPTs in the system

#define MPPT_NUM_TICKS			5 // Number of ticks between queries on the system

// MPPT Base addresses
#define MPPT_BASE_REQUEST_ID	(0x71 << 4)
#define MPPT_BASE_RESPONSE_ID	(0x77 << 4)

// Flag offsets
#define MPPT_FLALG_BVLR			7
#define MPPT_FLALG_OVT			6
#define MPPT_FLALG_NOC			5
#define MPPT_FLALG_UNDV			4

/**
 * Status information about an MPPT
 */
typedef struct {
    float temp;
    float vSolar;
    float vBat;
    float iSolar;
    uint8_t bvlr : 1; // Battery Voltage level reached
    uint8_t ovt : 1; // Over temperature
    uint8_t noc : 1; // No battery is connected
    uint8_t undv : 1; // Under voltage
} tMpptData;


/**
 * Initializes the MPPT driver
 */
void mppt_init();


/**
 * Tick routine to be run every 10ms. This is where this api can
 * periodically query, check stuff, etc.
 *
 */
void mppt_tick();


/**
 * Retrieves the instantaneous data for an MPPT
 *
 * \param mpptNum the index of the mppt to get data for
 */
tMpptData *mppt_data(uint32_t mpptNum);

#endif /* SRC_DEVICE_MPPT_H_ */
