/*
 * ina225.h
 *
 * Driver for the INA225 current sense shunt amplifier.
 *
 *  Created on: Mar 12, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_INA225_H_
#define SRC_DEVICE_INA225_H_


// Amount of microseconds to hold off sampling between
// gain changes
#define INA_HOLDOFF_USEC	15

// If the ADC reading is greater or equal to this, gain should
// decrease
#define INA_ADC_HIGH_LIMIT	4085

// If the ADC reading is lesser or equal to this, gain should
// increase
#define INA_ADC_LOW_LIMIT	(4096 / 3)


// Value of shunt resistor used
#define INA_SHUNT			1.0E-3f

// Nominal efficiency of the onboard dc-dc converter
#define INA_DCDC_EFF		0.9f

/**
 * Gain settings of the INA
 */
typedef enum {
	INA_GAIN_25,
	INA_GAIN_50,
	INA_GAIN_100,
	INA_GAIN_200
}tInaGain;



/**
 * Initializes the INA module.
 */
void ina_init();


/**
 * Calculates and returns the current through the shunt resistor based on
 * the last ADC reading, and the gain setting that was used during the sample
 * in question (not necessarily the current gain setting!). If the device
 * has not been initialized, or not sampled, the values returned from this will
 * be invalid.
 */
float ina_getCurrent();


/**
 * Sets the gain of the chip, via the gain selection pins
 * GS0 and GS1. It takes approximately 10us maximum for the output to
 * stabilize as per the datasheet, so samples should not be taken in that
 * period.
 */
void ina_setGain(uint32_t gain);
#endif /* SRC_DEVICE_INA225_H_ */
