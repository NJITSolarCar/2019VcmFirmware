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




void gpio_init() {
	// Set up relays
	GPIOPinTypeGPIOOutput(CHG_RLY_PORT, CHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(DISCHG_RLY_PORT, DISCHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(SOLAR_RLY_PORT, SOLAR_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATPOS_RLY_PORT, BATPOS_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATNEG_RLY_PORT, BATNEG_RLY_PIN);

}







