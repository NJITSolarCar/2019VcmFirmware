/*
 * adc.h
 *
 * Contains the HAL interface for using and configuring analog inputs
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef HAL_ADC_H_
#define HAL_ADC_H_

#define THERMO_OVERSAMPLE				64
#define INA_OVERSAMPLE					16

/**
 * Initializes and configures the ADC system
 */
void adc_init();


bool adc_startThermoSample();
bool adc_startInaSample();

#endif /* HAL_ADC_H_ */
