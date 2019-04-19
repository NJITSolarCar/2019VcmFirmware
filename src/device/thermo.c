/*
 * thermo.c
 *
 *  Created on: Apr 16, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <driverlib/adc.h>
#include <driverlib/gpio.h>

#include "../fault.h"
#include "../util.h"
#include "../hal/resource.h"
#include "thermo.h"


// Raw ADC readings
static uint32_t g_adcResults[THERM_NUM_THERM];

// True if the values in g_adcResults are valid
static bool g_adcValid = false;



void _thermo_onSampleDone() {
	// Save the data and set the valid flag
	ADCSequenceDataGet(THERM_ADC_MODULE, THERM_ADC_SEQUENCE, g_adcResults);
	g_adcValid = true;
}



void thermo_init() {
	// Initialize the ADC sequence to run off a processor trigger
	ADCSequenceConfigure(THERM_ADC_MODULE,
						 THERM_ADC_SEQUENCE,
						 ADC_TRIGGER_PROCESSOR,
						 0);

	// Add sample steps
	ADCSequenceStepConfigure(THERM_ADC_MODULE,
							 THERM_ADC_SEQUENCE,
							 0,
							 THERM1_ADC_CHANNEL);

	ADCSequenceStepConfigure(THERM_ADC_MODULE,
							 THERM_ADC_SEQUENCE,
							 1,
							 THERM2_ADC_CHANNEL);

	ADCSequenceStepConfigure(THERM_ADC_MODULE,
							 THERM_ADC_SEQUENCE,
							 2,
							 THERM3_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);

	ADCIntRegister(THERM_ADC_MODULE, THERM_ADC_SEQUENCE, _thermo_onSampleDone);
	ADCIntEnable(THERM_ADC_MODULE, THERM_ADC_SEQUENCE);
}



// Just run a processor trigger on the ADC
void thermo_doSample() {
	ADCProcessorTrigger(THERM_ADC_MODULE, THERM_ADC_SEQUENCE);
}




bool thermo_getTemp(float temp[THERM_NUM_THERM]) {
	float adcf; // inverse of 0-1 floating point ADC result
	float rTherm; // calculated thermistor resistance
	float lnR; // ln(R/Rref)

	if(!g_adcValid)
		return false;

	for(int i=0; i<THERM_NUM_THERM; i++) {
		adcf = 4096.0f / (float)g_adcResults[i];
		rTherm = THERM_BALLAST*(adcf-1) - THERM_FILTER_R;
	}
}












