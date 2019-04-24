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

// Status to color / blink mappings
const tLEDState LED_STAT_NOFLT_ENBL = {LED_BLINK_SLOW_PULSEOFF, LED_COLOR_GREEN};
const tLEDState LED_STAT_NOFLT_DISBL = {LED_BLINK_SOLID, LED_COLOR_GREEN};

const tLEDState LED_STAT_VCM_CRASH = {LED_BLINK_SOLID, LED_COLOR_RED};
const tLEDState LED_STAT_PACK_SHORT = {LED_BLINK_FAST_EVEN, LED_COLOR_RED};
const tLEDState LED_STAT_GEN_FAULT = {LED_BLINK_SLOW_EVEN, LED_COLOR_RED};

const tLEDState LED_STAT_OVER_VOLT = {LED_BLINK_MED_PULSEOFF, LED_COLOR_BLUE};
const tLEDState LED_STAT_VOLT_WARN = {LED_BLINK_SOLID, LED_COLOR_BLUE};
const tLEDState LED_STAT_UNDER_VOLT = {LED_BLINK_MED_PULSEON, LED_COLOR_BLUE};

const tLEDState LED_STAT_OVER_CHG_I = {LED_BLINK_MED_PULSEOFF, LED_COLOR_CYAN};
const tLEDState LED_STAT_CURRENT_WARN = {LED_BLINK_SOLID, LED_COLOR_CYAN};
const tLEDState LED_STAT_OVER_DISCHG_I = {LED_BLINK_MED_PULSEON, LED_COLOR_CYAN};

const tLEDState LED_STAT_COMM = {LED_BLINK_SLOW_PULSEOFF, LED_COLOR_MAGENTA};

const tLEDState LED_STAT_TEMP_WARN = {LED_BLINK_SLOW_PULSEOFF, LED_COLOR_YELLOW};
const tLEDState LED_STAT_TEMP_FAULT = {LED_BLINK_MED_PULSEON, LED_COLOR_ORANGE};
const tLEDState LED_STAT_MPPT_FAULT = {LED_BLINK_FAST_PULSEOFF, LED_COLOR_YELLOW};
const tLEDState LED_STAT_MPPT_LOCK = {LED_BLINK_SOLID, LED_COLOR_YELLOW};

const tLEDState LED_STAT_KBL_FAULT = {LED_BLINK_MED_EVEN, LED_COLOR_ORANGE};

static tLEDState g_currState;
static bool g_wasOn;

/**
 * Calculates the load value for the blink timer given a
 * blink pattern, and whether or not to invert the duty cycle. The
 * formula is as follows:
 *
 * load = (clk*duty) / (10*freq))
 */
uint32_t _indicator_calcBlinkTimerLoad(tBlinkPattern pattern, bool invert) {
	uint32_t duty = invert ? 100-pattern.duty : pattern.duty;
	return (UTIL_CLOCK_SPEED * duty) / (10*pattern.freq);
}


/**
 * Sets a color to the LEDs
 */
void _indicator_setColor(tRGBColor color) {
	TimerMatchSet(TIMER5_BASE, TIMER_A, 255 - color.red); // Red
	TimerMatchSet(TIMER4_BASE, TIMER_B, 255 - color.green); // Green
	TimerMatchSet(TIMER4_BASE, TIMER_A, 255 - color.blue); // Blue
}


/**
 * Sets the LED to either PWM mode or GPIO mode. This will not
 * load the PWM generator if in PWM mode, but will turnoff the
 * GPIO if set to gpio mode
 */
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
	uint32_t load;

	TimerIntClear(DASH_LED_BLINK_TIMER_BASE, TIMER_TIMA_TIMEOUT);

	// Turn off
	if(g_wasOn) {
		load = _indicator_calcBlinkTimerLoad(g_currState.blinkPattern, true);
		_indicator_setLEDMode(false);
	}

	// Turn on, load is clk * (duty/(10*freq))
	else {
		load = _indicator_calcBlinkTimerLoad(g_currState.blinkPattern, false);
		_indicator_setLEDMode(true);
		_indicator_setColor(g_currState.color);
	}

	// Load the clock
	TimerLoadSet(DASH_LED_BLINK_TIMER_BASE, TIMER_BOTH, load);
}




/**
 * Prepares the indicator hardware
 */
void indicator_init()
{
	// Default LED to gpio output
	_indicator_setLEDMode(false);

	// Configure the blink timer. Don't load yet, that can be done when
	// the blink pattern is set
	TimerConfigure(DASH_LED_BLINK_TIMER_BASE, TIMER_CFG_A_PERIODIC);
	TimerIntRegister(DASH_LED_BLINK_TIMER_BASE, TIMER_TIMA_TIMEOUT, _indicator_blinkISR);
}



/**
 * Applies a pattern to the indicator LED
 */
void indicator_setPattern(tLEDState state) {

	_indicator_setLEDMode(true);
	_indicator_setColor(state.color);


	if(state.blinkPattern.freq == 0) {
		TimerDisable(DASH_LED_BLINK_TIMER_BASE, DASH_LED_BLINK_TIMER_BASE);
	} else {
		TimerLoadSet(DASH_LED_BLINK_TIMER_BASE,
					 DASH_LED_BLINK_TIMER_BASE,
					 _indicator_calcBlinkTimerLoad(state.blinkPattern, !g_wasOn));

		TimerEnable(DASH_LED_BLINK_TIMER_BASE, DASH_LED_BLINK_TIMER_BASE);
	}

	// Don't set this until after turning off the blink timer, as a zero
	// frequency (which is legal) will cause a divide by 0 error in the blink
	// timer ISR. To avoid this, if the frequency is zero, turn off the timer,
	// and then set the state, to prevent the possibility of a spurious interrupt
	// occuring after the state is set but before the timer is disabled.
	g_currState = state;
}




