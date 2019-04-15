/*
 * fault.h
 *
 *  Provides an API for tracking,
 *  handling, and asserting faults on the system.
 *  Created on: Feb 25, 2019
 *      Author: Duemmer
 */

#ifndef FAULT_H_
#define FAULT_H_

#include <stdint.h>

/**
 * Represents a list of all of the different fault codes
 * in the system, as well as a flag for the number of faults
 */
typedef enum {
    FAULT_BMS_CELL_OVER_VOLT_WARN, // data: float[0]: cell voltage
    FAULT_BMS_CELL_UNDER_VOLT_WARN, // data: float[0]: cell voltage
    FAULT_BMS_PACK_OVER_VOLT_WARN, // data: float[0]: pack voltage
    FAULT_BMS_PACK_UNDER_VOLT_WARN, // data: float[0]: pack voltage
    FAULT_BMS_PACK_SHORT,
    FAULT_BMS_COMM, // data: ui64: true if wasKilled
    FAULT_BMS_OVER_CURRENT_CHG, // data: float[0]: Absolute vale of current at the time of the fault
    FAULT_BMS_OVER_CURRENT_DISCHG, // data: float[0]: Absolute vale of current at the time of the fault
    FAULT_BMS_CELL_OVER_VOLT, // data: float[0]: cell voltage
    FAULT_BMS_CELL_UNDER_VOLT, // data: float[0]: cell voltage
    FAULT_BMS_PACK_OVER_VOLT, // data: float[0]: pack voltage
    FAULT_BMS_PACK_UNDER_VOLT, // data: float[0]: pack voltage
    FAULT_BMS_LOW_TEMP, // data: ui8[0]: thermistor ID; float[1]: temperature
	FAULT_BMS_HIGH_TEMP, // data: ui8[0]: thermistor ID; ui8[1]: isBmsTempFault; float[1]: temperature
    FAULT_BMS_IMBALANCE,
    FAULT_BMS_GENERAL,
    FAULT_MPPT_TEMP_WARN,
    FAULT_MPPT_TEMP, // data: ui32[0]: MPPT ID; float[1]: MPPT temperature
    FAULT_MPPT_BATT_CHARGED, // data: ui32[0]: MPPT ID; float[1]: vBat
    FAULT_MPPT_NO_BATT, // data: ui32[0]: MPPT ID
    FAULT_MPPT_LOW_SOLAR_VOLTS, // data: ui32[0]: MPPT ID; float[1]: vSolar
    FAULT_MPPT_COMM, // data: ui32[0]: true if wasKilled; ui32[1]: MPPT ID
    FAULT_TELE_LORA,
    FAULT_TELE_PI,
    FAULT_TELE_COMM,
    FAULT_MOTOR_TEMP,
    FAULT_MOTOR_GEN,
    FAULT_MOTOR_COMM,
    FAULT_GEN_ESTOP,
    FAULT_GEN_AUX_OVER_DISCHARGE,
	FAULT_CVNP_INTERNAL,
	FAULT_VCM_COMM,
    FAULT_NUM_FAULTS
} tFaultCode;

/**
 * Generic 64 bit data container for fault specific data. Different data
 * representations are there to make it more versatile for different faults to
 * use
 */
typedef union {
    uint64_t ui64;
    uint32_t pui32[2];
    uint16_t pui16[4];
    uint8_t pui8[8];
    float pfloat[2];
} tFaultData;


/**
 * Fault hook to attach to a certain fault. Each fault registered to
 * the system will have one of these. Each contains some function pointers
 * to be called
 */
typedef struct {
    uint32_t bSet : 1;
    uint32_t ui32TSet : 31;
    void (*pfnOnAssert)(tFaultData);
    void (*pfnOnDeassert)(void);
    tFaultData uData;
} tFaultHook;


/**
 * Initializes the fault control module. Will populate the
 * fault hook table with default handlers, which will lock
 * the system.
 */
void fault_init();



/**
 * Returns a summary of which faults are set in the system, where each bit
 * in the return value corresponds to one fault. If the fault is set, then
 * the bit will be set, and vice versa.
 */
uint64_t fault_getFaultSummary();


/**
 * Asserts the fault, with the specified data
 */
void fault_assert(uint32_t ui32FaultNum, tFaultData uData);

/**
 * Deasserts the fault from the system
 */
void fault_deassert(uint32_t ui32FaultNum);


/**
 * Fetches the data for the specified fault
 */
tFaultData fault_getFaultData(uint32_t fault);


/**
 * Fetches the time asserted for the specified fault
 */
uint32_t fault_getFaultTime(uint32_t fault);


/**
 * Registers a set of handler functions for a certain fault number
 */
void fault_regHook(uint32_t faultNum, void (*pfnOnAssert)(tFaultData), void (*pfnOnDeassert)(void));

#endif /* FAULT_H_ */



