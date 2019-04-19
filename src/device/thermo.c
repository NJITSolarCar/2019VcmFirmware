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
	uint32_t idxHi = 0, idxLo = 0; // Indicies for min / max temperature
	bool ret; // Return value
	tFaultData dat; // Multipurpose fault data
	dat.ui64 = 0;

	if(!g_adcValid)
		return false;

	for(int i=0; i<THERM_NUM_THERM; i++) {
		// Infinite thermistor resistance, possibly open connection?
		// Flag a fault bit, and don't try to calculate temp
		if(g_adcResults[i] <= THERM_ADC_FAULT_LEVEL) {
			dat.pui32[0] |= (1 << i); // Set proper bit
			continue;
		}
		adcf = 4096.0f / (float)g_adcResults[i];
		rTherm = THERM_BALLAST*(adcf-1) - THERM_FILTER_R;
		lnR = logf(rTherm) - THERM_LOG_NOMINAL_R;

		// Polynomial evaluation to get temperature
		temp[i] = THERM_PARAM_D * lnR;
		temp[i] = lnR * (temp[i] + THERM_PARAM_C);
		temp[i] = lnR * (temp[i] + THERM_PARAM_B);
		temp[i] += THERM_PARAM_A;

		// Check for high / low temp
		if(temp[i] > temp[idxHi])
			idxHi = i;
		else if(temp[i] < temp[idxLo])
			idxLo = i;
	}
	// Check thermistor faults
	ret = dat.pui32[0] != 0;
	if(ret)
		fault_assert(FAULT_VCM_THERMISTOR, dat);

	// Check temperature limits
	if(temp[idxHi] >= THERM_MAX_TEMP) {
		dat.pui32[0] = idxHi;
		dat.pfloat[1] = temp[idxHi];
		fault_assert(FAULT_VCM_HIGH_TEMP, dat);
	}

	if(temp[idxLo] <= THERM_MIN_TEMP) {
		dat.pui32[0] = idxLo;
		dat.pfloat[1] = temp[idxLo];
		fault_assert(FAULT_VCM_LOW_TEMP, dat);
	}

	return ret;
}












