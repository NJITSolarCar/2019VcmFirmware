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


// Gain settings. These map to the GPIO outputs that control
// the gain of the chip: bit 0 corresponds to GS0 and bit 1 to
// GS1
#define INA_GAIN_25         0x00
#define INA_GAIN_50         0x02
#define INA_GAIN_100        0x01
#define INA_GAIN_200        0x04


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
float ina_calcCurrent();


/**
 * Sets the gain of the chip, via the gain selection pins
 * GS0 and GS1. It takes approximately 10us maximum for the output to
 * stabilize as per the datasheet, so samples should not be taken in that
 * period.
 */
void ina_setGain(uint32_t gain);
#endif /* SRC_DEVICE_INA225_H_ */
