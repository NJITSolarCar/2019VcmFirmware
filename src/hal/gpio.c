/*
 * gpio.c
 *
 * Main GPIO interface drivers for the VCM
 *
 *  Created on: Apr 10, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>

#include <driverlib/timer.h>
#include <driverlib/gpio.h>

#include "resource.h"
#include "gpio.h"

#include "../fault.h"

#include "../device/indicator.h"
#include "../device/bms.h"
#include "../device/relay.h"
#include "../device/mppt.h"


void _gpio_intPortF() {
	uint32_t intStat = GPIOIntStatus(GPIO_PORTF_BASE, true);
	uint8_t pinState = GPIOPinRead(GPIO_PORTF_BASE, 0xFF);

	if(intStat & BMS_DCL_PIN) {
		bool isSet = !!(pinState & BMS_DCL_PIN);
		bms_onDCLGpioChange(isSet);
	}
}


void _gpio_intPortB() {
	uint32_t intStat = GPIOIntStatus(GPIO_PORTB_BASE, true);
	uint8_t pinState = GPIOPinRead(GPIO_PORTB_BASE, 0xFF);

	if(intStat & DASH_MPPT_PIN) {
		if(pinState & DASH_MPPT_PIN) {
			tFaultData dat;
			dat.ui64 = 0;
			fault_assert(FAULT_MPPT_USER_LOCKOUT, dat);
		} else
			fault_deassert(FAULT_MPPT_USER_LOCKOUT);
	}

	// Set the correct relay enable and indicator state
	if(intStat & DASH_IGNITION_PIN) {
		if(pinState & DASH_IGNITION_PIN) {
			relay_enable(true);
			if(!fault_getFaultSummary())
				indicator_setPattern(LED_STAT_NOFLT_ENBL);
		} else {
			relay_enable(false);
			if(!fault_getFaultSummary())
				indicator_setPattern(LED_STAT_NOFLT_DISBL);
		}
	}
}


void gpio_init() {

	// Set up port F ISRs
	GPIOIntEnable(GPIO_PORTF_BASE, BMS_DCL_PIN);
	GPIOIntTypeSet(GPIO_PORTF_BASE, BMS_DCL_PIN, GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTF_BASE, _gpio_intPortF);
}







