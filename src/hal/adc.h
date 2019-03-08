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

/**
 * Initializes and configures the ADC system
 */
void adc_init();


bool adc_startThermoSample();
bool adc_startAmpSample();

#endif /* HAL_ADC_H_ */
