/*
 * ina225.c
 *
 *  Created on: Apr 12, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/adc.h>
#include <driverlib/gpio.h>

#include "ina225.h"
#include "../util.h"
#include "../fault.h"
#include "../hal/resource.h"


static float g_current = 0.0f;
static uint32_t g_currGain;

// Multiply the ADC reading by the relevant entry to give
// a current
static float g_adcScl[] = {
				1.0f/(25.0f * INA_SHUNT),
				1.0f/(50.0f * INA_SHUNT),
				1.0f/(100.0f * INA_SHUNT),
				1.0f/(200.0f * INA_SHUNT)
};




void _ina_onSampleDone() {

	// Read the ADC
	uint32_t adcVal;
	ADCIntClear(INA_ADC_MODULE, INA_ADC_SEQUENCE);
	ADCSequenceDataGet(INA_ADC_MODULE, INA_ADC_SEQUENCE, &adcVal);

	// Calculate the real current
	g_current = g_adcScl[g_currGain] * (float)adcVal;

	// Check for gain changes
	if(g_currGain > INA_GAIN_25 && adcVal >= INA_ADC_HIGH_LIMIT)
		ina_setGain(g_currGain-1);
	else if(g_currGain < INA_GAIN_200 && adcVal <= INA_ADC_LOW_LIMIT)
		ina_setGain(g_currGain+1);
}




/*
 * Clear the interrupt, disable the timer, and start sampling
 */
void _ina_onHoldoffDone() {
	TimerIntClear(INA_TIMER_BASE, INA_TIMER_INT);
	TimerDisable(INA_TIMER_BASE, INA_TIMER_PART);
	ADCSequenceEnable(INA_ADC_MODULE, INA_ADC_SEQUENCE);
}


/**
 * Write the GPIO setting, and hold off sampling for INA_HOLDOFF_USEC.
 * Bit 0 of gain corresponds to GS1, and bit 1 to GS0.
 */
void ina_setGain(uint32_t gain) {
	g_currGain = gain;
	uint8_t gsVal = gain & 0x01 ? INA_GS1_PIN : 0;
	gsVal |= gain & 0x02 ? INA_GS0_PIN : 0;

	// Set the GPIOs
	GPIOPinWrite(INA_GS_PORT, INA_GS0_PIN | INA_GS1_PIN, gsVal);

	// Disable the ADC and start the holdoff timer
	ADCSequenceDisable(INA_ADC_MODULE, INA_ADC_SEQUENCE);
	TimerLoadSet(INA_TIMER_BASE, INA_TIMER_PART,
				 (UTIL_CLOCK_SPEED*INA_HOLDOFF_USEC) / 1000000);
	TimerEnable(INA_TIMER_BASE, INA_TIMER_PART);
}




void ina_init() {
	// Initialize the ADC sequence
	ADCSequenceConfigure(INA_ADC_MODULE,
						 INA_ADC_SEQUENCE,
						 ADC_TRIGGER_ALWAYS,
						 INA_ADC_PRIORITY);

	ADCSequenceStepConfigure(INA_ADC_MODULE,
							 INA_ADC_SEQUENCE,
							 0,
							 INA_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);
	ADCIntRegister(INA_ADC_MODULE, INA_ADC_SEQUENCE, _ina_onSampleDone);

	// Initialize the holdoff timer as a one shot
	TimerConfigure(INA_TIMER_BASE, INA_TIMER_CFG);
	TimerIntRegister(INA_TIMER_BASE, INA_TIMER_PART, _ina_onHoldoffDone)
	TimerIntEnable(INA_TIMER_BASE, INA_TIMER_INT);

	// initialize the gain configuration to 200 V/V. This will hold off
	// for a small time, and then begin the sampling process
	ina_setGain(INA_GAIN_200);


}







