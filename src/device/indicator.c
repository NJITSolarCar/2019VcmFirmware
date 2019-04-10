/*
 * indicator.c
 *
 *  Created on: Apr 10, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>

#include "indicator.h"
#include "../hal/resource.h"

#include <driverlib/gpio.h>
#include <driverlib/timer.h>

/**
 * Called when the blink timer expires
 */
void _indicator_blinkISR() {

}


void indicator_init() {
	// LED GPIO configuration
	GPIOPinTypeTimer(DASH_LED_PORT, DASH_LED_RED_PIN |
					 DASH_LED_GREEN_PIN |
					 DASH_LED_BLUE_PIN);

	GPIOPinConfigure(DASH_LED_RED_PINMAP);
	GPIOPinConfigure(DASH_LED_GREEN_PINMAP);
	GPIOPinConfigure(DASH_LED_BLUE_PINMAP);

	// Configure the PWM timers
	TimerConfigure(TIMER4_BASE, TIMER_CFG_A_PERIODIC | TIMER_CFG_A_PWM);
	TimerConfigure(TIMER4_BASE, TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PWM);
	TimerConfigure(TIMER5_BASE, TIMER_CFG_A_PERIODIC | TIMER_CFG_A_PWM);
	TimerLoadSet(TIMER4_BASE, TIMER_BOTH, INDICATOR_PWM_FULL_LOAD);
	TimerLoadSet(TIMER5_BASE, TIMER_A, INDICATOR_PWM_FULL_LOAD);

	// Configure the blink timer. Don't load yet, that can be done when
	// the blink pattern is set
	TimerConfigure(DASH_LED_BLINK_TIMER_BASE, TIMER_CFG_A_PERIODIC);
}





