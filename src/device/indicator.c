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
#include "../util.h"

#include <driverlib/gpio.h>
#include <driverlib/timer.h>


static tLEDState g_currState;


/**
 * Sets a color to the LEDs
 */
void _indicator_setColor(tRGBColor color) {
	TimerMatchSet(TIMER5_BASE, TIMER_A, 255 - color.red); // Red
	TimerMatchSet(TIMER4_BASE, TIMER_B, 255 - color.green); // Green
	TimerMatchSet(TIMER4_BASE, TIMER_A, 255 - color.blue); // Blue
}



void _indicator_setLEDMode(bool isPWM) {
	if(isPWM) { // Set as PWM
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
	}
	else { // Set to GPIO and turn off
		GPIOPinTypeGPIOOutput(DASH_LED_PORT, DASH_LED_RED_PIN |
							  DASH_LED_GREEN_PIN |
							  DASH_LED_BLUE_PIN);
		GPIOPinWrite(DASH_LED_PORT, DASH_LED_RED_PIN |
					 DASH_LED_GREEN_PIN |
					 DASH_LED_BLUE_PIN,
					 0);
	}
}



/**
 * Called when the blink timer expires. Will reload the timer and
 * set the LED output
 */
void _indicator_blinkISR() {
	static bool wasOn = false;
	uint32_t load;

	TimerIntClear(DASH_LED_BLINK_TIMER_BASE, TIMER_TIMA_TIMEOUT);

	// Turn off, load is clk * ((100-duty)/(10*freq))
	if(wasOn) {
		load = (UTIL_CLOCK_SPEED * (100 - g_currState.blinkPattern.duty)) /
				(10 * g_currState.blinkPattern.freq);
		_indicator_setLEDMode(false);
	}

	// Turn on, load is clk * (duty/(10*freq))
	else {
		load = (UTIL_CLOCK_SPEED * g_currState.blinkPattern.duty) /
				(10 * g_currState.blinkPattern.freq);
		_indicator_setLEDMode(true);
		_indicator_setColor(g_currState.color);
	}

	// Load the clock
	TimerLoadSet(DASH_LED_BLINK_TIMER_BASE, TIMER_BOTH, load);
}





void indicator_init()
{
	// Default LED to gpio output
	_indicator_setLEDMode(false);

	// Configure the blink timer. Don't load yet, that can be done when
	// the blink pattern is set
	TimerConfigure(DASH_LED_BLINK_TIMER_BASE, TIMER_CFG_A_PERIODIC);
	TimerIntRegister(DASH_LED_BLINK_TIMER_BASE, TIMER_TIMA_TIMEOUT, _indicator_blinkISR);
}


void indicator_setPattern(tLEDState state) {
	g_currState = state;
}




