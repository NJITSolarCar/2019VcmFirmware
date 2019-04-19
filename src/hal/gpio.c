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

#include "../device/bms.h"


void _gpio_intPortF() {
	uint32_t intStat = GPIOIntStatus(GPIO_PORTF_BASE, true);
	uint8_t pinState = GPIOPinRead(GPIO_PORTF_BASE, 0xFF);

	if(intStat & BMS_DCL_PIN) {
		bool isSet = !!(pinState & BMS_DCL_PIN);
		bms_onDCLGpioChange(isSet);
	}
}


void gpio_init() {
	// Set up relays
	GPIOPinTypeGPIOOutput(CHG_RLY_PORT, CHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(DISCHG_RLY_PORT, DISCHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(SOLAR_RLY_PORT, SOLAR_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATPOS_RLY_PORT, BATPOS_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATNEG_RLY_PORT, BATNEG_RLY_PIN);

	// Set up port F ISRs
	GPIOIntEnable(GPIO_PORTF_BASE, BMS_DCL_PIN);
	GPIOIntTypeSet(GPIO_PORTF_BASE, BMS_DCL_PIN, GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTF_BASE, _gpio_intPortF);
}







