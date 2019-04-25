/*
 * thermo.h
 *
 *  Created on: Apr 16, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_THERMO_H_
#define SRC_DEVICE_THERMO_H_

#include <stdint.h>
#include <stdbool.h>

#define THERM_NUM_THERM			3

// Thermistor model parameters
#define THERM_PARAM_A			3.354016E-03f
#define THERM_PARAM_B			2.569850E-04f
#define THERM_PARAM_C			2.620131E-06f
#define THERM_PARAM_D			6.383091E-08f

#define THERM_BALLAST			10000
#define THERM_NOMINAL_R			10000
#define THERM_LOG_NOMINAL_R		9.21034037f // ln(THERM_NOMINAL_R)
#define THERM_FILTER_R			270

// If adc readout less or equal to this, assume thermistor fault
#define THERM_ADC_FAULT_LEVEL	5

// Temperature limits
#define THERM_MAX_TEMP			85.0f
#define THERM_MAX_TEMP_WARN		50.0f
#define THERM_MIN_TEMP			1.0f


/**
 * Initialize the onboard thermistors
 */
void thermo_init();

/**
 * Samples the thermistors, and checks the temperature against
 * preset limits
 */
void thermo_doSample();

/**
 * Calculates the floating point temperatures (in celsius) of the
 * thermistors. Checks for fault / over / under temperature conditions.
 * This will not start a sample, but use the latest results.
 * Returns true if the results are valid, false otherwise. Disconnected
 * thermistors will cause this to return false, but over / under temp will
 * return true.
 */
bool thermo_getTemp(float temp[THERM_NUM_THERM]);

#endif /* SRC_DEVICE_THERMO_H_ */
